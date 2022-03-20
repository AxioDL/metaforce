#pragma once

#include "../common.hpp"
#include "../gx.hpp"

namespace aurora::gfx::model {
struct DrawData {
  PipelineRef pipeline;
  Range vertRange;
  Range idxRange;
  gx::BindGroupRanges dataRanges;
  Range uniformRange;
  uint32_t indexCount;
  gx::GXBindGroups bindGroups;
  std::optional<float> dstAlpha;
};

struct PipelineConfig : gx::PipelineConfig {};

struct State {};

State construct_state();
wgpu::RenderPipeline create_pipeline(const State& state, [[maybe_unused]] PipelineConfig config);
void render(const State& state, const DrawData& data, const wgpu::RenderPassEncoder& pass);
} // namespace aurora::gfx::model
