#pragma once

#include <WInclude.hpp>
#include <WUniformBuffer.hpp>

struct WBindGroupLayoutBuilder {
    std::vector<WGPUBindGroupLayoutEntry> entries;
    WGPUBindGroupLayoutDescriptor desc;

    WBindGroupLayoutBuilder& addBindingUniform(uint32_t binding, WGPUShaderStageFlags visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment | WGPUShaderStage_Compute);
    WBindGroupLayoutBuilder& addBindingTexture(uint32_t binding, WGPUShaderStageFlags visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment | WGPUShaderStage_Compute);
    WBindGroupLayoutBuilder& addBindingSampler(uint32_t binding, WGPUShaderStageFlags visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment | WGPUShaderStage_Compute);

    WGPUBindGroupLayout build(WGPUDevice device);
};

struct WBindGroupBuilder {
    std::vector<WGPUBindGroupEntry> entries;
    WGPUBindGroupDescriptor desc;

    WBindGroupBuilder& addBindingUniform(uint32_t binding, WGPUBuffer buffer, size_t size);
    WBindGroupBuilder& addBindingUniform(uint32_t binding, WUniformBuffer buffer);
    WBindGroupBuilder& addBindingTexture(uint32_t binding, WGPUTextureView view);
    WBindGroupBuilder& addBindingSampler(uint32_t binding, WGPUSampler sampler);

    WGPUBindGroup build(WGPUDevice device, WGPUBindGroupLayout layout);
};