#pragma once

#include <WInclude.hpp>

struct WRenderPassBuilder {
    std::vector<WGPURenderPassColorAttachment> colorAttachments;

    WRenderPassBuilder& addColorAttachment(WGPUTextureView target,
                                           WGPUColor clearColor = WGPUColor{0.0, 0.0, 0.0, 1.0},
                                           WGPULoadOp loadOp = WGPULoadOp_Clear,
                                           WGPUStoreOp storeOp = WGPUStoreOp_Store);

    WGPURenderPassEncoder build(WGPUCommandEncoder commandEncoder, const char* label = "Render Pass Endoder");
};