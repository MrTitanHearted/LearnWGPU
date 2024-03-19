#include <WRenderBuffer.hpp>
#include <string.h>

WRenderBuffer::WRenderBuffer(WGPUDevice device, const void* vertices, size_t verticesCount, size_t verticesSize, const uint32_t* indices, size_t indicesCount) {
    this->verticesCount = verticesCount;
    this->verticesSize = verticesSize;
    this->indicesCount = indicesCount;
    this->indicesSize = sizeof(uint32_t) * indicesCount;

    vertex = wgpuDeviceCreateBufferInit(
        device,
        WGPUBufferDescriptor{
            .usage = WGPUBufferUsage_Vertex,
            .size = verticesSize,
        },
        vertices);
    index = wgpuDeviceCreateBufferInit(
        device,
        WGPUBufferDescriptor{
            .usage = WGPUBufferUsage_Index,
            .size = this->indicesSize,
        },
        indices);
}

void WRenderBuffer::render(WGPURenderPassEncoder encoder) {
    wgpuRenderPassEncoderSetVertexBuffer(encoder, 0, vertex, 0, verticesSize);
    wgpuRenderPassEncoderSetIndexBuffer(encoder, index, WGPUIndexFormat_Uint32, 0, indicesSize);
    wgpuRenderPassEncoderDrawIndexed(encoder, indicesCount, 1, 0, 0, 0);
}

void WRenderBuffer::render(WGPURenderBundleEncoder encoder) {
    wgpuRenderBundleEncoderSetVertexBuffer(encoder, 0, vertex, 0, verticesSize);
    wgpuRenderBundleEncoderSetIndexBuffer(encoder, index, WGPUIndexFormat_Uint32, 0, indicesSize);
    wgpuRenderBundleEncoderDrawIndexed(encoder, indicesCount, 1, 0, 0, 0);
}

WRenderBufferBuilder& WRenderBufferBuilder::setIndices(const std::vector<uint32_t>& indices) {
    this->indicesCount = indices.size();
    this->indices = indices.data();
    return *this;
}

WRenderBuffer WRenderBufferBuilder::build(WGPUDevice device) {
    return WRenderBuffer{
        device,
        vertices,
        verticesCount,
        verticesSize,
        indices,
        indicesCount,
    };
}

WGPUBuffer wgpuDeviceCreateBufferInit(WGPUDevice device, WGPUBufferDescriptor desc, const void* data) {
    if (desc.size == 0 || data == nullptr) {
        return wgpuDeviceCreateBuffer(device, &desc);
    }

    desc.usage |= WGPUBufferUsage_CopyDst;

    WGPUQueue queue = wgpuDeviceGetQueue(device);
    WGPUBuffer buffer = wgpuDeviceCreateBuffer(device, &desc);
    wgpuQueueWriteBuffer(queue, buffer, 0, data, desc.size);
    return buffer;
}