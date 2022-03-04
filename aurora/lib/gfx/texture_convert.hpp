#pragma once

#include "common.hpp"

#include "../gpu.hpp"

namespace aurora::gfx {
static wgpu::TextureFormat to_wgpu(metaforce::ETexelFormat format) {
  switch (format) {
  case metaforce::ETexelFormat::C8:
  case metaforce::ETexelFormat::R8PC:
    return wgpu::TextureFormat::R8Unorm;
  case metaforce::ETexelFormat::CMPR:
    if (gpu::g_device.HasFeature(wgpu::FeatureName::TextureCompressionBC)) {
      return wgpu::TextureFormat::BC1RGBAUnorm;
    }
    [[fallthrough]];
  default:
    return wgpu::TextureFormat::RGBA8Unorm;
  }
}

ByteBuffer convert_texture(metaforce::ETexelFormat format, uint32_t width, uint32_t height, uint32_t mips,
                           ArrayRef<uint8_t> data);
} // namespace aurora::gfx
