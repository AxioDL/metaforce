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
using gpu::g_graphicsConfig;
using gpu::utils::make_vertex_state;

wgpu::RenderPipeline create_pipeline(const State& state, [[maybe_unused]] PipelineConfig config) {
  std::array<wgpu::VertexAttribute, 4> attributes{};
  attributes[0] = wgpu::VertexAttribute{
      .format = wgpu::VertexFormat::Float32x3,
      .offset = 0,
      .shaderLocation = 0,
  };
  uint64_t offset = 12;
  uint32_t shaderLocation = 1;
  if (config.flags & metaforce::EStreamFlagBits::fHasNormal) {
    attributes[shaderLocation] = wgpu::VertexAttribute{
        .format = wgpu::VertexFormat::Float32x3,
        .offset = offset,
        .shaderLocation = shaderLocation,
    };
    offset += 12;
    shaderLocation++;
  }
  if (config.flags & metaforce::EStreamFlagBits::fHasColor) {
    attributes[shaderLocation] = wgpu::VertexAttribute{
        .format = wgpu::VertexFormat::Float32x4,
        .offset = offset,
        .shaderLocation = shaderLocation,
    };
    offset += 16;
    shaderLocation++;
  }
  if (config.flags & metaforce::EStreamFlagBits::fHasTexture) {
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

  wgpu::BindGroupLayout uniformLayout;
  if (state.uniform.contains(config.uniformSize)) {
    uniformLayout = state.uniform.at(config.uniformSize).layout;
  } else {
    const std::array uniformLayoutEntries{wgpu::BindGroupLayoutEntry{
        .binding = 0,
        .visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
        .buffer =
            wgpu::BufferBindingLayout{
                .type = wgpu::BufferBindingType::Uniform,
                .hasDynamicOffset = true,
                .minBindingSize = config.uniformSize,
            },
    }};
    const auto uniformLayoutDescriptor = wgpu::BindGroupLayoutDescriptor{
        .label = "Stream Uniform Bind Group Layout",
        .entryCount = uniformLayoutEntries.size(),
        .entries = uniformLayoutEntries.data(),
    };
    uniformLayout = g_device.CreateBindGroupLayout(&uniformLayoutDescriptor);

    const std::array uniformBindGroupEntries{wgpu::BindGroupEntry{
        .binding = 0,
        .buffer = g_uniformBuffer,
        .size = config.uniformSize,
    }};
    const auto uniformBindGroupDescriptor = wgpu::BindGroupDescriptor{
        .label = "Stream Quad Uniform Bind Group",
        .layout = uniformLayout,
        .entryCount = uniformBindGroupEntries.size(),
        .entries = uniformBindGroupEntries.data(),
    };
    auto uniformBindGroup = g_device.CreateBindGroup(&uniformBindGroupDescriptor);

    state.uniform.try_emplace(config.uniformSize, uniformLayout, std::move(uniformBindGroup));
  }

  const std::array bindGroupLayouts{
      uniformLayout,
      state.samplerLayout,
      state.textureLayout,
  };
  const auto pipelineLayoutDescriptor = wgpu::PipelineLayoutDescriptor{
      .label = "Stream Pipeline Layout",
      .bindGroupLayoutCount =
          static_cast<uint32_t>(config.flags & metaforce::EStreamFlagBits::fHasTexture ? bindGroupLayouts.size() : 1),
      .bindGroupLayouts = bindGroupLayouts.data(),
  };
  auto pipelineLayout = g_device.CreatePipelineLayout(&pipelineLayoutDescriptor);

  return build_pipeline(pipelineLayout, vertexBuffers, config, "Stream Pipeline");
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
  pass.SetBindGroup(0, state.uniform.at(data.uniformSize).bindGroup, offsets.size(), offsets.data());
  if (data.samplerBindGroup && data.textureBindGroup) {
    pass.SetBindGroup(1, find_bind_group(data.samplerBindGroup));
    pass.SetBindGroup(2, find_bind_group(data.textureBindGroup));
  }
  pass.SetVertexBuffer(0, g_vertexBuffer, data.vertRange.first, data.vertRange.second);
  pass.Draw(data.vertexCount);
}
} // namespace aurora::gfx::stream
