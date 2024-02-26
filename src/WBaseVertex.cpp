#include <WBaseVertex.hpp>

std::vector<WGPUVertexAttribute> WBaseVertex::Attributes = std::vector<WGPUVertexAttribute>{};
bool WBaseVertex::Initialized = false;
bool WBaseVertex::IsStrideSet = false;
size_t WBaseVertex::Stride = 0;
WGPUVertexStepMode WBaseVertex::StepMode = WGPUVertexStepMode_Vertex;

void WBaseVertex::addVertexAttribute(WGPUVertexFormat format, size_t offset, uint32_t location) {
    if (!Initialized) {
        Attributes.push_back(WGPUVertexAttribute{
            .format = format,
            .offset = offset,
            .shaderLocation = location,
        });
    }
}

void WBaseVertex::setVertexStride(size_t stride) {
    if (!Initialized) {
        Stride = stride;
        IsStrideSet = true;
    }
}

void WBaseVertex::setVertexStepMode(WGPUVertexStepMode stepMode) {
    if (!Initialized) {
        StepMode = stepMode;
    }
}

WGPUVertexBufferLayout WBaseVertex::desc() {
    if (!Initialized) {
        Initialized = true;
    }
    if (!IsStrideSet) {
        throw std::runtime_error("[WError]::[ERROR]: Before calling 'Vertex::desc()', you should set the stride size by calling: 'Vertex::setVertexStride(sizeof(Vertex))'!");
    }
    return WGPUVertexBufferLayout{
        .arrayStride = Stride,
        .stepMode = StepMode,
        .attributeCount = Attributes.size(),
        .attributes = Attributes.data(),
    };
}
