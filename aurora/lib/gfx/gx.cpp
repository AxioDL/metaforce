#include "gx.hpp"

#include "../gpu.hpp"
#include "common.hpp"

#include <unordered_map>

using aurora::gfx::gx::g_gxState;
static logvisor::Module Log("aurora::gx");

void GXSetNumChans(u8 num) noexcept { g_gxState.numChans = num; }
void GXSetNumIndStages(u8 num) noexcept { g_gxState.numIndStages = num; }
void GXSetNumTevStages(u8 num) noexcept { g_gxState.numTevStages = num; }
void GXSetNumTexGens(u8 num) noexcept { g_gxState.numTexGens = num; }
void GXSetTevAlphaIn(GX::TevStageID stageId, GX::TevAlphaArg a, GX::TevAlphaArg b, GX::TevAlphaArg c,
                     GX::TevAlphaArg d) noexcept {
  g_gxState.tevStages[stageId].alphaPass = {a, b, c, d};
}
void GXSetTevAlphaOp(GX::TevStageID stageId, GX::TevOp op, GX::TevBias bias, GX::TevScale scale, bool clamp,
                     GX::TevRegID outReg) noexcept {
  g_gxState.tevStages[stageId].alphaOp = {op, bias, scale, outReg, clamp};
}
void GXSetTevColorIn(GX::TevStageID stageId, GX::TevColorArg a, GX::TevColorArg b, GX::TevColorArg c,
                     GX::TevColorArg d) noexcept {
  g_gxState.tevStages[stageId].colorPass = {a, b, c, d};
}
void GXSetTevColorOp(GX::TevStageID stageId, GX::TevOp op, GX::TevBias bias, GX::TevScale scale, bool clamp,
                     GX::TevRegID outReg) noexcept {
  g_gxState.tevStages[stageId].colorOp = {op, bias, scale, outReg, clamp};
}
void GXSetCullMode(GX::CullMode mode) noexcept { g_gxState.cullMode = mode; }
void GXSetBlendMode(GX::BlendMode mode, GX::BlendFactor src, GX::BlendFactor dst, GX::LogicOp op) noexcept {
  g_gxState.blendMode = mode;
  g_gxState.blendFacSrc = src;
  g_gxState.blendFacDst = dst;
  g_gxState.blendOp = op;
}
void GXSetZMode(bool compare_enable, GX::Compare func, bool update_enable) noexcept {
  g_gxState.depthCompare = compare_enable;
  g_gxState.depthFunc = func;
  g_gxState.depthUpdate = update_enable;
}
void GXSetTevColor(GX::TevRegID id, const zeus::CColor& color) noexcept {
  if (id < GX::TEVREG0 || id > GX::TEVREG2) {
    Log.report(logvisor::Fatal, FMT_STRING("bad tevreg {}"), id);
    unreachable();
  }
  g_gxState.colorRegs[id - 1] = color;
}
void GXSetTevKColor(GX::TevKColorID id, const zeus::CColor& color) noexcept {
  if (id >= GX::MAX_KCOLOR) {
    Log.report(logvisor::Fatal, FMT_STRING("bad kcolor {}"), id);
    unreachable();
  }
  g_gxState.kcolors[id] = color;
}
void GXSetAlphaUpdate(bool enabled) noexcept { g_gxState.alphaUpdate = enabled; }
void GXSetDstAlpha(bool enabled, float value) noexcept {
  if (enabled) {
    g_gxState.dstAlpha = value;
  } else {
    g_gxState.dstAlpha.reset();
  }
}
void GXSetCopyClear(const zeus::CColor& color, float depth) noexcept { g_gxState.clearColor = color; }
void GXSetTevOrder(GX::TevStageID id, GX::TexCoordID tcid, GX::TexMapID tmid, GX::ChannelID cid) noexcept {
  auto& stage = g_gxState.tevStages[id];
  stage.texCoordId = tcid;
  stage.texMapId = tmid;
  stage.channelId = cid;
}
void GXSetTevKColorSel(GX::TevStageID id, GX::TevKColorSel sel) noexcept { g_gxState.tevStages[id].kcSel = sel; }
void GXSetTevKAlphaSel(GX::TevStageID id, GX::TevKAlphaSel sel) noexcept { g_gxState.tevStages[id].kaSel = sel; }
void GXSetChanAmbColor(GX::ChannelID id, const zeus::CColor& color) noexcept {
  if (id < GX::COLOR0A0 || id > GX::COLOR1A1) {
    Log.report(logvisor::Fatal, FMT_STRING("bad channel {}"), id);
    unreachable();
  }
  g_gxState.colorChannelState[id - GX::COLOR0A0].ambColor = color;
}
void GXSetChanMatColor(GX::ChannelID id, const zeus::CColor& color) noexcept {
  if (id < GX::COLOR0A0 || id > GX::COLOR1A1) {
    Log.report(logvisor::Fatal, FMT_STRING("bad channel {}"), id);
    unreachable();
  }
  g_gxState.colorChannelState[id - GX::COLOR0A0].matColor = color;
}
void GXSetChanCtrl(GX::ChannelID id, bool lightingEnabled, GX::ColorSrc ambSrc, GX::ColorSrc matSrc,
                   GX::LightMask lightState, GX::DiffuseFn diffFn, GX::AttnFn attnFn) noexcept {
  if (id < GX::COLOR0A0 || id > GX::COLOR1A1) {
    Log.report(logvisor::Fatal, FMT_STRING("bad channel {}"), id);
    unreachable();
  }
  if (diffFn != GX::DF_NONE && diffFn != GX::DF_CLAMP) {
    Log.report(logvisor::Fatal, FMT_STRING("unhandled diffuse fn {}"), diffFn);
    unreachable();
  }
  if (attnFn != GX::AF_NONE && attnFn != GX::AF_SPOT) {
    Log.report(logvisor::Fatal, FMT_STRING("unhandled attn fn {}"), attnFn);
    unreachable();
  }
  u32 idx = id - GX::COLOR0A0;
  auto& chan = g_gxState.colorChannelConfig[idx];
  chan.lightingEnabled = lightingEnabled;
  chan.ambSrc = ambSrc;
  chan.matSrc = matSrc;
  g_gxState.colorChannelState[idx].lightState = lightState;
}
void GXSetAlphaCompare(GX::Compare comp0, float ref0, GX::AlphaOp op, GX::Compare comp1, float ref1) noexcept {
  if (comp0 == GX::ALWAYS && comp1 == GX::ALWAYS) {
    g_gxState.alphaDiscard.reset();
  } else if (comp0 == GX::GEQUAL && comp1 == GX::NEVER) {
    g_gxState.alphaDiscard = ref0;
  } else {
    Log.report(logvisor::Fatal, FMT_STRING("GXSetAlphaCompare: unknown operands"));
    unreachable();
  }
}
void GXSetVtxDescv(GX::VtxDescList* list) noexcept;

namespace aurora::gfx {
static logvisor::Module Log("aurora::gfx::gx");

// TODO remove this hack for build_shader
extern std::mutex g_pipelineMutex;

// GX state
void bind_texture(GX::TexMapID id, metaforce::EClampMode clamp, const TextureHandle& tex, float lod) noexcept {
  gx::g_gxState.textures[static_cast<size_t>(id)] = {tex, clamp, lod};
}
void unbind_texture(GX::TexMapID id) noexcept { gx::g_gxState.textures[static_cast<size_t>(id)].reset(); }

void update_model_view(const zeus::CMatrix4f& mv, const zeus::CMatrix4f& mv_inv) noexcept {
  gx::g_gxState.mv = mv;
  gx::g_gxState.mvInv = mv_inv;
}
constexpr zeus::CMatrix4f DepthCorrect{
    // clang-format off
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 0.5f, 0.5f,
    0.f, 0.f, 0.f, 1.f,
    // clang-format on
};
void update_projection(const zeus::CMatrix4f& proj) noexcept { gx::g_gxState.proj = DepthCorrect * proj; }
void update_fog_state(const metaforce::CFogState& state) noexcept { gx::g_gxState.fogState = state; }

void load_light(GX::LightID id, const Light& light) noexcept { gx::g_gxState.lights[std::log2<u32>(id)] = light; }
void load_light_ambient(GX::LightID id, const zeus::CColor& ambient) noexcept {
  gx::g_gxState.lights[std::log2<u32>(id)] = ambient;
}
void set_light_state(GX::LightMask bits) noexcept { gx::g_gxState.lightState = bits; }

namespace gx {
using gpu::g_device;
using gpu::g_graphicsConfig;

GXState g_gxState;

const TextureBind& get_texture(GX::TexMapID id) noexcept { return g_gxState.textures[static_cast<size_t>(id)]; }

static inline wgpu::BlendFactor to_blend_factor(GX::BlendFactor fac) {
  switch (fac) {
  case GX::BL_ZERO:
    return wgpu::BlendFactor::Zero;
  case GX::BL_ONE:
    return wgpu::BlendFactor::One;
  case GX::BL_SRCCLR:
    return wgpu::BlendFactor::Src;
  case GX::BL_INVSRCCLR:
    return wgpu::BlendFactor::OneMinusSrc;
  case GX::BL_SRCALPHA:
    return wgpu::BlendFactor::SrcAlpha;
  case GX::BL_INVSRCALPHA:
    return wgpu::BlendFactor::OneMinusSrcAlpha;
  case GX::BL_DSTALPHA:
    return wgpu::BlendFactor::DstAlpha;
  case GX::BL_INVDSTALPHA:
    return wgpu::BlendFactor::OneMinusDstAlpha;
  case GX::BL_DSTCLR:
    return wgpu::BlendFactor::Dst;
  case GX::BL_INVDSTCLR:
    return wgpu::BlendFactor::OneMinusDst;
  default:
    Log.report(logvisor::Fatal, FMT_STRING("invalid blend factor {}"), fac);
    unreachable();
  }
}

static inline wgpu::CompareFunction to_compare_function(GX::Compare func) {
  switch (func) {
  case GX::NEVER:
    return wgpu::CompareFunction::Never;
  case GX::LESS:
    return wgpu::CompareFunction::Less;
  case GX::EQUAL:
    return wgpu::CompareFunction::Equal;
  case GX::LEQUAL:
    return wgpu::CompareFunction::LessEqual;
  case GX::GREATER:
    return wgpu::CompareFunction::Greater;
  case GX::NEQUAL:
    return wgpu::CompareFunction::NotEqual;
  case GX::GEQUAL:
    return wgpu::CompareFunction::GreaterEqual;
  case GX::ALWAYS:
    return wgpu::CompareFunction::Always;
  default:
    Log.report(logvisor::Fatal, FMT_STRING("invalid depth fn {}"), func);
    unreachable();
  }
}

static inline wgpu::BlendState to_blend_state(GX::BlendMode mode, GX::BlendFactor srcFac, GX::BlendFactor dstFac,
                                              std::optional<float> dstAlpha) {
  if (mode != GX::BM_BLEND) {
    Log.report(logvisor::Fatal, FMT_STRING("How to {}?"), mode);
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

static inline wgpu::PrimitiveState to_primitive_state(GX::Primitive gx_prim, GX::CullMode gx_cullMode) {
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
  switch (gx_cullMode) {
  case GX::CULL_FRONT:
    frontFace = wgpu::FrontFace::CW;
    cullMode = wgpu::CullMode::Front;
    break;
  case GX::CULL_BACK:
    cullMode = wgpu::CullMode::Back;
    break;
  case GX::CULL_ALL:
    Log.report(logvisor::Fatal, FMT_STRING("Unsupported cull mode {}"), gx_cullMode);
    unreachable();
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
  for (size_t i = 0; i < g_gxState.numTevStages; ++i) {
    config.shaderConfig.tevStages[i] = g_gxState.tevStages[i];
  }
  config.shaderConfig.colorChannels = g_gxState.colorChannelConfig;
  config.shaderConfig.alphaDiscard = g_gxState.alphaDiscard;
  config = {
      .shaderConfig = config.shaderConfig,
      .primitive = primitive,
      .depthFunc = g_gxState.depthFunc,
      .cullMode = g_gxState.cullMode,
      .blendMode = g_gxState.blendMode,
      .blendFacSrc = g_gxState.blendFacSrc,
      .blendFacDst = g_gxState.blendFacDst,
      .blendOp = g_gxState.blendOp,
      .dstAlpha = g_gxState.dstAlpha,
      .depthCompare = g_gxState.depthCompare,
      .depthUpdate = g_gxState.depthUpdate,
      .alphaUpdate = g_gxState.alphaUpdate,
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
    buf.append(&g_gxState.mv, 64);
    buf.append(&g_gxState.mvInv, 64);
    buf.append(&g_gxState.proj, 64);
  }
  for (int i = 0; i < info.usesTevReg.size(); ++i) {
    if (!info.usesTevReg.test(i)) {
      continue;
    }
    buf.append(&g_gxState.colorRegs[i], 16);
  }
  for (int i = 0; i < info.sampledColorChannels.size(); ++i) {
    if (!info.sampledColorChannels.test(i)) {
      continue;
    }
    buf.append(&g_gxState.colorChannelState[i].ambColor, 16);
    buf.append(&g_gxState.colorChannelState[i].matColor, 16);

    if (g_gxState.colorChannelConfig[i].lightingEnabled) {
      zeus::CColor ambient = zeus::skClear;
      int addedLights = 0;
      for (int li = 0; li < g_gxState.lightState.size(); ++li) {
        if (!g_gxState.lightState.test(li)) {
          continue;
        }
        const auto& variant = g_gxState.lights[li];
        if (std::holds_alternative<zeus::CColor>(variant)) {
          ambient += std::get<zeus::CColor>(variant);
        } else if (std::holds_alternative<Light>(variant)) {
          static_assert(sizeof(Light) == 80);
          buf.append(&std::get<Light>(variant), sizeof(Light));
          ++addedLights;
        }
      }
      constexpr Light emptyLight{};
      for (int li = addedLights; li < GX::MaxLights; ++li) {
        buf.append(&emptyLight, sizeof(Light));
      }
      buf.append(&ambient, 16);
    }
  }
  for (int i = 0; i < info.sampledKColors.size(); ++i) {
    if (!info.sampledKColors.test(i)) {
      continue;
    }
    buf.append(&g_gxState.kcolors[i], 16);
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
  std::array<wgpu::BindGroupEntry, MaxTextures> samplerEntries;
  std::array<wgpu::BindGroupEntry, MaxTextures> textureEntries;
  for (u32 texIdx = 0, i = 0; texIdx < info.sampledTextures.size(); ++texIdx) {
    if (!info.sampledTextures.test(texIdx)) {
      continue;
    }
    const auto& tex = g_gxState.textures[texIdx];
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
    std::array<wgpu::BindGroupLayoutEntry, MaxTextures> samplerEntries;
    std::array<wgpu::BindGroupLayoutEntry, MaxTextures> textureEntries;
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
  g_gxState.textures.fill({});
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
