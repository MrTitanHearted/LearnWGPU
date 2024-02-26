#pragma once

#include <WInclude.hpp>

struct WSamplerBuilder {
    WGPUSamplerDescriptor desc{
        .addressModeU = WGPUAddressMode_Repeat,
        .addressModeV = WGPUAddressMode_Repeat,
        .addressModeW = WGPUAddressMode_Repeat,
        .magFilter = WGPUFilterMode_Linear,
        .minFilter = WGPUFilterMode_Linear,
        .mipmapFilter = WGPUMipmapFilterMode_Linear,
        .lodMinClamp = 0.0f,
        .lodMaxClamp = 1.0f,
        .compare = WGPUCompareFunction_Undefined,
        .maxAnisotropy = 1,
    };

    WSamplerBuilder& setAddressMode(WGPUAddressMode mode);
    WSamplerBuilder& setAddressModeV(WGPUAddressMode mode);
    WSamplerBuilder& setAddressModeU(WGPUAddressMode mode);
    WSamplerBuilder& setAddressModeW(WGPUAddressMode mode);
    WSamplerBuilder& setMinMagFilter(WGPUFilterMode mode);
    WSamplerBuilder& setMagFilter(WGPUFilterMode mode);
    WSamplerBuilder& setMinFilter(WGPUFilterMode mode);
    WSamplerBuilder& setMipmapFilter(WGPUMipmapFilterMode mode);
    WSamplerBuilder& setLodMinClamp(float min);
    WSamplerBuilder& setLodMaxClamp(float max);
    WSamplerBuilder& setCompareFunction(WGPUCompareFunction compare);
    WSamplerBuilder& setMaxAnisotropy(uint16_t max);

    WGPUSampler build(WGPUDevice device);
};