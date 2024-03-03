#pragma once

#include <WInclude.hpp>

WGPUBuffer wgpuDeviceCreateBufferInit(WGPUDevice device, WGPUBufferDescriptor desc, const void *data);

struct WRenderBuffer {
    WGPUBuffer vertex;
    WGPUBuffer index;
    size_t verticesSize;
    size_t indicesSize;
    size_t verticesCount;
    size_t indicesCount;

    WRenderBuffer() {}
    WRenderBuffer(WGPUDevice device, const void *vertices, size_t verticesCount, size_t verticesSize, const uint32_t *indices, size_t indicesCount);

    void render(WGPURenderPassEncoder encoder);
};

struct WRenderBufferBuilder {
    const void *vertices;
    const uint32_t *indices;
    size_t verticesSize;
    size_t verticesCount;
    size_t indicesCount;

    template <class Vertex>
    WRenderBufferBuilder &setVertices(const std::vector<Vertex> &vertices) {
        this->verticesCount = vertices.size();
        this->verticesSize = sizeof(Vertex) * this->verticesCount;
        this->vertices = vertices.data();
        return *this;
    }
    WRenderBufferBuilder &setIndices(const std::vector<uint32_t> &indices);

    WRenderBuffer build(WGPUDevice device);
};
