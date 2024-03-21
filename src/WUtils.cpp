#include <WUtils.hpp>

WRenderPassBuilder &WRenderPassBuilder::addColorTarget(WColorAttachment attachment) {
    colorAttachments.push_back(attachment);
    return *this;
}
WRenderPassBuilder &WRenderPassBuilder::setDepthAttachment(WDepthStencilAttachment attachment) {
    depthStencilAttachment = attachment;

    depthTest = true;
    return *this;
}
WGPURenderPassEncoder WRenderPassBuilder::build(WGPUCommandEncoder commandEncoder, const char *label) {
    WGPURenderPassDescriptor desc{
        .label = label,
        .colorAttachmentCount = colorAttachments.size(),
        .colorAttachments = colorAttachments.data(),
    };

    if (!depthTest) {
        depthStencilAttachment.depthLoadOp = WGPULoadOp_Undefined;
        depthStencilAttachment.depthStoreOp = WGPUStoreOp_Undefined;
    }
    if (!stencilTest) {
        depthStencilAttachment.stencilLoadOp = WGPULoadOp_Undefined;
        depthStencilAttachment.stencilStoreOp = WGPUStoreOp_Undefined;
    }

    if (depthTest || stencilTest) {
        desc.depthStencilAttachment = &depthStencilAttachment;
    }

    return wgpuCommandEncoderBeginRenderPass(commandEncoder, &desc);
}

WTextureBuilder &WTextureBuilder::addTextureUsage(WGPUTextureUsageFlags usage) {
    desc.usage |= usage;
    return *this;
}
WTextureBuilder &WTextureBuilder::removeTextureUsages(WGPUTextureUsageFlags usage) {
    desc.usage &= usage;
    return *this;
}
WTextureBuilder &WTextureBuilder::setTextureUsages(WGPUTextureUsageFlags usages) {
    desc.usage = usages;
    return *this;
}
WTextureBuilder &WTextureBuilder::setDimension(WGPUTextureDimension dimension) {
    desc.dimension = dimension;
    return *this;
}
WTextureBuilder &WTextureBuilder::setFormat(WGPUTextureFormat format) {
    desc.format = format;
    return *this;
}
WTextureBuilder &WTextureBuilder::setMipLevelCount(uint32_t count) {
    desc.mipLevelCount = count;
    return *this;
}
WTextureBuilder &WTextureBuilder::setSampleCount(uint32_t count) {
    desc.sampleCount = count;
    return *this;
}
WTexture WTextureBuilder::build(WGPUDevice device,
                                WGPUExtent3D size,
                                uint32_t channels,
                                const unsigned char *data,
                                uint32_t stride) {
    desc.size = size;
    WGPUTexture texture = wgpuDeviceCreateTexture(device, &desc);

    if (data) {
        WGPUImageCopyTexture destination{
            .texture = texture,
            .mipLevel = 0,
            .origin = WGPUOrigin3D{0, 0, 0},
            .aspect = WGPUTextureAspect_All,
        };
        WGPUTextureDataLayout dataLayout{
            .offset = 0,
            .bytesPerRow = channels * size.width,
            .rowsPerImage = size.height * size.depthOrArrayLayers,
        };

        WGPUQueue queue = wgpuDeviceGetQueue(device);
        wgpuQueueWriteTexture(
            queue,
            &destination,
            data,
            stride * size.width * size.height * size.depthOrArrayLayers,
            &dataLayout,
            &size);
    }

    return WTexture::New(texture, desc);
}

WSamplerBuilder &WSamplerBuilder::setAddressMode(WGPUAddressMode mode) {
    desc.addressModeU = mode;
    desc.addressModeV = mode;
    desc.addressModeW = mode;
    return *this;
}
WSamplerBuilder &WSamplerBuilder::setAddressModeV(WGPUAddressMode mode) {
    desc.addressModeV = mode;
    return *this;
}
WSamplerBuilder &WSamplerBuilder::setAddressModeU(WGPUAddressMode mode) {
    desc.addressModeU = mode;
    return *this;
}
WSamplerBuilder &WSamplerBuilder::setAddressModeW(WGPUAddressMode mode) {
    desc.addressModeW = mode;
    return *this;
}
WSamplerBuilder &WSamplerBuilder::setMinMagFilter(WGPUFilterMode mode) {
    desc.magFilter = mode;
    desc.minFilter = mode;
    return *this;
}
WSamplerBuilder &WSamplerBuilder::setMagFilter(WGPUFilterMode mode) {
    desc.magFilter = mode;
    return *this;
}
WSamplerBuilder &WSamplerBuilder::setMinFilter(WGPUFilterMode mode) {
    desc.minFilter = mode;
    return *this;
}
WSamplerBuilder &WSamplerBuilder::setMipmapFilter(WGPUMipmapFilterMode mode) {
    desc.mipmapFilter = mode;
    return *this;
}
WSamplerBuilder &WSamplerBuilder::setLodMinClamp(float min) {
    desc.lodMinClamp = min;
    return *this;
}
WSamplerBuilder &WSamplerBuilder::setLodMaxClamp(float max) {
    desc.lodMaxClamp = max;
    return *this;
}
WSamplerBuilder &WSamplerBuilder::setCompareFunction(WGPUCompareFunction compare) {
    desc.compare = compare;
    return *this;
}
WSamplerBuilder &WSamplerBuilder::setMaxAnisotropy(uint16_t max) {
    desc.maxAnisotropy = max;
    return *this;
}
WGPUSampler WSamplerBuilder::build(WGPUDevice device) {
    return wgpuDeviceCreateSampler(device, &desc);
}

WBindGroupLayoutBuilder &WBindGroupLayoutBuilder::addBindingSampler(uint32_t binding, WGPUShaderStageFlags visibility) {
    entries.push_back(WGPUBindGroupLayoutEntry{
        .binding = binding,
        .visibility = visibility,
        .sampler = WGPUSamplerBindingLayout{
            .type = WGPUSamplerBindingType_Filtering,
        },
    });
    return *this;
}
WBindGroupLayoutBuilder &WBindGroupLayoutBuilder::addBindingTexture(uint32_t binding,
                                                                    WGPUTextureViewDimension viewDimension,
                                                                    WGPUShaderStageFlags visibility) {
    entries.push_back(WGPUBindGroupLayoutEntry{
        .binding = binding,
        .visibility = visibility,
        .texture = WGPUTextureBindingLayout{
            .sampleType = WGPUTextureSampleType_Float,
            .viewDimension = viewDimension,
            .multisampled = false,
        },
    });
    return *this;
}
WBindGroupLayoutBuilder &WBindGroupLayoutBuilder::addBindingUniform(uint32_t binding, WGPUShaderStageFlags visibility) {
    entries.push_back(WGPUBindGroupLayoutEntry{
        .binding = binding,
        .visibility = visibility,
        .buffer = WGPUBufferBindingLayout{
            .type = WGPUBufferBindingType_Uniform,
            .hasDynamicOffset = false,
        },
    });
    return *this;
}
WGPUBindGroupLayout WBindGroupLayoutBuilder::build(WGPUDevice device) {
    WGPUBindGroupLayoutDescriptor desc{
        .entryCount = entries.size(),
        .entries = entries.data(),
    };
    return wgpuDeviceCreateBindGroupLayout(device, &desc);
}

WBindGroupBuilder &WBindGroupBuilder::addBindingSampler(uint32_t binding, WGPUSampler sampler, WGPUShaderStageFlags visibility) {
    entries.push_back(WGPUBindGroupEntry{
        .binding = binding,
        .sampler = sampler,
    });
    layoutBuilder.addBindingSampler(binding, visibility);
    return *this;
}
WBindGroupBuilder &WBindGroupBuilder::addBindingTexture(uint32_t binding, WGPUTextureView texture, WGPUTextureViewDimension viewDimension, WGPUShaderStageFlags visibility) {
    entries.push_back(WGPUBindGroupEntry{
        .binding = binding,
        .textureView = texture,
    });
    layoutBuilder.addBindingTexture(binding, viewDimension, visibility);
    return *this;
}
WBindGroupBuilder &WBindGroupBuilder::addBindingUniform(uint32_t binding, WGPUBuffer buffer, size_t size, WGPUShaderStageFlags visibility) {
    entries.push_back(WGPUBindGroupEntry{
        .binding = binding,
        .buffer = buffer,
        .size = size,
    });
    layoutBuilder.addBindingUniform(binding, visibility);
    return *this;
}
WBindGroupBuilder &WBindGroupBuilder::addBindingUniform(uint32_t binding, WUniformBuffer buffer, WGPUShaderStageFlags visibility) {
    entries.push_back(WGPUBindGroupEntry{
        .binding = binding,
        .buffer = buffer,
        .size = buffer.getSize(),
    });
    layoutBuilder.addBindingUniform(binding, visibility);
    return *this;
}
WBindGroup WBindGroupBuilder::build(WGPUDevice device) {
    WGPUBindGroupLayout layout = buildBindGroupLayout(device);
    WGPUBindGroup bindGroup = buildBindGroup(device, layout);
    return WBindGroup::New(bindGroup, layout);
}
WBindGroup WBindGroupBuilder::buildWithLayout(WGPUDevice device, WGPUBindGroupLayout layout) {
    return WBindGroup::New(buildBindGroup(device, layout), layout);
}
WGPUBindGroup WBindGroupBuilder::buildBindGroup(WGPUDevice device, WGPUBindGroupLayout layout) {
    WGPUBindGroupDescriptor desc{
        .layout = layout,
        .entryCount = entries.size(),
        .entries = entries.data(),
    };
    return wgpuDeviceCreateBindGroup(device, &desc);
}
WGPUBindGroupLayout WBindGroupBuilder::buildBindGroupLayout(WGPUDevice device) {
    return layoutBuilder.build(device);
}

WRenderBufferBuilder &WRenderBufferBuilder::setIndices(const std::vector<uint32_t> &indices) {
    this->indices = indices.data();
    this->indicesCount = indices.size();
    return *this;
}
WRenderBuffer WRenderBufferBuilder::build(WGPUDevice device) {
    return WRenderBuffer::New(device, vertices, verticesSize, verticesCount, indices, indicesCount);
}

WPipelineLayoutBuilder &WPipelineLayoutBuilder::addBindGroupLayout(WGPUBindGroupLayout layout) {
    bindGroupLayouts.push_back(layout);
    return *this;
}
WGPUPipelineLayout WPipelineLayoutBuilder::build(WGPUDevice device) {
    WGPUPipelineLayoutDescriptor desc{
        .bindGroupLayoutCount = bindGroupLayouts.size(),
        .bindGroupLayouts = bindGroupLayouts.data(),
    };
    return wgpuDeviceCreatePipelineLayout(device, &desc);
}

WRenderPipelineBuilder &WRenderPipelineBuilder::addBindGroupLayout(WGPUBindGroupLayout layout) {
    this->layoutBuilder.addBindGroupLayout(layout);
    return *this;
}
WRenderPipelineBuilder &WRenderPipelineBuilder::addColorTarget(WGPUTextureFormat format) {
    this->colorTargetStates.push_back(WGPUColorTargetState{
        .format = format,
        .writeMask = WGPUColorWriteMask_All,
    });
    return *this;
}
WRenderPipelineBuilder &WRenderPipelineBuilder::addVertexBufferLayout(WVertexLayout layout) {
    this->vertexLayouts.push_back(layout);
    return *this;
}
WRenderPipelineBuilder &WRenderPipelineBuilder::setVertexState(WGPUShaderModule shader, const char *entry) {
    desc.vertex.module = shader,
    desc.vertex.entryPoint = entry;
    return *this;
}
WRenderPipelineBuilder &WRenderPipelineBuilder::setFragmentState(WGPUShaderModule shader, const char *entry) {
    fragmentState.module = shader;
    fragmentState.entryPoint = entry;
    return *this;
}
WRenderPipelineBuilder &WRenderPipelineBuilder::setDefaultDepthState(WDepthState state) {
    depthStencilState.format = state.format;
    depthStencilState.depthCompare = state.compareFunc;
    depthStencilState.depthWriteEnabled = state.depthWriteEnabled;
    depthTest = true;
    return *this;
}
WRenderPipeline WRenderPipelineBuilder::build(WGPUDevice device) {
    WGPUPipelineLayout layout = buildPipelineLayout(device);
    WGPURenderPipeline pipeline = buildRenderPipeline(device, layout);
    return WRenderPipeline::New(pipeline, layout);
}
WRenderPipeline WRenderPipelineBuilder::buildWithLayout(WGPUDevice device, WGPUPipelineLayout layout) {
    return WRenderPipeline();
}
WGPURenderPipeline WRenderPipelineBuilder::buildRenderPipeline(WGPUDevice device, WGPUPipelineLayout layout) {
    std::vector<WGPUVertexBufferLayout> vertexBufferLayouts;
    vertexBufferLayouts.reserve(vertexLayouts.size());
    for (const WVertexLayout &layout : vertexLayouts) {
        vertexBufferLayouts.push_back(WGPUVertexBufferLayout{
            .arrayStride = layout.arrayStride,
            .stepMode = layout.stepMode,
            .attributeCount = layout.attributes.size(),
            .attributes = layout.attributes.data(),
        });
    }

    desc.vertex.bufferCount = vertexBufferLayouts.size();
    desc.vertex.buffers = vertexBufferLayouts.data();

    fragmentState.targetCount = colorTargetStates.size();
    fragmentState.targets = colorTargetStates.data();
    desc.fragment = &fragmentState;

    desc.primitive = WGPUPrimitiveState{.topology = WGPUPrimitiveTopology_TriangleList};
    desc.multisample = WGPUMultisampleState{
        .count = 1,
        .mask = 0xFFFFFFFF,
    };

    if (!stencilTest) {
        depthStencilState.stencilBack = WGPUStencilFaceState{
            .compare = WGPUCompareFunction_Always,
            .failOp = WGPUStencilOperation_Keep,
            .depthFailOp = WGPUStencilOperation_Keep,
            .passOp = WGPUStencilOperation_Keep,
        };
        depthStencilState.stencilFront = WGPUStencilFaceState{
            .compare = WGPUCompareFunction_Always,
            .failOp = WGPUStencilOperation_Keep,
            .depthFailOp = WGPUStencilOperation_Keep,
            .passOp = WGPUStencilOperation_Keep,
        };
    }

    if (depthTest) {
        desc.depthStencil = &depthStencilState;
    }

    desc.layout = layout;
    return wgpuDeviceCreateRenderPipeline(device, &desc);
}
WGPUPipelineLayout WRenderPipelineBuilder::buildPipelineLayout(WGPUDevice device) {
    return layoutBuilder.build(device);
}

WRenderBundleBuilder &WRenderBundleBuilder::setRenderBuffer(WRenderBuffer renderBuffer) {
    this->renderBuffer = renderBuffer;
    return *this;
}
WRenderBundleBuilder &WRenderBundleBuilder::addBindGroup(WBindGroup bindGroup) {
    this->bindGroups.push_back(bindGroup);
    return *this;
}
WRenderBundleBuilder &WRenderBundleBuilder::addColorFormat(WGPUTextureFormat format) {
    this->colorFormats.push_back(format);
    return *this;
}
WRenderBundleBuilder &WRenderBundleBuilder::setDepthFormat(WGPUTextureFormat format) {
    this->depthFormat = format;
    return *this;
}
WRenderBundleBuilder &WRenderBundleBuilder::setRenderPipeline(WRenderPipeline pipeline) {
    this->pipeline = pipeline;
    return *this;
}
WRenderBundle WRenderBundleBuilder::build(WGPUDevice device) {
    WGPURenderBundleEncoderDescriptor encoderDesc{
        .colorFormatCount = colorFormats.size(),
        .colorFormats = colorFormats.data(),
        .depthStencilFormat = depthFormat,
        .sampleCount = 1,
    };
    WGPURenderBundleEncoder encoder = wgpuDeviceCreateRenderBundleEncoder(device, &encoderDesc);
    pipeline.bind(encoder);
    for (uint32_t i = 0; i < bindGroups.size(); i++) {
        bindGroups[i].bind(encoder, i);
    }
    renderBuffer.render(encoder);
    WGPURenderBundle renderBundle = wgpuRenderBundleEncoderFinish(encoder, nullptr);

    return WRenderBundle::New(renderBundle);
}
