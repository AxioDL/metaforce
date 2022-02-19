#include "../common.hpp"

namespace aurora::gfx::textured_quad {
struct DrawData {
  PipelineRef pipeline;
  Range vertRange;
  Range uniformRange;
  BindGroupRef textureBindGroup;
};

struct PipelineConfig {
  CameraFilterType filterType;
  ZTest zComparison;
  bool zTest;
};
static const std::array INITIAL_PIPELINES{
    PipelineConfig{}, // TODO
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
struct alignas(4) Uniform {
  Mat4x4<float> xf;
  Vec4<float> color;
  float lod;
};
static_assert(sizeof(Uniform) == 84);

State construct_state();
wgpu::RenderPipeline create_pipeline(const State& state, [[maybe_unused]] PipelineConfig config);
DrawData make_draw_data(const State& state, CameraFilterType filter_type, const TextureHandle& texture,
                        ZTest z_comparison, bool z_test, const zeus::CColor& color, float uv_scale,
                        const zeus::CRectangle& rect, float z);
void render(const State& state, const DrawData& data, const wgpu::RenderPassEncoder& pass);
} // namespace aurora::gfx::textured_quad
