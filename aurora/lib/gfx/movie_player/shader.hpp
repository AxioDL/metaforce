#pragma once

#include "../common.hpp"

namespace aurora::gfx::movie_player {
struct DrawData {
  PipelineRef pipeline;
  Range vertRange;
  Range uniformRange;
  BindGroupRef textureBindGroup;
};

struct PipelineConfig {
  u32 pad = 0; // empty
};
static_assert(std::has_unique_object_representations_v<PipelineConfig>);
static const std::array INITIAL_PIPELINES{
    PipelineConfig{},
};

struct State {
  wgpu::ShaderModule shader;
  wgpu::BindGroupLayout uniformLayout;
  wgpu::BindGroup uniformBindGroup;
  wgpu::BindGroupLayout textureLayout;
  wgpu::Sampler sampler;
  wgpu::PipelineLayout pipelineLayout;
};

struct alignas(4) Vert {
  Vec3<float> pos;
  Vec2<float> uv;
};

State construct_state();
wgpu::RenderPipeline create_pipeline(const State& state, [[maybe_unused]] const PipelineConfig& config);
DrawData make_draw_data(const State& state, const TextureHandle& tex_y, const TextureHandle& tex_u,
                        const TextureHandle& tex_v, float h_pad, float v_pad);
void render(const State& state, const DrawData& data, const wgpu::RenderPassEncoder& pass);
} // namespace aurora::gfx::movie_player
