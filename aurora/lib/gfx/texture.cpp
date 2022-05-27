#include "common.hpp"

#include "../gpu.hpp"
#include "texture_convert.hpp"

#include <logvisor/logvisor.hpp>
#include <magic_enum.hpp>

namespace aurora::gfx {
static logvisor::Module Log("aurora::gfx");

using gpu::g_device;
using gpu::g_queue;

struct TextureFormatInfo {
  uint8_t blockWidth;
  uint8_t blockHeight;
  uint8_t blockSize;
  bool compressed;
};
static TextureFormatInfo format_info(wgpu::TextureFormat format) {
  switch (format) {
  case wgpu::TextureFormat::R8Unorm:
    return {1, 1, 1, false};
  case wgpu::TextureFormat::R16Sint:
    return {1, 1, 2, false};
  case wgpu::TextureFormat::RGBA8Unorm:
  case wgpu::TextureFormat::R32Float:
    return {1, 1, 4, false};
  case wgpu::TextureFormat::BC1RGBAUnorm:
    return {4, 4, 8, true};
  default:
    Log.report(logvisor::Fatal, FMT_STRING("format_info: unimplemented format {}"), magic_enum::enum_name(format));
    unreachable();
  }
}
static wgpu::Extent3D physical_size(wgpu::Extent3D size, TextureFormatInfo info) {
  const uint32_t width = ((size.width + info.blockWidth - 1) / info.blockWidth) * info.blockWidth;
  const uint32_t height = ((size.height + info.blockHeight - 1) / info.blockHeight) * info.blockHeight;
  return {width, height, size.depthOrArrayLayers};
}

TextureHandle new_static_texture_2d(uint32_t width, uint32_t height, uint32_t mips, GX::TextureFormat format,
                                    ArrayRef<uint8_t> data, zstring_view label) noexcept {
  auto handle = new_dynamic_texture_2d(width, height, mips, format, label);
  const auto& ref = *handle;

  ByteBuffer buffer;
  if (ref.gxFormat != InvalidTextureFormat) {
    buffer = convert_texture(ref.gxFormat, ref.size.width, ref.size.height, ref.mipCount, data);
    if (!buffer.empty()) {
      data = {buffer.data(), buffer.size()};
    }
  }

  uint32_t offset = 0;
  for (uint32_t mip = 0; mip < mips; ++mip) {
    const auto mipSize = wgpu::Extent3D{
        .width = std::max(ref.size.width >> mip, 1u),
        .height = std::max(ref.size.height >> mip, 1u),
        .depthOrArrayLayers = ref.size.depthOrArrayLayers,
    };
    const auto info = format_info(ref.format);
    const auto physicalSize = physical_size(mipSize, info);
    const uint32_t widthBlocks = physicalSize.width / info.blockWidth;
    const uint32_t heightBlocks = physicalSize.height / info.blockHeight;
    const uint32_t bytesPerRow = widthBlocks * info.blockSize;
    const uint32_t dataSize = bytesPerRow * heightBlocks * mipSize.depthOrArrayLayers;
    if (offset + dataSize > data.size()) {
      Log.report(logvisor::Fatal, FMT_STRING("new_static_texture_2d[{}]: expected at least {} bytes, got {}"), label,
                 offset + dataSize, data.size());
      unreachable();
    }
    auto dstView = wgpu::ImageCopyTexture{
        .texture = ref.texture,
        .mipLevel = mip,
    };
    const auto range = push_texture_data(data.data() + offset, dataSize, bytesPerRow, heightBlocks);
    const auto dataLayout = wgpu::TextureDataLayout{
        .offset = range.offset,
        .bytesPerRow = bytesPerRow,
        .rowsPerImage = heightBlocks,
    };
    g_textureUploads.emplace_back(dataLayout, std::move(dstView), physicalSize);
    offset += dataSize;
  }
  if (data.size() != UINT32_MAX && offset < data.size()) {
    Log.report(logvisor::Warning, FMT_STRING("new_static_texture_2d[{}]: texture used {} bytes, but given {} bytes"),
               label, offset, data.size());
  }
  return handle;
}

TextureHandle new_dynamic_texture_2d(uint32_t width, uint32_t height, uint32_t mips, GX::TextureFormat format,
                                     zstring_view label) noexcept {
  const auto wgpuFormat = to_wgpu(format);
  const auto size = wgpu::Extent3D{
      .width = width,
      .height = height,
  };
  const auto textureDescriptor = wgpu::TextureDescriptor{
      .label = label.c_str(),
      .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst,
      .size = size,
      .format = wgpuFormat,
      .mipLevelCount = mips,
  };
  const auto viewLabel = fmt::format(FMT_STRING("{} view"), label);
  const auto textureViewDescriptor = wgpu::TextureViewDescriptor{
      .label = viewLabel.c_str(),
      .format = wgpuFormat,
      .dimension = wgpu::TextureViewDimension::e2D,
      .mipLevelCount = mips,
  };
  auto texture = g_device.CreateTexture(&textureDescriptor);
  auto textureView = texture.CreateView(&textureViewDescriptor);
  return std::make_shared<TextureRef>(std::move(texture), std::move(textureView), size, wgpuFormat, mips, format,
                                      false);
}

TextureHandle new_render_texture(uint32_t width, uint32_t height, GX::TextureFormat fmt, zstring_view label) noexcept {
  const auto wgpuFormat = gpu::g_graphicsConfig.colorFormat;
  const auto size = wgpu::Extent3D{
      .width = width,
      .height = height,
  };
  const auto textureDescriptor = wgpu::TextureDescriptor{
      .label = label.c_str(),
      .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst,
      .size = size,
      .format = wgpuFormat,
      .mipLevelCount = 1,
  };
  const auto viewLabel = fmt::format(FMT_STRING("{} view"), label);
  const auto textureViewDescriptor = wgpu::TextureViewDescriptor{
      .label = viewLabel.c_str(),
      .format = wgpuFormat,
      .dimension = wgpu::TextureViewDimension::e2D,
      .mipLevelCount = 1,
  };
  auto texture = g_device.CreateTexture(&textureDescriptor);
  auto textureView = texture.CreateView(&textureViewDescriptor);
  return std::make_shared<TextureRef>(std::move(texture), std::move(textureView), size, wgpuFormat, 1, fmt, true);
}

void write_texture(const TextureRef& ref, ArrayRef<uint8_t> data) noexcept {
  ByteBuffer buffer;
  if (ref.gxFormat != InvalidTextureFormat) {
    buffer = convert_texture(ref.gxFormat, ref.size.width, ref.size.height, ref.mipCount, data);
    if (!buffer.empty()) {
      data = {buffer.data(), buffer.size()};
    }
  }

  uint32_t offset = 0;
  for (uint32_t mip = 0; mip < ref.mipCount; ++mip) {
    const auto mipSize = wgpu::Extent3D{
        .width = std::max(ref.size.width >> mip, 1u),
        .height = std::max(ref.size.height >> mip, 1u),
        .depthOrArrayLayers = ref.size.depthOrArrayLayers,
    };
    const auto info = format_info(ref.format);
    const auto physicalSize = physical_size(mipSize, info);
    const uint32_t widthBlocks = physicalSize.width / info.blockWidth;
    const uint32_t heightBlocks = physicalSize.height / info.blockHeight;
    const uint32_t bytesPerRow = widthBlocks * info.blockSize;
    const uint32_t dataSize = bytesPerRow * heightBlocks * mipSize.depthOrArrayLayers;
    if (offset + dataSize > data.size()) {
      Log.report(logvisor::Fatal, FMT_STRING("write_texture: expected at least {} bytes, got {}"), offset + dataSize,
                 data.size());
      unreachable();
    }
//    auto dstView = wgpu::ImageCopyTexture{
//        .texture = ref.texture,
//        .mipLevel = mip,
//    };
//    const auto range = push_texture_data(data.data() + offset, dataSize, bytesPerRow, heightBlocks);
//    const auto dataLayout = wgpu::TextureDataLayout{
//        .offset = range.offset,
//        .bytesPerRow = bytesPerRow,
//        .rowsPerImage = heightBlocks,
//    };
//    g_textureUploads.emplace_back(dataLayout, std::move(dstView), physicalSize);
    const auto dstView = wgpu::ImageCopyTexture{
        .texture = ref.texture,
        .mipLevel = mip,
    };
    const auto dataLayout = wgpu::TextureDataLayout{
        .bytesPerRow = bytesPerRow,
        .rowsPerImage = heightBlocks,
    };
    g_queue.WriteTexture(&dstView, data.data() + offset, dataSize, &dataLayout, &physicalSize);
    offset += dataSize;
  }
  if (data.size() != UINT32_MAX && offset < data.size()) {
    Log.report(logvisor::Warning, FMT_STRING("write_texture: texture used {} bytes, but given {} bytes"), offset,
               data.size());
  }
}
} // namespace aurora::gfx
