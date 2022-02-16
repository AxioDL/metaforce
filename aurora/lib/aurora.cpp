#include <SDL.h>
#include <aurora/aurora.hpp>
#include <logvisor/logvisor.hpp>
#include <memory>
#include <dawn/native/DawnNative.h>
#ifdef AURORA_ENABLE_VULKAN
#include <SDL_vulkan.h>
#include <dawn/native/VulkanBackend.h>
#endif
#ifdef AURORA_ENABLE_OPENGL
#include <SDL_opengl.h>
#include <dawn/native/OpenGLBackend.h>
#endif
#include <dawn/webgpu_cpp.h>
#include <magic_enum.hpp>

namespace aurora {
// TODO: Move global state to a class/struct?
static logvisor::Module Log("aurora");
static std::unique_ptr<AppDelegate> g_AppDelegate;
// SDL
static SDL_Window* g_Window;

// Dawn / WebGPU
#ifdef AURORA_ENABLE_VULKAN
static wgpu::BackendType backendType = wgpu::BackendType::Vulkan;
#elif AURORA_ENABLE_METAL
static wgpu::BackendType backendType = wgpu::BackendType::Metal;
#else
static wgpu::BackendType backendType = wgpu::BackendType::OpenGL;
#endif
static std::vector<std::string> g_Args;
static wgpu::SwapChain g_SwapChain;
static DawnSwapChainImplementation g_SwapChainImpl;
static wgpu::Queue g_Queue;
static wgpu::Device g_Device;
static wgpu::TextureFormat g_SwapChainFormat;

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
        /* TODO: ImGui code? */
        /* TODO: Frame Time */
        // g_AppDelegate->onAppIdle(1/60.f);
        /* TODO: Render Texture */
        // g_AppDelegate->onAppDraw();
        /* TODO: ImGui present code? */
        /* TODO: Present */
        // g_AppDelegate->onAppPostDraw();
        break;
      }
      case SDL_WINDOWEVENT_MOVED: {
        g_AppDelegate->onAppWindowMoved(event.window.data1, event.window.data2);
        break;
      }
      case SDL_WINDOWEVENT_RESIZED: {
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
      break;
    }
    case SDL_KEYUP: {
      break;
    }
    case SDL_TEXTINPUT: {
      break;
    }
    case SDL_QUIT:
      g_AppDelegate->onAppExiting();
      Log.report(logvisor::Info, FMT_STRING("Received quit request"));
      return false;
    }
    // TODO why doesn't this work?
    const auto typedEvent = magic_enum::enum_cast<SDL_EventType>(event.type);
    if (typedEvent) {
      Log.report(logvisor::Info, FMT_STRING("Received SDL event: {}"), magic_enum::enum_name(typedEvent.value()));
    } else {
      Log.report(logvisor::Info, FMT_STRING("Received SDL event: {}"), event.type);
    }
  }
  return true;
}

void app_run(std::unique_ptr<AppDelegate> app, Icon icon, int argc, char** argv) noexcept {
  g_AppDelegate = std::move(app);
  /* Lets gather arguments skipping the program filename */
  for (size_t i = 1; i < argc; ++i) {
    g_Args.emplace_back(argv[i]);
  }

  Log.report(logvisor::Info, FMT_STRING("Creating Dawn instance"));
  auto instance = std::make_unique<dawn::native::Instance>();
  instance->DiscoverDefaultAdapters();

  dawn::native::Adapter backendAdapter;
  {
    std::vector<dawn::native::Adapter> adapters = instance->GetAdapters();
    auto adapterIt = std::find_if(adapters.begin(), adapters.end(), [](const dawn::native::Adapter adapter) -> bool {
      wgpu::AdapterProperties properties;
      adapter.GetProperties(&properties);
      return properties.backendType == backendType;
    });
    if (adapterIt == adapters.end()) {
      Log.report(logvisor::Fatal, FMT_STRING("Failed to find usable graphics backend"));
    }
    backendAdapter = *adapterIt;
  }
  wgpu::AdapterProperties adapterProperties;
  backendAdapter.GetProperties(&adapterProperties);
  const auto backendName = magic_enum::enum_name(adapterProperties.backendType);
  Log.report(logvisor::Info, FMT_STRING("Using {} graphics backend"), backendName);

  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    Log.report(logvisor::Fatal, FMT_STRING("Error initializing SDL: {}"), SDL_GetError());
  }

  Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
  switch (adapterProperties.backendType) {
#ifdef AURORA_ENABLE_VULKAN
  case wgpu::BackendType::Vulkan:
    flags |= SDL_WINDOW_VULKAN;
    break;
#endif
#ifdef AURORA_ENABLE_METAL
  case wgpu::BackendType::Metal:
    flags |= SDL_WINDOW_METAL;
    break;
#endif
#ifdef AURORA_ENABLE_OPENGL
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

  g_Device = wgpu::Device::Acquire(backendAdapter.CreateDevice());
  switch (adapterProperties.backendType) {
#ifdef AURORA_ENABLE_VULKAN
  case wgpu::BackendType::Vulkan: {
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (SDL_Vulkan_CreateSurface(g_Window, dawn::native::vulkan::GetInstance(g_Device.Get()), &surface) != SDL_TRUE) {
      Log.report(logvisor::Fatal, FMT_STRING("Failed to create Vulkan surface: {}"), SDL_GetError());
    }
    g_SwapChainImpl = dawn::native::vulkan::CreateNativeSwapChainImpl(g_Device.Get(), surface);
    g_SwapChainFormat =
        static_cast<wgpu::TextureFormat>(dawn::native::vulkan::GetNativeSwapChainPreferredFormat(&g_SwapChainImpl));
    break;
  }
#endif
#ifdef AURORA_ENABLE_METAL
  case wgpu::BackendType::Metal: {
    // TODO
    g_SwapChainFormat = WGPUTextureFormat_BGRA8Unorm;
    break;
  }
#endif
#ifdef AURORA_ENABLE_OPENGL
  case wgpu::BackendType::OpenGL: {
    g_SwapChainImpl = dawn::native::opengl::CreateNativeSwapChainImpl(
        g_Device.Get(), [](void* userdata) { SDL_GL_SwapWindow(static_cast<SDL_Window*>(userdata)); }, g_Window);
    g_SwapChainFormat =
        static_cast<wgpu::TextureFormat>(dawn::native::opengl::GetNativeSwapChainPreferredFormat(&g_SwapChainImpl));
    break;
  }
#endif
  default:
    Log.report(logvisor::Fatal, FMT_STRING("Unsupported backend {}"), backendName);
  }

  g_Queue = g_Device.GetQueue();
  {
    wgpu::SwapChainDescriptor descriptor{};
    descriptor.implementation = reinterpret_cast<uint64_t>(&g_SwapChainImpl);
    g_SwapChain = g_Device.CreateSwapChain(nullptr, &descriptor);
  }
  {
    int width, height;
    SDL_GetWindowSize(g_Window, &width, &height);
    g_SwapChain.Configure(g_SwapChainFormat, wgpu::TextureUsage::RenderAttachment, width, height);
  }

  g_AppDelegate->onAppLaunched();
  g_AppDelegate->onAppWindowResized(get_window_size());
  while (poll_events()) {}

  g_SwapChain.Release();
  g_Queue.Release();
  g_Device.Release();
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
  switch (backendType) {
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
  case wgpu::BackendType::OpenGLES:
    return Backend::OpenGL;
  default:
    return Backend::Invalid;
  }
}
std::string_view get_backend_string() noexcept { return magic_enum::enum_name(get_backend()); }
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
