#pragma once

#include "../common.hpp"
#include "../gx.hpp"

#include <unordered_map>

namespace aurora::gfx::model {
struct DrawData {
  PipelineRef pipeline;
  Range vertRange;
  Range uniformRange;
  uint32_t vertexCount;
  uint32_t uniformSize;
  BindGroupRef samplerBindGroup;
  BindGroupRef textureBindGroup;
};

struct PipelineConfig : gx::PipelineConfig {
  ShaderRef shader;
  uint32_t uniformSize;
};

struct CachedBindGroup {
  wgpu::BindGroupLayout layout;
  wgpu::BindGroup bindGroup;
  CachedBindGroup(wgpu::BindGroupLayout layout, wgpu::BindGroup&& group)
  : layout(std::move(layout)), bindGroup(std::move(group)) {}
};
struct State {
  wgpu::BindGroupLayout samplerLayout;
  wgpu::BindGroupLayout textureLayout;
  mutable std::unordered_map<uint32_t, CachedBindGroup> uniform;
  mutable std::unordered_map<uint64_t, wgpu::Sampler> sampler;
};

State construct_state();
wgpu::RenderPipeline create_pipeline(const State& state, [[maybe_unused]] PipelineConfig config);
void render(const State& state, const DrawData& data, const wgpu::RenderPassEncoder& pass);
} // namespace aurora::gfx::model
