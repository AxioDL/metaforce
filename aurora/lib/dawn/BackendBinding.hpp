#pragma once

#include <dawn/native/DawnNative.h>
#include <dawn/webgpu_cpp.h>
#include <optick.h>

struct SDL_Window;

namespace aurora::gpu::utils {

class BackendBinding {
public:
  virtual ~BackendBinding() = default;

  virtual uint64_t GetSwapChainImplementation() = 0;
  virtual WGPUTextureFormat GetPreferredSwapChainTextureFormat() = 0;
#if USE_OPTICK
  virtual void* GetNativeSwapChain() { return nullptr; };
  virtual Optick::GPUContext OptickSetGpuContext() { return {}; };
#endif

protected:
  BackendBinding(SDL_Window* window, WGPUAdapter adapter, WGPUDevice device);

  SDL_Window* m_window = nullptr;
  WGPUAdapter m_adapter = nullptr;
  WGPUDevice m_device = nullptr;
};

void DiscoverAdapter(dawn::native::Instance* instance, SDL_Window* window, wgpu::BackendType type);
BackendBinding* CreateBinding(wgpu::BackendType type, SDL_Window* window, WGPUAdapter adapter, WGPUDevice device);

} // namespace aurora::gpu::utils
