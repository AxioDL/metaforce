#include "BackendBinding.hpp"

#include <cassert>
#include <optick.h>

#if USE_OPTICK
// Internal headers
#include <dawn/native/vulkan/AdapterVk.h>
#include <dawn/native/vulkan/DeviceVk.h>
#define private public
#include <dawn/native/vulkan/NativeSwapChainImplVk.h>
#undef private
#endif

#include <SDL_vulkan.h>
#include <dawn/native/VulkanBackend.h>

namespace aurora::gpu::utils {
class VulkanBinding : public BackendBinding {
public:
  VulkanBinding(SDL_Window* window, WGPUAdapter adapter, WGPUDevice device) : BackendBinding(window, adapter, device) {}

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

#if USE_OPTICK
  void* GetNativeSwapChain() override {
    auto* impl = static_cast<dawn::native::vulkan::NativeSwapChainImpl*>(m_swapChainImpl.userData);
    return impl->mSwapChain;
  }

  Optick::GPUContext OptickSetGpuContext() override {
    auto* device =
        dawn::native::vulkan::ToBackend(static_cast<dawn::native::DeviceBase*>(static_cast<void*>(m_device)));
    return Optick::SetGpuContext({device->GetPendingRecordingContext()->commandBuffer});
  }
#endif

private:
  DawnSwapChainImplementation m_swapChainImpl{};

  void CreateSwapChainImpl() {
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (SDL_Vulkan_CreateSurface(m_window, dawn::native::vulkan::GetInstance(m_device), &surface) != SDL_TRUE) {
      assert(false);
    }
    m_swapChainImpl = dawn::native::vulkan::CreateNativeSwapChainImpl(m_device, surface);
#if USE_OPTICK
    auto* adapter =
        dawn::native::vulkan::ToBackend(static_cast<dawn::native::AdapterBase*>(static_cast<void*>(m_adapter)));
    auto* device =
        dawn::native::vulkan::ToBackend(static_cast<dawn::native::DeviceBase*>(static_cast<void*>(m_device)));
    VkDevice vkDevice = device->GetVkDevice();
    VkPhysicalDevice vkPhysicalDevice = adapter->GetPhysicalDevice();
    VkQueue vkQueue = device->GetQueue();
    uint32_t queueFamily = device->GetGraphicsQueueFamily();
    OPTICK_GPU_INIT_VULKAN(&vkDevice, &vkPhysicalDevice, &vkQueue, &queueFamily, 1, nullptr);
#endif
  }
};

BackendBinding* CreateVulkanBinding(SDL_Window* window, WGPUAdapter adapter, WGPUDevice device) {
  return new VulkanBinding(window, adapter, device);
}
} // namespace aurora::gpu::utils
