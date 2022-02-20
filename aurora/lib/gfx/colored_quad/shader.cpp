#include "shader.hpp"

#include "../../gpu.hpp"

#include <logvisor/logvisor.hpp>
#include <magic_enum.hpp>

namespace aurora::gfx::colored_quad {
static logvisor::Module Log("aurora::gfx::colored_quad");

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
};
@group(0) @binding(0)
var<uniform> ubuf: Uniform;

struct VertexOutput {
    @builtin(position) pos: vec4<f32>;
    //@builtin(normal) norm: vec4<f32>;
};

@stage(vertex)
fn vs_main(@location(0) in_pos: vec3<f32>) -> VertexOutput {//, @location(1) in_norm: vec3<f32>) -> VertexOutput {
    var out: VertexOutput;
    out.pos = ubuf.xf * vec4<f32>(in_pos, 1.0);
    //out.norm = in_norm;
    return out;
}

@stage(fragment)
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
    return ubuf.color;
}
)""";
  const auto shaderDescriptor = wgpu::ShaderModuleDescriptor{
      .nextInChain = &wgslDescriptor,
      .label = "Colored Quad Shader",
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
  };

  const auto uniformLayoutDescriptor = wgpu::BindGroupLayoutDescriptor{
      .label = "Colored Quad Uniform Bind Group Layout",
      .entryCount = uniformLayoutEntries.size(),
      .entries = uniformLayoutEntries.data(),
  };
  auto uniformLayout = g_device.CreateBindGroupLayout(&uniformLayoutDescriptor);

  const std::array uniformBindGroupEntries{
      wgpu::BindGroupEntry{
          .binding = 0,
          .buffer = g_uniformBuffer,
          .size = uniform_size,
      },
  };

  const auto uniformBindGroupDescriptor = wgpu::BindGroupDescriptor{
      .label = "Colored Quad Uniform Bind Group",
      .layout = uniformLayout,
      .entryCount = uniformBindGroupEntries.size(),
      .entries = uniformBindGroupEntries.data(),
  };
  auto uniformBindGroup = g_device.CreateBindGroup(&uniformBindGroupDescriptor);

  const std::array bindGroupLayouts{
      uniformLayout,
  };

  const auto pipelineLayoutDescriptor = wgpu::PipelineLayoutDescriptor{
      .label = "Colored Quad Pipeline Layout",
      .bindGroupLayoutCount = bindGroupLayouts.size(),
      .bindGroupLayouts = bindGroupLayouts.data(),
  };
  auto pipelineLayout = g_device.CreatePipelineLayout(&pipelineLayoutDescriptor);

  return {
      .shader = shader,
      .uniformLayout = uniformLayout,
      .uniformBindGroup = uniformBindGroup,
      .pipelineLayout = pipelineLayout,
  };
}

wgpu::RenderPipeline create_pipeline(const State& state, [[maybe_unused]] PipelineConfig config) {
  const auto attributes = make_vertex_attributes(std::array{wgpu::VertexFormat::Float32x3});
  const std::array vertexBuffers{make_vertex_buffer_layout(sizeof(Vert), attributes)};

  wgpu::CompareFunction depthCompare{};
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
  wgpu::BlendComponent blendComponent{};
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
      .label = "Colored Quad Pipeline",
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

DrawData make_draw_data(const State& state, CameraFilterType filter_type, ZComp z_comparison, bool z_test,
                        const zeus::CColor& color, const zeus::CRectangle& rect, float z) {
  auto pipeline = pipeline_ref(PipelineConfig{
      .filterType = filter_type,
      .zComparison = z_comparison,
      .zTest = z_test,
  });

  const std::array verts{
      Vert{{0.f, 0.f, z}},
      Vert{{0.f, 1.f, z}},
      Vert{{1.f, 0.f, z}},
      Vert{{1.f, 1.f, z}},
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
  };
  const auto uniformRange = push_uniform(uniform);

  return {
      .pipeline = pipeline,
      .vertRange = vertRange,
      .uniformRange = uniformRange,
  };
}

DrawData make_draw_data_verts(const State& state, CameraFilterType filter_type, ZComp z_comparison, bool z_test,
                              const zeus::CColor& color, const ArrayRef<zeus::CVector3f>& pos) {
  auto pipeline = pipeline_ref(PipelineConfig{
      .filterType = filter_type,
      .zComparison = z_comparison,
      .zTest = z_test,
  });

  assert(pos.size() == 4 && "Invalid pos size!");

  const std::array verts{
      Vert{pos[0]},
      Vert{pos[1]},
      Vert{pos[2]},
      Vert{pos[3]},
  };
  const auto vertRange = push_verts(ArrayRef{verts});

  const auto uniform = Uniform{
      .xf = get_combined_matrix(),
      .color = color,
  };
  const auto uniformRange = push_uniform(uniform);

  return {
      .pipeline = pipeline,
      .vertRange = vertRange,
      .uniformRange = uniformRange,
  };
}

void render(const State& state, const DrawData& data, const wgpu::RenderPassEncoder& pass) {
  if (!bind_pipeline(data.pipeline, pass)) {
    return;
  }

  const std::array offsets{data.uniformRange.first};
  pass.SetBindGroup(0, state.uniformBindGroup, offsets.size(), offsets.data());
  pass.SetVertexBuffer(0, g_vertexBuffer, data.vertRange.first, data.vertRange.second);
  pass.Draw(4);
}
} // namespace aurora::gfx::colored_quad
