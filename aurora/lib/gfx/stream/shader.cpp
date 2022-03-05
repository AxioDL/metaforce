#include "shader.hpp"

#include "../../gpu.hpp"
#include "../common.hpp"

#include <magic_enum.hpp>

namespace aurora::gfx {
extern std::unordered_map<ShaderRef, wgpu::ShaderModule> g_streamCachedShaders;
} // namespace aurora::gfx

namespace aurora::gfx::stream {
static logvisor::Module Log("aurora::gfx::stream");

using gpu::g_device;
using gpu::g_graphicsConfig;
using gpu::utils::make_vertex_state;

static wgpu::BlendFactor to_blend_factor(metaforce::ERglBlendFactor fac) {
  switch (fac) {
  case metaforce::ERglBlendFactor::Zero:
    return wgpu::BlendFactor::Zero;
  case metaforce::ERglBlendFactor::One:
    return wgpu::BlendFactor::One;
  case metaforce::ERglBlendFactor::SrcColor:
    return wgpu::BlendFactor::Src;
  case metaforce::ERglBlendFactor::InvSrcColor:
    return wgpu::BlendFactor::OneMinusSrc;
  case metaforce::ERglBlendFactor::SrcAlpha:
    return wgpu::BlendFactor::SrcAlpha;
  case metaforce::ERglBlendFactor::InvSrcAlpha:
    return wgpu::BlendFactor::OneMinusSrcAlpha;
  case metaforce::ERglBlendFactor::DstAlpha:
    return wgpu::BlendFactor::DstAlpha;
  case metaforce::ERglBlendFactor::InvDstAlpha:
    return wgpu::BlendFactor::OneMinusDstAlpha;
  case metaforce::ERglBlendFactor::DstColor:
    return wgpu::BlendFactor::Dst;
  case metaforce::ERglBlendFactor::InvDstColor:
    return wgpu::BlendFactor::OneMinusDst;
  }
}

wgpu::RenderPipeline create_pipeline(const State& state, [[maybe_unused]] PipelineConfig config) {
  std::array<wgpu::VertexAttribute, 4> attributes{};
  attributes[0] = wgpu::VertexAttribute{
      .format = wgpu::VertexFormat::Float32x3,
      .offset = 0,
      .shaderLocation = 0,
  };
  uint64_t offset = 12;
  uint32_t shaderLocation = 1;
  if (config.flags & metaforce::EStreamFlagBits::fHasNormal) {
    attributes[shaderLocation] = wgpu::VertexAttribute{
        .format = wgpu::VertexFormat::Float32x3,
        .offset = offset,
        .shaderLocation = shaderLocation,
    };
    offset += 12;
    shaderLocation++;
  }
  if (config.flags & metaforce::EStreamFlagBits::fHasColor) {
    attributes[shaderLocation] = wgpu::VertexAttribute{
        .format = wgpu::VertexFormat::Float32x4,
        .offset = offset,
        .shaderLocation = shaderLocation,
    };
    offset += 16;
    shaderLocation++;
  }
  if (config.flags & metaforce::EStreamFlagBits::fHasTexture) {
    attributes[shaderLocation] = wgpu::VertexAttribute{
        .format = wgpu::VertexFormat::Float32x2,
        .offset = offset,
        .shaderLocation = shaderLocation,
    };
    offset += 8;
    shaderLocation++;
  }
  const std::array vertexBuffers{wgpu::VertexBufferLayout{
      .arrayStride = offset,
      .attributeCount = shaderLocation,
      .attributes = attributes.data(),
  }};

  wgpu::CompareFunction depthCompare;
  switch (config.depthFunc) {
  case metaforce::ERglEnum::Never:
    depthCompare = wgpu::CompareFunction::Never;
    break;
  case metaforce::ERglEnum::Less:
    depthCompare = wgpu::CompareFunction::Less;
    break;
  case metaforce::ERglEnum::Equal:
    depthCompare = wgpu::CompareFunction::Equal;
    break;
  case metaforce::ERglEnum::LEqual:
    depthCompare = wgpu::CompareFunction::LessEqual;
    break;
  case metaforce::ERglEnum::Greater:
    depthCompare = wgpu::CompareFunction::Greater;
    break;
  case metaforce::ERglEnum::NEqual:
    depthCompare = wgpu::CompareFunction::NotEqual;
    break;
  case metaforce::ERglEnum::GEqual:
    depthCompare = wgpu::CompareFunction::GreaterEqual;
    break;
  case metaforce::ERglEnum::Always:
    depthCompare = wgpu::CompareFunction::Always;
    break;
  }
  const auto depthStencil = wgpu::DepthStencilState{
      .format = g_graphicsConfig.depthFormat,
      .depthWriteEnabled = config.depthUpdate,
      .depthCompare = depthCompare,
  };

  if (config.blendMode != metaforce::ERglBlendMode::Blend) {
    Log.report(logvisor::Fatal, FMT_STRING("How to {}?"), magic_enum::enum_name(config.blendMode));
  }
  const auto colorBlendComponent = wgpu::BlendComponent{
      .operation = wgpu::BlendOperation::Add,
      .srcFactor = to_blend_factor(config.blendFacSrc),
      .dstFactor = to_blend_factor(config.blendFacDst),
  };
  auto alphaBlendComponent = colorBlendComponent;
  if (config.dstAlpha) {
    alphaBlendComponent = wgpu::BlendComponent{
        .operation = wgpu::BlendOperation::Add,
        .srcFactor = wgpu::BlendFactor::Zero,
        .dstFactor = wgpu::BlendFactor::Constant,
    };
  }
  const auto blendState = wgpu::BlendState{
      .color = colorBlendComponent,
      .alpha = alphaBlendComponent,
  };
  auto writeMask = wgpu::ColorWriteMask::Red | wgpu::ColorWriteMask::Green | wgpu::ColorWriteMask::Blue;
  if (config.alphaUpdate) {
    writeMask = writeMask | wgpu::ColorWriteMask::Alpha;
  }
  const std::array colorTargets{wgpu::ColorTargetState{
      .format = g_graphicsConfig.colorFormat,
      .blend = &blendState,
      .writeMask = writeMask,
  }};
  const auto& shader = g_streamCachedShaders[config.shader];
  const auto fragmentState = wgpu::FragmentState{
      .module = shader,
      .entryPoint = "fs_main",
      .targetCount = colorTargets.size(),
      .targets = colorTargets.data(),
  };

  wgpu::PrimitiveTopology primitive;
  switch (config.primitive) {
  case GX::POINTS:
    primitive = wgpu::PrimitiveTopology::PointList;
    break;
  case GX::LINES:
    primitive = wgpu::PrimitiveTopology::LineList;
    break;
  case GX::LINESTRIP:
    primitive = wgpu::PrimitiveTopology::LineStrip;
    break;
  case GX::TRIANGLES:
    primitive = wgpu::PrimitiveTopology::TriangleList;
    break;
  case GX::TRIANGLESTRIP:
    primitive = wgpu::PrimitiveTopology::TriangleStrip;
    break;
  default:
    Log.report(logvisor::Fatal, FMT_STRING("Unsupported primitive type {}"), magic_enum::enum_name(config.primitive));
    unreachable();
  }

  wgpu::FrontFace frontFace;
  wgpu::CullMode cullMode;
  switch (config.cullMode) {
  case metaforce::ERglCullMode::Front:
    frontFace = wgpu::FrontFace::CW;
    cullMode = wgpu::CullMode::Front;
    break;
  case metaforce::ERglCullMode::Back:
    frontFace = wgpu::FrontFace::CCW;
    cullMode = wgpu::CullMode::Back;
    break;
  default:
    frontFace = wgpu::FrontFace::CCW;
    cullMode = wgpu::CullMode::None;
    break;
  }

  wgpu::BindGroupLayout uniformLayout;
  if (state.uniform.contains(config.uniformSize)) {
    uniformLayout = state.uniform.at(config.uniformSize).layout;
  } else {
    const std::array uniformLayoutEntries{wgpu::BindGroupLayoutEntry{
        .binding = 0,
        .visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
        .buffer =
            wgpu::BufferBindingLayout{
                .type = wgpu::BufferBindingType::Uniform,
                .hasDynamicOffset = true,
                .minBindingSize = config.uniformSize,
            },
    }};
    const auto uniformLayoutDescriptor = wgpu::BindGroupLayoutDescriptor{
        .label = "Stream Uniform Bind Group Layout",
        .entryCount = uniformLayoutEntries.size(),
        .entries = uniformLayoutEntries.data(),
    };
    uniformLayout = g_device.CreateBindGroupLayout(&uniformLayoutDescriptor);

    const std::array uniformBindGroupEntries{wgpu::BindGroupEntry{
        .binding = 0,
        .buffer = g_uniformBuffer,
        .size = config.uniformSize,
    }};
    const auto uniformBindGroupDescriptor = wgpu::BindGroupDescriptor{
        .label = "Stream Quad Uniform Bind Group",
        .layout = uniformLayout,
        .entryCount = uniformBindGroupEntries.size(),
        .entries = uniformBindGroupEntries.data(),
    };
    auto uniformBindGroup = g_device.CreateBindGroup(&uniformBindGroupDescriptor);

    state.uniform.try_emplace(config.uniformSize, uniformLayout, std::move(uniformBindGroup));
  }

  const std::array bindGroupLayouts{
      uniformLayout,
      state.samplerLayout,
      state.textureLayout,
  };
  const auto pipelineLayoutDescriptor = wgpu::PipelineLayoutDescriptor{
      .label = "Stream Pipeline Layout",
      .bindGroupLayoutCount =
          // TODO avoid creating bind group layouts if no tex?
      static_cast<uint32_t>(config.flags & metaforce::EStreamFlagBits::fHasTexture ? bindGroupLayouts.size() : 1),
      .bindGroupLayouts = bindGroupLayouts.data(),
  };
  auto pipelineLayout = g_device.CreatePipelineLayout(&pipelineLayoutDescriptor);

  const auto pipelineDescriptor = wgpu::RenderPipelineDescriptor{
      .label = "Stream Pipeline",
      .layout = pipelineLayout,
      .vertex = make_vertex_state(shader, vertexBuffers),
      .primitive =
          wgpu::PrimitiveState{
              .topology = primitive,
              .frontFace = frontFace,
              .cullMode = cullMode,
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

State construct_state() {
  const auto samplerBinding = wgpu::SamplerBindingLayout{
      .type = wgpu::SamplerBindingType::Filtering,
  };
  const std::array samplerLayoutEntries{
      wgpu::BindGroupLayoutEntry{
          .binding = 0,
          .visibility = wgpu::ShaderStage::Fragment,
          .sampler = samplerBinding,
      },
  };
  const auto samplerLayoutDescriptor = wgpu::BindGroupLayoutDescriptor{
      .label = "Stream Sampler Bind Group Layout",
      .entryCount = samplerLayoutEntries.size(),
      .entries = samplerLayoutEntries.data(),
  };
  auto samplerLayout = g_device.CreateBindGroupLayout(&samplerLayoutDescriptor);

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
      .label = "Stream Texture Bind Group Layout",
      .entryCount = textureLayoutEntries.size(),
      .entries = textureLayoutEntries.data(),
  };
  auto textureLayout = g_device.CreateBindGroupLayout(&textureLayoutDescriptor);

  return {
      .samplerLayout = samplerLayout,
      .textureLayout = textureLayout,
  };
}

void render(const State& state, const DrawData& data, const wgpu::RenderPassEncoder& pass) {
  if (!bind_pipeline(data.pipeline, pass)) {
    return;
  }

  const std::array offsets{data.uniformRange.first};
  pass.SetBindGroup(0, state.uniform.at(data.uniformSize).bindGroup, offsets.size(), offsets.data());
  if (data.samplerBindGroup && data.textureBindGroup) {
    pass.SetBindGroup(1, find_bind_group(data.samplerBindGroup));
    pass.SetBindGroup(2, find_bind_group(data.textureBindGroup));
  }
  pass.SetVertexBuffer(0, g_vertexBuffer, data.vertRange.first, data.vertRange.second);
  pass.Draw(data.vertexCount);
}
} // namespace aurora::gfx::stream
