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

State construct_state() { return {}; }

void render(const State& state, const DrawData& data, const wgpu::RenderPassEncoder& pass) {
  OPTICK_EVENT();
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
  pass.Draw(data.vertexCount);
}
} // namespace aurora::gfx::stream
