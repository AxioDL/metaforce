#include "BackendBinding.hpp"

#include <SDL_vulkan.h>
#include <cassert>
#include <dawn/native/VulkanBackend.h>

#include <logvisor/logvisor.hpp>

namespace aurora::gpu::utils {
static logvisor::Module Log("aurora::gpu::utils::VulkanBinding");

class VulkanBinding : public BackendBinding {
public:
  VulkanBinding(SDL_Window* window, WGPUDevice device) : BackendBinding(window, device) {}

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
    return dawn::native::vulkan::GetNativeSwapChainPreferredFormat(&m_swapChainImpl);
  }

private:
  DawnSwapChainImplementation m_swapChainImpl{};

  void CreateSwapChainImpl() {
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (SDL_Vulkan_CreateSurface(m_window, dawn::native::vulkan::GetInstance(m_device), &surface) != SDL_TRUE) {
      Log.report(logvisor::Fatal, FMT_STRING("Failed to create Vulkan surface: {}"), SDL_GetError());
    }
    m_swapChainImpl = dawn::native::vulkan::CreateNativeSwapChainImpl(m_device, surface);
  }
};

BackendBinding* CreateVulkanBinding(SDL_Window* window, WGPUDevice device) { return new VulkanBinding(window, device); }
} // namespace aurora::gpu::utils
