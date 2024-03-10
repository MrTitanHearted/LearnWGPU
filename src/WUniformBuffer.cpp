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
}

void WUniformBuffer::updateWithOffset(WGPUQueue queue, const void* data, uint32_t offset, uint32_t size) {
    wgpuQueueWriteBuffer(queue, buffer, offset, data, size);
}

WDynamicUniformBuffer::operator WGPUBuffer() const {
    return buffer;
}

WDynamicUniformBuffer::WDynamicUniformBuffer(size_t offsetAlignment) {
    this->offsetAlignment = offsetAlignment;
}

WDynamicUniformBuffer::WDynamicUniformBuffer(WGPUDevice device, size_t count, size_t size, size_t offsetAlignment) {
    this->offsetAlignment = offsetAlignment;
    this->count = count;
    this->size = size;
    WGPUBufferDescriptor desc{
        .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
        .size = this->count * this->offsetAlignment,
    };
    buffer = wgpuDeviceCreateBuffer(device, &desc);
}

WDynamicUniformBuffer::WDynamicUniformBuffer(WGPUDevice device, size_t count, size_t size,
                                             const std::vector<const void*>& datas, size_t offsetAlignment) {
    this->offsetAlignment = offsetAlignment;
    this->count = count;
    this->size = size;
    WGPUBufferDescriptor desc{
        .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
        .size = this->count * this->offsetAlignment,
    };
    buffer = wgpuDeviceCreateBuffer(device, &desc);
    WGPUQueue queue = wgpuDeviceGetQueue(device);
    for (size_t i = 0; i < count; i++) {
        wgpuQueueWriteBuffer(queue, buffer, i * this->offsetAlignment, datas.data()[i], this->size);
    }
}

void WDynamicUniformBuffer::update(WGPUQueue queue, size_t index, const void* data) {
    if (index >= count) {
        throw std::runtime_error("[WEngine]::[ERROR]: you cannot have index bigger than count in dynamic buffer!");
    }

    wgpuQueueWriteBuffer(queue, buffer, index * offsetAlignment, data, size);
}

void WDynamicUniformBuffer::updateAll(WGPUQueue queue, const std::vector<const void*>& datas) {
    if (datas.size() != count) {
        throw std::runtime_error("[WEngine]::[ERROR]: you have to have the same number of data to update all!");
    }

    for (size_t i = 0; i < count; i++) {
        wgpuQueueWriteBuffer(queue, buffer, i * offsetAlignment, datas.data()[i], size);
    }
}
