#include "BackendBinding.hpp"

#if defined(DAWN_ENABLE_BACKEND_D3D12)
#include <dawn/native/D3D12Backend.h>
#endif
#if defined(DAWN_ENABLE_BACKEND_METAL)
#include <dawn/native/MetalBackend.h>
#endif
#if defined(DAWN_ENABLE_BACKEND_VULKAN)
#include <dawn/native/VulkanBackend.h>
#endif
#if defined(DAWN_ENABLE_BACKEND_OPENGL)
#include <SDL_video.h>
#include <dawn/native/OpenGLBackend.h>
#endif
#if defined(DAWN_ENABLE_BACKEND_NULL)
#include <dawn/native/NullBackend.h>
#endif

namespace aurora::gpu::utils {

#if defined(DAWN_ENABLE_BACKEND_D3D12)
BackendBinding* CreateD3D12Binding(SDL_Window* window, WGPUDevice device);
#endif
#if defined(DAWN_ENABLE_BACKEND_METAL)
BackendBinding* CreateMetalBinding(SDL_Window* window, WGPUDevice device);
#endif
#if defined(DAWN_ENABLE_BACKEND_NULL)
BackendBinding* CreateNullBinding(SDL_Window* window, WGPUDevice device);
#endif
#if defined(DAWN_ENABLE_BACKEND_OPENGL)
BackendBinding* CreateOpenGLBinding(SDL_Window* window, WGPUDevice device);
#endif
#if defined(DAWN_ENABLE_BACKEND_VULKAN)
BackendBinding* CreateVulkanBinding(SDL_Window* window, WGPUDevice device);
#endif

BackendBinding::BackendBinding(SDL_Window* window, WGPUDevice device) : m_window(window), m_device(device) {}

bool DiscoverAdapter(dawn::native::Instance* instance, SDL_Window* window, wgpu::BackendType type) {
  switch (type) {
#if defined(DAWN_ENABLE_BACKEND_D3D12)
  case wgpu::BackendType::D3D12: {
    dawn::native::d3d12::AdapterDiscoveryOptions options;
    return instance->DiscoverAdapters(&options);
  }
#endif
#if defined(DAWN_ENABLE_BACKEND_METAL)
  case wgpu::BackendType::Metal: {
    dawn::native::metal::AdapterDiscoveryOptions options;
    return instance->DiscoverAdapters(&options);
  }
#endif
#if defined(DAWN_ENABLE_BACKEND_VULKAN)
  case wgpu::BackendType::Vulkan: {
    dawn::native::vulkan::AdapterDiscoveryOptions options;
    return instance->DiscoverAdapters(&options);
  }
#endif
#if defined(DAWN_ENABLE_BACKEND_OPENGL)
  case wgpu::BackendType::OpenGL:
  case wgpu::BackendType::OpenGLES: {
    SDL_GL_CreateContext(window);
    auto getProc = reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress);
    if (type == wgpu::BackendType::OpenGL) {
      dawn::native::opengl::AdapterDiscoveryOptions adapterOptions;
      adapterOptions.getProc = getProc;
      return instance->DiscoverAdapters(&adapterOptions);
    } else {
      dawn::native::opengl::AdapterDiscoveryOptionsES adapterOptions;
      adapterOptions.getProc = getProc;
      return instance->DiscoverAdapters(&adapterOptions);
    }
  }
#endif
#if defined(DAWN_ENABLE_BACKEND_NULL)
  case wgpu::BackendType::Null:
    instance->DiscoverDefaultAdapters();
    return true;
#endif
  default:
    return false;
  }
}

BackendBinding* CreateBinding(wgpu::BackendType type, SDL_Window* window, WGPUDevice device) {
  switch (type) {
#if defined(DAWN_ENABLE_BACKEND_D3D12)
  case wgpu::BackendType::D3D12:
    return CreateD3D12Binding(window, device);
#endif
#if defined(DAWN_ENABLE_BACKEND_METAL)
  case wgpu::BackendType::Metal:
    return CreateMetalBinding(window, device);
#endif
#if defined(DAWN_ENABLE_BACKEND_NULL)
  case wgpu::BackendType::Null:
    return CreateNullBinding(window, device);
#endif
#if defined(DAWN_ENABLE_BACKEND_DESKTOP_GL)
  case wgpu::BackendType::OpenGL:
    return CreateOpenGLBinding(window, device);
#endif
#if defined(DAWN_ENABLE_BACKEND_OPENGLES)
  case wgpu::BackendType::OpenGLES:
    return CreateOpenGLBinding(window, device);
#endif
#if defined(DAWN_ENABLE_BACKEND_VULKAN)
  case wgpu::BackendType::Vulkan:
    return CreateVulkanBinding(window, device);
#endif
  default:
    return nullptr;
  }
}

} // namespace aurora::gpu::utils
