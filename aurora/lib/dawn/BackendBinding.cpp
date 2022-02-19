#include "BackendBinding.hpp"

#if defined(DAWN_ENABLE_BACKEND_OPENGL)
#include <SDL_video.h>
#include <dawn/native/OpenGLBackend.h>
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

void DiscoverAdapter(dawn::native::Instance* instance, SDL_Window* window, wgpu::BackendType type) {
  if (type == wgpu::BackendType::OpenGL || type == wgpu::BackendType::OpenGLES) {
#if defined(DAWN_ENABLE_BACKEND_OPENGL)
    SDL_GL_CreateContext(window);
    auto getProc = reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress);
    if (type == wgpu::BackendType::OpenGL) {
      dawn::native::opengl::AdapterDiscoveryOptions adapterOptions;
      adapterOptions.getProc = getProc;
      instance->DiscoverAdapters(&adapterOptions);
    } else {
      dawn::native::opengl::AdapterDiscoveryOptionsES adapterOptions;
      adapterOptions.getProc = getProc;
      instance->DiscoverAdapters(&adapterOptions);
    }
#endif
  } else {
    instance->DiscoverDefaultAdapters();
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
