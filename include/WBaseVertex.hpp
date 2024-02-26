#pragma once

#include <WInclude.hpp>

class WBaseVertex {
   public:
    static void addVertexAttribute(WGPUVertexFormat format, size_t offset, uint32_t location);
    static void setVertexStride(size_t stride);
    static void setVertexStepMode(WGPUVertexStepMode stepMode);
    static WGPUVertexBufferLayout desc();

   private:
    static bool Initialized;
    static bool IsStrideSet;
    static std::vector<WGPUVertexAttribute> Attributes;
    static size_t Stride;
    static WGPUVertexStepMode StepMode;
};

// class WVertexBase {
//    public:
//     static void desc();
//    private:
//     static std::vector<std::vector<WGPUVertexAttribute>> attributesSet;
// };