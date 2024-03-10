#pragma once

#include <WInclude.hpp>
#include <WVertexLayout.hpp>

struct WPipelineLayoutBuilder {
    std::vector<WGPUBindGroupLayout> bindGroupLayouts;
    WGPUPipelineLayoutDescriptor layoutDesc;

    WPipelineLayoutBuilder& addBindGroupLayout(WGPUBindGroupLayout layout);

    WGPUPipelineLayout build(WGPUDevice device);
};

struct WRenderPipelineBuilder {
    WGPURenderPipelineDescriptor pipelineDesc;
    std::vector<WVertexLayout> vertexLayouts;
    std::vector<WGPUColorTargetState> colorTargetStates;
    WGPUFragmentState fragmentState;
    WGPUDepthStencilState depthStencilState;
    bool depthTest = false;
    bool stencilTest = false;

    WRenderPipelineBuilder& addColorTarget(WGPUTextureFormat format);
    WRenderPipelineBuilder& addVertexBufferLayout(WVertexLayout layout);
    WRenderPipelineBuilder& setFragmentState(WGPUShaderModule shader, const char* entry);
    WRenderPipelineBuilder& setVertexState(WGPUShaderModule shader, const char* entry);
    WRenderPipelineBuilder& setDefaultDepthState(WGPUCompareFunction depthCompare = WGPUCompareFunction_Less,
                                          WGPUTextureFormat format = WGPUTextureFormat_Depth32Float,
                                          bool depthWriteEnabled = true);

    WGPURenderPipeline build(WGPUDevice device, WGPUPipelineLayout layout);
};