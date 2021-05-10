#include "VISIRendererVulkan.hpp"
#include "utils.hpp"

extern "C" {
extern const uint8_t VK_FRAGMENT_SPV[];
extern const size_t VK_FRAGMENT_SPV_SZ;
extern const uint8_t VK_VERTEX_SPV[];
extern const size_t VK_VERTEX_SPV_SZ;
}

static char const* AppName = "VISIGen";

static zeus::CMatrix4f g_Proj;

constexpr zeus::CMatrix4f DepthCorrect(1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.f, 0.f, 0.f,
                                       1.f);

static void CalculateProjMatrix() {
  float znear = 0.2f;
  float zfar = 1000.f;
  float tfov = std::tan(zeus::degToRad(90.f * 0.5f));
  float top = znear * tfov;
  float bottom = -top;
  float right = znear * tfov;
  float left = -right;

  float rml = right - left;
  float rpl = right + left;
  float tmb = top - bottom;
  float tpb = top + bottom;
  float fpn = zfar + znear;
  float fmn = zfar - znear;

  zeus::CMatrix4f mat2{
      2.f * znear / rml,         0.f, rpl / rml, 0.f,  0.f, 2.f * znear / tmb, tpb / tmb, 0.f, 0.f, 0.f, -fpn / fmn,
      -2.f * zfar * znear / fmn, 0.f, 0.f,       -1.f, 0.f};
  g_Proj = DepthCorrect * mat2;
}

static constexpr std::array<uint16_t, 20> AABBIdxs{0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 1, 7, 3, 5, 5, 0, 0, 2, 6, 4};

static const zeus::CMatrix4f LookMATs[] = {
    {// Forward
     1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f},
    {// Backward
     -1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f},
    {// Up
     1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.f, 0.f, 1.f},
    {// Down
     1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f},
    {// Left
     0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f},
    {// Right
     0.f, -1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, -1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f},
};

template <typename T>
static vk::UniqueShaderModule createShaderModule(const vk::UniqueDevice& device, const T* code, size_t size) {
  return vk::su::assertSuccess(device->createShaderModuleUnique(
      vk::ShaderModuleCreateInfo().setCodeSize(size).setPCode(reinterpret_cast<const uint32_t*>(code))));
}

static int rateDeviceSuitability(const vk::PhysicalDevice& device) {
  int score = 0;
  const auto deviceProperties = device.getProperties();
  if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
    score += 1000;
  } else if (deviceProperties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
    score += 100;
  }
  //  const auto deviceFeatures = device.getFeatures();
  //  if (!deviceFeatures.geometryShader) {
  //    return 0;
  //  }
  return score;
}

static const vk::PhysicalDevice& pickPhysicalDevice(const std::vector<vk::PhysicalDevice>& devices) {
  std::multimap<int, const vk::PhysicalDevice&> candidates;
  for (const auto& device : devices) {
    int score = rateDeviceSuitability(device);
    candidates.insert(std::make_pair(score, device));
  }
  if (candidates.rbegin()->first > 0) {
    return candidates.rbegin()->second;
  }
  assert(false && "failed to find a suitable GPU!");
  return candidates.begin()->second;
}

bool VISIRendererVulkan::SetupShaders() {
  instance = vk::su::createInstance(AppName, AppName, {}, vk::su::getInstanceExtensions());
  physicalDevice = pickPhysicalDevice(vk::su::assertSuccess(instance->enumeratePhysicalDevices()));
  auto queueFamilyIndex = vk::su::findGraphicsQueueFamilyIndex(physicalDevice.getQueueFamilyProperties());
  device = vk::su::createDevice(physicalDevice, queueFamilyIndex, {});
  commandPool = vk::su::createCommandPool(device.get(), queueFamilyIndex);
  commandBuffer = vk::su::createCommandBuffer(device.get(), commandPool.get());
  graphicsQueue = device->getQueue(queueFamilyIndex, 0);
  vertexShader = createShaderModule(device, static_cast<const uint8_t*>(VK_VERTEX_SPV), VK_VERTEX_SPV_SZ);
  fragmentShader = createShaderModule(device, static_cast<const uint8_t*>(VK_FRAGMENT_SPV), VK_FRAGMENT_SPV_SZ);
  vk::Format colorFormat = vk::Format::eR8G8B8A8Unorm;
  vk::Format depthFormat = vk::Format::eD16Unorm;
  colorRenderPass = vk::su::createRenderPass(device.get(), colorFormat, depthFormat, vk::AttachmentLoadOp::eClear,
                                             vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear,
                                             vk::AttachmentStoreOp::eStore, vk::ImageLayout::eTransferSrcOptimal);
  depthRenderPass = vk::su::createRenderPass(device.get(), colorFormat, depthFormat, vk::AttachmentLoadOp::eDontCare,
                                             vk::AttachmentStoreOp::eDontCare, vk::AttachmentLoadOp::eLoad,
                                             vk::AttachmentStoreOp::eStore, vk::ImageLayout::eGeneral);
  pipelineCache = vk::su::assertSuccess(device->createPipelineCacheUnique(vk::PipelineCacheCreateInfo()));

  descriptorSetLayout = vk::su::createDescriptorSetLayout(
      device.get(), {{vk::DescriptorType::eUniformBufferDynamic, 1, vk::ShaderStageFlagBits::eVertex}});
  descriptorPool = vk::su::createDescriptorPool(device.get(),
                                                {vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1)});
  pipelineLayout = vk::su::assertSuccess(
      device->createPipelineLayoutUnique(vk::PipelineLayoutCreateInfo({}, descriptorSetLayout.get())));
  std::pair<vk::ShaderModule, vk::SpecializationInfo const*> vertexShaderData{vertexShader.get(), nullptr};
  std::pair<vk::ShaderModule, vk::SpecializationInfo const*> fragmentShaderData{fragmentShader.get(), nullptr};
  std::vector<std::pair<vk::Format, uint32_t>> vertexInputAttributeFormats{
      {vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)},
      {vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, color)},
  };
  colorPipeline = vk::su::createGraphicsPipeline(
      device.get(), pipelineCache.get(), vertexShaderData, fragmentShaderData, sizeof(Vertex),
      vertexInputAttributeFormats, vk::FrontFace::eClockwise, true, pipelineLayout.get(), colorRenderPass.get());
  depthPipeline = vk::su::createGraphicsPipeline(
      device.get(), pipelineCache.get(), vertexShaderData, fragmentShaderData, sizeof(Vertex),
      vertexInputAttributeFormats, vk::FrontFace::eClockwise, true, pipelineLayout.get(), depthRenderPass.get());
  m_extent = vk::Extent2D(768, 512);
  colorAttachment = std::make_unique<vk::su::ImageData>(
      physicalDevice, device.get(), colorFormat, m_extent, vk::ImageTiling::eOptimal,
      vk::ImageUsageFlags{vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc},
      vk::ImageLayout::eUndefined, vk::MemoryPropertyFlags{vk::MemoryPropertyFlagBits::eDeviceLocal},
      vk::ImageAspectFlags{vk::ImageAspectFlagBits::eColor});
  colorAttachmentRead = std::make_unique<vk::su::ImageData>(
      physicalDevice, device.get(), colorFormat, m_extent, vk::ImageTiling::eLinear,
      vk::ImageUsageFlags{vk::ImageUsageFlagBits::eTransferDst}, vk::ImageLayout::eUndefined,
      vk::MemoryPropertyFlags{vk::MemoryPropertyFlagBits::eHostVisible},
      vk::ImageAspectFlags{vk::ImageAspectFlagBits::eColor});
  //  depthAttachment = std::make_unique<vk::su::ImageData>(
  //      physicalDevice, device.get(), vk::Format::eD16Unorm, m_extent, vk::ImageTiling::eOptimal,
  //      vk::ImageUsageFlags{vk::ImageUsageFlagBits::eDepthStencilAttachment}, vk::ImageLayout::eUndefined,
  //      vk::MemoryPropertyFlags{vk::MemoryPropertyFlagBits::eDeviceLocal},
  //      vk::ImageAspectFlags{vk::ImageAspectFlagBits::eDepth});
  depthAttachment = std::make_unique<vk::su::DepthBufferData>(physicalDevice, device.get(), depthFormat, m_extent);
  colorFramebuffer = vk::su::createFramebuffer(device.get(), colorRenderPass.get(), colorAttachment->imageView.get(),
                                               depthAttachment->imageView.get(), m_extent);
  depthFramebuffer = vk::su::createFramebuffer(device.get(), depthRenderPass.get(), colorAttachment->imageView.get(),
                                               depthAttachment->imageView.get(), m_extent);
  return true;
}

bool VISIRendererVulkan::SetupVertexBuffersAndFormats() {
  size_t vertCount = 0;
  size_t indexCount = 0;
  for (const auto& model : m_models) {
    vertCount += model.verts.size();
    indexCount += model.idxs.size();
  }
  m_entityVertStart = vertCount;
  vertCount += 8 * m_entities.size();
  vertCount += m_lights.size();
  vertexBuffer = createBuffer(vertCount * sizeof(Vertex), {vk::BufferUsageFlagBits::eVertexBuffer});
  indexBuffer = createBuffer(indexCount * sizeof(uint32_t), {vk::BufferUsageFlagBits::eIndexBuffer});

  auto* vertMap = static_cast<Vertex*>(vertexBuffer->map(device.get()));
  auto* indexMap = static_cast<uint32_t*>(indexBuffer->map(device.get()));
  for (const auto& model : m_models) {
    memcpy(vertMap, model.verts.data(), model.verts.size() * sizeof(Vertex));
    memcpy(indexMap, model.idxs.data(), model.idxs.size() * sizeof(uint32_t));
    vertMap += model.verts.size();
    indexMap += model.idxs.size();
  }
  auto idx = static_cast<uint32_t>(m_models.size());
  for (const auto& ent : m_entities) {
    auto verts = VISIRenderer::AABBToVerts(ent.aabb, VISIRenderer::ColorForIndex(idx++));
    memcpy(vertMap, verts.data(), verts.size() * sizeof(Vertex));
    vertMap += verts.size();
  }
  for (const auto& light : m_lights) {
    auto* vert = vertMap++;
    vert->pos = light.point;
    vert->color = VISIRenderer::ColorForIndex(idx++);
  }
  vertexBuffer->unmap(device.get());
  indexBuffer->unmap(device.get());

  uniformBuffer = createBuffer(sizeof(Uniforms) * 6, {vk::BufferUsageFlagBits::eUniformBuffer});
  aabbIndexBuffer = createBuffer(AABBIdxs.size() * sizeof(uint16_t), {vk::BufferUsageFlagBits::eIndexBuffer});
  aabbIndexBuffer->upload(device.get(), AABBIdxs);

  {
    const std::array<const vk::DescriptorSetLayout, 1> layouts{descriptorSetLayout.get()};
    uniformBufferDescriptorSet =
        std::move(vk::su::assertSuccess(device->allocateDescriptorSetsUnique(
                                            vk::DescriptorSetAllocateInfo{descriptorPool.get(), layouts}))
                      .front());
  }
  vk::su::updateDescriptorSets(
      device.get(), uniformBufferDescriptorSet.get(),
      {{vk::DescriptorType::eUniformBufferDynamic, uniformBuffer->buffer.get(), sizeof(Uniforms)}}, {});
  return true;
}

void VISIRendererVulkan::SetupRenderPass(const zeus::CVector3f& pos) {
  auto posMat = zeus::CTransform::Translate(-pos).toMatrix4f();
  auto* buffer = static_cast<Uniforms*>(uniformBuffer->map(device.get()));
  for (uint16_t j = 0; j < 6; ++j) {
    zeus::CMatrix4f modelView = LookMATs[j] * posMat;
    m_frustums[j].updatePlanes(modelView, g_Proj);
    buffer->projectionMatrix = g_Proj;
    buffer->modelViewMatrix = modelView;
    buffer++;
  }
  uniformBuffer->unmap(device.get());
}

void VISIRendererVulkan::RenderPVSOpaque(VISIRenderer::RGBA8* out, bool& needTransparent) {
  commandBuffer->begin(vk::CommandBufferBeginInfo{});

  {
    std::array<vk::ClearValue, 2> clearValues{
        vk::ClearColorValue(std::array<float, 4>{0.f, 0.f, 0.f, 1.f}),
        vk::ClearDepthStencilValue(1.f, 0),
    };
    vk::RenderPassBeginInfo renderPassBeginInfo(colorRenderPass.get(), colorFramebuffer.get(),
                                                vk::Rect2D{{0, 0}, m_extent}, clearValues.size(), clearValues.data());
    commandBuffer->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
  }
  commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, colorPipeline.get());
  commandBuffer->setScissor(0, std::array{vk::Rect2D{{}, m_extent}});
  //  commandBuffer->bindVertexBuffers(0, std::array{vertexBuffer->buffer.get()}, std::array<vk::DeviceSize, 1>{0});
  commandBuffer->bindIndexBuffer(indexBuffer->buffer.get(), 0, vk::IndexType::eUint32);

  for (uint32_t j = 0; j < 6; ++j) {
    auto x = static_cast<float>((j % 3) * 256);
    auto y = static_cast<float>((j / 3) * 256); // NOLINT(bugprone-integer-division)
    commandBuffer->setViewport(0, std::array{vk::Viewport{x, y, 256.f, 256.f, 0.f, 1.f}});
    commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout.get(), 0,
                                      std::array{uniformBufferDescriptorSet.get()},
                                      std::array<uint32_t, 1>{j * sizeof(Uniforms)});
    size_t vertexBufferOffset = 0;
    size_t indexBufferOffset = 0;
    for (const auto& model : m_models) {
      if (m_frustums[j].aabbFrustumTest(model.aabb)) {
        commandBuffer->bindVertexBuffers(0, std::array{vertexBuffer->buffer.get()},
                                         std::array<vk::DeviceSize, 1>{vertexBufferOffset});
        for (const auto& surf : model.surfaces) {
          // Non-transparents first
          if (surf.transparent) {
            needTransparent = true;
          } else {
            assert(model.topology == hecl::HMDLTopology::TriStrips);
            commandBuffer->drawIndexed(surf.count, 1, indexBufferOffset + surf.first, 0, 0);
          }
        }
      }
      vertexBufferOffset += model.verts.size() * sizeof(Vertex);
      indexBufferOffset += model.idxs.size();
    }
  }

  commandBuffer->endRenderPass();

  {
//    const vk::ImageMemoryBarrier transferSrcBarrier{
//        vk::AccessFlagBits::eMemoryRead,
//        vk::AccessFlagBits::eTransferRead,
//        vk::ImageLayout::eGeneral,
//        vk::ImageLayout::eTransferSrcOptimal,
//        0,
//        0,
//        colorAttachment->image.get(),
//        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1},
//    };
    const vk::ImageMemoryBarrier transferDstBarrier{
        vk::AccessFlags{},
        vk::AccessFlagBits::eTransferWrite,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal,
        0,
        0,
        colorAttachmentRead->image.get(),
        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1},
    };
    commandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eTransfer,
                                   vk::DependencyFlags{}, std::array<vk::MemoryBarrier, 0>{},
                                   std::array<vk::BufferMemoryBarrier, 0>{},
//                                   std::array<vk::ImageMemoryBarrier, 2>{transferSrcBarrier, transferDstBarrier}
                                   std::array<vk::ImageMemoryBarrier, 1>{transferDstBarrier}
 );
  }
  {
    const vk::ImageCopy imageCopy{vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1}, vk::Offset3D{},
                                  vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1}, vk::Offset3D{},
                                  vk::Extent3D{m_extent, 1}};
    commandBuffer->copyImage(colorAttachment->image.get(), vk::ImageLayout::eTransferSrcOptimal,
                             colorAttachmentRead->image.get(), vk::ImageLayout::eTransferDstOptimal,
                             std::array{imageCopy});
  }
//  {
////    const vk::ImageMemoryBarrier transferSrcBarrier{
////        vk::AccessFlagBits::eTransferRead,
////        vk::AccessFlags{},
////        vk::ImageLayout::eTransferSrcOptimal,
////        vk::ImageLayout::eColorAttachmentOptimal,
////        0,
////        0,
////        colorAttachment->image.get(),
////        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1},
////    };
//    const vk::ImageMemoryBarrier transferDstBarrier{
//        vk::AccessFlagBits::eTransferWrite,
//        vk::AccessFlagBits::eHostRead,
//        vk::ImageLayout::eTransferDstOptimal,
//        vk::ImageLayout::eGeneral,
//        0,
//        0,
//        colorAttachmentRead->image.get(),
//        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1},
//    };
//    commandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
//                                   vk::PipelineStageFlagBits::eHost, vk::DependencyFlags{},
//                                   std::array<vk::MemoryBarrier, 0>{}, std::array<vk::BufferMemoryBarrier, 0>{},
////                                   std::array<vk::ImageMemoryBarrier, 2>{transferSrcBarrier, transferDstBarrier}
//                                   std::array<vk::ImageMemoryBarrier, 1>{transferDstBarrier}
// );
//  }
  commandBuffer->end();
  vk::su::submitAndWait(device.get(), graphicsQueue, commandBuffer.get());

  size_t size = sizeof(VISIRenderer::RGBA8) * m_extent.height * m_extent.width;
  assert(size == colorAttachmentRead->deviceSize);
  void* imageMemory = vk::su::assertSuccess(device->mapMemory(colorAttachmentRead->deviceMemory.get(), 0, size));
  memcpy(out, imageMemory, size);
  device->unmapMemory(colorAttachmentRead->deviceMemory.get());
}

void VISIRendererVulkan::RenderPVSTransparent(const std::function<void(int)>& passFunc) {}

void VISIRendererVulkan::RenderPVSEntitiesAndLights(const std::function<void(int)>& passFunc,
                                                    const std::function<void(int, EPVSVisSetState)>& lightPassFunc) {}
