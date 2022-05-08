#include <aurora/aurora.hpp>
#include "gfx/common.hpp"
#include "gfx/gx.hpp"
#include "gpu.hpp"
#include "input.hpp"
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
static SDL_Window* g_window;
WindowSize g_windowSize;

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
  SDL_SetWindowIcon(g_window, iconSurface);
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
      case SDL_WINDOWEVENT_MOVED: {
        g_AppDelegate->onAppWindowMoved(event.window.data1, event.window.data2);
        break;
      }
      case SDL_WINDOWEVENT_EXPOSED:
      case SDL_WINDOWEVENT_DISPLAY_CHANGED:
      case SDL_WINDOWEVENT_RESIZED:
      case SDL_WINDOWEVENT_SIZE_CHANGED:
      case SDL_WINDOWEVENT_MINIMIZED:
      case SDL_WINDOWEVENT_MAXIMIZED: {
        const auto size = get_window_size();
        if (size == g_windowSize) {
          break;
        }
        if (size.scale != g_windowSize.scale) {
          if (g_windowSize.scale > 0.f) {
            Log.report(logvisor::Info, FMT_STRING("Display scale changed to {}"), size.scale);
          }
          g_AppDelegate->onAppDisplayScaleChanged(size.scale);
        }
        g_windowSize = size;
        gpu::resize_swapchain(size.fb_width, size.fb_height);
        g_AppDelegate->onAppWindowResized(size);
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
      }
      break;
    }
    case SDL_CONTROLLERDEVICEADDED: {
      auto instance = input::add_controller(event.cdevice.which);
      if (instance != -1) {
        g_AppDelegate->onControllerAdded(instance);
      }
      break;
    }
    case SDL_CONTROLLERDEVICEREMOVED: {
      input::remove_controller(event.cdevice.which);
      break;
    }
    case SDL_CONTROLLERBUTTONUP:
    case SDL_CONTROLLERBUTTONDOWN: {
      g_AppDelegate->onControllerButton(
          event.cbutton.which,
          input::translate_controller_button(static_cast<SDL_GameControllerButton>(event.cbutton.button)),
          event.cbutton.state == SDL_PRESSED);
      break;
    }
    case SDL_CONTROLLERAXISMOTION: {
      if (event.caxis.value > 8000 || event.caxis.value < -8000) {
        g_AppDelegate->onControllerAxis(
            event.caxis.which, input::translate_controller_axis(static_cast<SDL_GameControllerAxis>(event.caxis.axis)),
            event.caxis.value);
      } else {
        g_AppDelegate->onControllerAxis(
            event.caxis.which, input::translate_controller_axis(static_cast<SDL_GameControllerAxis>(event.caxis.axis)),
            0);
      }
      break;
    }
    case SDL_KEYDOWN: {
      if (!ImGui::GetIO().WantCaptureKeyboard) {
        SpecialKey specialKey{};
        ModifierKey modifierKey{};
        char chr = input::translate_key(event.key.keysym, specialKey, modifierKey);
        if (chr != 0) {
          modifierKey = input::translate_modifiers(event.key.keysym.mod);
          g_AppDelegate->onCharKeyDown(chr, modifierKey, event.key.repeat != 0u);
        } else if (specialKey != SpecialKey::None) {
          modifierKey = input::translate_modifiers(event.key.keysym.mod);
          g_AppDelegate->onSpecialKeyDown(specialKey, modifierKey, event.key.repeat != 0u);
        } else if (modifierKey != ModifierKey::None) {
          g_AppDelegate->onModifierKeyDown(modifierKey, event.key.repeat != 0u);
        }
      }
      break;
    }
    case SDL_KEYUP: {
      if (!ImGui::GetIO().WantCaptureKeyboard) {
        SpecialKey specialKey{};
        ModifierKey modifierKey{};
        char chr = input::translate_key(event.key.keysym, specialKey, modifierKey);
        if (chr != 0) {
          modifierKey = input::translate_modifiers(event.key.keysym.mod);
          g_AppDelegate->onCharKeyUp(chr, modifierKey);
        } else if (specialKey != SpecialKey::None) {
          modifierKey = input::translate_modifiers(event.key.keysym.mod);
          g_AppDelegate->onSpecialKeyUp(specialKey, modifierKey);
        } else if (modifierKey != ModifierKey::None) {
          g_AppDelegate->onModifierKeyUp(modifierKey);
        }
      }
      break;
    }
    case SDL_TEXTINPUT: {
      if (!ImGui::GetIO().WantCaptureKeyboard) {
        std::string str;
        str.assign(&event.text.text[0], SDL_TEXTINPUTEVENT_TEXT_SIZE);
        g_AppDelegate->onTextInput(str);
      }
      break;
    }
    case SDL_MOUSEBUTTONDOWN: {
      if (!ImGui::GetIO().WantCaptureMouse) {
        g_AppDelegate->onMouseButtonDown(event.button.x, event.button.y,
                                         input::translate_mouse_button(event.button.button), event.button.clicks);
      }
      break;
    }
    case SDL_MOUSEBUTTONUP: {
      if (!ImGui::GetIO().WantCaptureMouse) {
        g_AppDelegate->onMouseButtonUp(event.button.x, event.button.y,
                                       input::translate_mouse_button(event.button.button));
      }
      break;
    }
    case SDL_MOUSEMOTION: {
      if (!ImGui::GetIO().WantCaptureMouse) {
        g_AppDelegate->onMouseMove(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel,
                                   input::translate_mouse_button_state(event.motion.state));
      }
      break;
    }
    case SDL_QUIT:
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

#if !defined(_WIN32) && !defined(__APPLE__)
  SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
#endif
  SDL_SetHint(SDL_HINT_JOYSTICK_GAMECUBE_RUMBLE_BRAKE, "1");

  SDL_SetHint(SDL_HINT_SCREENSAVER_INHIBIT_ACTIVITY_NAME, "Metaforce");

  SDL_DisableScreenSaver();
  /* TODO: Make this an option rather than hard coding it */
  SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

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
  g_window = SDL_CreateWindow("Metaforce", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 960, flags);
  if (g_window == nullptr) {
    Log.report(logvisor::Fatal, FMT_STRING("Error creating window: {}"), SDL_GetError());
    unreachable();
  }
  set_window_icon(std::move(icon));

  gpu::initialize(g_window);
  gfx::initialize();

  imgui::create_context();
  const auto size = get_window_size();
  Log.report(logvisor::Info, FMT_STRING("Using framebuffer size {}x{} scale {}"), size.fb_width, size.fb_height,
             size.scale);
  g_AppDelegate->onImGuiInit(size.scale);
  imgui::initialize(g_window);
  g_AppDelegate->onImGuiAddTextures();

  g_AppDelegate->onAppLaunched();
  g_AppDelegate->onAppWindowResized(size);

  while (poll_events()) {
    imgui::new_frame(g_windowSize);
    if (!g_AppDelegate->onAppIdle(ImGui::GetIO().DeltaTime)) {
      break;
    }

    const wgpu::TextureView view = g_swapChain.GetCurrentTextureView();
    gfx::begin_frame();
    g_AppDelegate->onAppDraw();

    const auto encoderDescriptor = wgpu::CommandEncoderDescriptor{
        .label = "Redraw encoder",
    };
    auto encoder = g_device.CreateCommandEncoder(&encoderDescriptor);
    gfx::end_frame(encoder);
    {
      const std::array attachments{
          wgpu::RenderPassColorAttachment{
              .view = view,
              // .resolveTarget = g_frameBufferResolved.view,
              .loadOp = wgpu::LoadOp::Clear,
              .storeOp = wgpu::StoreOp::Store,
              .clearColor =
                  {
                      .r = gfx::gx::g_gxState.clearColor.r(),
                      .g = gfx::gx::g_gxState.clearColor.g(),
                      .b = gfx::gx::g_gxState.clearColor.b(),
                      .a = gfx::gx::g_gxState.clearColor.a(),
                  },
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

  Log.report(logvisor::Info, FMT_STRING("Application exiting"));
  g_AppDelegate->onAppExiting();

  imgui::shutdown();
  gfx::shutdown();
  gpu::shutdown();
  SDL_DestroyWindow(g_window);
  SDL_EnableScreenSaver();
  SDL_Quit();
}

std::vector<std::string> get_args() noexcept { return g_Args; }

WindowSize get_window_size() noexcept {
  int width, height, fb_w, fb_h;
  SDL_GetWindowSize(g_window, &width, &height);
  SDL_GL_GetDrawableSize(g_window, &fb_w, &fb_h);
  float scale = static_cast<float>(fb_w) / static_cast<float>(width);
#ifndef __APPLE__
  if (SDL_GetDisplayDPI(SDL_GetWindowDisplayIndex(g_window), nullptr, &scale, nullptr) == 0) {
    scale /= 96.f;
  }
#endif
  return {
      .width = static_cast<uint32_t>(width),
      .height = static_cast<uint32_t>(height),
      .fb_width = static_cast<uint32_t>(fb_w),
      .fb_height = static_cast<uint32_t>(fb_h),
      .scale = scale,
  };
}

void set_window_title(zstring_view title) noexcept { SDL_SetWindowTitle(g_window, title.c_str()); }

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
  SDL_SetWindowFullscreen(g_window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
}

uint32_t get_which_controller_for_player(int32_t index) noexcept { return input::get_instance_for_player(index); }
int32_t get_controller_player_index(uint32_t instance) noexcept { return input::player_index(instance); }

void set_controller_player_index(uint32_t instance, int32_t index) noexcept {
  input::set_player_index(instance, index);
}

bool is_controller_gamecube(uint32_t instance) noexcept { return input::is_gamecube(instance); }

bool controller_has_rumble(uint32_t instance) noexcept { return input::controller_has_rumble(instance); }

void controller_rumble(uint32_t instance, uint16_t low_freq_intensity, uint16_t high_freq_intensity,
                       uint32_t duration_ms) noexcept {
  input::controller_rumble(instance, low_freq_intensity, high_freq_intensity, duration_ms);
}

uint32_t get_controller_count() noexcept { return input::controller_count(); }
std::string get_controller_name(uint32_t instance) noexcept { return input::controller_name(instance); }
} // namespace aurora
