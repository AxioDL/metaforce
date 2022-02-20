#include "../common.hpp"

namespace aurora::gfx::colored_quad {
struct DrawData {
  PipelineRef pipeline;
  Range vertRange;
  Range uniformRange;
};

struct PipelineConfig {
  CameraFilterType filterType;
  ZComp zComparison;
  bool zTest;
};

static const std::array INITIAL_PIPELINES {
  PipelineConfig{}, // TODO
};

struct State {
  wgpu::ShaderModule shader;
  wgpu::BindGroupLayout uniformLayout;
  wgpu::BindGroup uniformBindGroup;
  wgpu::Sampler sampler;
  wgpu::PipelineLayout pipelineLayout;
};

struct alignas(4) Vert {
  Vec3<float> pos;
};

struct alignas(4) Uniform {
  Mat4x4<float> xf;
  Vec4<float> color;
};
static_assert(sizeof(Uniform) == 80);

State construct_state();
wgpu::RenderPipeline create_pipeline(const State& state, [[maybe_unused]] PipelineConfig config);
DrawData make_draw_data(const State& state, CameraFilterType filter_type, ZComp z_comparison, bool z_test,
                        const zeus::CColor& color, const zeus::CRectangle& rect, float z);

DrawData make_draw_data_verts(const State& state, CameraFilterType filter_type, ZComp z_comparison, bool z_test,
                        const zeus::CColor& color, const ArrayRef<zeus::CVector3f>& pos);

void render(const State& state, const DrawData& data, const wgpu::RenderPassEncoder& pass);
} // namespace aurora::gfx::colored_quad