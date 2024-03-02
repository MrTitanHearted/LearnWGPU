#pragma once

#include <WInclude.hpp>

struct WVertexLayout{
    std::vector<WGPUVertexAttribute> attributes;
    WGPUVertexStepMode stepMode;
    size_t arrayStride;

    WVertexLayout() {}
    WVertexLayout(size_t arrayStride, WGPUVertexStepMode stepMode = WGPUVertexStepMode_Vertex);

    WVertexLayout& setStepMode(WGPUVertexStepMode mode);
    WVertexLayout& setArrayStride(size_t stride);
    WVertexLayout& setAttributes(std::vector<WGPUVertexAttribute> attributes);
    WVertexLayout& addAttributes(WGPUVertexFormat format, size_t offset, uint32_t shaderLocation);
};