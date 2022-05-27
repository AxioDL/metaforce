#include "shader.hpp"

#include "../../gpu.hpp"

namespace aurora::gfx::movie_player {
using gpu::g_device;
using gpu::g_graphicsConfig;
using gpu::utils::make_vertex_attributes;
using gpu::utils::make_vertex_buffer_layout;
using gpu::utils::make_vertex_state;

State construct_state() {
  wgpu::ShaderModuleWGSLDescriptor wgslDescriptor{};
  wgslDescriptor.source = R"""(
@group(0) @binding(0)
var tex_sampler: sampler;
@group(1) @binding(0)
var tex_y: texture_2d<f32>;
@group(1) @binding(1)
var tex_u: texture_2d<f32>;
@group(1) @binding(2)
var tex_v: texture_2d<f32>;

struct VertexOutput {
    @builtin(position) pos: vec4<f32>;
    @location(0) uv: vec2<f32>;
};

@stage(vertex)
fn vs_main(@location(0) in_pos: vec3<f32>, @location(1) in_uv: vec2<f32>) -> VertexOutput {
    var out: VertexOutput;
    out.pos = vec4<f32>(in_pos, 1.0);
    out.uv = in_uv;
    return out;
}

@stage(fragment)
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
    var yuv = vec3<f32>(
        1.1643 * (textureSample(tex_y, tex_sampler, in.uv).x - 0.0625),
        textureSample(tex_u, tex_sampler, in.uv).x - 0.5,
        textureSample(tex_v, tex_sampler, in.uv).x - 0.5
    );
    return vec4<f32>(
        yuv.x + 1.5958 * yuv.z,
        yuv.x - 0.39173 * yuv.y - 0.8129 * yuv.z,
        yuv.x + 2.017 * yuv.y,
        1.0
    );
}
)""";
  const auto shaderDescriptor = wgpu::ShaderModuleDescriptor{
      .nextInChain = &wgslDescriptor,
      .label = "Movie Player Shader",
  };
  auto shader = g_device.CreateShaderModule(&shaderDescriptor);

  const std::array uniformLayoutEntries{
      wgpu::BindGroupLayoutEntry{
          .binding = 0,
          .visibility = wgpu::ShaderStage::Fragment,
          .sampler =
              wgpu::SamplerBindingLayout{
                  .type = wgpu::SamplerBindingType::Filtering,
              },
      },
  };
  const auto uniformLayoutDescriptor = wgpu::BindGroupLayoutDescriptor{
      .label = "Movie Player Uniform Bind Group Layout",
      .entryCount = uniformLayoutEntries.size(),
      .entries = uniformLayoutEntries.data(),
  };
  auto uniformLayout = g_device.CreateBindGroupLayout(&uniformLayoutDescriptor);

  const auto samplerDescriptor = wgpu::SamplerDescriptor{
      .addressModeU = wgpu::AddressMode::Repeat,
      .addressModeV = wgpu::AddressMode::Repeat,
      .addressModeW = wgpu::AddressMode::Repeat,
      .magFilter = wgpu::FilterMode::Linear,
      .minFilter = wgpu::FilterMode::Linear,
      .mipmapFilter = wgpu::FilterMode::Linear,
      .maxAnisotropy = 1,
  };
  auto sampler = g_device.CreateSampler(&samplerDescriptor);

  const std::array uniformBindGroupEntries{
      wgpu::BindGroupEntry{
          .binding = 0,
          .sampler = sampler,
      },
  };
  const auto uniformBindGroupDescriptor = wgpu::BindGroupDescriptor{
      .label = "Movie Player Uniform Bind Group",
      .layout = uniformLayout,
      .entryCount = uniformBindGroupEntries.size(),
      .entries = uniformBindGroupEntries.data(),
  };
  auto uniformBindGroup = g_device.CreateBindGroup(&uniformBindGroupDescriptor);

  const auto textureBinding = wgpu::TextureBindingLayout{
      .sampleType = wgpu::TextureSampleType::Float,
      .viewDimension = wgpu::TextureViewDimension::e2D,
  };
  const std::array textureLayoutEntries{
      wgpu::BindGroupLayoutEntry{
          .binding = 0,
          .visibility = wgpu::ShaderStage::Fragment,
          .texture = textureBinding,
      },
      wgpu::BindGroupLayoutEntry{
          .binding = 1,
          .visibility = wgpu::ShaderStage::Fragment,
          .texture = textureBinding,
      },
      wgpu::BindGroupLayoutEntry{
          .binding = 2,
          .visibility = wgpu::ShaderStage::Fragment,
          .texture = textureBinding,
      },
  };
  const auto textureLayoutDescriptor = wgpu::BindGroupLayoutDescriptor{
      .label = "Movie Player Texture Bind Group Layout",
      .entryCount = textureLayoutEntries.size(),
      .entries = textureLayoutEntries.data(),
  };
  auto textureLayout = g_device.CreateBindGroupLayout(&textureLayoutDescriptor);

  const std::array bindGroupLayouts{
      uniformLayout,
      textureLayout,
  };
  const auto pipelineLayoutDescriptor = wgpu::PipelineLayoutDescriptor{
      .label = "Movie Player Pipeline Layout",
      .bindGroupLayoutCount = bindGroupLayouts.size(),
      .bindGroupLayouts = bindGroupLayouts.data(),
  };
  auto pipelineLayout = g_device.CreatePipelineLayout(&pipelineLayoutDescriptor);

  return {
      .shader = shader,
      .uniformLayout = uniformLayout,
      .uniformBindGroup = uniformBindGroup,
      .textureLayout = textureLayout,
      .sampler = sampler,
      .pipelineLayout = pipelineLayout,
  };
}

wgpu::RenderPipeline create_pipeline(const State& state, [[maybe_unused]] const PipelineConfig& config) {
  const auto attributes =
      make_vertex_attributes(std::array{wgpu::VertexFormat::Float32x3, wgpu::VertexFormat::Float32x2});
  const std::array vertexBuffers{make_vertex_buffer_layout(sizeof(Vert), attributes)};
  const auto depthStencil = wgpu::DepthStencilState{
      .format = g_graphicsConfig.depthFormat,
  };
  const auto blendComponent = wgpu::BlendComponent{
      .srcFactor = wgpu::BlendFactor::SrcAlpha,
      .dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha,
  };
  const auto blendState = wgpu::BlendState{
      .color = blendComponent,
      .alpha = blendComponent,
  };
  const std::array colorTargets{
      wgpu::ColorTargetState{
          .format = g_graphicsConfig.colorFormat,
          .blend = &blendState,
          .writeMask = wgpu::ColorWriteMask::Red | wgpu::ColorWriteMask::Green | wgpu::ColorWriteMask::Blue,
      },
  };
  const auto fragmentState = wgpu::FragmentState{
      .module = state.shader,
      .entryPoint = "fs_main",
      .targetCount = colorTargets.size(),
      .targets = colorTargets.data(),
  };
  const auto pipelineDescriptor = wgpu::RenderPipelineDescriptor{
      .label = "Movie Player Pipeline",
      .layout = state.pipelineLayout,
      .vertex = make_vertex_state(state.shader, vertexBuffers),
      .primitive =
          wgpu::PrimitiveState{
              .topology = wgpu::PrimitiveTopology::TriangleStrip,
          },
      .depthStencil = &depthStencil,
      .multisample =
          wgpu::MultisampleState{
              .count = g_graphicsConfig.msaaSamples,
          },
      .fragment = &fragmentState,
  };
  return g_device.CreateRenderPipeline(&pipelineDescriptor);
}

DrawData make_draw_data(const State& state, const TextureHandle& tex_y, const TextureHandle& tex_u,
                        const TextureHandle& tex_v, float h_pad, float v_pad) {
  auto pipeline = pipeline_ref(PipelineConfig{});

  const std::array verts{
      Vert{{-h_pad, v_pad, 0.f}, {0.0, 0.0}},
      Vert{{-h_pad, -v_pad, 0.f}, {0.0, 1.0}},
      Vert{{h_pad, v_pad, 0.f}, {1.0, 0.0}},
      Vert{{h_pad, -v_pad, 0.f}, {1.0, 1.0}},
  };
  const auto vertRange = push_verts(ArrayRef{verts});

  const std::array entries{
      wgpu::BindGroupEntry{
          .binding = 0,
          .textureView = tex_y->view,
      },
      wgpu::BindGroupEntry{
          .binding = 1,
          .textureView = tex_u->view,
      },
      wgpu::BindGroupEntry{
          .binding = 2,
          .textureView = tex_v->view,
      },
  };
  const auto textureBindGroup = bind_group_ref(wgpu::BindGroupDescriptor{
      .label = "Movie Player Texture Bind Group",
      .layout = state.textureLayout,
      .entryCount = entries.size(),
      .entries = entries.data(),
  });

  return {
      .pipeline = pipeline,
      .vertRange = vertRange,
      .textureBindGroup = textureBindGroup,
  };
}

void render(const State& state, const DrawData& data, const wgpu::RenderPassEncoder& pass) {
  if (!bind_pipeline(data.pipeline, pass)) {
    return;
  }

  pass.SetBindGroup(0, state.uniformBindGroup);
  pass.SetBindGroup(1, find_bind_group(data.textureBindGroup));
  pass.SetVertexBuffer(0, g_vertexBuffer, data.vertRange.offset, data.vertRange.size);
  pass.Draw(4);
}
} // namespace aurora::gfx::movie_player
