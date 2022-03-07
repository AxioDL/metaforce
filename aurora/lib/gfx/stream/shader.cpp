#include "shader.hpp"

#include "../../gpu.hpp"
#include "../common.hpp"

#include <magic_enum.hpp>
#include <utility>

namespace aurora::gfx {
extern std::unordered_map<ShaderRef, wgpu::ShaderModule> g_gxCachedShaders;
} // namespace aurora::gfx

namespace aurora::gfx::stream {
static logvisor::Module Log("aurora::gfx::stream");

using gpu::g_device;

wgpu::RenderPipeline create_pipeline(const State& state, [[maybe_unused]] PipelineConfig config) {
  const auto [shader, info] = build_shader(config.shaderConfig);

  std::array<wgpu::VertexAttribute, 4> attributes{};
  attributes[0] = wgpu::VertexAttribute{
      .format = wgpu::VertexFormat::Float32x3,
      .offset = 0,
      .shaderLocation = 0,
  };
  uint64_t offset = 12;
  uint32_t shaderLocation = 1;
  if (info.usesNormal) {
    attributes[shaderLocation] = wgpu::VertexAttribute{
        .format = wgpu::VertexFormat::Float32x3,
        .offset = offset,
        .shaderLocation = shaderLocation,
    };
    offset += 12;
    shaderLocation++;
  }
  if (info.usesVtxColor) {
    attributes[shaderLocation] = wgpu::VertexAttribute{
        .format = wgpu::VertexFormat::Float32x4,
        .offset = offset,
        .shaderLocation = shaderLocation,
    };
    offset += 16;
    shaderLocation++;
  }
  // TODO only sample 1?
  for (int i = 0; i < info.sampledTextures.size(); ++i) {
    if (!info.sampledTextures.test(i)) {
      continue;
    }
    attributes[shaderLocation] = wgpu::VertexAttribute{
        .format = wgpu::VertexFormat::Float32x2,
        .offset = offset,
        .shaderLocation = shaderLocation,
    };
    offset += 8;
    shaderLocation++;
  }
  const std::array vertexBuffers{wgpu::VertexBufferLayout{
      .arrayStride = offset,
      .attributeCount = shaderLocation,
      .attributes = attributes.data(),
  }};

  return build_pipeline(config, info, vertexBuffers, shader, "Stream Pipeline");
}

State construct_state() {
  const auto samplerBinding = wgpu::SamplerBindingLayout{
      .type = wgpu::SamplerBindingType::Filtering,
  };
  const std::array samplerLayoutEntries{
      wgpu::BindGroupLayoutEntry{
          .binding = 0,
          .visibility = wgpu::ShaderStage::Fragment,
          .sampler = samplerBinding,
      },
  };
  const auto samplerLayoutDescriptor = wgpu::BindGroupLayoutDescriptor{
      .label = "Stream Sampler Bind Group Layout",
      .entryCount = samplerLayoutEntries.size(),
      .entries = samplerLayoutEntries.data(),
  };
  auto samplerLayout = g_device.CreateBindGroupLayout(&samplerLayoutDescriptor);

  const auto textureBinding = wgpu::TextureBindingLayout{
      .sampleType = wgpu::TextureSampleType::Float,
      .viewDimension = wgpu::TextureViewDimension::e2D,
  };
  const std::array textureLayoutEntries{
      wgpu::BindGroupLayoutEntry{
          .binding = 0,
          .visibility = wgpu::ShaderStage::Fragment,
          .texture = textureBinding,
      },
  };
  const auto textureLayoutDescriptor = wgpu::BindGroupLayoutDescriptor{
      .label = "Stream Texture Bind Group Layout",
      .entryCount = textureLayoutEntries.size(),
      .entries = textureLayoutEntries.data(),
  };
  auto textureLayout = g_device.CreateBindGroupLayout(&textureLayoutDescriptor);

  return {
      .samplerLayout = samplerLayout,
      .textureLayout = textureLayout,
  };
}

void render(const State& state, const DrawData& data, const wgpu::RenderPassEncoder& pass) {
  if (!bind_pipeline(data.pipeline, pass)) {
    return;
  }

  const std::array offsets{data.uniformRange.first};
  pass.SetBindGroup(0, find_bind_group(data.bindGroups.uniformBindGroup), offsets.size(), offsets.data());
  if (data.bindGroups.samplerBindGroup && data.bindGroups.textureBindGroup) {
    pass.SetBindGroup(1, find_bind_group(data.bindGroups.samplerBindGroup));
    pass.SetBindGroup(2, find_bind_group(data.bindGroups.textureBindGroup));
  }
  pass.SetVertexBuffer(0, g_vertexBuffer, data.vertRange.first, data.vertRange.second);
  pass.Draw(data.vertexCount);
}
} // namespace aurora::gfx::stream
