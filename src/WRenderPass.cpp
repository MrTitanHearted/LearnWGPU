#include <WRenderPass.hpp>

WRenderPassBuilder& WRenderPassBuilder::addColorAttachment(WGPUTextureView target,
                                                           WGPUColor clearColor,
                                                           WGPULoadOp loadOp,
                                                           WGPUStoreOp storeOp) {
    colorAttachments.push_back(WGPURenderPassColorAttachment{
        .view = target,
        .loadOp = loadOp,
        .storeOp = storeOp,
        .clearValue = clearColor,
    });
    return *this;
}

WRenderPassBuilder& WRenderPassBuilder::setDepthAttachment(WGPUTextureView view,
                                                           float clearValue,
                                                           WGPULoadOp loadOp,
                                                           WGPUStoreOp storeOp) {
    depthStencilAttachment.view = view;
    depthStencilAttachment.depthLoadOp = loadOp;
    depthStencilAttachment.depthClearValue = clearValue;
    depthStencilAttachment.depthStoreOp = storeOp;

    depthTest = true;

    return *this;
}

WGPURenderPassEncoder WRenderPassBuilder::build(WGPUCommandEncoder commandEncoder, const char* label) {
    WGPURenderPassDescriptor desc{
        .label = label,
        .colorAttachmentCount = colorAttachments.size(),
        .colorAttachments = colorAttachments.data(),
    };

    if (!stencilTest) {
        depthStencilAttachment.stencilLoadOp = WGPULoadOp_Undefined;
        depthStencilAttachment.stencilStoreOp = WGPUStoreOp_Undefined;
    }

    if (depthTest) {
        desc.depthStencilAttachment = &depthStencilAttachment;
    }

    return wgpuCommandEncoderBeginRenderPass(commandEncoder, &desc);
}
