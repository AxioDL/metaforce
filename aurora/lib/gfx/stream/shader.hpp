#pragma once

#include "../common.hpp"
#include "../gx.hpp"

#include <unordered_map>

namespace aurora::gfx::stream {
struct DrawData {
  PipelineRef pipeline;
  Range vertRange;
  Range uniformRange;
  uint32_t vertexCount;
  gx::GXBindGroups bindGroups;
};

struct PipelineConfig : public gx::PipelineConfig {};

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
  mutable std::unordered_map<PipelineRef, gx::ShaderInfo> shaderInfo;
};

State construct_state();
wgpu::RenderPipeline create_pipeline(const State& state, [[maybe_unused]] PipelineConfig config);
void render(const State& state, const DrawData& data, const wgpu::RenderPassEncoder& pass);
} // namespace aurora::gfx::stream
