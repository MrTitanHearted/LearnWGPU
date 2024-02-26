#pragma once

#include <WInclude.hpp>

struct WTextureBuilder {
    WGPUTextureDescriptor desc{
        .usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst,
        .dimension = WGPUTextureDimension_2D,
        .format = WGPUTextureFormat_RGBA8Unorm,
        .mipLevelCount = 1,
        .sampleCount = 1,
    };

    WTextureBuilder& addTextureUsage(WGPUTextureUsage usage);
    WTextureBuilder& setTextureUsage(WGPUTextureUsage usage);
    WTextureBuilder& setDimension(WGPUTextureDimension dimension);
    WTextureBuilder& setFormat(WGPUTextureFormat format);
    WTextureBuilder& setMipLevelCount(uint32_t count);
    WTextureBuilder& setSampleCount(uint32_t count);

    WGPUTexture build(WGPUDevice device, WGPUExtent3D size, const unsigned char* data, size_t stride);
    WGPUTexture fromFileAsRgba8(WGPUDevice device, std::string path);
};