#include "../common.hpp"

namespace aurora::gfx::movie_player {
struct DrawData {
  PipelineRef pipeline;
  Range vertRange;
  Range uniformRange;
  uint64_t bindGroupId;
};

struct PipelineConfig {
  // nothing
};
const std::array INITIAL_PIPELINES{
    PipelineConfig{},
};

struct State {
  wgpu::ShaderModule shader;
  wgpu::BindGroupLayout uniformLayout;
  wgpu::BindGroup uniformBindGroup;
  wgpu::BindGroupLayout textureLayout;
  wgpu::Sampler sampler;
  wgpu::PipelineLayout pipelineLayout;
  // Transient state
  std::unordered_map<uint64_t, wgpu::BindGroup> textureBindGroups;
  std::vector<uint64_t> frameUsedTextures;
};
} // namespace aurora::gfx::movie_player
