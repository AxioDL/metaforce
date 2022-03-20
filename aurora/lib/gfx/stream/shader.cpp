#include "shader.hpp"

#include "../../gpu.hpp"
#include "../common.hpp"

#include <magic_enum.hpp>
#include <utility>

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
  if (config.shaderConfig.vtxAttrs[GX::VA_NRM] == GX::DIRECT) {
    attributes[shaderLocation] = wgpu::VertexAttribute{
        .format = wgpu::VertexFormat::Float32x3,
        .offset = offset,
        .shaderLocation = shaderLocation,
    };
    offset += 12;
    shaderLocation++;
  }
  if (config.shaderConfig.vtxAttrs[GX::VA_CLR0] == GX::DIRECT) {
    attributes[shaderLocation] = wgpu::VertexAttribute{
        .format = wgpu::VertexFormat::Float32x4,
        .offset = offset,
        .shaderLocation = shaderLocation,
    };
    offset += 16;
    shaderLocation++;
  }
  for (int i = GX::VA_TEX0; i < GX::VA_TEX7; ++i) {
    if (config.shaderConfig.vtxAttrs[i] != GX::DIRECT) {
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

State construct_state() { return {}; }

void render(const State& state, const DrawData& data, const wgpu::RenderPassEncoder& pass) {
  if (!bind_pipeline(data.pipeline, pass)) {
    return;
  }

  const std::array offsets{data.uniformRange.offset};
  pass.SetBindGroup(0, find_bind_group(data.bindGroups.uniformBindGroup), offsets.size(), offsets.data());
  if (data.bindGroups.samplerBindGroup && data.bindGroups.textureBindGroup) {
    pass.SetBindGroup(1, find_bind_group(data.bindGroups.samplerBindGroup));
    pass.SetBindGroup(2, find_bind_group(data.bindGroups.textureBindGroup));
  }
  pass.SetVertexBuffer(0, g_vertexBuffer, data.vertRange.offset, data.vertRange.size);
  pass.SetIndexBuffer(g_indexBuffer, wgpu::IndexFormat::Uint16, data.indexRange.offset, data.indexRange.size);
  if (data.dstAlpha) {
    const wgpu::Color color{0.f, 0.f, 0.f, *data.dstAlpha};
    pass.SetBlendConstant(&color);
  }
  pass.DrawIndexed(data.indexCount);
}
} // namespace aurora::gfx::stream
