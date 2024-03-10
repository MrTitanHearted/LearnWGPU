#pragma once

#include <WInclude.hpp>

struct WUniformBuffer {
    WGPUBuffer buffer;
    size_t size;

    operator WGPUBuffer() const;

    WUniformBuffer() {}
    WUniformBuffer(WGPUDevice device, const void *data, size_t size);

    void update(WGPUQueue queue, const void *data);
    void update(WGPUQueue queue, const void *data, uint32_t offset, uint32_t size);
};