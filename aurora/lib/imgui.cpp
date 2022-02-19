#include "imgui.hpp"

#include "gpu.hpp"

#include <aurora/imgui.hpp>
#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_wgpu.h>
#include <dawn/webgpu_cpp.h>

namespace aurora::imgui {
using gpu::g_device;
using gpu::g_queue;

void create_context() noexcept {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.IniFilename = nullptr;
}

void initialize(SDL_Window* window) noexcept {
  // this just passes through to ImGui_ImplSDL2_Init (private)
  // may need to change in the future
  ImGui_ImplSDL2_InitForMetal(window);
  ImGui_ImplWGPU_Init(g_device.Get(), 1, static_cast<WGPUTextureFormat>(gpu::g_graphicsConfig.colorFormat));
}

void shutdown() noexcept {
  ImGui_ImplWGPU_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

void process_event(const SDL_Event& event) noexcept { ImGui_ImplSDL2_ProcessEvent(&event); }

void new_frame() noexcept {
  ImGui_ImplWGPU_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
}

void render(const wgpu::RenderPassEncoder& pass) noexcept {
  ImGui::Render();
  ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), pass.Get());
}

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
  auto texture = g_device.CreateTexture(&textureDescriptor);
  auto textureView = texture.CreateView(&textureViewDescriptor);
  {
    const auto dstView = wgpu::ImageCopyTexture{
        .texture = texture,
    };
    const auto dataLayout = wgpu::TextureDataLayout{
        .bytesPerRow = 4 * width,
        .rowsPerImage = height,
    };
    g_queue.WriteTexture(&dstView, data.data(), data.size(), &dataLayout, &size);
  }
  texture.Release(); // leak some memory!
  return textureView.Release();
}
} // namespace aurora::imgui
