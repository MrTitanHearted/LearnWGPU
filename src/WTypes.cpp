#include <WTypes.hpp>

#include <WUtils.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

WGPUBuffer wgpuDeviceCreateBufferInit(WGPUDevice device, WGPUBufferDescriptor desc, const void *data) {
    if (desc.size == 0 || data == nullptr) {
        return wgpuDeviceCreateBuffer(device, &desc);
    }

    desc.mappedAtCreation = true;
    WGPUBuffer buffer = wgpuDeviceCreateBuffer(device, &desc);

    void *bufferData = wgpuBufferGetMappedRange(buffer, 0, desc.size);
    memcpy(bufferData, data, desc.size);
    wgpuBufferUnmap(buffer);
    return buffer;
}

WColorAttachment WColorAttachment::New(WGPUTextureView target) {
    WGPURenderPassColorAttachment desc{
        .view = target,
        .loadOp = WGPULoadOp_Clear,
        .storeOp = WGPUStoreOp_Store,
        .clearValue = WGPUColor{0.0, 0.0, 0.0, 1.0},
    };
    WColorAttachment colorAttachment;
    colorAttachment.desc = desc;
    return colorAttachment;
}
WColorAttachment &WColorAttachment::setClearColor(float r, float g, float b, float a) {
    desc.clearValue = WGPUColor{r, g, b, a};
    return *this;
}
WColorAttachment &WColorAttachment::setLoadOp(WGPULoadOp op) {
    desc.loadOp = op;
    return *this;
}
WColorAttachment &WColorAttachment::setStoreOp(WGPUStoreOp op) {
    desc.storeOp = op;
    return *this;
}
WColorAttachment &WColorAttachment::setResolveTarget(WGPUTextureView target) {
    desc.resolveTarget = target;
    return *this;
}

WDepthStencilAttachment WDepthStencilAttachment::New(WGPUTextureView target) {
    WGPURenderPassDepthStencilAttachment desc{
        .view = target,
        .depthLoadOp = WGPULoadOp_Clear,
        .depthStoreOp = WGPUStoreOp_Store,
        .depthClearValue = 1.0,
    };
    WDepthStencilAttachment attachment;
    attachment.desc = desc;
    return attachment;
}
WDepthStencilAttachment &WDepthStencilAttachment::setClearValue(float value) {
    desc.depthClearValue = value;
    return *this;
}
WDepthStencilAttachment &WDepthStencilAttachment::setLoadOp(WGPULoadOp op) {
    desc.depthLoadOp = op;
    return *this;
}
WDepthStencilAttachment &WDepthStencilAttachment::setStoreOp(WGPUStoreOp op) {
    desc.depthStoreOp = op;
    return *this;
}

WDepthState &WDepthState::setFormat(WGPUTextureFormat format) {
    this->format = format;
    return *this;
}
WDepthState &WDepthState::setCompareFunction(WGPUCompareFunction compareFunction) {
    this->compareFunc = compareFunction;
    return *this;
}
WDepthState &WDepthState::setDepthWriteEnabled(bool enabled) {
    this->depthWriteEnabled = enabled;
    return *this;
}

WTexture WTexture::New(WGPUTexture texture, WGPUTextureDescriptor desc) {
    WTexture wtexture;
    wtexture.texture = texture;
    wtexture.view = wgpuTextureCreateView(texture, nullptr);
    wtexture.desc = desc;
    return wtexture;
}
WTexture WTexture::GetDepthTexture(WGPUDevice device, uint32_t width, uint32_t height, WGPUTextureFormat format) {
    return WTextureBuilder::New()
        .setFormat(format)
        .setTextureUsages(WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding)
        .build(device, WGPUExtent3D{.width = width, .height = height, .depthOrArrayLayers = 1});
}
WTexture WTexture::fromFileAsRgba8(WGPUDevice device, std::string path, bool flipUV) {
    stbi_set_flip_vertically_on_load(flipUV);

    int32_t width, height, channels;
    stbi_uc *data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (data == nullptr) {
        throw std::exception(fmt::format("[WEngine]::[ERROR]: Failed to load texture from path: '{}'", path).c_str());
    }

    WGPUExtent3D size{
        .width = (uint32_t)width,
        .height = (uint32_t)height,
        .depthOrArrayLayers = 1,
    };

    if (flipUV) {
        stbi_set_flip_vertically_on_load(!flipUV);
    }

    return WTextureBuilder::New()
        .setFormat(WGPUTextureFormat_RGBA8Unorm)
        .build(device, size, 4, data, 4 * sizeof(stbi_uc));
}

WUniformBuffer WUniformBuffer::New(WGPUDevice device, void *data, size_t size) {
    WGPUBufferDescriptor desc{
        .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
        .size = size,
        .mappedAtCreation = true,
    };
    WUniformBuffer uniform;
    uniform.size = size;
    uniform.buffer = wgpuDeviceCreateBufferInit(device, WGPUBufferDescriptor{
                                                            .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
                                                            .size = size,
                                                        },
                                                data);

    return uniform;
}
void WUniformBuffer::update(WGPUQueue queue, void *data) {
    wgpuQueueWriteBuffer(queue, buffer, 0, data, size);
}
void WUniformBuffer::updateWithOffset(WGPUQueue queue, void *data, uint32_t offset) {
    wgpuQueueWriteBuffer(queue, buffer, offset, data, size);
}

WBindGroup WBindGroup::New(WGPUBindGroup bindGroup, WGPUBindGroupLayout bindGroupLayout) {
    WBindGroup wBindGroup;
    wBindGroup.bindGroup = bindGroup;
    wBindGroup.bindGroupLayout = bindGroupLayout;
    return wBindGroup;
}
void WBindGroup::bind(WGPURenderPassEncoder encoder, uint32_t groupIndex) {
    wgpuRenderPassEncoderSetBindGroup(encoder, groupIndex, bindGroup, 0, nullptr);
}
void WBindGroup::bind(WGPURenderBundleEncoder encoder, uint32_t groupIndex) {
    wgpuRenderBundleEncoderSetBindGroup(encoder, groupIndex, bindGroup, 0, nullptr);
}

WVertexLayout WVertexLayout::New(size_t arrayStride) {
    WVertexLayout vertexLayout;
    vertexLayout.arrayStride = arrayStride;
    return vertexLayout;
}

WVertexLayout &WVertexLayout::setStepMode(WGPUVertexStepMode mode) {
    stepMode = mode;
    return *this;
}
WVertexLayout &WVertexLayout::setArrayStride(size_t stride) {
    arrayStride = stride;
    return *this;
}
WVertexLayout &WVertexLayout::setAttributes(std::vector<WGPUVertexAttribute> attributes) {
    this->attributes = attributes;
    return *this;
}
WVertexLayout &WVertexLayout::addAttribute(WGPUVertexFormat format, size_t offset, uint32_t shaderLocation) {
    this->attributes.push_back(WGPUVertexAttribute{
        .format = format,
        .offset = offset,
        .shaderLocation = shaderLocation,
    });
    return *this;
}

WRenderBuffer WRenderBuffer::New(WGPUDevice device, const void *vertices, size_t verticesSize, size_t verticesCount, const uint32_t *indices, size_t indicesCount) {
    WRenderBuffer renderBuffer;
    renderBuffer.verticesCount = verticesCount;
    renderBuffer.verticesSize = verticesSize;
    renderBuffer.indicesCount = indicesCount;
    renderBuffer.indicesSize = sizeof(uint32_t) * indicesCount;

    renderBuffer.vertex = wgpuDeviceCreateBufferInit(
        device,
        WGPUBufferDescriptor{
            .usage = WGPUBufferUsage_Vertex,
            .size = renderBuffer.verticesSize,
        },
        vertices);

    renderBuffer.index = wgpuDeviceCreateBufferInit(
        device,
        WGPUBufferDescriptor{
            .usage = WGPUBufferUsage_Index,
            .size = renderBuffer.indicesSize,
        },
        indices);

    return renderBuffer;
}
void WRenderBuffer::render(WGPURenderPassEncoder encoder) {
    wgpuRenderPassEncoderSetVertexBuffer(encoder, 0, vertex, 0, verticesSize);
    wgpuRenderPassEncoderSetIndexBuffer(encoder, index, WGPUIndexFormat_Uint32, 0, indicesSize);
    wgpuRenderPassEncoderDrawIndexed(encoder, indicesCount, 1, 0, 0, 0);
}
void WRenderBuffer::render(WGPURenderBundleEncoder encoder) {
    wgpuRenderBundleEncoderSetVertexBuffer(encoder, 0, vertex, 0, verticesSize);
    wgpuRenderBundleEncoderSetIndexBuffer(encoder, index, WGPUIndexFormat_Uint32, 0, indicesSize);
    wgpuRenderBundleEncoderDrawIndexed(encoder, indicesCount, 1, 0, 0, 0);
}

WRenderPipeline WRenderPipeline::New(WGPURenderPipeline pipeline, WGPUPipelineLayout layout) {
    WRenderPipeline renderPipeline;
    renderPipeline.pipeline = pipeline;
    renderPipeline.layout = layout;
    return renderPipeline;
}
void WRenderPipeline::bind(WGPURenderPassEncoder encoder) {
    wgpuRenderPassEncoderSetPipeline(encoder, pipeline);
}
void WRenderPipeline::bind(WGPURenderBundleEncoder encoder) {
    wgpuRenderBundleEncoderSetPipeline(encoder, pipeline);
}

WRenderBundle WRenderBundle::New(WGPURenderBundle renderBundle) {
    WRenderBundle wRenderBundle;
    wRenderBundle.renderBundle = renderBundle;
    return wRenderBundle;
}
void WRenderBundle::render(WGPURenderPassEncoder encoder) {
    wgpuRenderPassEncoderExecuteBundles(encoder, 1, &renderBundle);
}
