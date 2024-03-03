#pragma once

#include <WInclude.hpp>

struct WTexture {
    WGPUTextureDescriptor desc;
    WGPUTexture texture;
    WGPUTextureView view;

    operator WGPUTexture() const;
    operator WGPUTextureView() const;

    static WTexture GetDepthTexture(WGPUDevice device, WGPUExtent3D size, WGPUTextureFormat format = WGPUTextureFormat_Depth32Float);
};

struct WTextureBuilder {
    WGPUTextureDescriptor desc{
        .usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst,
        .dimension = WGPUTextureDimension_2D,
        .format = WGPUTextureFormat_RGBA8Unorm,
        .mipLevelCount = 1,
        .sampleCount = 1,
    };

    WTextureBuilder& addTextureUsage(WGPUTextureUsageFlags usage);
    WTextureBuilder& setTextureUsage(WGPUTextureUsageFlags usage);
    WTextureBuilder& setDimension(WGPUTextureDimension dimension);
    WTextureBuilder& setFormat(WGPUTextureFormat format);
    WTextureBuilder& setMipLevelCount(uint32_t count);
    WTextureBuilder& setSampleCount(uint32_t count);

    WGPUTexture buildWGPU(WGPUDevice device, WGPUExtent3D size, const unsigned char* data, size_t stride);
    static WGPUTexture fromFileAsRgba8WGPU(WGPUDevice device, std::string path);

    WTexture build(WGPUDevice device, WGPUExtent3D size, const unsigned char* data, size_t stride);
    static WTexture fromFileAsRgba8(WGPUDevice device, std::string path);
};