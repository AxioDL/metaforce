#include "gx.hpp"

#include "common.hpp"
#include "../gpu.hpp"

#include <magic_enum.hpp>

namespace aurora::gfx {
static logvisor::Module Log("aurora::gfx::gx");

zeus::CMatrix4f g_mv;
zeus::CMatrix4f g_mvInv;
zeus::CMatrix4f g_proj;
metaforce::CFogState g_fogState;
metaforce::ERglCullMode g_cullMode;
metaforce::ERglBlendMode g_blendMode;
metaforce::ERglBlendFactor g_blendFacSrc;
metaforce::ERglBlendFactor g_blendFacDst;
metaforce::ERglLogicOp g_blendOp;
bool g_depthCompare;
bool g_depthUpdate;
metaforce::ERglEnum g_depthFunc;
std::array<zeus::CColor, 3> g_colorRegs;
std::array<zeus::CColor, GX::MAX_KCOLOR> g_kcolors;
bool g_alphaUpdate;
std::optional<float> g_dstAlpha;
zeus::CColor g_clearColor = zeus::skClear;
bool g_alphaDiscard;

std::array<SChannelState, 2> g_colorChannels;
std::array<LightVariant, MaxLights> g_lights;
std::bitset<MaxLights> g_lightState;

std::array<std::optional<STevStage>, maxTevStages> g_tevStages;
std::array<STextureBind, maxTextures> g_textures;

// GX state
void bind_texture(GX::TexMapID id, metaforce::EClampMode clamp, const TextureHandle& tex, float lod) noexcept {
  g_textures[static_cast<size_t>(id)] = {tex, clamp, lod};
}
void unbind_texture(GX::TexMapID id) noexcept { g_textures[static_cast<size_t>(id)].reset(); }
const STextureBind& get_texture(GX::TexMapID id) noexcept { return g_textures[static_cast<size_t>(id)]; }

void set_cull_mode(metaforce::ERglCullMode mode) noexcept { g_cullMode = mode; }
void set_blend_mode(metaforce::ERglBlendMode mode, metaforce::ERglBlendFactor src, metaforce::ERglBlendFactor dst,
                    metaforce::ERglLogicOp op) noexcept {
  g_blendMode = mode;
  g_blendFacSrc = src;
  g_blendFacDst = dst;
  g_blendOp = op;
}
void set_depth_mode(bool compare_enable, metaforce::ERglEnum func, bool update_enable) noexcept {
  g_depthCompare = compare_enable;
  g_depthFunc = func;
  g_depthUpdate = update_enable;
}
void set_tev_reg_color(GX::TevRegID id, const zeus::CColor& color) noexcept {
  if (id < GX::TEVREG0 || id > GX::TEVREG2) {
    Log.report(logvisor::Fatal, FMT_STRING("set_tev_reg_color: bad reg {}"), id);
    unreachable();
  }
  g_colorRegs[id - 1] = color;
}
void set_tev_k_color(GX::TevKColorID id, const zeus::CColor& color) noexcept {
  if (id >= GX::MAX_KCOLOR) {
    Log.report(logvisor::Fatal, FMT_STRING("set_tev_k_color: bad reg {}"), id);
    unreachable();
  }
  g_kcolors[id] = color;
}
void set_alpha_update(bool enabled) noexcept { g_alphaUpdate = enabled; }
void set_dst_alpha(bool enabled, float value) noexcept {
  if (enabled) {
    g_dstAlpha = value;
  } else {
    g_dstAlpha.reset();
  }
}
void set_clear_color(const zeus::CColor& color) noexcept { g_clearColor = color; }
void set_alpha_discard(bool v) { g_alphaDiscard = v; }

void update_model_view(const zeus::CMatrix4f& mv, const zeus::CMatrix4f& mv_inv) noexcept {
  g_mv = mv;
  g_mvInv = mv_inv;
}
constexpr zeus::CMatrix4f DepthCorrect{
    // clang-format off
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 0.5f, 0.5f,
    0.f, 0.f, 0.f, 1.f,
    // clang-format on
};
void update_projection(const zeus::CMatrix4f& proj) noexcept { g_proj = DepthCorrect * proj; }
void update_fog_state(const metaforce::CFogState& state) noexcept { g_fogState = state; }

void disable_tev_stage(metaforce::ERglTevStage stage) noexcept { g_tevStages[static_cast<size_t>(stage)].reset(); }
void update_tev_stage(metaforce::ERglTevStage stage, const metaforce::CTevCombiners::ColorPass& colPass,
                      const metaforce::CTevCombiners::AlphaPass& alphaPass,
                      const metaforce::CTevCombiners::CTevOp& colorOp,
                      const metaforce::CTevCombiners::CTevOp& alphaOp) noexcept {
  g_tevStages[static_cast<size_t>(stage)] = {colPass, alphaPass, colorOp, alphaOp};
}

void set_tev_order(GX::TevStageID id, GX::TexCoordID tcid, GX::TexMapID tmid, GX::ChannelID cid) noexcept {
  auto& stage = g_tevStages[id];
  if (!stage) {
    Log.report(logvisor::Fatal, FMT_STRING("set_tev_order: disabled stage {}"), id);
    unreachable();
  }
  stage->texCoordId = tcid;
  stage->texMapId = tmid;
  stage->channelId = cid;
}
void set_tev_k_color_sel(GX::TevStageID id, GX::TevKColorSel sel) noexcept {
  auto& stage = g_tevStages[id];
  if (!stage) {
    Log.report(logvisor::Fatal, FMT_STRING("set_tev_k_color_sel: disabled stage {}"), id);
    unreachable();
  }
  stage->kcSel = sel;
}
void set_tev_k_alpha_sel(GX::TevStageID id, GX::TevKAlphaSel sel) noexcept {
  auto& stage = g_tevStages[id];
  if (!stage) {
    Log.report(logvisor::Fatal, FMT_STRING("set_tev_k_alpha_sel: disabled stage {}"), id);
    unreachable();
  }
  stage->kaSel = sel;
}

void set_chan_amb_color(GX::ChannelID id, const zeus::CColor& color) noexcept {
  if (id < GX::COLOR0A0 || id > GX::COLOR1A1) {
    Log.report(logvisor::Fatal, FMT_STRING("set_chan_amb_color: invalid channel {}"), id);
    unreachable();
  }
  g_colorChannels[id - GX::COLOR0A0].ambColor = color;
}
void set_chan_mat_color(GX::ChannelID id, const zeus::CColor& color) noexcept {
  if (id < GX::COLOR0A0 || id > GX::COLOR1A1) {
    Log.report(logvisor::Fatal, FMT_STRING("set_chan_mat_color: invalid channel {}"), id);
    unreachable();
  }
  g_colorChannels[id - GX::COLOR0A0].matColor = color;
}

void load_light(GX::LightID id, const Light& light) noexcept { g_lights[id] = light; }
void load_light_ambient(GX::LightID id, const zeus::CColor& ambient) noexcept { g_lights[id] = ambient; }
void set_light_state(std::bitset<MaxLights> bits) noexcept { g_lightState = bits; }

// Pipeline helpers
static inline wgpu::BlendFactor to_blend_factor(metaforce::ERglBlendFactor fac) {
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

static inline wgpu::CompareFunction to_compare_function(metaforce::ERglEnum func) {
  switch (func) {
  case metaforce::ERglEnum::Never:
    return wgpu::CompareFunction::Never;
  case metaforce::ERglEnum::Less:
    return wgpu::CompareFunction::Less;
  case metaforce::ERglEnum::Equal:
    return wgpu::CompareFunction::Equal;
  case metaforce::ERglEnum::LEqual:
    return wgpu::CompareFunction::LessEqual;
  case metaforce::ERglEnum::Greater:
    return wgpu::CompareFunction::Greater;
  case metaforce::ERglEnum::NEqual:
    return wgpu::CompareFunction::NotEqual;
  case metaforce::ERglEnum::GEqual:
    return wgpu::CompareFunction::GreaterEqual;
  case metaforce::ERglEnum::Always:
    return wgpu::CompareFunction::Always;
  }
}

static inline wgpu::BlendState to_blend_state(metaforce::ERglBlendMode mode, metaforce::ERglBlendFactor srcFac,
                                              metaforce::ERglBlendFactor dstFac, std::optional<float> dstAlpha) {
  if (mode != metaforce::ERglBlendMode::Blend) {
    Log.report(logvisor::Fatal, FMT_STRING("How to {}?"), magic_enum::enum_name(mode));
  }
  const auto colorBlendComponent = wgpu::BlendComponent{
      .operation = wgpu::BlendOperation::Add,
      .srcFactor = to_blend_factor(srcFac),
      .dstFactor = to_blend_factor(dstFac),
  };
  auto alphaBlendComponent = colorBlendComponent;
  if (dstAlpha) {
    alphaBlendComponent = wgpu::BlendComponent{
        .operation = wgpu::BlendOperation::Add,
        .srcFactor = wgpu::BlendFactor::Zero,
        .dstFactor = wgpu::BlendFactor::Constant,
    };
  }
  return {
      .color = colorBlendComponent,
      .alpha = alphaBlendComponent,
  };
}

static inline wgpu::ColorWriteMask to_write_mask(bool alphaUpdate) {
  auto writeMask = wgpu::ColorWriteMask::Red | wgpu::ColorWriteMask::Green | wgpu::ColorWriteMask::Blue;
  if (alphaUpdate) {
    writeMask = writeMask | wgpu::ColorWriteMask::Alpha;
  }
  return writeMask;
}

static inline wgpu::PrimitiveState to_primitive_state(GX::Primitive gx_prim, metaforce::ERglCullMode e_cullMode) {
  wgpu::PrimitiveTopology primitive = wgpu::PrimitiveTopology::TriangleList;
  switch (gx_prim) {
  case GX::TRIANGLES:
    break;
  case GX::TRIANGLESTRIP:
    primitive = wgpu::PrimitiveTopology::TriangleStrip;
    break;
  default:
    Log.report(logvisor::Fatal, FMT_STRING("Unsupported primitive type {}"), gx_prim);
    unreachable();
  }
  wgpu::FrontFace frontFace = wgpu::FrontFace::CCW;
  wgpu::CullMode cullMode = wgpu::CullMode::None;
  switch (e_cullMode) {
  case metaforce::ERglCullMode::Front:
    frontFace = wgpu::FrontFace::CW;
    cullMode = wgpu::CullMode::Front;
    break;
  case metaforce::ERglCullMode::Back:
    cullMode = wgpu::CullMode::Back;
    break;
  default:
    break;
  }
  return {
      .topology = primitive,
      .frontFace = frontFace,
      .cullMode = cullMode,
  };
}

wgpu::RenderPipeline build_pipeline(wgpu::PipelineLayout layout, ArrayRef<wgpu::VertexBufferLayout> vtxBuffers,
                                    const GXPipelineConfig& config, zstring_view label) noexcept {
  using gpu::g_graphicsConfig;
  const auto depthStencil = wgpu::DepthStencilState{
      .format = g_graphicsConfig.depthFormat,
      .depthWriteEnabled = config.depthUpdate,
      .depthCompare = to_compare_function(config.depthFunc),
  };
  const auto blendState = to_blend_state(config.blendMode, config.blendFacSrc, config.blendFacDst, config.dstAlpha);
  const std::array colorTargets{wgpu::ColorTargetState{
      .format = g_graphicsConfig.colorFormat,
      .blend = &blendState,
      .writeMask = to_write_mask(config.alphaUpdate),
  }};
  const auto shader = build_shader(config.shaderConfig);
  const auto fragmentState = wgpu::FragmentState{
      .module = shader,
      .entryPoint = "fs_main",
      .targetCount = colorTargets.size(),
      .targets = colorTargets.data(),
  };
  const auto descriptor = wgpu::RenderPipelineDescriptor{
      .label = label.c_str(),
      .layout = std::move(layout),
      .vertex =
          {
              .module = shader,
              .entryPoint = "vs_main",
              .bufferCount = static_cast<uint32_t>(vtxBuffers.size()),
              .buffers = vtxBuffers.data(),
          },
      .primitive = to_primitive_state(config.primitive, config.cullMode),
      .depthStencil = &depthStencil,
      .multisample =
          wgpu::MultisampleState{
              .count = g_graphicsConfig.msaaSamples,
          },
      .fragment = &fragmentState,
  };
  return gpu::g_device.CreateRenderPipeline(&descriptor);
}

void populate_gx_pipeline_config(GXPipelineConfig& config, GX::Primitive primitive,
                                 std::bitset<maxTextures> enabledTextures) noexcept {
  for (size_t idx = 0; const auto& item : g_tevStages) {
    // Copy until disabled TEV stage (indicating end)
    if (!item) {
      break;
    }
    config.shaderConfig.tevStages[idx++] = item;
  }
  config.shaderConfig.alphaDiscard = g_alphaDiscard;
  for (size_t idx = 0; const auto& item : g_textures) {
    if (enabledTextures.test(idx) && item) {
      config.shaderConfig.boundTextures.set(idx);
    }
  }
  config = {
      .shaderConfig = config.shaderConfig,
      .primitive = primitive,
      .depthFunc = g_depthFunc,
      .cullMode = g_cullMode,
      .blendMode = g_blendMode,
      .blendFacSrc = g_blendFacSrc,
      .blendFacDst = g_blendFacDst,
      .blendOp = g_blendOp,
      .dstAlpha = g_dstAlpha,
      .depthCompare = g_depthCompare,
      .depthUpdate = g_depthUpdate,
      .alphaUpdate = g_alphaUpdate,
  };
}

wgpu::SamplerDescriptor STextureBind::get_descriptor() const noexcept {
  wgpu::AddressMode mode;
  switch (clampMode) {
  case metaforce::EClampMode::Clamp:
    mode = wgpu::AddressMode::ClampToEdge;
    break;
  case metaforce::EClampMode::Repeat:
    mode = wgpu::AddressMode::Repeat;
    break;
  case metaforce::EClampMode::Mirror:
    mode = wgpu::AddressMode::MirrorRepeat;
    break;
  }
  return {
      .label = "Generated Sampler",
      .addressModeU = mode,
      .addressModeV = mode,
      .addressModeW = mode,
      // TODO logic from CTexture?
      .magFilter = wgpu::FilterMode::Linear,
      .minFilter = wgpu::FilterMode::Linear,
      .mipmapFilter = wgpu::FilterMode::Linear,
      .maxAnisotropy = gpu::g_graphicsConfig.textureAnistropy,
  };
}
} // namespace aurora::gfx
