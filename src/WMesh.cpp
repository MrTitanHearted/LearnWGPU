#include <WMesh.hpp>

// WMeshVertex::WMeshVertex(float x, float y, float z, float nx, float ny, float nz, float u, float v) {
//     this->position = glm::vec3(x, y, z);
//     this->normal = glm::vec3(nx, ny, nz);
//     this->uv = glm::vec2(u, v);
// }

// WMeshVertex WMeshVertex::withPosition(glm::vec3 position) {
//     this->position = position;
//     return *this;
// }

// WMeshVertex WMeshVertex::withNormal(glm::vec3 normal) {
//     this->normal = normal;
//     return *this;
// }

// WMeshVertex WMeshVertex::withUV(glm::vec2 uv) {
//     this->uv = uv;
//     return *this;
// }

// WVertexLayout WMeshVertex::desc() {
//     return WVertexLayout{sizeof(WMeshVertex)}
//         .addAttributes(WGPUVertexFormat_Float32x3, offsetof(WMeshVertex, position), 0)
//         .addAttributes(WGPUVertexFormat_Float32x3, offsetof(WMeshVertex, normal), 1)
//         .addAttributes(WGPUVertexFormat_Float32x2, offsetof(WMeshVertex, uv), 2);
// }

WMesh::WMesh(WRenderBuffer renderBuffer, std::vector<WGPUBindGroup> bindGroups, WGPURenderBundle renderBundle, WGPURenderPipeline pipeline) {
    this->renderBuffer = renderBuffer;
    this->bindGroups = bindGroups;
    this->renderBundle = renderBundle;
    this->pipeline = pipeline;
}

void WMesh::render(WGPURenderPassEncoder encoder) {
    wgpuRenderPassEncoderExecuteBundles(encoder, 1, &renderBundle);
}

WMesh::operator WGPURenderBundle() const {
    return renderBundle;
}

WMeshBuilder& WMeshBuilder::setRenderBuffer(WRenderBuffer renderBuffer) {
    this->renderBuffer = renderBuffer;
    return *this;
}

WMeshBuilder& WMeshBuilder::addBindGroup(WGPUBindGroup bindGroup) {
    this->bindGroups.push_back(bindGroup);
    return *this;
}

WMeshBuilder& WMeshBuilder::addColorFormat(WGPUTextureFormat format) {
    this->colorFormats.push_back(format);
    return *this;
}

WMeshBuilder& WMeshBuilder::setDefaultDepthFormat(WGPUTextureFormat format) {
    this->depthFormat = format;
    return *this;
}

WMeshBuilder& WMeshBuilder::setRenderPipeline(WGPURenderPipeline pipeline) {
    this->pipeline = pipeline;
    return *this;
}

WMesh WMeshBuilder::build(WGPUDevice device) {
    WGPURenderBundleEncoderDescriptor renderBundleEncoderDescriptor{
        .colorFormatCount = colorFormats.size(),
        .colorFormats = colorFormats.data(),
        .depthStencilFormat = depthFormat,
        .sampleCount = 1,
    };

    WGPURenderBundleEncoder encoder = wgpuDeviceCreateRenderBundleEncoder(device, &renderBundleEncoderDescriptor);
    wgpuRenderBundleEncoderSetPipeline(encoder, pipeline);
    for (uint32_t index = 0; index < bindGroups.size(); index++) {
        wgpuRenderBundleEncoderSetBindGroup(encoder, index, bindGroups[index], 0, nullptr);
    }
    renderBuffer.render(encoder);
    WGPURenderBundle renderBundle = wgpuRenderBundleEncoderFinish(encoder, nullptr);

    return WMesh{renderBuffer, bindGroups, renderBundle, pipeline};
}
