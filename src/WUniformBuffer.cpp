#include <WUniformBuffer.hpp>

WUniformBuffer::operator WGPUBuffer() const {
    return buffer;
}

WUniformBuffer::WUniformBuffer(WGPUDevice device, const void* data, size_t size) {
    this->size = size;
    WGPUBufferDescriptor desc{
        .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
        .size = size,
    };
    buffer = wgpuDeviceCreateBuffer(device, &desc);
    WGPUQueue queue = wgpuDeviceGetQueue(device);
    update(queue, data);
}

void WUniformBuffer::update(WGPUQueue queue, const void* data) {
    wgpuQueueWriteBuffer(queue, buffer, 0, data, size);
    // update(queue, data, 0, size);
}

void WUniformBuffer::update(WGPUQueue queue, const void* data, uint32_t offset, uint32_t size) {
    wgpuQueueWriteBuffer(queue, buffer, offset, data, size);
}
