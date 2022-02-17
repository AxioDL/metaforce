#include <aurora/imgui.hpp>

#include <dawn/webgpu_cpp.h>

namespace aurora {
extern wgpu::Device g_Device;
extern wgpu::Queue g_Queue;
} // namespace aurora

namespace aurora::imgui {
ImTextureID add_texture(uint32_t width, uint32_t height, ArrayRef<uint8_t> data) noexcept {
  const auto size = wgpu::Extent3D{
      .width = width,
      .height = height,
  };
  const auto textureDescriptor = wgpu::TextureDescriptor{
      .label = "imgui texture",
      .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst,
      .size = size,
      .format = wgpu::TextureFormat::RGBA8Unorm,
  };
  const auto textureViewDescriptor = wgpu::TextureViewDescriptor{
      .label = "imgui texture view",
      .format = wgpu::TextureFormat::RGBA8Unorm,
      .dimension = wgpu::TextureViewDimension::e2D,
      .mipLevelCount = 1,
      .arrayLayerCount = 1,
  };
  auto texture = g_Device.CreateTexture(&textureDescriptor);
  auto textureView = texture.CreateView(&textureViewDescriptor);
  {
    const auto dstView = wgpu::ImageCopyTexture{
        .texture = texture,
    };
    const auto dataLayout = wgpu::TextureDataLayout{
        .bytesPerRow = 4 * width,
        .rowsPerImage = height,
    };
    g_Queue.WriteTexture(&dstView, data.data(), data.size(), &dataLayout, &size);
  }
  texture.Release(); // leak some memory!
  return textureView.Release();
}
} // namespace aurora::imgui
