#include <WVertexLayout.hpp>

WVertexLayout::WVertexLayout(size_t arrayStride, WGPUVertexStepMode stepMode) {
    this->arrayStride = arrayStride;
    this->stepMode = stepMode;
}

WVertexLayout& WVertexLayout::setStepMode(WGPUVertexStepMode mode) {
    stepMode = mode;
    return *this;
}

WVertexLayout& WVertexLayout::setArrayStride(size_t stride) {
    arrayStride = stride;
    return *this;
}

WVertexLayout& WVertexLayout::setAttributes(std::vector<WGPUVertexAttribute> attributes) {
    this->attributes = attributes;
    return *this;
}

WVertexLayout& WVertexLayout::addAttributes(WGPUVertexFormat format, size_t offset, uint32_t shaderLocation) {
    attributes.push_back(WGPUVertexAttribute {
        .format = format,
        .offset = offset,
        .shaderLocation = shaderLocation,
    });
    return *this;
}
