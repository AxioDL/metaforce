#include "imgui.hpp"

#include "gpu.hpp"

#include "Runtime/ConsoleVariables/FileStoreManager.hpp"
#include <SDL.h>
#include <aurora/aurora.hpp>
#include <aurora/imgui.hpp>
#include <dawn/webgpu_cpp.h>

#include "../extern/imgui/backends/imgui_impl_sdl.cpp"
#include "../extern/imgui/backends/imgui_impl_sdlrenderer.cpp"
#include "../extern/imgui/backends/imgui_impl_wgpu.cpp"

namespace aurora::imgui {
using gpu::g_device;
using gpu::g_queue;

static float g_scale;

static std::string g_imguiSettings{};
static std::string g_imguiLog{};
void create_context() noexcept {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  g_imguiSettings = std::string(metaforce::FileStoreManager::instance()->getStoreRoot()) + "/imgui.ini";
  g_imguiLog = std::string(metaforce::FileStoreManager::instance()->getStoreRoot()) + "/imgui.log";
  io.IniFilename = g_imguiSettings.c_str();
  io.LogFilename = g_imguiLog.c_str();
}

static bool g_useSdlRenderer = false;
void initialize(SDL_Window* window, SDL_Renderer* renderer) noexcept {
  ImGui_ImplSDL2_Init(window, renderer);
#ifdef __APPLE__
  // Disable MouseCanUseGlobalState for scaling purposes
  ImGui_ImplSDL2_GetBackendData()->MouseCanUseGlobalState = false;
#endif
  g_useSdlRenderer = renderer != nullptr;
  if (g_useSdlRenderer) {
    ImGui_ImplSDLRenderer_Init(renderer);
  } else {
    ImGui_ImplWGPU_Init(g_device.Get(), 1, static_cast<WGPUTextureFormat>(gpu::g_graphicsConfig.colorFormat));
  }
}

void shutdown() noexcept {
  if (g_useSdlRenderer) {
    ImGui_ImplSDLRenderer_Shutdown();
  } else {
    ImGui_ImplWGPU_Shutdown();
  }
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

void process_event(const SDL_Event& event) noexcept {
#ifdef __APPLE__
  if (event.type == SDL_MOUSEMOTION) {
    auto& io = ImGui::GetIO();
    // Scale up mouse coordinates
    io.AddMousePosEvent(static_cast<float>(event.motion.x) * g_scale, static_cast<float>(event.motion.y) * g_scale);
    return;
  }
#endif
  ImGui_ImplSDL2_ProcessEvent(&event);
}

void new_frame(const WindowSize& size) noexcept {
  if (g_useSdlRenderer) {
    ImGui_ImplSDLRenderer_NewFrame();
  } else {
    if (g_scale != size.scale) {
      if (g_scale > 0.f) {
        // TODO wgpu backend bug: doesn't clear bind groups on invalidate
        g_resources.ImageBindGroups.Clear();
        ImGui_ImplWGPU_CreateDeviceObjects();
      }
      g_scale = size.scale;
    }
    ImGui_ImplWGPU_NewFrame();
  }
  ImGui_ImplSDL2_NewFrame();

  // Render at full DPI
  ImGui::GetIO().DisplaySize = {static_cast<float>(size.fb_width), static_cast<float>(size.fb_height)};
  ImGui::NewFrame();
}

void render(const wgpu::RenderPassEncoder& pass) noexcept {
  ImGui::Render();

  auto* data = ImGui::GetDrawData();
  // io.DisplayFramebufferScale is informational; we're rendering at full DPI
  data->FramebufferScale = {1.f, 1.f};
  if (g_useSdlRenderer) {
    SDL_Renderer* renderer = ImGui_ImplSDLRenderer_GetBackendData()->SDLRenderer;
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer_RenderDrawData(data);
    SDL_RenderPresent(renderer);
  } else {
    ImGui_ImplWGPU_RenderDrawData(data, pass.Get());
  }
}

ImTextureID add_texture(uint32_t width, uint32_t height, ArrayRef<uint8_t> data) noexcept {
  if (g_useSdlRenderer) {
    SDL_Renderer* renderer = ImGui_ImplSDLRenderer_GetBackendData()->SDLRenderer;
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height);
    SDL_UpdateTexture(texture, nullptr, data.data(), width * 4);
    SDL_SetTextureScaleMode(texture, SDL_ScaleModeLinear);
    return texture;
  }
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
