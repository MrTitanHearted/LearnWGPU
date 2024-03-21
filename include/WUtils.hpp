#pragma once

#include <WInclude.hpp>

class WRenderPassBuilder {
   public:
    static inline WRenderPassBuilder New() { return WRenderPassBuilder(); }

    WRenderPassBuilder &addColorTarget(WColorAttachment attachment);
    WRenderPassBuilder &setDepthAttachment(WDepthStencilAttachment attachment);

    WGPURenderPassEncoder build(WGPUCommandEncoder commandEncoder, const char *label = "Render Pass Endoder");

   private:
    std::vector<WGPURenderPassColorAttachment> colorAttachments;
    WGPURenderPassDepthStencilAttachment depthStencilAttachment;
    bool depthTest = false;
    bool stencilTest = false;
};

class WTextureBuilder {
   public:
    static inline WTextureBuilder New() { return WTextureBuilder(); }

    WTextureBuilder &addTextureUsage(WGPUTextureUsageFlags usage);
    WTextureBuilder &removeTextureUsages(WGPUTextureUsageFlags usage);
    WTextureBuilder &setTextureUsages(WGPUTextureUsageFlags usages);
    WTextureBuilder &setDimension(WGPUTextureDimension dimension);
    WTextureBuilder &setFormat(WGPUTextureFormat format);
    WTextureBuilder &setMipLevelCount(uint32_t count);
    WTextureBuilder &setSampleCount(uint32_t count);

    WTexture build(WGPUDevice device,
                   WGPUExtent3D size,
                   uint32_t channels = 4,
                   const unsigned char *data = nullptr,
                   uint32_t stride = 0);

   private:
    WGPUTextureDescriptor desc{
        .usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst,
        .dimension = WGPUTextureDimension_2D,
        .format = WGPUTextureFormat_RGBA8Unorm,
        .mipLevelCount = 1,
        .sampleCount = 1,
    };
};

class WSamplerBuilder {
   public:
    static inline WSamplerBuilder New() { return WSamplerBuilder(); }

    WSamplerBuilder &setAddressMode(WGPUAddressMode mode);
    WSamplerBuilder &setAddressModeV(WGPUAddressMode mode);
    WSamplerBuilder &setAddressModeU(WGPUAddressMode mode);
    WSamplerBuilder &setAddressModeW(WGPUAddressMode mode);
    WSamplerBuilder &setMinMagFilter(WGPUFilterMode mode);
    WSamplerBuilder &setMagFilter(WGPUFilterMode mode);
    WSamplerBuilder &setMinFilter(WGPUFilterMode mode);
    WSamplerBuilder &setMipmapFilter(WGPUMipmapFilterMode mode);
    WSamplerBuilder &setLodMinClamp(float min);
    WSamplerBuilder &setLodMaxClamp(float max);
    WSamplerBuilder &setCompareFunction(WGPUCompareFunction compare);
    WSamplerBuilder &setMaxAnisotropy(uint16_t max);

    WGPUSampler build(WGPUDevice device);

   private:
    WGPUSamplerDescriptor desc{
        .addressModeU = WGPUAddressMode_Repeat,
        .addressModeV = WGPUAddressMode_Repeat,
        .addressModeW = WGPUAddressMode_Repeat,
        .magFilter = WGPUFilterMode_Linear,
        .minFilter = WGPUFilterMode_Linear,
        .mipmapFilter = WGPUMipmapFilterMode_Linear,
        .lodMinClamp = 0.0f,
        .lodMaxClamp = 1.0f,
        .compare = WGPUCompareFunction_Undefined,
        .maxAnisotropy = 1,
    };
};

class WBindGroupLayoutBuilder {
   public:
    static inline WBindGroupLayoutBuilder New() { return WBindGroupLayoutBuilder(); }

    WBindGroupLayoutBuilder &addBindingSampler(uint32_t binding,
                                               WGPUShaderStageFlags visibility = WGPUShaderStage_Fragment);
    WBindGroupLayoutBuilder &addBindingTexture(uint32_t binding,
                                               WGPUTextureViewDimension viewDimension = WGPUTextureViewDimension_2D,
                                               WGPUShaderStageFlags visibility = WGPUShaderStage_Fragment);
    WBindGroupLayoutBuilder &addBindingUniform(uint32_t binding,
                                               WGPUShaderStageFlags visibility = WGPUShaderStage_Vertex |
                                                                                 WGPUShaderStage_Fragment |
                                                                                 WGPUShaderStage_Compute);
    WGPUBindGroupLayout build(WGPUDevice device);

   private:
    std::vector<WGPUBindGroupLayoutEntry> entries;
};

class WBindGroupBuilder {
   public:
    static inline WBindGroupBuilder New() { return WBindGroupBuilder(); }

    WBindGroupBuilder &addBindingSampler(uint32_t binding, WGPUSampler sampler,
                                         WGPUShaderStageFlags visibility = WGPUShaderStage_Fragment);
    WBindGroupBuilder &addBindingTexture(uint32_t binding, WGPUTextureView texture,
                                         WGPUTextureViewDimension viewDimension = WGPUTextureViewDimension_2D,
                                         WGPUShaderStageFlags visibility = WGPUShaderStage_Fragment);
    WBindGroupBuilder &addBindingUniform(uint32_t binding, WGPUBuffer buffer, size_t size,
                                         WGPUShaderStageFlags visibility = WGPUShaderStage_Vertex |
                                                                           WGPUShaderStage_Fragment |
                                                                           WGPUShaderStage_Compute);
    WBindGroupBuilder &addBindingUniform(uint32_t binding, WUniformBuffer buffer,
                                         WGPUShaderStageFlags visibility = WGPUShaderStage_Vertex |
                                                                           WGPUShaderStage_Fragment |
                                                                           WGPUShaderStage_Compute);

    WBindGroup build(WGPUDevice device);
    WBindGroup buildWithLayout(WGPUDevice device, WGPUBindGroupLayout layout);

    WGPUBindGroup buildBindGroup(WGPUDevice device, WGPUBindGroupLayout layout);
    WGPUBindGroupLayout buildBindGroupLayout(WGPUDevice device);

   private:
    std::vector<WGPUBindGroupEntry> entries;
    WBindGroupLayoutBuilder layoutBuilder;
};

class WRenderBufferBuilder {
   public:
    static inline WRenderBufferBuilder New() { return WRenderBufferBuilder(); }

    template <typename Vertex>
    WRenderBufferBuilder &setVertices(const std::vector<Vertex> &vertices) {
        this->vertices = vertices.data();
        this->verticesCount = vertices.size();
        this->verticesSize = sizeof(Vertex) * this->verticesCount;
        return *this;
    }
    WRenderBufferBuilder &setIndices(const std::vector<uint32_t> &indices);

    WRenderBuffer build(WGPUDevice device);

   private:
    const void *vertices;
    const uint32_t *indices;
    size_t verticesSize;
    size_t verticesCount;
    size_t indicesCount;
};

class WPipelineLayoutBuilder {
   public:
    static inline WPipelineLayoutBuilder New() { return WPipelineLayoutBuilder(); }

    WPipelineLayoutBuilder &addBindGroupLayout(WGPUBindGroupLayout layout);

    WGPUPipelineLayout build(WGPUDevice device);

   private:
    std::vector<WGPUBindGroupLayout> bindGroupLayouts;
};

class WRenderPipelineBuilder {
   public:
    static inline WRenderPipelineBuilder New() { return WRenderPipelineBuilder(); }

    WRenderPipelineBuilder &addBindGroupLayout(WGPUBindGroupLayout layout);
    WRenderPipelineBuilder &addColorTarget(WGPUTextureFormat format);
    WRenderPipelineBuilder &addVertexBufferLayout(WVertexLayout layout);
    WRenderPipelineBuilder &setVertexState(WGPUShaderModule shader, const char *entry = "vs_main");
    WRenderPipelineBuilder &setFragmentState(WGPUShaderModule shader, const char *entry = "fs_main");
    WRenderPipelineBuilder &setDefaultDepthState(WDepthState state = WDepthState::New());

    WRenderPipeline build(WGPUDevice device);
    WRenderPipeline buildWithLayout(WGPUDevice device, WGPUPipelineLayout layout);

    WGPURenderPipeline buildRenderPipeline(WGPUDevice device, WGPUPipelineLayout layout);
    WGPUPipelineLayout buildPipelineLayout(WGPUDevice device);

   private:
    WPipelineLayoutBuilder layoutBuilder;

    WGPURenderPipelineDescriptor desc;
    std::vector<WVertexLayout> vertexLayouts;
    std::vector<WGPUColorTargetState> colorTargetStates;
    WGPUFragmentState fragmentState;
    WGPUDepthStencilState depthStencilState;
    bool depthTest = false;
    bool stencilTest = false;
};

class WRenderBundleBuilder {
   public:
    static inline WRenderBundleBuilder New() { return WRenderBundleBuilder(); }

    WRenderBundleBuilder &setRenderBuffer(WRenderBuffer renderBuffer);
    WRenderBundleBuilder &addBindGroup(WBindGroup bindGroup);
    WRenderBundleBuilder &addColorFormat(WGPUTextureFormat format);
    WRenderBundleBuilder &setDepthFormat(WGPUTextureFormat format);
    WRenderBundleBuilder &setRenderPipeline(WRenderPipeline pipeline);
    WRenderBundleBuilder &setDefaultDepthFormat() { return setDepthFormat(WGPUTextureFormat_Depth32Float); }

    WRenderBundle build(WGPUDevice device);

   private:
    WRenderBuffer renderBuffer;
    std::vector<WBindGroup> bindGroups;
    std::vector<WGPUTextureFormat> colorFormats;
    WGPUTextureFormat depthFormat;
    WRenderPipeline pipeline;
};
