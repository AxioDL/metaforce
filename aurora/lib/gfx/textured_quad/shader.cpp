#include "shader.hpp"

#include "../gx.hpp"
#include "../../gpu.hpp"

#include <logvisor/logvisor.hpp>
#include <magic_enum.hpp>

namespace aurora::gfx::textured_quad {
static logvisor::Module Log("aurora::gfx::textured_quad");

using gpu::g_device;
using gpu::g_graphicsConfig;
using gpu::utils::make_vertex_attributes;
using gpu::utils::make_vertex_buffer_layout;
using gpu::utils::make_vertex_state;

State construct_state() {
  wgpu::ShaderModuleWGSLDescriptor wgslDescriptor{};
  wgslDescriptor.source = R"""(
struct Uniform {
    xf: mat4x4<f32>;
    color: vec4<f32>;
    lod: f32;
};
@group(0) @binding(0)
var<uniform> ubuf: Uniform;
@group(0) @binding(1)
var texture_sampler: sampler;
@group(1) @binding(0)
var texture: texture_2d<f32>;

struct VertexOutput {
    @builtin(position) pos: vec4<f32>;
    @location(0) uv: vec2<f32>;
};

@stage(vertex)
fn vs_main(@location(0) in_pos: vec3<f32>, @location(1) in_uv: vec2<f32>) -> VertexOutput {
    var out: VertexOutput;
    out.pos = ubuf.xf * vec4<f32>(in_pos, 1.0);
    out.uv = in_uv;
    return out;
}

@stage(fragment)
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
    return ubuf.color * textureSampleBias(texture, texture_sampler, in.uv, ubuf.lod);
}
)""";
  const auto shaderDescriptor = wgpu::ShaderModuleDescriptor{
      .nextInChain = &wgslDescriptor,
      .label = "Textured Quad Shader",
  };
  auto shader = g_device.CreateShaderModule(&shaderDescriptor);

  wgpu::SupportedLimits limits;
  g_device.GetLimits(&limits);
  const auto uniform_alignment = limits.limits.minUniformBufferOffsetAlignment;
  const auto uniform_size = ALIGN(sizeof(Uniform), uniform_alignment);

  const std::array uniformLayoutEntries{
      wgpu::BindGroupLayoutEntry{
          .binding = 0,
          .visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
          .buffer =
              wgpu::BufferBindingLayout{
                  .type = wgpu::BufferBindingType::Uniform,
                  .hasDynamicOffset = true,
                  .minBindingSize = uniform_size,
              },
      },
      wgpu::BindGroupLayoutEntry{
          .binding = 1,
          .visibility = wgpu::ShaderStage::Fragment,
          .sampler =
              wgpu::SamplerBindingLayout{
                  .type = wgpu::SamplerBindingType::Filtering,
              },
      },
  };
  const auto uniformLayoutDescriptor = wgpu::BindGroupLayoutDescriptor{
      .label = "Textured Quad Uniform Bind Group Layout",
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
      .maxAnisotropy = g_graphicsConfig.textureAnistropy,
  };
  auto sampler = g_device.CreateSampler(&samplerDescriptor);

  const std::array uniformBindGroupEntries{
      wgpu::BindGroupEntry{
          .binding = 0,
          .buffer = g_uniformBuffer,
          .size = uniform_size,
      },
      wgpu::BindGroupEntry{
          .binding = 1,
          .sampler = sampler,
      },
  };
  const auto uniformBindGroupDescriptor = wgpu::BindGroupDescriptor{
      .label = "Textured Quad Uniform Bind Group",
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
  };
  const auto textureLayoutDescriptor = wgpu::BindGroupLayoutDescriptor{
      .label = "Textured Quad Texture Bind Group Layout",
      .entryCount = textureLayoutEntries.size(),
      .entries = textureLayoutEntries.data(),
  };
  auto textureLayout = g_device.CreateBindGroupLayout(&textureLayoutDescriptor);

  const std::array bindGroupLayouts{
      uniformLayout,
      textureLayout,
  };
  const auto pipelineLayoutDescriptor = wgpu::PipelineLayoutDescriptor{
      .label = "Textured Quad Pipeline Layout",
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

wgpu::RenderPipeline create_pipeline(const State& state, [[maybe_unused]] PipelineConfig config) {
  const auto attributes =
      make_vertex_attributes(std::array{wgpu::VertexFormat::Float32x3, wgpu::VertexFormat::Float32x2});
  const std::array vertexBuffers{make_vertex_buffer_layout(sizeof(Vert), attributes)};

  wgpu::CompareFunction depthCompare;
  switch (config.zComparison) {
  case ZComp::Never:
    depthCompare = wgpu::CompareFunction::Never;
    break;
  case ZComp::Less:
    depthCompare = wgpu::CompareFunction::Less;
    break;
  case ZComp::Equal:
    depthCompare = wgpu::CompareFunction::Equal;
    break;
  case ZComp::LEqual:
    depthCompare = wgpu::CompareFunction::LessEqual;
    break;
  case ZComp::Greater:
    depthCompare = wgpu::CompareFunction::Greater;
    break;
  case ZComp::NEqual:
    depthCompare = wgpu::CompareFunction::NotEqual;
    break;
  case ZComp::GEqual:
    depthCompare = wgpu::CompareFunction::GreaterEqual;
    break;
  case ZComp::Always:
    depthCompare = wgpu::CompareFunction::Always;
    break;
  }
  const auto depthStencil = wgpu::DepthStencilState{
      .format = g_graphicsConfig.depthFormat,
      .depthWriteEnabled = config.zTest,
      .depthCompare = depthCompare,
  };

  bool alphaWrite = false;
  wgpu::BlendComponent blendComponent;
  switch (config.filterType) {
  case CameraFilterType::Multiply:
    blendComponent = wgpu::BlendComponent{
        .srcFactor = wgpu::BlendFactor::Zero,
        .dstFactor = wgpu::BlendFactor::Src,
    };
    alphaWrite = true;
    break;
  case CameraFilterType::Add:
    blendComponent = wgpu::BlendComponent{
        .srcFactor = wgpu::BlendFactor::SrcAlpha,
        .dstFactor = wgpu::BlendFactor::One,
    };
    alphaWrite = false;
    break;
  case CameraFilterType::Subtract:
    blendComponent = wgpu::BlendComponent{
        .operation = wgpu::BlendOperation::Subtract,
        .srcFactor = wgpu::BlendFactor::SrcAlpha,
        .dstFactor = wgpu::BlendFactor::One,
    };
    alphaWrite = false;
    break;
  case CameraFilterType::Blend:
    blendComponent = wgpu::BlendComponent{
        .srcFactor = wgpu::BlendFactor::SrcAlpha,
        .dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha,
    };
    alphaWrite = false;
    break;
  case CameraFilterType::InvDstMultiply:
    blendComponent = wgpu::BlendComponent{
        .srcFactor = wgpu::BlendFactor::Zero,
        .dstFactor = wgpu::BlendFactor::OneMinusSrc,
    };
    alphaWrite = true;
    break;
  default:
    Log.report(logvisor::Fatal, FMT_STRING("unimplemented filter type {}"), magic_enum::enum_name(config.filterType));
    unreachable();
  }

  const auto blendState = wgpu::BlendState{
      .color = blendComponent,
      .alpha = blendComponent,
  };
  auto writeMask = wgpu::ColorWriteMask::Red | wgpu::ColorWriteMask::Green | wgpu::ColorWriteMask::Blue;
  if (alphaWrite) {
    writeMask = writeMask | wgpu::ColorWriteMask::Alpha;
  }
  const std::array colorTargets{
      wgpu::ColorTargetState{
          .format = g_graphicsConfig.colorFormat,
          .blend = &blendState,
          .writeMask = writeMask,
      },
  };
  const auto fragmentState = wgpu::FragmentState{
      .module = state.shader,
      .entryPoint = "fs_main",
      .targetCount = colorTargets.size(),
      .targets = colorTargets.data(),
  };

  const auto pipelineDescriptor = wgpu::RenderPipelineDescriptor{
      .label = "Textured Quad Pipeline",
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

DrawData make_draw_data(const State& state, CameraFilterType filter_type, const TextureHandle& texture,
                        ZComp z_comparison, bool z_test, const zeus::CColor& color, float uv_scale,
                        const zeus::CRectangle& rect, float z, float lod) {
  auto pipeline = pipeline_ref(PipelineConfig{
      .filterType = filter_type,
      .zComparison = z_comparison,
      .zTest = z_test,
  });

  const std::array verts{
      Vert{{0.f, 0.f, z}, {0.0, 0.0}},
      Vert{{0.f, 1.f, z}, {0.0, uv_scale}},
      Vert{{1.f, 0.f, z}, {uv_scale, 0.0}},
      Vert{{1.f, 1.f, z}, {uv_scale, uv_scale}},
  };
  const auto vertRange = push_verts(ArrayRef{verts});

  const auto uniform = Uniform{
      .xf =
          Mat4x4<float>{
              Vec4<float>{rect.size.x() * 2.f, 0.f, 0.f, 0.f},
              Vec4<float>{0.f, rect.size.y() * 2.f, 0.f, 0.f},
              Vec4<float>{0.f, 0.f, 1.f, 0.f},
              Vec4<float>{rect.position.x() * 2.f - 1.f, rect.position.y() * 2.f - 1.f, 0.f, 1.f},
          },
      .color = color,
      .lod = lod,
  };
  const auto uniformRange = push_uniform(uniform);

  const std::array entries{
      wgpu::BindGroupEntry{
          .binding = 0,
          .textureView = texture.ref->view,
      },
  };
  const auto textureBindGroup = bind_group_ref(wgpu::BindGroupDescriptor{
      .label = "Textured Quad Texture Bind Group",
      .layout = state.textureLayout,
      .entryCount = entries.size(),
      .entries = entries.data(),
  });

  return {
      .pipeline = pipeline,
      .vertRange = vertRange,
      .uniformRange = uniformRange,
      .textureBindGroup = textureBindGroup,
  };
}

DrawData make_draw_data_verts(const State& state, CameraFilterType filter_type, const TextureHandle& texture,
                        ZComp z_comparison, bool z_test, const zeus::CColor& color,
                        const ArrayRef<zeus::CVector3f>& pos, const ArrayRef<zeus::CVector2f>& uvs, float lod) {
  auto pipeline = pipeline_ref(PipelineConfig{
      .filterType = filter_type,
      .zComparison = z_comparison,
      .zTest = z_test,
  });

  assert(pos.size() == 4 && uvs.size() == 4 && "Invalid pos/uv sizes!");

  const std::array verts{
      Vert{pos[0], uvs[0]},
      Vert{pos[1], uvs[1]},
      Vert{pos[2], uvs[2]},
      Vert{pos[3], uvs[3]},
  };
  const auto vertRange = push_verts(ArrayRef{verts});

  const auto uniform = Uniform{
      .xf = gx::get_combined_matrix(),
      .color = color,
      .lod = lod,
  };
  const auto uniformRange = push_uniform(uniform);

  const std::array entries{
      wgpu::BindGroupEntry{
          .binding = 0,
          .textureView = texture.ref->view,
      },
  };
  const auto textureBindGroup = bind_group_ref(wgpu::BindGroupDescriptor{
      .label = "Textured Quad Texture Bind Group",
      .layout = state.textureLayout,
      .entryCount = entries.size(),
      .entries = entries.data(),
  });

  return {
      .pipeline = pipeline,
      .vertRange = vertRange,
      .uniformRange = uniformRange,
      .textureBindGroup = textureBindGroup,
  };
}

void render(const State& state, const DrawData& data, const wgpu::RenderPassEncoder& pass) {
  OPTICK_EVENT();
  if (!bind_pipeline(data.pipeline, pass)) {
    return;
  }

  const std::array offsets{data.uniformRange.offset};
  pass.SetBindGroup(0, state.uniformBindGroup, offsets.size(), offsets.data());
  pass.SetBindGroup(1, find_bind_group(data.textureBindGroup));
  pass.SetVertexBuffer(0, g_vertexBuffer, data.vertRange.offset, data.vertRange.size);
  pass.Draw(4);
}
} // namespace aurora::gfx::textured_quad
