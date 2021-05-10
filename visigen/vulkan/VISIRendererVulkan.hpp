#pragma once

#include "../VISIRenderer.hpp"

#include "utils.hpp"

#include <zeus/CFrustum.hpp>

class VISIRendererVulkan : public VISIRenderer {
  vk::PhysicalDevice physicalDevice;
  vk::UniqueInstance instance;
  vk::UniqueDevice device;
  vk::UniqueCommandPool commandPool;
  vk::UniqueCommandBuffer commandBuffer;
  vk::Queue graphicsQueue;
  vk::UniqueShaderModule vertexShader;
  vk::UniqueShaderModule fragmentShader;
  vk::UniqueRenderPass colorRenderPass;
  vk::UniqueRenderPass depthRenderPass;
  vk::UniquePipelineCache pipelineCache;
  vk::UniquePipeline colorPipeline;
  vk::UniquePipeline depthPipeline;
  vk::UniqueDescriptorSetLayout descriptorSetLayout;
  vk::UniqueDescriptorPool descriptorPool;
  vk::UniqueDescriptorSet uniformBufferDescriptorSet;
  vk::UniquePipelineLayout pipelineLayout;
  std::unique_ptr<vk::su::ImageData> colorAttachment;
  std::unique_ptr<vk::su::ImageData> colorAttachmentRead;
  std::unique_ptr<vk::su::ImageData> depthAttachment;
  std::unique_ptr<vk::su::BufferData> uniformBuffer;
  std::unique_ptr<vk::su::BufferData> vertexBuffer;
  std::unique_ptr<vk::su::BufferData> indexBuffer;
  std::unique_ptr<vk::su::BufferData> aabbIndexBuffer;
  vk::UniqueFramebuffer colorFramebuffer;
  vk::UniqueFramebuffer depthFramebuffer;

  bool SetupShaders() override;
  bool SetupVertexBuffersAndFormats() override;
  void SetupRenderPass(const zeus::CVector3f& pos) override;

private:
  using Vertex = VISIRenderer::Model::Vert;
  using Uniforms = struct {
    zeus::CMatrix4f projectionMatrix;
    zeus::CMatrix4f modelViewMatrix;
  };
  size_t m_entityVertStart;
  std::array<zeus::CFrustum, 6> m_frustums;
  vk::Extent2D m_extent;

  inline std::unique_ptr<vk::su::BufferData> createBuffer(size_t size, vk::BufferUsageFlags usageFlags) {
    return std::make_unique<vk::su::BufferData>(physicalDevice, device.get(), size, usageFlags);
  }

public:
  VISIRendererVulkan(int argc, const hecl::SystemChar** argv) : VISIRenderer(argc, argv) {}
  void RenderPVSOpaque(RGBA8* out, bool& needTransparent) override;
  void RenderPVSTransparent(const std::function<void(int)>& passFunc) override;
  void RenderPVSEntitiesAndLights(const std::function<void(int)>& passFunc,
                                  const std::function<void(int, EPVSVisSetState)>& lightPassFunc) override;
};
