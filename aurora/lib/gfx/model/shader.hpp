#pragma once

#include "../common.hpp"
#include "../gx.hpp"

#include <unordered_map>

namespace aurora::gfx::model {
struct DrawData {
  PipelineRef pipeline;
  Range vertRange;
  Range idxRange;
  Range sVtxRange;
  Range sNrmRange;
  Range sTcRange;
  Range sPackedTcRange;
  Range uniformRange;
  uint32_t indexCount;
  gx::GXBindGroups bindGroups;
};

struct PipelineConfig : gx::PipelineConfig {

};

struct CachedBindGroup {
  wgpu::BindGroupLayout layout;
  wgpu::BindGroup bindGroup;
  CachedBindGroup(wgpu::BindGroupLayout layout, wgpu::BindGroup&& group)
  : layout(std::move(layout)), bindGroup(std::move(group)) {}
};
struct State {
};

State construct_state();
wgpu::RenderPipeline create_pipeline(const State& state, [[maybe_unused]] PipelineConfig config);
void render(const State& state, const DrawData& data, const wgpu::RenderPassEncoder& pass);
} // namespace aurora::gfx::model
