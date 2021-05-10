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

#if defined(_MSC_VER)
// no need to ignore any warnings with MSVC
#elif defined(__clang__)
#pragma clang diagnostic ignored "-Wmissing-braces"
#elif defined(__GNUC__)
// no need to ignore any warnings with GCC
#else
// unknow compiler... just ignore the warnings for yourselves ;)
#endif

#include "utils.hpp"

#include <vulkan/vulkan.hpp>

#include <iomanip>
#include <memory>
#include <numeric>
#include <utility>

#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif

namespace vk::su {
vk::UniqueDeviceMemory allocateDeviceMemory(vk::Device const& device,
                                            vk::PhysicalDeviceMemoryProperties const& memoryProperties,
                                            vk::MemoryRequirements const& memoryRequirements,
                                            vk::MemoryPropertyFlags memoryPropertyFlags) {
  uint32_t memoryTypeIndex = findMemoryType(memoryProperties, memoryRequirements.memoryTypeBits, memoryPropertyFlags);

  return assertSuccess(device.allocateMemoryUnique(vk::MemoryAllocateInfo(memoryRequirements.size, memoryTypeIndex)));
}

bool contains(std::vector<vk::ExtensionProperties> const& extensionProperties, std::string const& extensionName) {
  auto propertyIterator =
      std::find_if(extensionProperties.begin(), extensionProperties.end(),
                   [&extensionName](vk::ExtensionProperties const& ep) { return extensionName == ep.extensionName; });
  return (propertyIterator != extensionProperties.end());
}

vk::UniqueCommandPool createCommandPool(vk::Device const& device, uint32_t queueFamilyIndex) {
  vk::CommandPoolCreateInfo commandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueFamilyIndex);
  return assertSuccess(device.createCommandPoolUnique(commandPoolCreateInfo));
}

vk::UniqueCommandBuffer createCommandBuffer(vk::Device const& device, vk::CommandPool const& commandPool) {
  const vk::CommandBufferAllocateInfo info(commandPool, vk::CommandBufferLevel::ePrimary, 1);
  auto commandBuffers = vk::su::assertSuccess(device.allocateCommandBuffersUnique(info));
  return std::move(commandBuffers.front());
}

vk::DebugUtilsMessengerEXT createDebugUtilsMessengerEXT(vk::Instance const& instance) {
  return instance.createDebugUtilsMessengerEXT(vk::su::makeDebugUtilsMessengerCreateInfoEXT()).value;
}

vk::UniqueDescriptorPool createDescriptorPool(vk::Device const& device,
                                        std::vector<vk::DescriptorPoolSize> const& poolSizes) {
  assert(!poolSizes.empty());
  uint32_t maxSets =
      std::accumulate(poolSizes.begin(), poolSizes.end(), 0,
                      [](uint32_t sum, vk::DescriptorPoolSize const& dps) { return sum + dps.descriptorCount; });
  assert(0 < maxSets);

  vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, maxSets,
                                                        poolSizes);
  return assertSuccess(device.createDescriptorPoolUnique(descriptorPoolCreateInfo));
}

vk::UniqueDescriptorSetLayout createDescriptorSetLayout(
    vk::Device const& device,
    std::vector<std::tuple<vk::DescriptorType, uint32_t, vk::ShaderStageFlags>> const& bindingData,
    vk::DescriptorSetLayoutCreateFlags flags) {
  std::vector<vk::DescriptorSetLayoutBinding> bindings(bindingData.size());
  for (size_t i = 0; i < bindingData.size(); i++) {
    bindings[i] = vk::DescriptorSetLayoutBinding(checked_cast<uint32_t>(i), std::get<0>(bindingData[i]),
                                                 std::get<1>(bindingData[i]), std::get<2>(bindingData[i]));
  }
  return assertSuccess(device.createDescriptorSetLayoutUnique(vk::DescriptorSetLayoutCreateInfo(flags, bindings)));
}

vk::UniqueDevice createDevice(vk::PhysicalDevice const& physicalDevice, uint32_t queueFamilyIndex,
                              std::vector<std::string> const& extensions,
                              vk::PhysicalDeviceFeatures const* physicalDeviceFeatures, void const* pNext) {
  std::vector<char const*> enabledExtensions;
  enabledExtensions.reserve(extensions.size());
  for (auto const& ext : extensions) {
    enabledExtensions.push_back(ext.data());
  }

  float queuePriority = 0.0f;
  vk::DeviceQueueCreateInfo deviceQueueCreateInfo({}, queueFamilyIndex, 1, &queuePriority);
  vk::DeviceCreateInfo deviceCreateInfo({}, deviceQueueCreateInfo, {}, enabledExtensions, physicalDeviceFeatures);
  deviceCreateInfo.pNext = pNext;

  vk::UniqueDevice device = assertSuccess(physicalDevice.createDeviceUnique(deviceCreateInfo));
#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
  // initialize function pointers for instance
  VULKAN_HPP_DEFAULT_DISPATCHER.init(device.get());
#endif
  return device;
}

vk::UniqueFramebuffer createFramebuffer(vk::Device const& device, vk::RenderPass& renderPass,
                                        vk::ImageView const& colorImageView, vk::ImageView const& depthImageView,
                                        vk::Extent2D const& extent) {
  std::array<vk::ImageView, 2> attachments{colorImageView, depthImageView};
  vk::FramebufferCreateInfo framebufferCreateInfo(vk::FramebufferCreateFlags(), renderPass, depthImageView ? 2 : 1,
                                                  attachments.data(), extent.width, extent.height, 1);
  return assertSuccess(device.createFramebufferUnique(framebufferCreateInfo));
}

vk::UniquePipeline createGraphicsPipeline(
    vk::Device const& device, vk::PipelineCache const& pipelineCache,
    std::pair<vk::ShaderModule, vk::SpecializationInfo const*> const& vertexShaderData,
    std::pair<vk::ShaderModule, vk::SpecializationInfo const*> const& fragmentShaderData, uint32_t vertexStride,
    std::vector<std::pair<vk::Format, uint32_t>> const& vertexInputAttributeFormatOffset, vk::FrontFace frontFace,
    bool depthBuffered, vk::PipelineLayout const& pipelineLayout, vk::RenderPass const& renderPass) {
  std::array<vk::PipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos = {
      vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex,
                                        vertexShaderData.first, "main", vertexShaderData.second),
      vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment,
                                        fragmentShaderData.first, "main", fragmentShaderData.second)};

  std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions;
  vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo;
  vk::VertexInputBindingDescription vertexInputBindingDescription(0, vertexStride);

  if (0 < vertexStride) {
    vertexInputAttributeDescriptions.reserve(vertexInputAttributeFormatOffset.size());
    for (uint32_t i = 0; i < vertexInputAttributeFormatOffset.size(); i++) {
      vertexInputAttributeDescriptions.emplace_back(i, 0, vertexInputAttributeFormatOffset[i].first,
                                                    vertexInputAttributeFormatOffset[i].second);
    }
    pipelineVertexInputStateCreateInfo.setVertexBindingDescriptions(vertexInputBindingDescription);
    pipelineVertexInputStateCreateInfo.setVertexAttributeDescriptions(vertexInputAttributeDescriptions);
  }

  vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
      vk::PipelineInputAssemblyStateCreateFlags(), vk::PrimitiveTopology::eTriangleStrip);

  vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(vk::PipelineViewportStateCreateFlags(), 1,
                                                                      nullptr, 1, nullptr);

  vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
      vk::PipelineRasterizationStateCreateFlags(), VK_FALSE, VK_FALSE, vk::PolygonMode::eFill,
      vk::CullModeFlagBits::eBack, frontFace, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);

  vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo({}, vk::SampleCountFlagBits::e1);

  vk::StencilOpState stencilOpState(vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep,
                                    vk::CompareOp::eAlways);
  vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
      vk::PipelineDepthStencilStateCreateFlags(), static_cast<vk::Bool32>(depthBuffered),
      static_cast<vk::Bool32>(depthBuffered), vk::CompareOp::eLessOrEqual, VK_FALSE, VK_FALSE, stencilOpState,
      stencilOpState);

  vk::ColorComponentFlags colorComponentFlags(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                              vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
  vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
      VK_FALSE, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eZero,
      vk::BlendFactor::eZero, vk::BlendOp::eAdd, colorComponentFlags);
  vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
      vk::PipelineColorBlendStateCreateFlags(), VK_FALSE, vk::LogicOp::eNoOp, pipelineColorBlendAttachmentState,
      {{1.0f, 1.0f, 1.0f, 1.0f}});

  std::array<vk::DynamicState, 2> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
  vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(vk::PipelineDynamicStateCreateFlags(),
                                                                    dynamicStates);

  vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo(
      vk::PipelineCreateFlags(), pipelineShaderStageCreateInfos, &pipelineVertexInputStateCreateInfo,
      &pipelineInputAssemblyStateCreateInfo, nullptr, &pipelineViewportStateCreateInfo,
      &pipelineRasterizationStateCreateInfo, &pipelineMultisampleStateCreateInfo, &pipelineDepthStencilStateCreateInfo,
      &pipelineColorBlendStateCreateInfo, &pipelineDynamicStateCreateInfo, pipelineLayout, renderPass);

  return assertSuccess(device.createGraphicsPipelineUnique(pipelineCache, graphicsPipelineCreateInfo));
}

std::vector<char const*> gatherExtensions(std::vector<std::string> const& extensions
#if !defined(NDEBUG)
                                          ,
                                          std::vector<vk::ExtensionProperties> const& extensionProperties
#endif
) {
  std::vector<char const*> enabledExtensions;
  enabledExtensions.reserve(extensions.size());
  for (auto const& ext : extensions) {
    assert(std::find_if(extensionProperties.begin(), extensionProperties.end(),
                        [ext](vk::ExtensionProperties const& ep) { return ext == ep.extensionName; }) !=
           extensionProperties.end());
    enabledExtensions.push_back(ext.data());
  }
#if !defined(NDEBUG)
  if (std::find(extensions.begin(), extensions.end(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == extensions.end() &&
      std::find_if(extensionProperties.begin(), extensionProperties.end(), [](vk::ExtensionProperties const& ep) {
        return (strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, ep.extensionName) == 0);
      }) != extensionProperties.end()) {
    enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
#endif
  return enabledExtensions;
}

std::vector<char const*> gatherLayers(std::vector<std::string> const& layers
#if !defined(NDEBUG)
                                      ,
                                      std::vector<vk::LayerProperties> const& layerProperties
#endif
) {
  std::vector<char const*> enabledLayers;
  enabledLayers.reserve(layers.size());
  for (auto const& layer : layers) {
    assert(std::find_if(layerProperties.begin(), layerProperties.end(), [layer](vk::LayerProperties const& lp) {
             return layer == lp.layerName;
           }) != layerProperties.end());
    enabledLayers.push_back(layer.data());
  }
#if !defined(NDEBUG)
  // Enable standard validation layer to find as much errors as possible!
  if (std::find(layers.begin(), layers.end(), "VK_LAYER_KHRONOS_validation") == layers.end() &&
      std::find_if(layerProperties.begin(), layerProperties.end(), [](vk::LayerProperties const& lp) {
        return (strcmp("VK_LAYER_KHRONOS_validation", lp.layerName) == 0);
      }) != layerProperties.end()) {
    enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
  }
#endif
  return enabledLayers;
}

vk::UniqueInstance createInstance(std::string const& appName, std::string const& engineName,
                                  std::vector<std::string> const& layers, std::vector<std::string> const& extensions,
                                  uint32_t apiVersion) {
#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
  static vk::DynamicLoader dl;
  auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
  VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
#endif

  vk::ApplicationInfo applicationInfo(appName.c_str(), 1, engineName.c_str(), 1, apiVersion);
  std::vector<char const*> enabledLayers = vk::su::gatherLayers(layers
#if !defined(NDEBUG)
                                                                , vk::enumerateInstanceLayerProperties().value
#endif
                                                                );
  std::vector<char const*> enabledExtensions =
      vk::su::gatherExtensions(extensions
#if !defined(NDEBUG)
                               , vk::enumerateInstanceExtensionProperties().value
#endif
                               );

  vk::UniqueInstance instance = assertSuccess(vk::createInstanceUnique(
      makeInstanceCreateInfoChain(applicationInfo, enabledLayers, enabledExtensions).get<vk::InstanceCreateInfo>()));

#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
  // initialize function pointers for instance
  VULKAN_HPP_DEFAULT_DISPATCHER.init(instance.get());
#endif

  return instance;
}

vk::UniqueRenderPass createRenderPass(vk::Device const& device, vk::Format colorFormat, vk::Format depthFormat,
                                      vk::AttachmentLoadOp colorLoadOp, vk::AttachmentStoreOp colorStoreOp,
                                      vk::AttachmentLoadOp depthLoadOp, vk::AttachmentStoreOp depthStoreOp,
                                      vk::ImageLayout colorFinalLayout) {
  std::vector<vk::AttachmentDescription> attachmentDescriptions;
  assert(colorFormat != vk::Format::eUndefined);
  attachmentDescriptions.emplace_back(vk::AttachmentDescriptionFlags(), colorFormat, vk::SampleCountFlagBits::e1,
                                      colorLoadOp, colorStoreOp, vk::AttachmentLoadOp::eDontCare,
                                      vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, colorFinalLayout);
  if (depthFormat != vk::Format::eUndefined) {
    attachmentDescriptions.emplace_back(
        vk::AttachmentDescriptionFlags(), depthFormat, vk::SampleCountFlagBits::e1, depthLoadOp, depthStoreOp,
        vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
  }
  vk::AttachmentReference colorAttachment(0, vk::ImageLayout::eColorAttachmentOptimal);
  vk::AttachmentReference depthAttachment(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
  vk::SubpassDescription subpassDescription(vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, {},
                                            colorAttachment, {},
                                            (depthFormat != vk::Format::eUndefined) ? &depthAttachment : nullptr);
  return assertSuccess(device.createRenderPassUnique(
      vk::RenderPassCreateInfo(vk::RenderPassCreateFlags(), attachmentDescriptions, subpassDescription)));
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                           VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                           VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
                                                           void* /*pUserData*/) {
#if !defined(NDEBUG)
  if (pCallbackData->messageIdNumber == 648835635) {
    // UNASSIGNED-khronos-Validation-debug-build-warning-message
    return VK_FALSE;
  }
  if (pCallbackData->messageIdNumber == 767975156) {
    // UNASSIGNED-BestPractices-vkCreateInstance-specialuse-extension
    return VK_FALSE;
  }
#endif

  std::cerr << vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)) << ": "
            << vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageTypes)) << ":\n";
  std::cerr << "\t"
            << "messageIDName   = <" << pCallbackData->pMessageIdName << ">\n";
  std::cerr << "\t"
            << "messageIdNumber = " << pCallbackData->messageIdNumber << "\n";
  std::cerr << "\t"
            << "message         = <" << pCallbackData->pMessage << ">\n";
  if (0 < pCallbackData->queueLabelCount) {
    std::cerr << "\t"
              << "Queue Labels:\n";
    for (uint8_t i = 0; i < pCallbackData->queueLabelCount; i++) {
      std::cerr << "\t\t"
                << "labelName = <" << pCallbackData->pQueueLabels[i].pLabelName << ">\n";
    }
  }
  if (0 < pCallbackData->cmdBufLabelCount) {
    std::cerr << "\t"
              << "CommandBuffer Labels:\n";
    for (uint8_t i = 0; i < pCallbackData->cmdBufLabelCount; i++) {
      std::cerr << "\t\t"
                << "labelName = <" << pCallbackData->pCmdBufLabels[i].pLabelName << ">\n";
    }
  }
  if (0 < pCallbackData->objectCount) {
    std::cerr << "\t"
              << "Objects:\n";
    for (uint8_t i = 0; i < pCallbackData->objectCount; i++) {
      std::cerr << "\t\t"
                << "Object " << i << "\n";
      std::cerr << "\t\t\t"
                << "objectType   = "
                << vk::to_string(static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType)) << "\n";
      std::cerr << "\t\t\t"
                << "objectHandle = " << pCallbackData->pObjects[i].objectHandle << "\n";
      if (pCallbackData->pObjects[i].pObjectName != nullptr) {
        std::cerr << "\t\t\t"
                  << "objectName   = <" << pCallbackData->pObjects[i].pObjectName << ">\n";
      }
    }
  }
  return VK_TRUE;
}

uint32_t findGraphicsQueueFamilyIndex(std::vector<vk::QueueFamilyProperties> const& queueFamilyProperties) {
  // get the first index into queueFamiliyProperties which supports graphics
  const auto graphicsQueueFamilyProperty =
      std::find_if(queueFamilyProperties.begin(), queueFamilyProperties.end(),
                   [](vk::QueueFamilyProperties const& qfp) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; });
  assert(graphicsQueueFamilyProperty != queueFamilyProperties.end());
  return static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
}

std::pair<uint32_t, uint32_t> findGraphicsAndPresentQueueFamilyIndex(vk::PhysicalDevice physicalDevice,
                                                                     vk::SurfaceKHR const& surface) {
  std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
  assert(queueFamilyProperties.size() < std::numeric_limits<uint32_t>::max());

  uint32_t graphicsQueueFamilyIndex = findGraphicsQueueFamilyIndex(queueFamilyProperties);
  if (physicalDevice.getSurfaceSupportKHR(graphicsQueueFamilyIndex, surface).value == VK_TRUE) {
    return std::make_pair(graphicsQueueFamilyIndex,
                          graphicsQueueFamilyIndex); // the first graphicsQueueFamilyIndex does also support presents
  }

  // the graphicsQueueFamilyIndex doesn't support present -> look for an other family index that supports both
  // graphics and present
  for (size_t i = 0; i < queueFamilyProperties.size(); i++) {
    if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
        physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface).value == VK_TRUE) {
      return std::make_pair(static_cast<uint32_t>(i), static_cast<uint32_t>(i));
    }
  }

  // there's nothing like a single family index that supports both graphics and present -> look for an other family
  // index that supports present
  for (size_t i = 0; i < queueFamilyProperties.size(); i++) {
    if (physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface).value == VK_TRUE) {
      return std::make_pair(graphicsQueueFamilyIndex, static_cast<uint32_t>(i));
    }
  }

  assert(false && "Could not find queues for both graphics or present -> terminating");
  return {};
}

uint32_t findMemoryType(vk::PhysicalDeviceMemoryProperties const& memoryProperties, uint32_t typeBits,
                        vk::MemoryPropertyFlags requirementsMask) {
  auto typeIndex = uint32_t(~0);
  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
    if (((typeBits & 1) != 0u) &&
        ((memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask)) {
      typeIndex = i;
      break;
    }
    typeBits >>= 1;
  }
  assert(typeIndex != uint32_t(~0));
  return typeIndex;
}

std::vector<std::string> getDeviceExtensions() { return {VK_KHR_SWAPCHAIN_EXTENSION_NAME}; }

std::vector<std::string> getInstanceExtensions() {
  std::vector<std::string> extensions;
  extensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  extensions.emplace_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
  extensions.emplace_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
  extensions.emplace_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MIR_KHR)
  extensions.emplace_back(VK_KHR_MIR_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_VI_NN)
  extensions.emplace_back(VK_NN_VI_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
  extensions.emplace_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
  extensions.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
  extensions.emplace_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
  extensions.emplace_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
  extensions.emplace_back(VK_EXT_ACQUIRE_XLIB_DISPLAY_EXTENSION_NAME);
#endif
  return extensions;
}

vk::Format pickDepthFormat(vk::PhysicalDevice const& physicalDevice) {
  std::vector<vk::Format> candidates = {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint,
                                        vk::Format::eD24UnormS8Uint};
  for (vk::Format format : candidates) {
    vk::FormatProperties props = physicalDevice.getFormatProperties(format);

    if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
      return format;
    }
  }
  return vk::Format::eUndefined;
}

vk::PresentModeKHR pickPresentMode(std::vector<vk::PresentModeKHR> const& presentModes) {
  vk::PresentModeKHR pickedMode = vk::PresentModeKHR::eFifo;
  for (const auto& presentMode : presentModes) {
    if (presentMode == vk::PresentModeKHR::eMailbox) {
      pickedMode = presentMode;
      break;
    }

    if (presentMode == vk::PresentModeKHR::eImmediate) {
      pickedMode = presentMode;
    }
  }
  return pickedMode;
}

vk::SurfaceFormatKHR pickSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& formats) {
  assert(!formats.empty());
  vk::SurfaceFormatKHR pickedFormat = formats[0];
  if (formats.size() == 1) {
    if (formats[0].format == vk::Format::eUndefined) {
      pickedFormat.format = vk::Format::eB8G8R8A8Unorm;
      pickedFormat.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
    }
  } else {
    // request several formats, the first found will be used
    vk::Format requestedFormats[] = {vk::Format::eB8G8R8A8Unorm, vk::Format::eR8G8B8A8Unorm, vk::Format::eB8G8R8Unorm,
                                     vk::Format::eR8G8B8Unorm};
    vk::ColorSpaceKHR requestedColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
    for (size_t i = 0; i < sizeof(requestedFormats) / sizeof(requestedFormats[0]); i++) {
      vk::Format requestedFormat = requestedFormats[i];
      auto it = std::find_if(formats.begin(), formats.end(),
                             [requestedFormat, requestedColorSpace](vk::SurfaceFormatKHR const& f) {
                               return (f.format == requestedFormat) && (f.colorSpace == requestedColorSpace);
                             });
      if (it != formats.end()) {
        pickedFormat = *it;
        break;
      }
    }
  }
  assert(pickedFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear);
  return pickedFormat;
}

void setImageLayout(vk::CommandBuffer const& commandBuffer, vk::Image image, vk::Format format,
                    vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout) {
  vk::AccessFlags sourceAccessMask;
  switch (oldImageLayout) {
  case vk::ImageLayout::eTransferDstOptimal:
    sourceAccessMask = vk::AccessFlagBits::eTransferWrite;
    break;
  case vk::ImageLayout::ePreinitialized:
    sourceAccessMask = vk::AccessFlagBits::eHostWrite;
    break;
  case vk::ImageLayout::eGeneral: // sourceAccessMask is empty
  case vk::ImageLayout::eUndefined:
    break;
  default:
    assert(false);
    break;
  }

  vk::PipelineStageFlags sourceStage;
  switch (oldImageLayout) {
  case vk::ImageLayout::eGeneral:
  case vk::ImageLayout::ePreinitialized:
    sourceStage = vk::PipelineStageFlagBits::eHost;
    break;
  case vk::ImageLayout::eTransferDstOptimal:
    sourceStage = vk::PipelineStageFlagBits::eTransfer;
    break;
  case vk::ImageLayout::eUndefined:
    sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
    break;
  default:
    assert(false);
    break;
  }

  vk::AccessFlags destinationAccessMask;
  switch (newImageLayout) {
  case vk::ImageLayout::eColorAttachmentOptimal:
    destinationAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
    break;
  case vk::ImageLayout::eDepthStencilAttachmentOptimal:
    destinationAccessMask =
        vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    break;
  case vk::ImageLayout::eGeneral: // empty destinationAccessMask
  case vk::ImageLayout::ePresentSrcKHR:
    break;
  case vk::ImageLayout::eShaderReadOnlyOptimal:
    destinationAccessMask = vk::AccessFlagBits::eShaderRead;
    break;
  case vk::ImageLayout::eTransferSrcOptimal:
    destinationAccessMask = vk::AccessFlagBits::eTransferRead;
    break;
  case vk::ImageLayout::eTransferDstOptimal:
    destinationAccessMask = vk::AccessFlagBits::eTransferWrite;
    break;
  default:
    assert(false);
    break;
  }

  vk::PipelineStageFlags destinationStage;
  switch (newImageLayout) {
  case vk::ImageLayout::eColorAttachmentOptimal:
    destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    break;
  case vk::ImageLayout::eDepthStencilAttachmentOptimal:
    destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    break;
  case vk::ImageLayout::eGeneral:
    destinationStage = vk::PipelineStageFlagBits::eHost;
    break;
  case vk::ImageLayout::ePresentSrcKHR:
    destinationStage = vk::PipelineStageFlagBits::eBottomOfPipe;
    break;
  case vk::ImageLayout::eShaderReadOnlyOptimal:
    destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    break;
  case vk::ImageLayout::eTransferDstOptimal:
  case vk::ImageLayout::eTransferSrcOptimal:
    destinationStage = vk::PipelineStageFlagBits::eTransfer;
    break;
  default:
    assert(false);
    break;
  }

  vk::ImageAspectFlags aspectMask;
  if (newImageLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
    aspectMask = vk::ImageAspectFlagBits::eDepth;
    if (format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint) {
      aspectMask |= vk::ImageAspectFlagBits::eStencil;
    }
  } else {
    aspectMask = vk::ImageAspectFlagBits::eColor;
  }

  vk::ImageSubresourceRange imageSubresourceRange(aspectMask, 0, 1, 0, 1);
  vk::ImageMemoryBarrier imageMemoryBarrier(sourceAccessMask, destinationAccessMask, oldImageLayout, newImageLayout,
                                            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image,
                                            imageSubresourceRange);
  return commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, nullptr, nullptr, imageMemoryBarrier);
}

void submitAndWait(vk::Device const& device, vk::Queue const& queue, vk::CommandBuffer const& commandBuffer) {
  vk::Fence fence = device.createFence(vk::FenceCreateInfo()).value;
  queue.submit(vk::SubmitInfo(0, nullptr, nullptr, 1, &commandBuffer), fence);
  while (vk::Result::eTimeout == device.waitForFences(fence, VK_TRUE, vk::su::FenceTimeout)) {}
  device.destroyFence(fence);
}

void updateDescriptorSets(
    vk::Device const& device, vk::DescriptorSet const& descriptorSet,
    std::vector<std::tuple<vk::DescriptorType, vk::Buffer const&, vk::DeviceSize>> const& bufferData,
    uint32_t bindingOffset) {
  std::vector<vk::DescriptorBufferInfo> bufferInfos;
  bufferInfos.reserve(bufferData.size());

  std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
  writeDescriptorSets.reserve(bufferData.size() + 1);
  uint32_t dstBinding = bindingOffset;
  for (auto const& bd : bufferData) {
    bufferInfos.emplace_back(std::get<1>(bd), 0, std::get<2>(bd));
    writeDescriptorSets.emplace_back(descriptorSet, dstBinding++, 0, 1, std::get<0>(bd), nullptr, &bufferInfos.back());
  }

  device.updateDescriptorSets(writeDescriptorSets, nullptr);
}

BufferData::BufferData(vk::PhysicalDevice const& physicalDevice, vk::Device const& device, vk::DeviceSize size,
                       vk::BufferUsageFlags usage, vk::MemoryPropertyFlags propertyFlags)
: size(size)
#if !defined(NDEBUG)
, m_usage(usage)
, m_propertyFlags(propertyFlags)
#endif
{
  buffer = assertSuccess(device.createBufferUnique(vk::BufferCreateInfo(vk::BufferCreateFlags(), size, usage)));
  deviceMemory = vk::su::allocateDeviceMemory(device, physicalDevice.getMemoryProperties(),
                                              device.getBufferMemoryRequirements(buffer.get()), propertyFlags);
  device.bindBufferMemory(buffer.get(), deviceMemory.get(), 0);
}

DepthBufferData::DepthBufferData(vk::PhysicalDevice const& physicalDevice, vk::Device const& device, vk::Format format,
                                 vk::Extent2D const& extent)
: ImageData(physicalDevice, device, format, extent, vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageLayout::eUndefined,
            vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eDepth) {}

ImageData::ImageData(vk::PhysicalDevice const& physicalDevice, vk::Device const& device, vk::Format format_,
                     vk::Extent2D const& extent, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                     vk::ImageLayout initialLayout, vk::MemoryPropertyFlags memoryProperties,
                     vk::ImageAspectFlags aspectMask)
: format(format_) {
  vk::ImageCreateInfo imageCreateInfo(vk::ImageCreateFlags(), vk::ImageType::e2D, format, vk::Extent3D(extent, 1), 1, 1,
                                      vk::SampleCountFlagBits::e1, tiling, usage | vk::ImageUsageFlagBits::eSampled,
                                      vk::SharingMode::eExclusive, {}, initialLayout);
  image = assertSuccess(device.createImageUnique(imageCreateInfo));

  auto memoryRequirements = device.getImageMemoryRequirements(image.get());
  deviceSize = memoryRequirements.size;
  deviceMemory = vk::su::allocateDeviceMemory(device, physicalDevice.getMemoryProperties(), memoryRequirements, memoryProperties);

  device.bindImageMemory(image.get(), deviceMemory.get(), 0);

  vk::ComponentMapping componentMapping(ComponentSwizzle::eR, ComponentSwizzle::eG, ComponentSwizzle::eB,
                                        ComponentSwizzle::eA);
  vk::ImageSubresourceRange imageSubresourceRange(aspectMask, 0, 1, 0, 1);
  vk::ImageViewCreateInfo imageViewCreateInfo({}, image.get(), vk::ImageViewType::e2D, format, componentMapping,
                                              imageSubresourceRange);
  imageView = assertSuccess(device.createImageViewUnique(imageViewCreateInfo));
}

TextureData::TextureData(vk::PhysicalDevice const& physicalDevice, vk::Device const& device,
                         vk::Extent2D const& extent_, vk::ImageUsageFlags usageFlags,
                         vk::FormatFeatureFlags formatFeatureFlags, bool anisotropyEnable, bool forceStaging)
: format(vk::Format::eR8G8B8A8Unorm), extent(extent_) {
  vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(format);

  formatFeatureFlags |= vk::FormatFeatureFlagBits::eSampledImage;
  needsStaging = forceStaging || ((formatProperties.linearTilingFeatures & formatFeatureFlags) != formatFeatureFlags);
  vk::ImageTiling imageTiling;
  vk::ImageLayout initialLayout;
  vk::MemoryPropertyFlags requirements;
  if (needsStaging) {
    assert((formatProperties.optimalTilingFeatures & formatFeatureFlags) == formatFeatureFlags);
    stagingBufferData = std::make_unique<BufferData>(physicalDevice, device, extent.width * extent.height * 4,
                                                     vk::BufferUsageFlagBits::eTransferSrc);
    imageTiling = vk::ImageTiling::eOptimal;
    usageFlags |= vk::ImageUsageFlagBits::eTransferDst;
    initialLayout = vk::ImageLayout::eUndefined;
  } else {
    imageTiling = vk::ImageTiling::eLinear;
    initialLayout = vk::ImageLayout::ePreinitialized;
    requirements = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;
  }
  imageData = std::make_unique<ImageData>(physicalDevice, device, format, extent, imageTiling,
                                          usageFlags | vk::ImageUsageFlagBits::eSampled, initialLayout, requirements,
                                          vk::ImageAspectFlagBits::eColor);

  sampler = assertSuccess(device.createSamplerUnique(vk::SamplerCreateInfo(
      vk::SamplerCreateFlags(), vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
      vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, 0.0f,
      static_cast<vk::Bool32>(anisotropyEnable), 16.0f, VK_FALSE, vk::CompareOp::eNever, 0.0f, 0.0f,
      vk::BorderColor::eFloatOpaqueBlack)));
}

UUID::UUID(uint8_t const data[VK_UUID_SIZE]) { memcpy(m_data, data, VK_UUID_SIZE * sizeof(uint8_t)); }

vk::DebugUtilsMessengerCreateInfoEXT makeDebugUtilsMessengerCreateInfoEXT() {
  return {{},
          vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
          vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
              vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
          &vk::su::debugUtilsMessengerCallback};
}

#if defined(NDEBUG)
vk::StructureChain<vk::InstanceCreateInfo>
#elif defined(VULKAN_HPP_UTILS_USE_BEST_PRACTICES)
vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT, vk::ValidationFeaturesEXT>
#else
vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT>
#endif
makeInstanceCreateInfoChain(vk::ApplicationInfo const& applicationInfo, std::vector<char const*> const& enabledLayers,
                            std::vector<char const*> const& enabledExtensions) {
#if defined(NDEBUG)
  // in non-debug mode just use the InstanceCreateInfo for instance creation
  vk::StructureChain<vk::InstanceCreateInfo> instanceCreateInfo(
      {{}, &applicationInfo, enabledLayers, enabledExtensions});
#else
  // in debug mode, addionally use the debugUtilsMessengerCallback in instance creation!
  vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
  vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                                     vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                                     vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
#if defined(VULKAN_HPP_UTILS_USE_BEST_PRACTICES)
  vk::ValidationFeatureEnableEXT validationFeatureEnable = vk::ValidationFeatureEnableEXT::eBestPractices;
  vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT, vk::ValidationFeaturesEXT>
      instanceCreateInfo({{}, &applicationInfo, enabledLayers, enabledExtensions},
                         {{}, severityFlags, messageTypeFlags, &vk::su::debugUtilsMessengerCallback},
                         {validationFeatureEnable});
#else
  vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT> instanceCreateInfo(
      {{}, &applicationInfo, enabledLayers, enabledExtensions},
      {{}, severityFlags, messageTypeFlags, &vk::su::debugUtilsMessengerCallback});
#endif
#endif
  return instanceCreateInfo;
}

} // namespace vk::su

std::ostream& operator<<(std::ostream& os, vk::su::UUID const& uuid) {
  os << std::setfill('0') << std::hex;
  for (uint32_t j = 0; j < VK_UUID_SIZE; ++j) {
    os << std::setw(2) << static_cast<uint32_t>(uuid.m_data[j]);
    if (j == 3 || j == 5 || j == 7 || j == 9) {
      std::cout << '-';
    }
  }
  os << std::setfill(' ') << std::dec;
  return os;
}
