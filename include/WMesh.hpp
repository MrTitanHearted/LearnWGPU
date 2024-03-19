#pragma once

#include <WInclude.hpp>
#include <WVertexLayout.hpp>
#include <WRenderBuffer.hpp>
#include <WBindings.hpp>

// struct WMeshVertex {
//     glm::vec3 position;
//     glm::vec3 normal;
//     glm::vec2 uv;

//     WMeshVertex() {}
//     WMeshVertex(float x, float y, float z, float nx, float ny, float nz, float u, float v);

//     inline static WMeshVertex New() { return WMeshVertex{}; }
//     WMeshVertex withPosition(glm::vec3 position);
//     WMeshVertex withNormal(glm::vec3 normal);
//     WMeshVertex withUV(glm::vec2 uv);

//     static WVertexLayout desc();
// };

struct WMesh {
    WRenderBuffer renderBuffer;
    std::vector<WGPUBindGroup> bindGroups;
    WGPURenderBundle renderBundle;
    WGPURenderPipeline pipeline;

    WMesh() {}
    WMesh(WRenderBuffer renderBuffer, std::vector<WGPUBindGroup> bindGroups, WGPURenderBundle renderBundle, WGPURenderPipeline pipeline);

    void render(WGPURenderPassEncoder encoder);

    operator WGPURenderBundle() const;
    inline WGPURenderBundle getRenderBundle() { return *this; }
};

struct WMeshBuilder {
    WRenderBuffer renderBuffer;
    std::vector<WGPUBindGroup> bindGroups;
    std::vector<WGPUTextureFormat> colorFormats;
    WGPUTextureFormat depthFormat;
    WGPURenderPipeline pipeline;

    inline static WMeshBuilder New() { return WMeshBuilder{}; }
    WMeshBuilder& setRenderBuffer(WRenderBuffer renderBuffer);
    WMeshBuilder& addBindGroup(WGPUBindGroup bindGroup);
    WMeshBuilder& addColorFormat(WGPUTextureFormat format);
    WMeshBuilder& setDefaultDepthFormat(WGPUTextureFormat format = WGPUTextureFormat_Depth32Float);
    WMeshBuilder& setRenderPipeline(WGPURenderPipeline pipeline);

    WMesh build(WGPUDevice device);
};