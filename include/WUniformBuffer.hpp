#pragma once

#include <WInclude.hpp>

struct WUniformBuffer {
    WGPUBuffer buffer;
    size_t size;

    operator WGPUBuffer() const;

    WUniformBuffer() {}
    WUniformBuffer(WGPUDevice device, const void *data, size_t size);

    void update(WGPUQueue queue, const void *data);
    void updateWithOffset(WGPUQueue queue, const void *data, uint32_t offset, uint32_t size);
};

struct WDynamicUniformBuffer {
    WGPUBuffer buffer;
    size_t size;
    size_t count;
    size_t offsetAlignment;

    operator WGPUBuffer() const;

    WDynamicUniformBuffer(size_t offsetAlignment = 256);
    WDynamicUniformBuffer(WGPUDevice device, size_t count, size_t size, size_t offsetAlignment = 256);
    WDynamicUniformBuffer(WGPUDevice device, size_t count, size_t size, const std::vector<const void *> &datas, size_t offsetAlignment = 256);

    void update(WGPUQueue queue, size_t index, const void *data);
    void updateAll(WGPUQueue queue, const std::vector<const void *> &datas);
};