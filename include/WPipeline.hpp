#pragma once

#include <WInclude.hpp>

// struct WPipelineLayout {
//     // std::vecotr<WGPUBindGroupLayout> bindgroupLayouts;
//     // std::map<uint32_t, std::vector<WGPUBindGroupLayoutEntry>> bindgroups;
//     WGPUPipelineLayout pipelineLayout;
// };
// struct WRenderPipeline {
//     WGPURenderPipeline renderPipeline;
// };

struct WPipelineLayoutBuilder {
    std::vector<WGPUBindGroupLayout> bindGroupLayouts;
    WGPUPipelineLayoutDescriptor layoutDesc;

    WPipelineLayoutBuilder& addBindGroupLayout(WGPUBindGroupLayout layout);

    WGPUPipelineLayout build(WGPUDevice device);
};

struct WRenderPipelineBuilder {
    WGPURenderPipelineDescriptor pipelineDesc;
    std::vector<WGPUVertexBufferLayout> vertexBufferLayouts;
    std::vector<WGPUColorTargetState> colorTargetStates;
    WGPUFragmentState fragmentState;

    WRenderPipelineBuilder& addColorTarget(WGPUTextureFormat format);
    WRenderPipelineBuilder& addVertexBufferLayout(WGPUVertexBufferLayout layout);
    WRenderPipelineBuilder& setFragmentState(WGPUShaderModule shader, const char* entry);
    WRenderPipelineBuilder& setVertexState(WGPUShaderModule shader, const char* entry);

    WGPURenderPipeline build(WGPUDevice device, WGPUPipelineLayout layout);
};