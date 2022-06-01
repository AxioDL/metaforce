#include "BackendBinding.hpp"

#include <SDL_video.h>
#include <dawn/native/NullBackend.h>

namespace aurora::gpu::utils {
class NullBinding : public BackendBinding {
public:
  NullBinding(SDL_Window* window, WGPUDevice device) : BackendBinding(window, device) {}

  uint64_t GetSwapChainImplementation() override {
    if (m_swapChainImpl.userData == nullptr) {
      m_swapChainImpl = dawn::native::null::CreateNativeSwapChainImpl();
    }
    return reinterpret_cast<uint64_t>(&m_swapChainImpl);
  }

  WGPUTextureFormat GetPreferredSwapChainTextureFormat() override {
    return WGPUTextureFormat_RGBA8Unorm;
  }

private:
  DawnSwapChainImplementation m_swapChainImpl{};
};

BackendBinding* CreateNullBinding(SDL_Window* window, WGPUDevice device) { return new NullBinding(window, device); }
} // namespace aurora::gpu::utils
