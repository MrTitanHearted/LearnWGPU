#include <WModel.hpp>

#include <WUtils.hpp>

#include <filesystem>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <assimp/vector3.h>
#include <assimp/quaternion.h>
#include <assimp/matrix4x4.h>

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>

namespace fs = std::filesystem;

void processNode(WGPUDevice device,
                 std::vector<WMesh> &meshes,
                 WGPUTextureFormat colorTargetFormat,
                 WBindGroup globalGroup,
                 WGPUBindGroupLayout localBindGroupLayout,
                 WUniformBuffer modelBuffer,
                 WRenderPipeline pipeline,
                 const std::string &directory,
                 const aiNode *node,
                 const aiScene *scene);
WMesh processMesh(WGPUDevice device,
                  WGPUTextureFormat colorTargetFormat,
                  WBindGroup globalGroup,
                  WGPUBindGroupLayout localBindGroupLayout,
                  WUniformBuffer modelBuffer,
                  WRenderPipeline pipeline,
                  const std::string &directory,
                  const aiMesh *mesh,
                  const aiScene *scene);
WTexture loadMaterialTextures(WGPUDevice device,
                              aiTextureType type,
                              const std::string directory,
                              const aiMaterial *material,
                              const aiScene *scene);

class AssimpToGlm {
   public:
    static glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4 &from) {
        glm::mat4 to;

        to[0][0] = from.a1;
        to[1][0] = from.a2;
        to[2][0] = from.a3;
        to[3][0] = from.a4;
        to[0][1] = from.b1;
        to[1][1] = from.b2;
        to[2][1] = from.b3;
        to[3][1] = from.b4;
        to[0][2] = from.c1;
        to[1][2] = from.c2;
        to[2][2] = from.c3;
        to[3][2] = from.c4;
        to[0][3] = from.d1;
        to[1][3] = from.d2;
        to[2][3] = from.d3;
        to[3][3] = from.d4;

        return to;
    }
    static glm::vec3 aiVector3ToGlm(const aiVector3D &from) {
        return glm::vec3(from.x, from.y, from.z);
    }
    static glm::vec2 aiVector3ToGlmVec2(const aiVector3D &from) {
        return glm::vec2(from.x, from.y);
    }
    static glm::quat aiQuaternionToGlm(const aiQuaternion &from) {
        return glm::quat(from.w, from.x, from.y, from.z);
    }
};

class WTextureCache {
   public:
    static WTexture New(WGPUDevice device, std::string path) {
        if (cache.find(path) == cache.end()) {
            WTexture texture = WTexture::fromFileAsRgba8(device, path);
            cache[path] = texture;
            return texture;
        } else {
            return cache[path];
        }
    }
    static WTexture New(WGPUDevice device, std::string path, void *data, size_t size) {
        if (cache.find(path) == cache.end()) {
            WTexture texture = WTexture::fromMemoryAsRgba8(device, data, size);
            cache[path] = texture;
            return texture;
        } else {
            return cache[path];
        }
    }
    static void RemoveTexture(std::string path) {
        cache.erase(cache.find(path));
    }
    static const WTexture &GetTexture(std::string path) {
        return cache[path];
    }

   private:
    static std::map<std::string, WTexture> cache;
};

std::map<std::string, WTexture> WTextureCache::cache = std::map<std::string, WTexture>{};

WVertexLayout WModelVertex::desc() {
    return WVertexLayout::New(sizeof(WModelVertex))
        .addAttribute(WGPUVertexFormat_Float32x3, offsetof(WModelVertex, position), 0)
        .addAttribute(WGPUVertexFormat_Float32x3, offsetof(WModelVertex, normal), 1)
        .addAttribute(WGPUVertexFormat_Float32x2, offsetof(WModelVertex, uv), 2);
}

WModelVertex &WModelVertex::withPosition(glm::vec3 position) {
    this->position = position;
    return *this;
}
WModelVertex &WModelVertex::withNormal(glm::vec3 normal) {
    this->normal = normal;
    return *this;
}
WModelVertex &WModelVertex::withUV(glm::vec2 uv) {
    this->uv = uv;
    return *this;
}

WMesh WMesh::New(WRenderBundle renderBundle, std::vector<WTexture> textures, std::vector<WBindGroup> bindGroups) {
    WMesh mesh;
    mesh.renderBundle = renderBundle;
    mesh.textures = textures;
    mesh.bindGroups = bindGroups;
    return mesh;
}
void WMesh::render(WGPURenderPassEncoder encoder) {
    renderBundle.render(encoder);
}

WModel WModel::New(std::string path, std::vector<WMesh> meshes, WRenderPipeline pipeline, WUniformBuffer modelBuffer, glm::mat4 modelData) {
    WModel model;
    model.pipeline = pipeline;
    model.modelBuffer = modelBuffer;
    model.modelData = modelData;

    model.meshes = meshes;
    std::vector<WGPURenderBundle> renderBundles;
    renderBundles.reserve(model.meshes.size());
    for (const WMesh mesh : meshes) {
        renderBundles.push_back(mesh);
    }
    model.renderBundles = renderBundles;

    fs::path fpath{path};
    model.path = path;
    model.name = fpath.filename().string();
    model.directory = fpath.parent_path().string();

    return model;
}
WModel WModel::New(std::vector<WMesh> meshes,
                   WRenderPipeline pipeline,
                   WUniformBuffer modelBuffer,
                   glm::mat4 modelData) {
    return WModel::New("", meshes, pipeline, modelBuffer, modelData);
}
void WModel::render(WGPURenderPassEncoder encoder) {
    wgpuRenderPassEncoderExecuteBundles(encoder, renderBundles.size(), renderBundles.data());
}
void WModel::updateModel(WGPUQueue queue, glm::mat4 model) {
    modelData = model;
    modelBuffer.update(queue, &modelData);
}

WModelBuilder WModelBuilder::New(std::string path) {
    return New().setPath(path);
}
WModelBuilder &WModelBuilder::setPath(std::string path) {
    this->path = path;
    return *this;
}
WModelBuilder &WModelBuilder::setGlobalBindGroup(WBindGroup bindGroup) {
    this->globalBindGroup = bindGroup;
    return *this;
}
WModelBuilder &WModelBuilder::setColorTarget(WGPUTextureFormat format) {
    colorTargetFormat = format;
    return *this;
}
WModelBuilder &WModelBuilder::setVertexShader(WGPUShaderModule vshader, const char *entry) {
    this->vshader = vshader;
    this->ventry = entry;
    return *this;
}
WModelBuilder &WModelBuilder::setFragmentShader(WGPUShaderModule fshader, const char *entry) {
    this->fshader = fshader;
    this->fentry = entry;
    return *this;
}
WModel WModelBuilder::buildFromFile(WGPUDevice device) {
    WGPUBindGroupLayout localGroupLayout =
        WBindGroupLayoutBuilder::New()
            .addBindingTexture(0)
            .addBindingUniform(1)
            .build(device);

    WRenderPipeline pipeline =
        WRenderPipelineBuilder::New()
            .addBindGroupLayout(globalBindGroup)
            .addBindGroupLayout(localGroupLayout)
            .setVertexState(vshader, ventry)
            .setFragmentState(fshader, fentry)
            .addVertexBufferLayout(WModelVertex::desc())
            .addColorTarget(colorTargetFormat)
            .setDefaultDepthState()
            .build(device);

    glm::mat4 modelData{1.0f};
    WUniformBuffer modelBuffer = WUniformBuffer::New(device, &modelData, sizeof(modelData));

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate |
                                                       aiProcess_GenNormals |
                                                       aiProcess_GenUVCoords |
                                                       aiProcess_FlipUVs |
                                                       aiProcess_JoinIdenticalVertices |
                                                       aiProcess_OptimizeGraph |
                                                       aiProcess_OptimizeMeshes);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::exception(fmt::format("[WEngine]::[ERROR]: Failed to load model from path: '{}'", path).c_str());
    }

    fs::path fpath{path};
    std::string directory = fpath.parent_path().string();

    std::vector<WMesh> meshes{};
    processNode(device, meshes, colorTargetFormat, globalBindGroup, localGroupLayout, modelBuffer,
                pipeline, directory, scene->mRootNode, scene);

    return WModel::New(path, meshes, pipeline, modelBuffer, modelData);
}

void processNode(WGPUDevice device,
                 std::vector<WMesh> &meshes,
                 WGPUTextureFormat colorTargetFormat,
                 WBindGroup globalGroup,
                 WGPUBindGroupLayout localBindGroupLayout,
                 WUniformBuffer modelBuffer,
                 WRenderPipeline pipeline,
                 const std::string &directory,
                 const aiNode *node,
                 const aiScene *scene) {
    for (uint32_t i = 0; i < node->mNumMeshes; i++) {
        meshes.push_back(processMesh(device, colorTargetFormat, globalGroup, localBindGroupLayout, modelBuffer,
                                     pipeline, directory, scene->mMeshes[node->mMeshes[i]], scene));
    }
    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        processNode(device, meshes, colorTargetFormat, globalGroup, localBindGroupLayout, modelBuffer,
                    pipeline, directory, node->mChildren[i], scene);
    }
}
WMesh processMesh(WGPUDevice device,
                  WGPUTextureFormat colorTargetFormat,
                  WBindGroup globalGroup,
                  WGPUBindGroupLayout localBindGroupLayout,
                  WUniformBuffer modelBuffer,
                  WRenderPipeline pipeline,
                  const std::string &directory,
                  const aiMesh *mesh,
                  const aiScene *scene) {
    std::vector<WModelVertex> vertices{};
    std::vector<uint32_t> indices{};
    std::vector<WTexture> textures;

    vertices.reserve(mesh->mNumVertices);
    indices.reserve(mesh->mNumFaces * 3);

    for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
        glm::vec3 position = AssimpToGlm::aiVector3ToGlm(mesh->mVertices[i]);
        glm::vec3 normal = AssimpToGlm::aiVector3ToGlm(mesh->mNormals[i]);
        glm::vec2 uv = mesh->mTextureCoords[0] ? AssimpToGlm::aiVector3ToGlmVec2(mesh->mTextureCoords[0][i])
                                               : glm::vec2(0.0f);
        vertices.push_back(WModelVertex::New()
                               .withPosition(position)
                               .withNormal(normal)
                               .withUV(uv));
    }
    for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
        for (uint32_t j = 0; j < mesh->mFaces[i].mNumIndices; j++) {
            indices.push_back(mesh->mFaces[i].mIndices[j]);
        }
    }

    if (mesh->mMaterialIndex >= 0) {
        textures.push_back(loadMaterialTextures(device,
                                                aiTextureType_DIFFUSE,
                                                directory,
                                                scene->mMaterials[mesh->mMaterialIndex],
                                                scene));
        // textures.push_back(loadMaterialTextures(device,
        //                                         aiTextureType_SPECULAR,
        //                                         directory,
        //                                         scene->mMaterials[mesh->mMaterialIndex],
        //                                         scene));
        // textures.push_back(loadMaterialTextures(device,
        //                                         aiTextureType_NORMALS,
        //                                         directory,
        //                                         scene->mMaterials[mesh->mMaterialIndex],
        //                                         scene));
    }

    WBindGroupBuilder localGroupBuilder = WBindGroupBuilder::New();
    for (uint32_t i = 0; i < textures.size(); i++) {
        localGroupBuilder.addBindingTexture(i, textures[i]);
    }
    localGroupBuilder.addBindingUniform(textures.size(), modelBuffer);
    WBindGroup localGroup = localGroupBuilder.buildWithLayout(device, localBindGroupLayout);

    WRenderBuffer renderBuffer =
        WRenderBufferBuilder::New()
            .setVertices(vertices)
            .setIndices(indices)
            .build(device);

    WRenderBundle renderBundle =
        WRenderBundleBuilder::New()
            .addBindGroup(globalGroup)
            .addBindGroup(localGroup)
            .setRenderPipeline(pipeline)
            .setRenderBuffer(renderBuffer)
            .addColorFormat(colorTargetFormat)
            .setDefaultDepthFormat()
            .build(device);

    return WMesh::New(renderBundle, textures, {globalGroup, localGroup});
}
WTexture loadMaterialTextures(WGPUDevice device,
                              aiTextureType type,
                              const std::string directory,
                              const aiMaterial *material,
                              const aiScene *scene) {
    switch (type) {
        case aiTextureType_DIFFUSE:
        case aiTextureType_SPECULAR:
        case aiTextureType_NORMALS:
            break;
        default:
            throw std::exception("[WEngine]::[ERROR]: There is no such assimp material texture type!");
    }

    for (uint32_t i = 0; i < material->GetTextureCount(type); i++) {
        aiString name;
        material->GetTexture(type, i, &name);
        std::string path = directory + "/" + name.C_Str();

        const aiTexture *assimpTexture = scene->GetEmbeddedTexture(name.C_Str());
        return assimpTexture == nullptr ? WTextureCache::New(device, path)
                                        : WTextureCache::New(device, path, assimpTexture->pcData, assimpTexture->mWidth);
    }

    throw std::exception("[WEngine]::[ERROR]: Assimp material texture should have existed with this type at least once!");
}
