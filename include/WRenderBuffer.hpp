#pragma once

#include <WInclude.hpp>

WGPUBuffer wgpuDeviceCreateBufferInit(WGPUDevice device, WGPUBufferDescriptor desc, void *data);

struct WRenderBuffer {
    WGPUBuffer vertex;
    WGPUBuffer index;
    size_t verticesSize;
    size_t indicesSize;
    size_t verticesCount;
    size_t indicesCount;

    WRenderBuffer() {}
    WRenderBuffer(WGPUDevice device, void *vertices, size_t verticesCount, size_t verticesSize, uint32_t *indices, size_t indicesCount);

    void render(WGPURenderPassEncoder encoder);
};

struct WRenderBufferBuilder {
    void *vertices;
    uint32_t *indices;
    size_t verticesSize;
    size_t verticesCount;
    size_t indicesCount;

    template <class Vertex>
    WRenderBufferBuilder &setVertices(std::vector<Vertex> vertices) {
        this->verticesCount = vertices.size();
        this->verticesSize = sizeof(Vertex) * this->verticesCount;
        this->vertices = vertices.data();
        return *this;
    }
    WRenderBufferBuilder &setIndices(std::vector<uint32_t> indices);

    WRenderBuffer build(WGPUDevice device);
};

struct WUniformBuffer {
    WGPUBuffer buffer;
    size_t size;

    WUniformBuffer() {}
    WUniformBuffer(WGPUDevice device, void *data, size_t size);

    void update(WGPUQueue queue, void *data);
};