#include <WBindings.hpp>

WBindGroupLayoutBuilder& WBindGroupLayoutBuilder::addBindingDynamicUniform(uint32_t binding, WGPUShaderStageFlags visibility) {
    entries.push_back(WGPUBindGroupLayoutEntry{
        .binding = binding,
        .visibility = visibility,
        .buffer = WGPUBufferBindingLayout{
            .type = WGPUBufferBindingType_Uniform,
            .hasDynamicOffset = true,
            .minBindingSize = 0,
        },
    });

    return *this;
}

WBindGroupLayoutBuilder& WBindGroupLayoutBuilder::addBindingUniform(uint32_t binding, WGPUShaderStageFlags visibility) {
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

WBindGroupLayoutBuilder& WBindGroupLayoutBuilder::addBindingTexture(uint32_t binding, WGPUShaderStageFlags visibility) {
    entries.push_back(WGPUBindGroupLayoutEntry{
        .binding = binding,
        .visibility = visibility,
        .texture = WGPUTextureBindingLayout{
            .sampleType = WGPUTextureSampleType_Float,
            .viewDimension = WGPUTextureViewDimension_2D,
            .multisampled = false,
        },
    });

    return *this;
}

WBindGroupLayoutBuilder& WBindGroupLayoutBuilder::addBindingSampler(uint32_t binding, WGPUShaderStageFlags visibility) {
    entries.push_back(WGPUBindGroupLayoutEntry{
        .binding = binding,
        .visibility = visibility,
        .sampler = WGPUSamplerBindingLayout{
            .type = WGPUSamplerBindingType_Filtering,
        },
    });

    return *this;
}

WGPUBindGroupLayout WBindGroupLayoutBuilder::build(WGPUDevice device) {
    desc.entryCount = entries.size();
    desc.entries = entries.data();

    return wgpuDeviceCreateBindGroupLayout(device, &desc);
}

WBindGroupBuilder& WBindGroupBuilder::addBindingDynamicUniform(uint32_t binding, WGPUBuffer buffer, size_t size) {
    return addBindingUniform(binding, buffer, size);
}

WBindGroupBuilder& WBindGroupBuilder::addBindingDynamicUniform(uint32_t binding, WDynamicUniformBuffer buffer) {
    return addBindingDynamicUniform(binding, buffer, buffer.size);
}

WBindGroupBuilder& WBindGroupBuilder::addBindingUniform(uint32_t binding, WGPUBuffer buffer, size_t size) {
    entries.push_back(WGPUBindGroupEntry{
        .binding = binding,
        .buffer = buffer,
        .offset = 0,
        .size = size,
    });

    return *this;
}

WBindGroupBuilder& WBindGroupBuilder::addBindingUniform(uint32_t binding, WUniformBuffer buffer) {
    return addBindingUniform(binding, buffer.buffer, buffer.size);
}

WBindGroupBuilder& WBindGroupBuilder::addBindingTexture(uint32_t binding, WGPUTextureView view) {
    entries.push_back(WGPUBindGroupEntry{
        .binding = binding,
        .offset = 0,
        .textureView = view,
    });

    return *this;
}

WBindGroupBuilder& WBindGroupBuilder::addBindingSampler(uint32_t binding, WGPUSampler sampler) {
    entries.push_back(WGPUBindGroupEntry{
        .binding = binding,
        .offset = 0,
        .sampler = sampler,
    });

    return *this;
}

WGPUBindGroup WBindGroupBuilder::build(WGPUDevice device, WGPUBindGroupLayout layout) {
    desc.layout = layout;
    desc.entryCount = entries.size();
    desc.entries = entries.data();

    return wgpuDeviceCreateBindGroup(device, &desc);
}
