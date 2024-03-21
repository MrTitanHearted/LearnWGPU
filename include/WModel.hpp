#pragma once

#include <WInclude.hpp>

struct WModelVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;

    inline static WModelVertex New() { return WModelVertex(); }
    static WVertexLayout desc();

    WModelVertex &withPosition(glm::vec3 position);
    WModelVertex &withNormal(glm::vec3 normal);
    WModelVertex &withUV(glm::vec2 uv);
};

class WMesh {
   public:
    static WMesh New(WRenderBundle renderBundle, std::vector<WTexture> textures, std::vector<WBindGroup> bindGroups);

    void render(WGPURenderPassEncoder encoder);

    inline operator WGPURenderBundle() const { return renderBundle; }
    inline const WGPURenderBundle &getRenderBundle() const { return renderBundle.getRenderBundle(); }

   private:
    WRenderBundle renderBundle;
    std::vector<WTexture> textures;
    std::vector<WBindGroup> bindGroups;
};

class WModel {
   public:
    static WModel New(std::string path,
                      std::vector<WMesh> meshes,
                      WRenderPipeline pipeline,
                      WUniformBuffer modelBuffer,
                      glm::mat4 modelData = glm::mat4{1.0f});
    static WModel New(std::vector<WMesh> meshes,
                      WRenderPipeline pipeline,
                      WUniformBuffer modelBuffer,
                      glm::mat4 modelData = glm::mat4{1.0f});

    void render(WGPURenderPassEncoder encoder);
    void updateModel(WGPUQueue queue, glm::mat4 model);

   private:
    std::vector<WMesh> meshes;
    std::vector<WGPURenderBundle> renderBundles;
    WRenderPipeline pipeline;
    WUniformBuffer modelBuffer;
    glm::mat4 modelData;
    WGPUShaderModule shader;

    std::string path;
    std::string name;
    std::string directory;
};

class WModelBuilder {
   public:
    static inline WModelBuilder New() { return WModelBuilder(); }
    static WModelBuilder New(std::string path);

    WModelBuilder &setPath(std::string path);
    WModelBuilder &setGlobalBindGroup(WBindGroup bindGroup);
    WModelBuilder &setColorTarget(WGPUTextureFormat format);
    WModelBuilder &setVertexShader(WGPUShaderModule vshader, const char *entry = "vs_main");
    WModelBuilder &setFragmentShader(WGPUShaderModule fshader, const char *entry = "fs_main");

    WModel buildFromFile(WGPUDevice device);

   private:
    std::string path;
    WBindGroup globalBindGroup;
    WGPUTextureFormat colorTargetFormat;
    WGPUShaderModule vshader;
    WGPUShaderModule fshader;
    const char *ventry;
    const char *fentry;
};