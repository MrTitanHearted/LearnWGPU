#include <WRenderPass.hpp>

WRenderPassBuilder& WRenderPassBuilder::addColorAttachment(WGPUTextureView target, WGPUColor clearColor, WGPULoadOp loadOp, WGPUStoreOp storeOp) {
    colorAttachments.push_back(WGPURenderPassColorAttachment{
        .view = target,
        .loadOp = loadOp,
        .storeOp = storeOp,
        .clearValue = clearColor,
    });
    return *this;
}

WGPURenderPassEncoder WRenderPassBuilder::build(WGPUCommandEncoder commandEncoder, const char* label) {
    WGPURenderPassDescriptor desc{
        .label = label,
        .colorAttachmentCount = colorAttachments.size(),
        .colorAttachments = colorAttachments.data(),
    };

    return wgpuCommandEncoderBeginRenderPass(commandEncoder, &desc);
}
