#include "BackendBinding.hpp"

#include <SDL_syswm.h>
#include <dawn/native/D3D12Backend.h>
#include <optick.h>

#if USE_OPTICK
// Internal headers
#include <dawn/native/d3d12/CommandBufferD3D12.h>
#include <dawn/native/d3d12/DeviceD3D12.h>
#define private public
#include <dawn/native/d3d12/NativeSwapChainImplD3D12.h>
#undef private
#endif

namespace aurora::gpu::utils {
class D3D12Binding : public BackendBinding {
public:
  D3D12Binding(SDL_Window* window, WGPUAdapter adapter, WGPUDevice device) : BackendBinding(window, adapter, device) {}

  uint64_t GetSwapChainImplementation() override {
    if (m_swapChainImpl.userData == nullptr) {
      CreateSwapChainImpl();
    }
    return reinterpret_cast<uint64_t>(&m_swapChainImpl);
  }

  WGPUTextureFormat GetPreferredSwapChainTextureFormat() override {
    if (m_swapChainImpl.userData == nullptr) {
      CreateSwapChainImpl();
    }
    return dawn::native::d3d12::GetNativeSwapChainPreferredFormat(&m_swapChainImpl);
  }

#if USE_OPTICK
  void* GetNativeSwapChain() override {
    auto* impl = static_cast<dawn::native::d3d12::NativeSwapChainImpl*>(m_swapChainImpl.userData);
    return impl->mSwapChain.Get();
  }

  Optick::GPUContext OptickSetGpuContext() override {
    auto* device = dawn::native::d3d12::ToBackend(static_cast<dawn::native::DeviceBase*>(static_cast<void*>(m_device)));
    auto* commandList = device->GetPendingCommandContext().AcquireSuccess()->GetCommandList();
    return Optick::SetGpuContext({commandList});
  }
#endif

private:
  DawnSwapChainImplementation m_swapChainImpl{};

  void CreateSwapChainImpl() {
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(m_window, &wmInfo);
    m_swapChainImpl = dawn::native::d3d12::CreateNativeSwapChainImpl(m_device, wmInfo.info.win.window);
#if USE_OPTICK
    auto* device = dawn::native::d3d12::ToBackend(static_cast<dawn::native::DeviceBase*>(static_cast<void*>(m_device)));
    auto* d3d12Device = device->GetD3D12Device();
    auto* d3d12CommandQueue = device->GetCommandQueue().Get();
    OPTICK_GPU_INIT_D3D12(d3d12Device, &d3d12CommandQueue, 1);
#endif
  }
};

BackendBinding* CreateD3D12Binding(SDL_Window* window, WGPUAdapter adapter, WGPUDevice device) {
  return new D3D12Binding(window, adapter, device);
}
} // namespace aurora::gpu::utils
