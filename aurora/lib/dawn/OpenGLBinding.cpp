#include "BackendBinding.hpp"

#include <SDL_video.h>
#include <dawn/native/OpenGLBackend.h>

namespace aurora::utils {
class OpenGLBinding : public BackendBinding {
public:
  OpenGLBinding(SDL_Window* window, WGPUDevice device) : BackendBinding(window, device) {}

  uint64_t GetSwapChainImplementation() override {
    if (m_swapChainImpl.userData == nullptr) {
      m_swapChainImpl = dawn::native::opengl::CreateNativeSwapChainImpl(
          m_device, [](void* userdata) { SDL_GL_SwapWindow(static_cast<SDL_Window*>(userdata)); }, m_window);
    }
    return reinterpret_cast<uint64_t>(&m_swapChainImpl);
  }

  WGPUTextureFormat GetPreferredSwapChainTextureFormat() override {
    return dawn::native::opengl::GetNativeSwapChainPreferredFormat(&m_swapChainImpl);
  }

private:
  DawnSwapChainImplementation m_swapChainImpl{};
};

BackendBinding* CreateOpenGLBinding(SDL_Window* window, WGPUDevice device) { return new OpenGLBinding(window, device); }
} // namespace aurora::utils
