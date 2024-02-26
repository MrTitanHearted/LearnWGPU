#include <WSampler.hpp>

WSamplerBuilder& WSamplerBuilder::setAddressMode(WGPUAddressMode mode) {
    desc.addressModeU = mode;
    desc.addressModeV = mode;
    desc.addressModeW = mode;
    return *this;
}

WSamplerBuilder& WSamplerBuilder::setAddressModeV(WGPUAddressMode mode) {
    desc.addressModeV = mode;
    return *this;
}

WSamplerBuilder& WSamplerBuilder::setAddressModeU(WGPUAddressMode mode) {
    desc.addressModeU = mode;
    return *this;
}

WSamplerBuilder& WSamplerBuilder::setAddressModeW(WGPUAddressMode mode) {
    desc.addressModeW = mode;
    return *this;
}

WSamplerBuilder& WSamplerBuilder::setMinMagFilter(WGPUFilterMode mode) {
    desc.magFilter = mode;
    desc.minFilter = mode;
    return *this;
}

WSamplerBuilder& WSamplerBuilder::setMagFilter(WGPUFilterMode mode) {
    desc.magFilter = mode;
    return *this;
}

WSamplerBuilder& WSamplerBuilder::setMinFilter(WGPUFilterMode mode) {
    desc.minFilter = mode;
    return *this;
}

WSamplerBuilder& WSamplerBuilder::setMipmapFilter(WGPUMipmapFilterMode mode) {
    desc.mipmapFilter = mode;
    return *this;
}

WSamplerBuilder& WSamplerBuilder::setLodMinClamp(float min) {
    desc.lodMinClamp = min;
    return *this;
}

WSamplerBuilder& WSamplerBuilder::setLodMaxClamp(float max) {
    desc.lodMaxClamp = max;
    return *this;
}

WSamplerBuilder& WSamplerBuilder::setCompareFunction(WGPUCompareFunction compare) {
    desc.compare = compare;
    return *this;
}

WSamplerBuilder& WSamplerBuilder::setMaxAnisotropy(uint16_t max) {
    desc.maxAnisotropy = max;
    return *this;
}

WGPUSampler WSamplerBuilder::build(WGPUDevice device) {
    return wgpuDeviceCreateSampler(device, &desc);
}