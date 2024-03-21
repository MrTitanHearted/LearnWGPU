#pragma once

#include <WInclude.hpp>

class WColorAttachment {
   public:
    static WColorAttachment New(WGPUTextureView target);

    WColorAttachment &setClearColor(float r, float g, float b, float a);
    WColorAttachment &setLoadOp(WGPULoadOp op);
    WColorAttachment &setStoreOp(WGPUStoreOp op);
    WColorAttachment &setResolveTarget(WGPUTextureView target);

    inline operator WGPURenderPassColorAttachment() const { return desc; }

   private:
    WGPURenderPassColorAttachment desc;
};

class WDepthStencilAttachment {
   public:
    static WDepthStencilAttachment New(WGPUTextureView target);

    WDepthStencilAttachment &setClearValue(float value);
    WDepthStencilAttachment &setLoadOp(WGPULoadOp op);
    WDepthStencilAttachment &setStoreOp(WGPUStoreOp op);

    inline operator WGPURenderPassDepthStencilAttachment() const { return desc; }

   private:
    WGPURenderPassDepthStencilAttachment desc;
};

class WDepthState {
   public:
    static inline WDepthState New() { return WDepthState(); }

    WDepthState &setFormat(WGPUTextureFormat format);
    WDepthState &setCompareFunction(WGPUCompareFunction compareFunction);
    WDepthState &setDepthWriteEnabled(bool enabled);

    friend class WRenderPipelineBuilder;

   private:
    WGPUTextureFormat format = WGPUTextureFormat_Depth32Float;
    WGPUCompareFunction compareFunc = WGPUCompareFunction_Less;
    bool depthWriteEnabled = true;
};

class WTexture {
   public:
    static WTexture New(WGPUTexture texture, WGPUTextureDescriptor desc);
    static WTexture GetDepthTexture(WGPUDevice device, uint32_t width, uint32_t height,
                                    WGPUTextureFormat format = WGPUTextureFormat_Depth32Float);

    inline operator WGPUTexture() const { return texture; };
    inline operator WGPUTextureView() const { return view; };
    inline operator WGPUTextureDescriptor() const { return desc; };

    static WTexture fromFileAsRgba8(WGPUDevice device, std::string path, bool flipUV = true);
    static WTexture fromMemoryAsRgba8(WGPUDevice device, const void *data, size_t size, bool flipUV = true);

   private:
    WGPUTexture texture;
    WGPUTextureView view;
    WGPUTextureDescriptor desc;
};

class WUniformBuffer {
   public:
    static WUniformBuffer New(WGPUDevice device, void *data, size_t size);

    inline operator WGPUBuffer() const { return buffer; }

    void update(WGPUQueue queue, void *data);
    void updateWithOffset(WGPUQueue queue, void *data, uint32_t offset);

    inline size_t getSize() const { return size; }

   private:
    WGPUBuffer buffer;
    size_t size;
};

class WBindGroup {
   public:
    static WBindGroup New(WGPUBindGroup bindGroup, WGPUBindGroupLayout bindGroupLayout);

    inline operator WGPUBindGroup() const { return bindGroup; }
    inline operator WGPUBindGroupLayout() const { return bindGroupLayout; }

    void bind(WGPURenderPassEncoder encoder, uint32_t groupIndex);
    void bind(WGPURenderBundleEncoder encoder, uint32_t groupIndex);

   private:
    WGPUBindGroup bindGroup;
    WGPUBindGroupLayout bindGroupLayout;
};

class WVertexLayout {
   public:
    static WVertexLayout New(size_t arrayStride);

    WVertexLayout &setStepMode(WGPUVertexStepMode mode);
    WVertexLayout &setArrayStride(size_t stride);
    WVertexLayout &setAttributes(std::vector<WGPUVertexAttribute> attributes);
    WVertexLayout &addAttribute(WGPUVertexFormat format, size_t offset, uint32_t shaderLocation);

    friend class WRenderPipelineBuilder;

   private:
    std::vector<WGPUVertexAttribute> attributes;
    WGPUVertexStepMode stepMode = WGPUVertexStepMode_Vertex;
    size_t arrayStride;
};

class WRenderBuffer {
   public:
    static WRenderBuffer New(WGPUDevice device,
                             const void *vertices,
                             size_t verticesSize,
                             size_t verticesCount,
                             const uint32_t *indices,
                             size_t indicesCount);

    void render(WGPURenderPassEncoder encoder);
    void render(WGPURenderBundleEncoder encoder);

   private:
    WGPUBuffer vertex;
    WGPUBuffer index;
    size_t verticesSize;
    size_t indicesSize;
    size_t verticesCount;
    size_t indicesCount;
};

class WRenderPipeline {
   public:
    static WRenderPipeline New(WGPURenderPipeline pipeline, WGPUPipelineLayout layout);

    inline operator WGPURenderPipeline() const { return pipeline; }
    inline operator WGPUPipelineLayout() const { return layout; }

    void bind(WGPURenderPassEncoder encoder);
    void bind(WGPURenderBundleEncoder encoder);

   private:
    WGPUPipelineLayout layout;
    WGPURenderPipeline pipeline;
};

class WRenderBundle {
   public:
    static WRenderBundle New(WGPURenderBundle renderBundle);

    inline operator WGPURenderBundle() const { return renderBundle; }
    inline const WGPURenderBundle &getRenderBundle() const { return renderBundle; }

    void render(WGPURenderPassEncoder encoder);

   private:
    WGPURenderBundle renderBundle;
};
