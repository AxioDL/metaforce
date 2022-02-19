#include "BackendBinding.hpp"

#include <SDL_video.h>
#include <dawn/native/OpenGLBackend.h>

namespace aurora::gpu::utils {
class OpenGLBinding : public BackendBinding {
public:
  OpenGLBinding(SDL_Window* window, WGPUDevice device) : BackendBinding(window, device) {}

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
    return dawn::native::opengl::GetNativeSwapChainPreferredFormat(&m_swapChainImpl);
  }

private:
  DawnSwapChainImplementation m_swapChainImpl{};

  void CreateSwapChainImpl() {
    m_swapChainImpl = dawn::native::opengl::CreateNativeSwapChainImpl(
        m_device, [](void* userdata) { SDL_GL_SwapWindow(static_cast<SDL_Window*>(userdata)); }, m_window);
  }
};

BackendBinding* CreateOpenGLBinding(SDL_Window* window, WGPUDevice device) { return new OpenGLBinding(window, device); }
} // namespace aurora::gpu::utils
