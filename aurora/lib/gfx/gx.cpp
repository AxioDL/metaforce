#include "gx.hpp"

#include "../gpu.hpp"
#include "common.hpp"

#include <unordered_map>
#include <magic_enum.hpp>

namespace aurora::gfx {
static logvisor::Module Log("aurora::gfx::gx");

// TODO remove this hack for build_shader
extern std::mutex g_pipelineMutex;

// GX state
void bind_texture(GX::TexMapID id, metaforce::EClampMode clamp, const TextureHandle& tex, float lod) noexcept {
  gx::g_textures[static_cast<size_t>(id)] = {tex, clamp, lod};
}
void unbind_texture(GX::TexMapID id) noexcept { gx::g_textures[static_cast<size_t>(id)].reset(); }

void set_cull_mode(metaforce::ERglCullMode mode) noexcept { gx::g_cullMode = mode; }
void set_blend_mode(metaforce::ERglBlendMode mode, metaforce::ERglBlendFactor src, metaforce::ERglBlendFactor dst,
                    metaforce::ERglLogicOp op) noexcept {
  gx::g_blendMode = mode;
  gx::g_blendFacSrc = src;
  gx::g_blendFacDst = dst;
  gx::g_blendOp = op;
}
void set_depth_mode(bool compare_enable, metaforce::ERglEnum func, bool update_enable) noexcept {
  gx::g_depthCompare = compare_enable;
  gx::g_depthFunc = func;
  gx::g_depthUpdate = update_enable;
}
void set_tev_reg_color(GX::TevRegID id, const zeus::CColor& color) noexcept {
  if (id < GX::TEVREG0 || id > GX::TEVREG2) {
    Log.report(logvisor::Fatal, FMT_STRING("set_tev_reg_color: bad reg {}"), id);
    unreachable();
  }
  gx::g_colorRegs[id - 1] = color;
}
void set_tev_k_color(GX::TevKColorID id, const zeus::CColor& color) noexcept {
  if (id >= GX::MAX_KCOLOR) {
    Log.report(logvisor::Fatal, FMT_STRING("set_tev_k_color: bad reg {}"), id);
    unreachable();
  }
  gx::g_kcolors[id] = color;
}
void set_alpha_update(bool enabled) noexcept { gx::g_alphaUpdate = enabled; }
void set_dst_alpha(bool enabled, float value) noexcept {
  if (enabled) {
    gx::g_dstAlpha = value;
  } else {
    gx::g_dstAlpha.reset();
  }
}
void set_clear_color(const zeus::CColor& color) noexcept { gx::g_clearColor = color; }
void set_alpha_discard(bool v) { gx::g_alphaDiscard = v; }

void update_model_view(const zeus::CMatrix4f& mv, const zeus::CMatrix4f& mv_inv) noexcept {
  gx::g_mv = mv;
  gx::g_mvInv = mv_inv;
}
constexpr zeus::CMatrix4f DepthCorrect{
    // clang-format off
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 0.5f, 0.5f,
    0.f, 0.f, 0.f, 1.f,
    // clang-format on
};
void update_projection(const zeus::CMatrix4f& proj) noexcept { gx::g_proj = DepthCorrect * proj; }
void update_fog_state(const metaforce::CFogState& state) noexcept { gx::g_fogState = state; }

void disable_tev_stage(metaforce::ERglTevStage stage) noexcept { gx::g_tevStages[static_cast<size_t>(stage)].reset(); }
void update_tev_stage(metaforce::ERglTevStage stage, const metaforce::CTevCombiners::ColorPass& colPass,
                      const metaforce::CTevCombiners::AlphaPass& alphaPass,
                      const metaforce::CTevCombiners::CTevOp& colorOp,
                      const metaforce::CTevCombiners::CTevOp& alphaOp) noexcept {
  gx::g_tevStages[static_cast<size_t>(stage)] = {colPass, alphaPass, colorOp, alphaOp};
}

void set_tev_order(GX::TevStageID id, GX::TexCoordID tcid, GX::TexMapID tmid, GX::ChannelID cid) noexcept {
  auto& stage = gx::g_tevStages[id];
  if (!stage) {
    // Log.report(logvisor::Fatal, FMT_STRING("set_tev_order: disabled stage {}"), id);
    // unreachable();
    return;
  }
  stage->texCoordId = tcid;
  stage->texMapId = tmid;
  stage->channelId = cid;
}
void set_tev_k_color_sel(GX::TevStageID id, GX::TevKColorSel sel) noexcept {
  auto& stage = gx::g_tevStages[id];
  if (!stage) {
    Log.report(logvisor::Fatal, FMT_STRING("set_tev_k_color_sel: disabled stage {}"), id);
    unreachable();
  }
  stage->kcSel = sel;
}
void set_tev_k_alpha_sel(GX::TevStageID id, GX::TevKAlphaSel sel) noexcept {
  auto& stage = gx::g_tevStages[id];
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
  gx::g_colorChannels[id - GX::COLOR0A0].ambColor = color;
}
void set_chan_mat_color(GX::ChannelID id, const zeus::CColor& color) noexcept {
  if (id < GX::COLOR0A0 || id > GX::COLOR1A1) {
    Log.report(logvisor::Fatal, FMT_STRING("set_chan_mat_color: invalid channel {}"), id);
    unreachable();
  }
  gx::g_colorChannels[id - GX::COLOR0A0].matColor = color;
}
void set_chan_mat_src(GX::ChannelID id, GX::ColorSrc src) noexcept {
  if (id < GX::COLOR0A0 || id > GX::COLOR1A1) {
    Log.report(logvisor::Fatal, FMT_STRING("set_chan_mat_src: invalid channel {}"), id);
    unreachable();
  }
  gx::g_colorChannels[id - GX::COLOR0A0].matSrc = src;
}

static inline u8 light_idx(GX::LightID id) {
#ifdef _MSC_VER
  unsigned long r = 0;
  _BitScanForward(&r, id);
  return r;
#else
  return __builtin_ctz(id);
#endif
}
void load_light(GX::LightID id, const Light& light) noexcept { gx::g_lights[light_idx(id)] = light; }
void load_light_ambient(GX::LightID id, const zeus::CColor& ambient) noexcept { gx::g_lights[light_idx(id)] = ambient; }
void set_light_state(std::bitset<MaxLights> bits) noexcept { gx::g_lightState = bits; }

namespace gx {
using gpu::g_device;
using gpu::g_graphicsConfig;

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
std::array<TextureBind, maxTextures> g_textures;

const gx::TextureBind& get_texture(GX::TexMapID id) noexcept { return gx::g_textures[static_cast<size_t>(id)]; }

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
  default:
    Log.report(logvisor::Fatal, FMT_STRING("invalid blend factor {}"), fac);
    unreachable();
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
  default:
    Log.report(logvisor::Fatal, FMT_STRING("invalid depth fn {}"), func);
    unreachable();
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

wgpu::RenderPipeline build_pipeline(const PipelineConfig& config, const ShaderInfo& info,
                                    ArrayRef<wgpu::VertexBufferLayout> vtxBuffers, wgpu::ShaderModule shader,
                                    zstring_view label) noexcept {
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
  const auto fragmentState = wgpu::FragmentState{
      .module = shader,
      .entryPoint = "fs_main",
      .targetCount = colorTargets.size(),
      .targets = colorTargets.data(),
  };
  auto layouts = build_bind_group_layouts(info, config.shaderConfig);
  const std::array bindGroupLayouts{
      std::move(layouts.uniformLayout),
      std::move(layouts.samplerLayout),
      std::move(layouts.textureLayout),
  };
  const auto pipelineLayoutDescriptor = wgpu::PipelineLayoutDescriptor{
      .label = "GX Pipeline Layout",
      .bindGroupLayoutCount = static_cast<uint32_t>(info.sampledTextures.any() ? bindGroupLayouts.size() : 1),
      .bindGroupLayouts = bindGroupLayouts.data(),
  };
  auto pipelineLayout = g_device.CreatePipelineLayout(&pipelineLayoutDescriptor);
  const auto descriptor = wgpu::RenderPipelineDescriptor{
      .label = label.c_str(),
      .layout = std::move(pipelineLayout),
      .vertex =
          {
              .module = std::move(shader),
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
  return g_device.CreateRenderPipeline(&descriptor);
}

ShaderInfo populate_pipeline_config(PipelineConfig& config, GX::Primitive primitive,
                                    const BindGroupRanges& ranges) noexcept {
  for (size_t idx = 0; const auto& item : g_tevStages) {
    // Copy until disabled TEV stage (indicating end)
    if (!item) {
      break;
    }
    config.shaderConfig.tevStages[idx++] = item;
  }
  config.shaderConfig.channelMatSrcs[0] = g_colorChannels[0].matSrc;
  config.shaderConfig.channelMatSrcs[1] = g_colorChannels[1].matSrc;
  config.shaderConfig.alphaDiscard = g_alphaDiscard;
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
  // TODO separate shader info from build_shader for async
  {
    std::lock_guard lk{g_pipelineMutex};
    auto [_, info] = build_shader(config.shaderConfig);
    info.bindGroups = build_bind_groups(info, config.shaderConfig, ranges); // TODO this is hack
    return info;
  }
}

Range build_uniform(const ShaderInfo& info) noexcept {
  auto [buf, range] = map_uniform(info.uniformSize);
  {
    buf.append(&g_mv, 64);
    buf.append(&g_mvInv, 64);
    buf.append(&g_proj, 64);
  }
  for (int i = 0; i < info.usesTevReg.size(); ++i) {
    if (!info.usesTevReg.test(i)) {
      continue;
    }
    buf.append(&g_colorRegs[i], 16);
  }
  if (info.sampledColorChannels.any()) {
    zeus::CColor ambient = zeus::skClear;
    int addedLights = 0;
    for (int i = 0; i < g_lightState.size(); ++i) {
      if (!g_lightState.test(i)) {
        continue;
      }
      const auto& variant = g_lights[i];
      if (std::holds_alternative<zeus::CColor>(variant)) {
        ambient += std::get<zeus::CColor>(variant);
      } else if (std::holds_alternative<Light>(variant)) {
        static_assert(sizeof(Light) == 80);
        buf.append(&std::get<Light>(variant), sizeof(Light));
        ++addedLights;
      }
    }
    constexpr Light emptyLight{};
    for (int i = addedLights; i < MaxLights; ++i) {
      buf.append(&emptyLight, sizeof(Light));
    }
    buf.append(&ambient, 16);
//    fmt::print(FMT_STRING("Added lights: {}, ambient: {},{},{},{}\n"), addedLights, ambient.r(), ambient.g(), ambient.b(), ambient.a());
  }
  for (int i = 0; i < info.sampledColorChannels.size(); ++i) {
    if (!info.sampledColorChannels.test(i)) {
      continue;
    }
    buf.append(&g_colorChannels[i].ambColor, 16);
    buf.append(&g_colorChannels[i].matColor, 16);
  }
  for (int i = 0; i < info.sampledKcolors.size(); ++i) {
    if (!info.sampledKcolors.test(i)) {
      continue;
    }
    buf.append(&g_kcolors[i], 16);
  }
  for (int i = 0; i < info.sampledTextures.size(); ++i) {
    if (!info.sampledTextures.test(i)) {
      continue;
    }
    const auto& tex = get_texture(static_cast<GX::TexMapID>(i));
    if (!tex) {
      Log.report(logvisor::Fatal, FMT_STRING("unbound texture {}"), i);
      unreachable();
    }
    buf.append(&tex.lod, 4);
  }
  return range;
}

static std::unordered_map<u32, wgpu::BindGroupLayout> sUniformBindGroupLayouts;
static std::unordered_map<u32, std::pair<wgpu::BindGroupLayout, wgpu::BindGroupLayout>> sTextureBindGroupLayouts;

GXBindGroups build_bind_groups(const ShaderInfo& info, const ShaderConfig& config,
                               const BindGroupRanges& ranges) noexcept {
  const auto layouts = build_bind_group_layouts(info, config);
  u32 textureCount = info.sampledTextures.count();

  const std::array uniformEntries{
      wgpu::BindGroupEntry{
          .binding = 0,
          .buffer = g_uniformBuffer,
          .size = info.uniformSize,
      },
      // Vertices
      wgpu::BindGroupEntry{
          .binding = 1,
          .buffer = g_storageBuffer,
          .size = ranges.vtxDataRange.second - ranges.vtxDataRange.first,
      },
      // Normals
      wgpu::BindGroupEntry{
          .binding = 2,
          .buffer = g_storageBuffer,
          .size = ranges.nrmDataRange.second - ranges.nrmDataRange.first,
      },
      // UVs
      wgpu::BindGroupEntry{
          .binding = 3,
          .buffer = g_storageBuffer,
          .size = ranges.tcDataRange.second - ranges.tcDataRange.first,
      },
      // Packed UVs
      wgpu::BindGroupEntry{
          .binding = 4,
          .buffer = g_storageBuffer,
          .size = ranges.packedTcDataRange.second - ranges.packedTcDataRange.first,
      },
  };
  std::array<wgpu::BindGroupEntry, maxTextures> samplerEntries;
  std::array<wgpu::BindGroupEntry, maxTextures> textureEntries;
  for (u32 texIdx = 0, i = 0; texIdx < info.sampledTextures.size(); ++texIdx) {
    if (!info.sampledTextures.test(texIdx)) {
      continue;
    }
    const auto& tex = g_textures[texIdx];
    if (!tex) {
      Log.report(logvisor::Fatal, FMT_STRING("unbound texture {}"), texIdx);
      unreachable();
    }
    samplerEntries[i] = {
        .binding = i,
        .sampler = sampler_ref(tex.get_descriptor()),
    };
    textureEntries[i] = {
        .binding = i,
        .textureView = tex.handle.ref->view,
    };
    i++;
  }
  return {
      .uniformBindGroup = bind_group_ref(wgpu::BindGroupDescriptor{
          .label = "GX Uniform Bind Group",
          .layout = layouts.uniformLayout,
          .entryCount = static_cast<uint32_t>(config.denormalizedVertexAttributes ? 1 : uniformEntries.size()),
          .entries = uniformEntries.data(),
      }),
      .samplerBindGroup = bind_group_ref(wgpu::BindGroupDescriptor{
          .label = "GX Sampler Bind Group",
          .layout = layouts.samplerLayout,
          .entryCount = textureCount,
          .entries = samplerEntries.data(),
      }),
      .textureBindGroup = bind_group_ref(wgpu::BindGroupDescriptor{
          .label = "GX Texture Bind Group",
          .layout = layouts.textureLayout,
          .entryCount = textureCount,
          .entries = textureEntries.data(),
      }),
  };
}

GXBindGroupLayouts build_bind_group_layouts(const ShaderInfo& info, const ShaderConfig& config) noexcept {
  GXBindGroupLayouts out;
  u32 uniformSizeKey = info.uniformSize + (config.denormalizedVertexAttributes ? 0 : 1);
  if (sUniformBindGroupLayouts.contains(uniformSizeKey)) {
    out.uniformLayout = sUniformBindGroupLayouts[uniformSizeKey];
  } else {
    const std::array uniformLayoutEntries{
        wgpu::BindGroupLayoutEntry{
            .binding = 0,
            .visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
            .buffer =
                wgpu::BufferBindingLayout{
                    .type = wgpu::BufferBindingType::Uniform,
                    .hasDynamicOffset = true,
                    .minBindingSize = info.uniformSize,
                },
        },
        wgpu::BindGroupLayoutEntry{
            .binding = 1,
            .visibility = wgpu::ShaderStage::Vertex,
            .buffer =
                {
                    .type = wgpu::BufferBindingType::ReadOnlyStorage,
                    .hasDynamicOffset = true,
                },
        },
        wgpu::BindGroupLayoutEntry{
            .binding = 2,
            .visibility = wgpu::ShaderStage::Vertex,
            .buffer =
                {
                    .type = wgpu::BufferBindingType::ReadOnlyStorage,
                    .hasDynamicOffset = true,
                },
        },
        wgpu::BindGroupLayoutEntry{
            .binding = 3,
            .visibility = wgpu::ShaderStage::Vertex,
            .buffer =
                {
                    .type = wgpu::BufferBindingType::ReadOnlyStorage,
                    .hasDynamicOffset = true,
                },
        },
        wgpu::BindGroupLayoutEntry{
            .binding = 4,
            .visibility = wgpu::ShaderStage::Vertex,
            .buffer =
                {
                    .type = wgpu::BufferBindingType::ReadOnlyStorage,
                    .hasDynamicOffset = true,
                },
        },
    };
    const auto uniformLayoutDescriptor = wgpu::BindGroupLayoutDescriptor{
        .label = "GX Uniform Bind Group Layout",
        .entryCount = static_cast<uint32_t>(config.denormalizedVertexAttributes ? 1 : uniformLayoutEntries.size()),
        .entries = uniformLayoutEntries.data(),
    };
    out.uniformLayout = g_device.CreateBindGroupLayout(&uniformLayoutDescriptor);
    sUniformBindGroupLayouts.try_emplace(uniformSizeKey, out.uniformLayout);
  }

  u32 textureCount = info.sampledTextures.count();
  if (sTextureBindGroupLayouts.contains(textureCount)) {
    const auto& [sl, tl] = sTextureBindGroupLayouts[textureCount];
    out.samplerLayout = sl;
    out.textureLayout = tl;
  } else {
    std::array<wgpu::BindGroupLayoutEntry, maxTextures> samplerEntries;
    std::array<wgpu::BindGroupLayoutEntry, maxTextures> textureEntries;
    for (u32 i = 0; i < textureCount; ++i) {
      samplerEntries[i] = {
          .binding = i,
          .visibility = wgpu::ShaderStage::Fragment,
          .sampler = {.type = wgpu::SamplerBindingType::Filtering},
      };
      textureEntries[i] = {
          .binding = i,
          .visibility = wgpu::ShaderStage::Fragment,
          .texture =
              {
                  .sampleType = wgpu::TextureSampleType::Float,
                  .viewDimension = wgpu::TextureViewDimension::e2D,
              },
      };
    }
    {
      const wgpu::BindGroupLayoutDescriptor descriptor{
          .label = "GX Sampler Bind Group",
          .entryCount = textureCount,
          .entries = samplerEntries.data(),
      };
      out.samplerLayout = g_device.CreateBindGroupLayout(&descriptor);
    }
    {
      const wgpu::BindGroupLayoutDescriptor descriptor{
          .label = "GX Texture Bind Group",
          .entryCount = textureCount,
          .entries = textureEntries.data(),
      };
      out.textureLayout = g_device.CreateBindGroupLayout(&descriptor);
    }
    sTextureBindGroupLayouts.try_emplace(textureCount, out.samplerLayout, out.textureLayout);
  }
  return out;
}

// TODO this is awkward
extern std::unordered_map<ShaderRef, std::pair<wgpu::ShaderModule, gx::ShaderInfo>> g_gxCachedShaders;
void shutdown() noexcept {
  // TODO we should probably store this all in g_state.gx instead
  sUniformBindGroupLayouts.clear();
  sTextureBindGroupLayouts.clear();
  g_textures.fill({});
  g_gxCachedShaders.clear();
}

wgpu::SamplerDescriptor TextureBind::get_descriptor() const noexcept {
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
      .maxAnisotropy = g_graphicsConfig.textureAnistropy,
  };
}
} // namespace gx
} // namespace aurora::gfx
