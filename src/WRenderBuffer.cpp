#include <WRenderBuffer.hpp>
#include <string.h>

WRenderBuffer::WRenderBuffer(WGPUDevice device, void* vertices, size_t verticesCount, size_t verticesSize, uint32_t* indices, size_t indicesCount) {
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

WRenderBufferBuilder& WRenderBufferBuilder::setIndices(std::vector<uint32_t> indices) {
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

WGPUBuffer wgpuDeviceCreateBufferInit(WGPUDevice device, WGPUBufferDescriptor desc, void* data) {
    if (desc.size == 0 || data == nullptr) {
        return wgpuDeviceCreateBuffer(device, &desc);
    }

    desc.mappedAtCreation = true;

    WGPUBuffer buffer = wgpuDeviceCreateBuffer(device, &desc);

    void* mapped = wgpuBufferGetMappedRange(buffer, 0, desc.size);
    memcpy(mapped, data, desc.size);
    wgpuBufferUnmap(buffer);

    return buffer;
}

WUniformBuffer::WUniformBuffer(WGPUDevice device, void* data, size_t size) {
    this->size = size;
    WGPUBufferDescriptor desc{
        .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
        .size = size,
        .mappedAtCreation = true,
    };
    buffer = wgpuDeviceCreateBufferInit(device, desc, data);
}

void WUniformBuffer::update(WGPUQueue queue, void* data) {
    wgpuQueueWriteBuffer(queue, buffer, 0, data, size);
}
