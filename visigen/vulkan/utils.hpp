#pragma once

// Copyright(c) 2019, NVIDIA CORPORATION. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#define VULKAN_HPP_NO_EXCEPTIONS 1
#define VULKAN_HPP_ASSERT_ON_RESULT
//#define VULKAN_HPP_UTILS_USE_BEST_PRACTICES 1
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include <vulkan/vulkan.hpp>

#include <iostream>
#include <map>

namespace vk::su {
template <typename T>
inline static T assertSuccess(vk::ResultValue<T> result) {
  assert(result.result == vk::Result::eSuccess);
  return std::move(result.value);
}

const uint64_t FenceTimeout = 100000000;

template <typename Func>
void oneTimeSubmit(vk::CommandBuffer const& commandBuffer, vk::Queue const& queue, Func const& func) {
  commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
  func(commandBuffer);
  commandBuffer.end();
  queue.submit(vk::SubmitInfo(0, nullptr, nullptr, 1, &commandBuffer), nullptr);
  queue.waitIdle();
}

template <typename Func>
void oneTimeSubmit(vk::Device const& device, vk::CommandPool const& commandPool, vk::Queue const& queue,
                   Func const& func) {
  vk::CommandBuffer commandBuffer =
      device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1))
          .value.front();
  oneTimeSubmit(commandBuffer, queue, func);
}

template <class T>
void copyToDevice(vk::Device const& device, vk::DeviceMemory const& deviceMemory, T const* pData, size_t count,
                  vk::DeviceSize stride = sizeof(T)) {
  assert(sizeof(T) <= stride);
  uint8_t* deviceData = static_cast<uint8_t*>(device.mapMemory(deviceMemory, 0, count * stride).value);
  if (stride == sizeof(T)) {
    memcpy(deviceData, pData, count * sizeof(T));
  } else {
    for (size_t i = 0; i < count; i++) {
      memcpy(deviceData, &pData[i], sizeof(T));
      deviceData += stride;
    }
  }
  device.unmapMemory(deviceMemory);
}

template <class T>
void copyToDevice(vk::Device const& device, vk::DeviceMemory const& deviceMemory, T const& data) {
  copyToDevice<T>(device, deviceMemory, &data, 1);
}

template <class T>
VULKAN_HPP_INLINE constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
  return v < lo ? lo : hi < v ? hi : v;
}

void setImageLayout(vk::CommandBuffer const& commandBuffer, vk::Image image, vk::Format format,
                    vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout);

struct BufferData {
  BufferData(vk::PhysicalDevice const& physicalDevice, vk::Device const& device, vk::DeviceSize size,
             vk::BufferUsageFlags usage,
             vk::MemoryPropertyFlags propertyFlags = vk::MemoryPropertyFlagBits::eHostVisible |
                                                     vk::MemoryPropertyFlagBits::eHostCoherent);

  template <typename DataType>
  void upload(vk::Device const& device, DataType const& data) const {
    assert((m_propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent) &&
           (m_propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible));
    assert(sizeof(DataType) <= size);

    void* dataPtr = assertSuccess(device.mapMemory(deviceMemory.get(), 0, sizeof(DataType)));
    memcpy(dataPtr, &data, sizeof(DataType));
    device.unmapMemory(deviceMemory.get());
  }

  template <typename DataType>
  void upload(vk::Device const& device, std::vector<DataType> const& data, size_t stride = 0) const {
    assert(m_propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible);

    size_t elementSize = stride ? stride : sizeof(DataType);
    assert(sizeof(DataType) <= elementSize);

    copyToDevice(device, deviceMemory, data.data(), data.size(), elementSize);
  }

  template <typename DataType>
  void upload(vk::PhysicalDevice const& physicalDevice, vk::Device const& device, vk::CommandPool const& commandPool,
              vk::Queue queue, std::vector<DataType> const& data, size_t stride) const {
    assert(m_usage & vk::BufferUsageFlagBits::eTransferDst);
    assert(m_propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal);

    size_t elementSize = stride ? stride : sizeof(DataType);
    assert(sizeof(DataType) <= elementSize);

    size_t dataSize = data.size() * elementSize;
    assert(dataSize <= size);

    vk::su::BufferData stagingBuffer(physicalDevice, device, dataSize, vk::BufferUsageFlagBits::eTransferSrc);
    copyToDevice(device, stagingBuffer.deviceMemory, data.data(), data.size(), elementSize);

    vk::su::oneTimeSubmit(device, commandPool, queue, [&](vk::CommandBuffer const& commandBuffer) {
      commandBuffer.copyBuffer(stagingBuffer.buffer.get(), buffer.get(), vk::BufferCopy(0, 0, dataSize));
    });
  }

  void* map(vk::Device const& device) { return assertSuccess(device.mapMemory(deviceMemory.get(), 0, size)); }

  void unmap(vk::Device const& device) { device.unmapMemory(deviceMemory.get()); }

  vk::UniqueBuffer buffer;
  vk::UniqueDeviceMemory deviceMemory;
  vk::DeviceSize size;
#if !defined(NDEBUG)
private:
  vk::BufferUsageFlags m_usage;
  vk::MemoryPropertyFlags m_propertyFlags;
#endif
};

struct ImageData {
  ImageData(vk::PhysicalDevice const& physicalDevice, vk::Device const& device, vk::Format format,
            vk::Extent2D const& extent, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
            vk::ImageLayout initialLayout, vk::MemoryPropertyFlags memoryProperties, vk::ImageAspectFlags aspectMask);

  vk::Format format;
  vk::UniqueImage image;
  vk::UniqueDeviceMemory deviceMemory;
  vk::DeviceSize deviceSize;
  vk::UniqueImageView imageView;
};

struct DepthBufferData : public ImageData {
  DepthBufferData(vk::PhysicalDevice const& physicalDevice, vk::Device const& device, vk::Format format,
                  vk::Extent2D const& extent);
};

struct TextureData {
  TextureData(vk::PhysicalDevice const& physicalDevice, vk::Device const& device,
              vk::Extent2D const& extent_ = {256, 256}, vk::ImageUsageFlags usageFlags = {},
              vk::FormatFeatureFlags formatFeatureFlags = {}, bool anisotropyEnable = false, bool forceStaging = false);

  template <typename ImageGenerator>
  void setImage(vk::Device const& device, vk::CommandBuffer const& commandBuffer,
                ImageGenerator const& imageGenerator) {
    void* data =
        needsStaging
            ? assertSuccess(device.mapMemory(stagingBufferData->deviceMemory.get(), 0, stagingBufferData->size))
            : assertSuccess(device.mapMemory(imageData->deviceMemory.get(), 0, imageData->deviceSize));
    imageGenerator(data, extent);
    device.unmapMemory(needsStaging ? stagingBufferData->deviceMemory.get() : imageData->deviceMemory.get());

    if (needsStaging) {
      // Since we're going to blit to the texture image, set its layout to eTransferDstOptimal
      vk::su::setImageLayout(commandBuffer, imageData->image.get(), imageData->format, vk::ImageLayout::eUndefined,
                             vk::ImageLayout::eTransferDstOptimal);
      vk::BufferImageCopy copyRegion(0, extent.width, extent.height,
                                     vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
                                     vk::Offset3D(0, 0, 0), vk::Extent3D(extent, 1));
      commandBuffer.copyBufferToImage(stagingBufferData->buffer.get(), imageData->image.get(),
                                      vk::ImageLayout::eTransferDstOptimal, copyRegion);
      // Set the layout for the texture image from eTransferDstOptimal to SHADER_READ_ONLY
      vk::su::setImageLayout(commandBuffer, imageData->image.get(), imageData->format,
                             vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    } else {
      // If we can use the linear tiled image as a texture, just do it
      vk::su::setImageLayout(commandBuffer, imageData->image.get(), imageData->format, vk::ImageLayout::ePreinitialized,
                             vk::ImageLayout::eShaderReadOnlyOptimal);
    }
  }

  vk::Format format;
  vk::Extent2D extent;
  bool needsStaging;
  std::unique_ptr<BufferData> stagingBufferData;
  std::unique_ptr<ImageData> imageData;
  vk::UniqueSampler sampler;
};

struct UUID {
public:
  UUID(uint8_t const data[VK_UUID_SIZE]);

  uint8_t m_data[VK_UUID_SIZE];
};

template <typename TargetType, typename SourceType>
VULKAN_HPP_INLINE TargetType checked_cast(SourceType value) {
  static_assert(sizeof(TargetType) <= sizeof(SourceType), "No need to cast from smaller to larger type!");
  static_assert(std::numeric_limits<SourceType>::is_integer, "Only integer types supported!");
  static_assert(!std::numeric_limits<SourceType>::is_signed, "Only unsigned types supported!");
  static_assert(std::numeric_limits<TargetType>::is_integer, "Only integer types supported!");
  static_assert(!std::numeric_limits<TargetType>::is_signed, "Only unsigned types supported!");
  assert(value <= std::numeric_limits<TargetType>::max());
  return static_cast<TargetType>(value);
}

vk::UniqueDeviceMemory allocateDeviceMemory(vk::Device const& device,
                                            vk::PhysicalDeviceMemoryProperties const& memoryProperties,
                                            vk::MemoryRequirements const& memoryRequirements,
                                            vk::MemoryPropertyFlags memoryPropertyFlags);
bool contains(std::vector<vk::ExtensionProperties> const& extensionProperties, std::string const& extensionName);
vk::UniqueCommandPool createCommandPool(vk::Device const& device, uint32_t queueFamilyIndex);
vk::UniqueCommandBuffer createCommandBuffer(vk::Device const& device, vk::CommandPool const& commandPool);
vk::DebugUtilsMessengerEXT createDebugUtilsMessengerEXT(vk::Instance const& instance);
vk::UniqueDescriptorPool createDescriptorPool(vk::Device const& device,
                                              std::vector<vk::DescriptorPoolSize> const& poolSizes);
vk::UniqueDescriptorSetLayout createDescriptorSetLayout(
    vk::Device const& device,
    std::vector<std::tuple<vk::DescriptorType, uint32_t, vk::ShaderStageFlags>> const& bindingData,
    vk::DescriptorSetLayoutCreateFlags flags = {});
vk::UniqueDevice createDevice(vk::PhysicalDevice const& physicalDevice, uint32_t queueFamilyIndex,
                              std::vector<std::string> const& extensions = {},
                              vk::PhysicalDeviceFeatures const* physicalDeviceFeatures = nullptr,
                              void const* pNext = nullptr);
vk::UniqueFramebuffer createFramebuffer(vk::Device const& device, vk::RenderPass& renderPass,
                                        vk::ImageView const& colorImageView, vk::ImageView const& depthImageView,
                                        vk::Extent2D const& extent);
vk::UniquePipeline createGraphicsPipeline(
    vk::Device const& device, vk::PipelineCache const& pipelineCache,
    std::pair<vk::ShaderModule, vk::SpecializationInfo const*> const& vertexShaderData,
    std::pair<vk::ShaderModule, vk::SpecializationInfo const*> const& fragmentShaderData, uint32_t vertexStride,
    std::vector<std::pair<vk::Format, uint32_t>> const& vertexInputAttributeFormatOffset, vk::FrontFace frontFace,
    bool depthBuffered, vk::PipelineLayout const& pipelineLayout, vk::RenderPass const& renderPass);
vk::UniqueInstance createInstance(std::string const& appName, std::string const& engineName,
                                  std::vector<std::string> const& layers = {},
                                  std::vector<std::string> const& extensions = {},
                                  uint32_t apiVersion = VK_API_VERSION_1_0);
vk::UniqueRenderPass createRenderPass(vk::Device const& device, vk::Format colorFormat, vk::Format depthFormat,
                                      vk::AttachmentLoadOp colorloadOp = vk::AttachmentLoadOp::eDontCare,
                                      vk::AttachmentStoreOp colorStoreOp = vk::AttachmentStoreOp::eStore,
                                      vk::AttachmentLoadOp depthLoadOp = vk::AttachmentLoadOp::eClear,
                                      vk::AttachmentStoreOp depthStoreOp = vk::AttachmentStoreOp::eDontCare,
                                      vk::ImageLayout colorFinalLayout = vk::ImageLayout::ePresentSrcKHR);
VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                           VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                           VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
                                                           void* /*pUserData*/);
uint32_t findGraphicsQueueFamilyIndex(std::vector<vk::QueueFamilyProperties> const& queueFamilyProperties);
std::pair<uint32_t, uint32_t> findGraphicsAndPresentQueueFamilyIndex(vk::PhysicalDevice physicalDevice,
                                                                     vk::SurfaceKHR const& surface);
uint32_t findMemoryType(vk::PhysicalDeviceMemoryProperties const& memoryProperties, uint32_t typeBits,
                        vk::MemoryPropertyFlags requirementsMask);
std::vector<char const*> gatherExtensions(std::vector<std::string> const& extensions
#if !defined(NDEBUG)
                                          ,
                                          std::vector<vk::ExtensionProperties> const& extensionProperties
#endif
);
std::vector<char const*> gatherLayers(std::vector<std::string> const& layers
#if !defined(NDEBUG)
                                      ,
                                      std::vector<vk::LayerProperties> const& layerProperties
#endif
);
std::vector<std::string> getDeviceExtensions();
std::vector<std::string> getInstanceExtensions();
vk::DebugUtilsMessengerCreateInfoEXT makeDebugUtilsMessengerCreateInfoEXT();
#if defined(NDEBUG)
vk::StructureChain<vk::InstanceCreateInfo>
#elif defined(VULKAN_HPP_UTILS_USE_BEST_PRACTICES)
vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT, vk::ValidationFeaturesEXT>
#else
vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT>
#endif
makeInstanceCreateInfoChain(vk::ApplicationInfo const& applicationInfo, std::vector<char const*> const& enabledLayers,
                            std::vector<char const*> const& enabledExtensions);
vk::Format pickDepthFormat(vk::PhysicalDevice const& physicalDevice);
vk::PresentModeKHR pickPresentMode(std::vector<vk::PresentModeKHR> const& presentModes);
vk::SurfaceFormatKHR pickSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& formats);
void submitAndWait(vk::Device const& device, vk::Queue const& queue, vk::CommandBuffer const& commandBuffer);
void updateDescriptorSets(
    vk::Device const& device, vk::DescriptorSet const& descriptorSet,
    std::vector<std::tuple<vk::DescriptorType, vk::Buffer const&, vk::DeviceSize>> const& bufferData,
    uint32_t bindingOffset = 0);

} // namespace vk::su

std::ostream& operator<<(std::ostream& os, vk::su::UUID const& uuid);
