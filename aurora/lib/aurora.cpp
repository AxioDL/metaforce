#include <aurora/aurora.hpp>

#include "gfx/common.hpp"
#include "gpu.hpp"
#include "imgui.hpp"

#include <SDL.h>
#include <imgui.h>
#include <logvisor/logvisor.hpp>
#include <magic_enum.hpp>

namespace aurora {
static logvisor::Module Log("aurora");

// TODO: Move global state to a class/struct?
static std::unique_ptr<AppDelegate> g_AppDelegate;
static std::vector<std::string> g_Args;

// SDL
static SDL_Window* g_Window;

// GPU
using gpu::g_depthBuffer;
using gpu::g_device;
using gpu::g_frameBuffer;
using gpu::g_frameBufferResolved;
using gpu::g_queue;
using gpu::g_swapChain;

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
    unreachable();
  }
  SDL_SetWindowIcon(g_Window, iconSurface);
  SDL_FreeSurface(iconSurface);
}

static bool poll_events() noexcept {
  SDL_Event event;
  while (SDL_PollEvent(&event) != 0) {
    imgui::process_event(event);

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
        gpu::resize_swapchain(event.window.data1, event.window.data2);
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
    unreachable();
  }

  Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;
  switch (gpu::preferredBackendType) {
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
    unreachable();
  }
  set_window_icon(std::move(icon));

  gpu::initialize(g_Window);
  gfx::initialize();

  imgui::create_context();
  g_AppDelegate->onImGuiInit(1.f); // TODO scale
  imgui::initialize(g_Window);
  g_AppDelegate->onImGuiAddTextures();

  g_AppDelegate->onAppLaunched();
  g_AppDelegate->onAppWindowResized(get_window_size());

  while (poll_events()) {
    imgui::new_frame();
    if (!g_AppDelegate->onAppIdle(ImGui::GetIO().DeltaTime)) {
      break;
    }

    const wgpu::TextureView view = g_swapChain.GetCurrentTextureView();
    g_AppDelegate->onAppDraw();

    const auto encoderDescriptor = wgpu::CommandEncoderDescriptor{
        .label = "Redraw encoder",
    };
    auto encoder = g_device.CreateCommandEncoder(&encoderDescriptor);
    {
      const std::array attachments{
          wgpu::RenderPassColorAttachment{
              .view = view,
              // .resolveTarget = g_frameBufferResolved.view,
              .loadOp = wgpu::LoadOp::Clear,
              .storeOp = wgpu::StoreOp::Store,
              .clearColor = {0.f, 0.f, 0.f, 0.f},
          },
      };
      const auto depthStencilAttachment = wgpu::RenderPassDepthStencilAttachment{
          .view = g_depthBuffer.view,
          .depthLoadOp = wgpu::LoadOp::Clear,
          .depthStoreOp = wgpu::StoreOp::Discard,
          .clearDepth = 1.f,
          .stencilLoadOp = wgpu::LoadOp::Clear,
          .stencilStoreOp = wgpu::StoreOp::Discard,
      };
      auto renderPassDescriptor = wgpu::RenderPassDescriptor{
          .label = "Main render pass",
          .colorAttachmentCount = attachments.size(),
          .colorAttachments = attachments.data(),
          .depthStencilAttachment = &depthStencilAttachment,
      };
      auto pass = encoder.BeginRenderPass(&renderPassDescriptor);
      gfx::render(pass);
      pass.End();
    }
    {
      const std::array attachments{
          wgpu::RenderPassColorAttachment{
              .view = view,
              .loadOp = wgpu::LoadOp::Load,
              .storeOp = wgpu::StoreOp::Store,
          },
      };
      auto renderPassDescriptor = wgpu::RenderPassDescriptor{
          .label = "ImGui render pass",
          .colorAttachmentCount = attachments.size(),
          .colorAttachments = attachments.data(),
      };
      auto pass = encoder.BeginRenderPass(&renderPassDescriptor);
      imgui::render(pass);
      pass.End();
    }
    const auto buffer = encoder.Finish();
    g_queue.Submit(1, &buffer);
    g_swapChain.Present();

    g_AppDelegate->onAppPostDraw();

    ImGui::EndFrame();
  }

  g_AppDelegate->onAppExiting();

  imgui::shutdown();
  gfx::shutdown();
  gpu::shutdown();
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
  switch (gpu::g_backendType) {
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

std::string_view get_backend_string() noexcept { return magic_enum::enum_name(gpu::g_backendType); }

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
