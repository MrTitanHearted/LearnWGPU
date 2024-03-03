#include <WTexture.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

WTexture::operator WGPUTexture() const {
    return texture;
}

WTexture::operator WGPUTextureView() const {
    return view;
}

WTexture WTexture::GetDepthTexture(WGPUDevice device, WGPUExtent3D size, WGPUTextureFormat format) {
    return WTextureBuilder{}
        .setFormat(format)
        .setTextureUsage(WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding)
        .build(device, size, nullptr, 0);
}

WTextureBuilder& WTextureBuilder::addTextureUsage(WGPUTextureUsageFlags usage) {
    desc.usage |= usage;
    return *this;
}

WTextureBuilder& WTextureBuilder::setTextureUsage(WGPUTextureUsageFlags usage) {
    desc.usage = usage;
    return *this;
}

WTextureBuilder& WTextureBuilder::setDimension(WGPUTextureDimension dimension) {
    desc.dimension = dimension;
    return *this;
}

WTextureBuilder& WTextureBuilder::setFormat(WGPUTextureFormat format) {
    desc.format = format;
    return *this;
}

WTextureBuilder& WTextureBuilder::setMipLevelCount(uint32_t count) {
    desc.mipLevelCount = count;
    return *this;
}

WTextureBuilder& WTextureBuilder::setSampleCount(uint32_t count) {
    desc.sampleCount = count;
    return *this;
}

WGPUTexture WTextureBuilder::buildWGPU(WGPUDevice device, WGPUExtent3D size, const unsigned char* data, size_t stride) {
    desc.size = size;
    WGPUTexture texture = wgpuDeviceCreateTexture(device, &desc);

    if (data != nullptr) {
        WGPUImageCopyTexture destination{
            .texture = texture,
            .mipLevel = 0,
            .origin = WGPUOrigin3D{0, 0, 0},
            .aspect = WGPUTextureAspect_All,
        };

        WGPUTextureDataLayout dataLayout{
            .offset = 0,
            .bytesPerRow = 4 * size.width,
            .rowsPerImage = size.height * size.depthOrArrayLayers,
        };

        WGPUQueue queue = wgpuDeviceGetQueue(device);
        wgpuQueueWriteTexture(
            queue,
            &destination,
            data,
            stride * size.width * size.height * size.depthOrArrayLayers,
            &dataLayout,
            &size);
    }

    return texture;
}

WGPUTexture WTextureBuilder::fromFileAsRgba8WGPU(WGPUDevice device, std::string path) {
    stbi_set_flip_vertically_on_load(1);
    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (data == nullptr) {
        throw std::runtime_error(fmt::format("[WEngine]::[ERROR]: Failed to load texture from: {}", path));
    }

    WGPUExtent3D size{
        .width = (uint32_t)width,
        .height = (uint32_t)height,
        .depthOrArrayLayers = 1,
    };

    WTextureBuilder builder{};

    builder.desc.format = WGPUTextureFormat_RGBA8Unorm;

    WGPUTexture texture = builder.buildWGPU(device, size, data, 4);
    stbi_image_free(data);
    stbi_set_flip_vertically_on_load(0);

    return texture;
}

WTexture WTextureBuilder::build(WGPUDevice device, WGPUExtent3D size, const unsigned char* data, size_t stride) {
    WGPUTexture texture = buildWGPU(device, size, data, stride);
    WGPUTextureView view = wgpuTextureCreateView(texture, nullptr);

    return WTexture{
        .desc = desc,
        .texture = texture,
        .view = view,
    };
}

WTexture WTextureBuilder::fromFileAsRgba8(WGPUDevice device, std::string path) {
    stbi_set_flip_vertically_on_load(1);
    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (data == nullptr) {
        throw std::runtime_error(fmt::format("[WEngine]::[ERROR]: Failed to load texture from: {}", path));
    }

    WGPUExtent3D size{
        .width = (uint32_t)width,
        .height = (uint32_t)height,
        .depthOrArrayLayers = 1,
    };

    WTextureBuilder builder{};
    builder.desc.format = WGPUTextureFormat_RGBA8Unorm;

    WTexture texture = builder.build(device, size, data, 4);
    stbi_image_free(data);
    stbi_set_flip_vertically_on_load(0);
    return texture;
}
