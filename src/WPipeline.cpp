#include <WPipeline.hpp>

WPipelineLayoutBuilder& WPipelineLayoutBuilder::addBindGroupLayout(WGPUBindGroupLayout layout) {
    bindGroupLayouts.push_back(layout);
    return *this;
}

WGPUPipelineLayout WPipelineLayoutBuilder::build(WGPUDevice device) {
    layoutDesc.bindGroupLayoutCount = bindGroupLayouts.size();
    layoutDesc.bindGroupLayouts = bindGroupLayouts.data();

    return wgpuDeviceCreatePipelineLayout(device, &layoutDesc);
}

WRenderPipelineBuilder& WRenderPipelineBuilder::addColorTarget(WGPUTextureFormat format) {
    colorTargetStates.push_back(WGPUColorTargetState{
        .format = format,
        .writeMask = WGPUColorWriteMask_All,
    });
    return *this;
}

WRenderPipelineBuilder& WRenderPipelineBuilder::addVertexBufferLayout(WVertexLayout layout) {
    vertexLayouts.push_back(layout);
    return *this;
}

WRenderPipelineBuilder& WRenderPipelineBuilder::setFragmentState(WGPUShaderModule shader, const char* entry) {
    fragmentState.module = shader;
    fragmentState.entryPoint = entry;
    return *this;
}

WRenderPipelineBuilder& WRenderPipelineBuilder::setVertexState(WGPUShaderModule shader, const char* entry) {
    pipelineDesc.vertex.module = shader;
    pipelineDesc.vertex.entryPoint = entry;
    return *this;
}

WGPURenderPipeline WRenderPipelineBuilder::build(WGPUDevice device, WGPUPipelineLayout layout) {
    std::vector<WGPUVertexBufferLayout> vertexBufferLayouts{};
    vertexBufferLayouts.reserve(vertexLayouts.size());
    for (const WVertexLayout& vertexLayout : vertexLayouts) {
        vertexBufferLayouts.push_back(WGPUVertexBufferLayout{
            .arrayStride = vertexLayout.arrayStride,
            .stepMode = vertexLayout.stepMode,
            .attributeCount = vertexLayout.attributes.size(),
            .attributes = vertexLayout.attributes.data(),
        });
    }

    pipelineDesc.layout = layout;

    fragmentState.targetCount = colorTargetStates.size();
    fragmentState.targets = colorTargetStates.data();
    pipelineDesc.fragment = &fragmentState;

    pipelineDesc.vertex.bufferCount = vertexBufferLayouts.size();
    pipelineDesc.vertex.buffers = vertexBufferLayouts.data();

    pipelineDesc.primitive = WGPUPrimitiveState{.topology = WGPUPrimitiveTopology_TriangleList};
    pipelineDesc.multisample = WGPUMultisampleState{
        .count = 1,
        .mask = 0xFFFFFFFF,
    };

    return wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);
}
