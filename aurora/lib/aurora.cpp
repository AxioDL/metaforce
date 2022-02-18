#include <aurora/aurora.hpp>

#include <SDL.h>
#include <dawn/native/DawnNative.h>
// TODO HACK: dawn doesn't expose device toggles
#include "../extern/dawn/src/dawn/native/Toggles.h"
#include <dawn/webgpu_cpp.h>
#include <logvisor/logvisor.hpp>
#include <magic_enum.hpp>

#ifdef DAWN_ENABLE_BACKEND_VULKAN
#include <SDL_vulkan.h>
#include <dawn/native/VulkanBackend.h>
#endif
#ifdef DAWN_ENABLE_BACKEND_OPENGL
#include <SDL_opengl.h>
#include <dawn/native/OpenGLBackend.h>
#endif
#ifdef DAWN_ENABLE_BACKEND_METAL
#include <SDL_metal.h>
#include <dawn/native/MetalBackend.h>
#endif

#include "dawn/BackendBinding.hpp"

// imgui
#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_wgpu.h>

// TODO HACK: dawn doesn't expose device toggles
namespace dawn::native {
class DeviceBase {
public:
  void SetToggle(Toggle toggle, bool isEnabled);
};
} // namespace dawn::native

namespace aurora {
// TODO: Move global state to a class/struct?
static logvisor::Module Log("aurora");

static std::unique_ptr<AppDelegate> g_AppDelegate;
static std::vector<std::string> g_Args;

// SDL
static SDL_Window* g_Window;

// Dawn / WebGPU
#ifdef DAWN_ENABLE_BACKEND_VULKAN
static wgpu::BackendType preferredBackendType = wgpu::BackendType::Vulkan;
#elif DAWN_ENABLE_BACKEND_METAL
static wgpu::BackendType preferredBackendType = wgpu::BackendType::Metal;
#else
static wgpu::BackendType preferredBackendType = wgpu::BackendType::OpenGL;
#endif
static std::unique_ptr<dawn::native::Instance> g_Instance;
static dawn::native::Adapter g_Adapter;
static wgpu::AdapterProperties g_AdapterProperties;
wgpu::Device g_Device;
wgpu::Queue g_Queue;
static wgpu::SwapChain g_SwapChain;
static std::unique_ptr<utils::BackendBinding> g_BackendBinding;

static void set_window_icon(Icon icon) noexcept {
  SDL_Surface* iconSurface = SDL_CreateRGBSurfaceFrom(icon.data.get(), icon.width, icon.height, 32, 4 * icon.width,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                                                      0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#else
                                                      0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#endif
  );
  if (iconSurface == nullptr) {
    Log.report(logvisor::Fatal, FMT_STRING("Failed to create icon surface: {}"), SDL_GetError());
  }
  SDL_SetWindowIcon(g_Window, iconSurface);
  SDL_FreeSurface(iconSurface);
}

static bool poll_events() noexcept {
  SDL_Event event;
  while (SDL_PollEvent(&event) != 0) {
    ImGui_ImplSDL2_ProcessEvent(&event);

    switch (event.type) {
    case SDL_WINDOWEVENT: {
      switch (event.window.event) {
      case SDL_WINDOWEVENT_SHOWN: {
        break;
      }
      case SDL_WINDOWEVENT_HIDDEN: {
        break;
      }
      case SDL_WINDOWEVENT_EXPOSED: {
        break;
      }
      case SDL_WINDOWEVENT_MOVED: {
        g_AppDelegate->onAppWindowMoved(event.window.data1, event.window.data2);
        break;
      }
      case SDL_WINDOWEVENT_RESIZED: {
        auto format = static_cast<wgpu::TextureFormat>(g_BackendBinding->GetPreferredSwapChainTextureFormat());
        g_SwapChain.Configure(format, wgpu::TextureUsage::RenderAttachment, event.window.data1, event.window.data2);
        g_AppDelegate->onAppWindowResized(
            {static_cast<uint32_t>(event.window.data1), static_cast<uint32_t>(event.window.data2)});
        break;
      }
      case SDL_WINDOWEVENT_SIZE_CHANGED: {
        // TODO: handle size changed event
        break;
      }
      case SDL_WINDOWEVENT_MINIMIZED: {
        // TODO: handle minimized event
        break;
      }
      case SDL_WINDOWEVENT_MAXIMIZED: {
        // TODO: handle maximized event
        break;
      }
      case SDL_WINDOWEVENT_RESTORED: {
        // TODO: handle restored event
        break;
      }
      case SDL_WINDOWEVENT_ENTER: {
        // TODO: handle enter event (mouse focus)
        break;
      }
      case SDL_WINDOWEVENT_LEAVE: {
        // TODO: handle leave event (mouse focus)
        break;
      }
      case SDL_WINDOWEVENT_FOCUS_GAINED: {
        // TODO: handle focus gained event
        break;
      }
      case SDL_WINDOWEVENT_FOCUS_LOST: {
        // TODO: handle focus lost event
        break;
      }
      case SDL_WINDOWEVENT_CLOSE: {
        // TODO: handle window close event
        break;
      }
      case SDL_WINDOWEVENT_TAKE_FOCUS: {
        // TODO: handle take focus event
        break;
      }
      case SDL_WINDOWEVENT_HIT_TEST: {
        // TODO: handle hit test?
        break;
      }
      case SDL_WINDOWEVENT_DISPLAY_CHANGED: {
        // TODO: handle display chaaged event
        break;
      }
      }
      /* TODO: More window events */
      break;
    }
    case SDL_CONTROLLERDEVICEADDED: {
      break;
    }
    case SDL_CONTROLLERDEVICEREMOVED: {
      break;
    }
    case SDL_CONTROLLERBUTTONDOWN: {
      break;
    }
    case SDL_CONTROLLERBUTTONUP: {
      break;
    }
    case SDL_CONTROLLERAXISMOTION: {
      break;
    }
    case SDL_KEYDOWN: {
      if (!ImGui::GetIO().WantCaptureKeyboard) {
        // TODO
      }
      break;
    }
    case SDL_KEYUP: {
      if (!ImGui::GetIO().WantCaptureKeyboard) {
        // TODO
      }
      break;
    }
    case SDL_TEXTINPUT: {
      if (!ImGui::GetIO().WantCaptureKeyboard) {
        // TODO
      }
      break;
    }
    case SDL_QUIT:
      g_AppDelegate->onAppExiting();
      Log.report(logvisor::Info, FMT_STRING("Received quit request"));
      return false;
    }
    // Log.report(logvisor::Info, FMT_STRING("Received SDL event: {}"), event.type);
  }
  return true;
}

void app_run(std::unique_ptr<AppDelegate> app, Icon icon, int argc, char** argv) noexcept {
  g_AppDelegate = std::move(app);
  /* Lets gather arguments skipping the program filename */
  for (size_t i = 1; i < argc; ++i) {
    g_Args.emplace_back(argv[i]);
  }

  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    Log.report(logvisor::Fatal, FMT_STRING("Error initializing SDL: {}"), SDL_GetError());
  }

  Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;
  switch (preferredBackendType) {
#ifdef DAWN_ENABLE_BACKEND_VULKAN
  case wgpu::BackendType::Vulkan:
    flags |= SDL_WINDOW_VULKAN;
    break;
#endif
#ifdef DAWN_ENABLE_BACKEND_METAL
  case wgpu::BackendType::Metal:
    flags |= SDL_WINDOW_METAL;
    break;
#endif
#ifdef DAWN_ENABLE_BACKEND_OPENGL
  case wgpu::BackendType::OpenGL:
    flags |= SDL_WINDOW_OPENGL;
    break;
#endif
  default:
    break;
  }
  g_Window = SDL_CreateWindow("Metaforce", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, flags);
  if (g_Window == nullptr) {
    Log.report(logvisor::Fatal, FMT_STRING("Error creating window: {}"), SDL_GetError());
  }
  set_window_icon(std::move(icon));

  Log.report(logvisor::Info, FMT_STRING("Creating Dawn instance"));
  g_Instance = std::make_unique<dawn::native::Instance>();
  utils::DiscoverAdapter(g_Instance.get(), g_Window, preferredBackendType);

  {
    std::vector<dawn::native::Adapter> adapters = g_Instance->GetAdapters();
    auto adapterIt = std::find_if(adapters.begin(), adapters.end(), [](const dawn::native::Adapter adapter) -> bool {
      wgpu::AdapterProperties properties;
      adapter.GetProperties(&properties);
      return properties.backendType == preferredBackendType;
    });
    if (adapterIt == adapters.end()) {
      Log.report(logvisor::Fatal, FMT_STRING("Failed to find usable graphics backend"));
    }
    g_Adapter = *adapterIt;
  }
  g_Adapter.GetProperties(&g_AdapterProperties);
  const auto backendName = magic_enum::enum_name(g_AdapterProperties.backendType);
  Log.report(logvisor::Info, FMT_STRING("Using {} graphics backend"), backendName);

  {
    const std::array<wgpu::FeatureName, 1> requiredFeatures{
        wgpu::FeatureName::TextureCompressionBC,
    };
    const auto deviceDescriptor = wgpu::DeviceDescriptor{
        .requiredFeaturesCount = requiredFeatures.size(),
        .requiredFeatures = requiredFeatures.data(),
    };
    g_Device = wgpu::Device::Acquire(g_Adapter.CreateDevice(&deviceDescriptor));
    // TODO HACK: dawn doesn't expose device toggles
    static_cast<dawn::native::DeviceBase*>(static_cast<void*>(g_Device.Get()))
        ->SetToggle(dawn::native::Toggle::UseUserDefinedLabelsInBackend, true);
  }

  g_BackendBinding = std::unique_ptr<utils::BackendBinding>(
      utils::CreateBinding(g_AdapterProperties.backendType, g_Window, g_Device.Get()));
  if (!g_BackendBinding) {
    Log.report(logvisor::Fatal, FMT_STRING("Unsupported backend {}"), backendName);
  }

  g_Queue = g_Device.GetQueue();
  {
    wgpu::SwapChainDescriptor descriptor{};
    descriptor.implementation = g_BackendBinding->GetSwapChainImplementation();
    g_SwapChain = g_Device.CreateSwapChain(nullptr, &descriptor);
  }
  {
    auto size = get_window_size();
    auto format = static_cast<wgpu::TextureFormat>(g_BackendBinding->GetPreferredSwapChainTextureFormat());
    Log.report(logvisor::Info, FMT_STRING("Using swapchain format {}"), magic_enum::enum_name(format));
    g_SwapChain.Configure(format, wgpu::TextureUsage::RenderAttachment, size.width, size.height);
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.IniFilename = nullptr;
  g_AppDelegate->onImGuiInit(1.f); // TODO scale
  ImGui_ImplSDL2_InitForMetal(g_Window);
  ImGui_ImplWGPU_Init(g_Device.Get(), 1, g_BackendBinding->GetPreferredSwapChainTextureFormat());
  g_AppDelegate->onImGuiAddTextures();

  g_AppDelegate->onAppLaunched();
  g_AppDelegate->onAppWindowResized(get_window_size());

  while (poll_events()) {
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    g_AppDelegate->onAppIdle(ImGui::GetIO().DeltaTime);

    const wgpu::TextureView view = g_SwapChain.GetCurrentTextureView();
    g_AppDelegate->onAppDraw();
    ImGui::Render();

    auto encoder = g_Device.CreateCommandEncoder();
    {
      std::array<wgpu::RenderPassColorAttachment, 1> attachments{wgpu::RenderPassColorAttachment{
          .view = view,
          .loadOp = wgpu::LoadOp::Clear,
          .storeOp = wgpu::StoreOp::Store,
          .clearColor = {0.f, 0.f, 0.f, 0.f},
      }};
      auto renderPassDescriptor = wgpu::RenderPassDescriptor{
          .label = "Render Pass",
          .colorAttachmentCount = attachments.size(),
          .colorAttachments = attachments.data(),
      };
      auto pass = encoder.BeginRenderPass(&renderPassDescriptor);
      ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), pass.Get());
      pass.End();
    }
    const auto buffer = encoder.Finish();
    g_Queue.Submit(1, &buffer);
    g_SwapChain.Present();

    g_AppDelegate->onAppPostDraw();
  }

  g_AppDelegate->onAppExiting();

  wgpuSwapChainRelease(g_SwapChain.Release());
  wgpuQueueRelease(g_Queue.Release());
  g_BackendBinding.reset();
  wgpuDeviceRelease(g_Device.Release());
  g_Instance.reset();
  SDL_DestroyWindow(g_Window);
  SDL_Quit();
}

std::vector<std::string> get_args() noexcept { return g_Args; }
WindowSize get_window_size() noexcept {
  int width, height;
  SDL_GetWindowSize(g_Window, &width, &height);
  return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}
void set_window_title(zstring_view title) noexcept { SDL_SetWindowTitle(g_Window, title.c_str()); }
Backend get_backend() noexcept {
  switch (g_AdapterProperties.backendType) {
  case wgpu::BackendType::WebGPU:
    return Backend::WebGPU;
  case wgpu::BackendType::D3D11:
    return Backend::D3D11;
  case wgpu::BackendType::D3D12:
    return Backend::D3D12;
  case wgpu::BackendType::Metal:
    return Backend::Metal;
  case wgpu::BackendType::Vulkan:
    return Backend::Vulkan;
  case wgpu::BackendType::OpenGL:
    return Backend::OpenGL;
  case wgpu::BackendType::OpenGLES:
    return Backend::OpenGLES;
  default:
    return Backend::Invalid;
  }
}
std::string_view get_backend_string() noexcept { return magic_enum::enum_name(g_AdapterProperties.backendType); }
void set_fullscreen(bool fullscreen) noexcept {
  auto flags = SDL_GetWindowFlags(g_Window);
  if (fullscreen) {
    flags |= SDL_WINDOW_FULLSCREEN;
  } else {
    flags &= ~SDL_WINDOW_FULLSCREEN;
  }
  SDL_SetWindowFullscreen(g_Window, flags);
}

int32_t get_controller_player_index(uint32_t which) noexcept {
  return -1; // TODO
}
void set_controller_player_index(uint32_t which, int32_t index) noexcept {
  // TODO
}
bool is_controller_gamecube(uint32_t which) noexcept {
  return true; // TODO
}
std::string get_controller_name(uint32_t which) noexcept {
  return ""; // TODO
}
} // namespace aurora
