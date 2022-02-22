#include "BackendBinding.hpp"

#include <SDL_syswm.h>
#include <dawn/native/D3D12Backend.h>

namespace aurora::gpu::utils {
class D3D12Binding : public BackendBinding {
public:
  D3D12Binding(SDL_Window* window, WGPUDevice device) : BackendBinding(window, device) {}

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

private:
  DawnSwapChainImplementation m_swapChainImpl{};

  void CreateSwapChainImpl() {
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(m_window, &wmInfo);
    m_swapChainImpl = dawn::native::d3d12::CreateNativeSwapChainImpl(m_device, wmInfo.info.win.window);
  }
};

BackendBinding* CreateD3D12Binding(SDL_Window* window, WGPUDevice device) { return new D3D12Binding(window, device); }
} // namespace aurora::gpu::utils
