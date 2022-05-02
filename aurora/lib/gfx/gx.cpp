#include "gx.hpp"

#include "../gpu.hpp"
#include "Runtime/Graphics/GX.hpp"
#include "common.hpp"

#include <absl/container/flat_hash_map.h>

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
void GXSetDstAlpha(bool enabled, u8 value) noexcept {
  if (enabled) {
    g_gxState.dstAlpha = value;
  } else {
    g_gxState.dstAlpha = UINT32_MAX;
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
void GXSetAlphaCompare(GX::Compare comp0, u8 ref0, GX::AlphaOp op, GX::Compare comp1, u8 ref1) noexcept {
  g_gxState.alphaCompare = {comp0, ref0, op, comp1, ref1};
}
void GXSetTexCoordGen2(GX::TexCoordID dst, GX::TexGenType type, GX::TexGenSrc src, GX::TexMtx mtx, GXBool normalize,
                       GX::PTTexMtx postMtx) noexcept {
  if (dst < GX::TEXCOORD0 || dst > GX::TEXCOORD7) {
    Log.report(logvisor::Fatal, FMT_STRING("invalid tex coord {}"), dst);
    unreachable();
  }
  g_gxState.tcgs[dst] = {type, src, mtx, postMtx, normalize};
}
void GXLoadTexMtxImm(const void* data, u32 id, GX::TexMtxType type) noexcept {
  if ((id < GX::TEXMTX0 || id > GX::IDENTITY) && (id < GX::PTTEXMTX0 || id > GX::PTIDENTITY)) {
    Log.report(logvisor::Fatal, FMT_STRING("invalid tex mtx {}"), id);
    unreachable();
  }
  if (id >= GX::PTTEXMTX0) {
    if (type != GX::MTX3x4) {
      Log.report(logvisor::Fatal, FMT_STRING("invalid pt mtx type {}"), type);
      unreachable();
    }
    const auto idx = (id - GX::PTTEXMTX0) / 3;
    g_gxState.ptTexMtxs[idx] = *static_cast<const zeus::CTransform*>(data);
  } else {
    const auto idx = (id - GX::TEXMTX0) / 3;
    switch (type) {
    case GX::MTX3x4:
      g_gxState.texMtxs[idx] = aurora::Mat4x4<float>{*static_cast<const zeus::CTransform*>(data)};
      break;
    case GX::MTX2x4:
      g_gxState.texMtxs[idx] = *static_cast<const aurora::Mat4x2<float>*>(data);
      break;
    }
  }
}
void GXLoadPosMtxImm(const zeus::CTransform& xf, GX::PosNrmMtx id) noexcept {
  if (id != GX::PNMTX0) {
    Log.report(logvisor::Fatal, FMT_STRING("invalid pn mtx {}"), id);
    unreachable();
  }
  g_gxState.mv = xf.toMatrix4f();
}
void GXLoadNrmMtxImm(const zeus::CTransform& xf, GX::PosNrmMtx id) noexcept {
  if (id != GX::PNMTX0) {
    Log.report(logvisor::Fatal, FMT_STRING("invalid pn mtx {}"), id);
    unreachable();
  }
  g_gxState.mvInv = xf.toMatrix4f();
}
constexpr zeus::CMatrix4f DepthCorrect{
    // clang-format off
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 0.5f, 0.5f,
    0.f, 0.f, 0.f, 1.f,
    // clang-format on
};
void GXSetProjection(const zeus::CMatrix4f& mtx, GX::ProjectionType type) noexcept {
  if (type == GX::PERSPECTIVE) {
    g_gxState.proj = DepthCorrect * mtx;
  } else {
    g_gxState.proj = mtx;
  }
}
void GXSetViewport(float left, float top, float width, float height, float nearZ, float farZ) noexcept {
  aurora::gfx::set_viewport(left, top, width, height, nearZ, farZ);
}
void GXSetScissor(u32 left, u32 top, u32 width, u32 height) noexcept {
  aurora::gfx::set_scissor(left, top, width, height);
}
void GXSetFog(GX::FogType type, float startZ, float endZ, float nearZ, float farZ, const GXColor& color) noexcept {
  g_gxState.fog = {type, startZ, endZ, nearZ, farZ, color};
}
void GXSetFogColor(const GXColor& color) noexcept { g_gxState.fog.color = color; }
void GXSetVtxDesc(GX::Attr attr, GX::AttrType type) noexcept { g_gxState.vtxDesc[attr] = type; }
void GXSetVtxDescv(GX::VtxDescList* list) noexcept {
  g_gxState.vtxDesc.fill({});
  while (*list) {
    g_gxState.vtxDesc[list->attr] = list->type;
    ++list;
  }
}
void GXClearVtxDesc() noexcept { g_gxState.vtxDesc.fill({}); }
void GXSetTevSwapModeTable(GX::TevSwapSel id, GX::TevColorChan red, GX::TevColorChan green, GX::TevColorChan blue,
                           GX::TevColorChan alpha) noexcept {
  if (id < GX::TEV_SWAP0 || id >= GX::MAX_TEVSWAP) {
    Log.report(logvisor::Fatal, FMT_STRING("invalid tev swap sel {}"), id);
    unreachable();
  }
  g_gxState.tevSwapTable[id] = {red, green, blue, alpha};
}
void GXSetTevSwapMode(GX::TevStageID stageId, GX::TevSwapSel rasSel, GX::TevSwapSel texSel) noexcept {
  auto& stage = g_gxState.tevStages[stageId];
  stage.tevSwapRas = rasSel;
  stage.tevSwapTex = texSel;
}
void GXSetLineWidth(u8 width, GX::TexOffset offs) noexcept {
  // TODO
}

namespace aurora::gfx {
static logvisor::Module Log("aurora::gfx::gx");

// TODO remove this hack for build_shader
extern std::mutex g_pipelineMutex;

// GX state
void bind_texture(GX::TexMapID id, metaforce::EClampMode clamp, const TextureHandle& tex, float lod) noexcept {
  gx::g_gxState.textures[static_cast<size_t>(id)] = {tex, clamp, lod};
}
void unbind_texture(GX::TexMapID id) noexcept { gx::g_gxState.textures[static_cast<size_t>(id)].reset(); }

void load_light(GX::LightID id, const Light& light) noexcept { gx::g_gxState.lights[std::log2<u32>(id)] = light; }
void load_light_ambient(GX::LightID id, const zeus::CColor& ambient) noexcept {
  gx::g_gxState.lights[std::log2<u32>(id)] = ambient;
}

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
                                              GX::LogicOp op, u32 dstAlpha) {
  wgpu::BlendComponent colorBlendComponent;
  switch (mode) {
  case GX::BM_NONE:
    colorBlendComponent = {
        .operation = wgpu::BlendOperation::Add,
        .srcFactor = wgpu::BlendFactor::Src,
        .dstFactor = wgpu::BlendFactor::Zero,
    };
    break;
  case GX::BM_BLEND:
    colorBlendComponent = {
        .operation = wgpu::BlendOperation::Add,
        .srcFactor = to_blend_factor(srcFac),
        .dstFactor = to_blend_factor(dstFac),
    };
    break;
  case GX::BM_SUBTRACT:
    colorBlendComponent = {
        .operation = wgpu::BlendOperation::ReverseSubtract,
        .srcFactor = wgpu::BlendFactor::Src,
        .dstFactor = wgpu::BlendFactor::Dst,
    };
    break;
  case GX::BM_LOGIC:
    switch (op) {
    case GX::LO_CLEAR:
      colorBlendComponent = {
          .operation = wgpu::BlendOperation::Add,
          .srcFactor = wgpu::BlendFactor::Zero,
          .dstFactor = wgpu::BlendFactor::Zero,
      };
      break;
    case GX::LO_COPY:
      colorBlendComponent = {
          .operation = wgpu::BlendOperation::Add,
          .srcFactor = wgpu::BlendFactor::Src,
          .dstFactor = wgpu::BlendFactor::Zero,
      };
      break;
    case GX::LO_NOOP:
      colorBlendComponent = {
          .operation = wgpu::BlendOperation::Add,
          .srcFactor = wgpu::BlendFactor::Zero,
          .dstFactor = wgpu::BlendFactor::Dst,
      };
      break;
    case GX::LO_INV:
      colorBlendComponent = {
          .operation = wgpu::BlendOperation::Add,
          .srcFactor = wgpu::BlendFactor::Zero,
          .dstFactor = wgpu::BlendFactor::OneMinusDst,
      };
      break;
    case GX::LO_INVCOPY:
      colorBlendComponent = {
          .operation = wgpu::BlendOperation::Add,
          .srcFactor = wgpu::BlendFactor::OneMinusSrc,
          .dstFactor = wgpu::BlendFactor::Zero,
      };
      break;
    case GX::LO_SET:
      colorBlendComponent = {
          .operation = wgpu::BlendOperation::Add,
          .srcFactor = wgpu::BlendFactor::One,
          .dstFactor = wgpu::BlendFactor::Zero,
      };
      break;
    default:
      Log.report(logvisor::Fatal, FMT_STRING("unsupported logic op {}"), op);
      unreachable();
    }
    break;
  default:
    Log.report(logvisor::Fatal, FMT_STRING("unsupported blend mode {}"), mode);
    unreachable();
  }
  wgpu::BlendComponent alphaBlendComponent{
      .operation = wgpu::BlendOperation::Add,
      .srcFactor = wgpu::BlendFactor::SrcAlpha,
      .dstFactor = wgpu::BlendFactor::Zero,
  };
  if (dstAlpha != UINT32_MAX) {
    alphaBlendComponent = wgpu::BlendComponent{
        .operation = wgpu::BlendOperation::Add,
        .srcFactor = wgpu::BlendFactor::Constant,
        .dstFactor = wgpu::BlendFactor::Zero,
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
  const auto blendState =
      to_blend_state(config.blendMode, config.blendFacSrc, config.blendFacDst, config.blendOp, config.dstAlpha);
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

void populate_pipeline_config(PipelineConfig& config, GX::Primitive primitive) noexcept {
  config.shaderConfig.fogType = g_gxState.fog.type;
  config.shaderConfig.vtxAttrs = g_gxState.vtxDesc;
  config.shaderConfig.tevSwapTable = g_gxState.tevSwapTable;
  for (u8 i = 0; i < g_gxState.numTevStages; ++i) {
    config.shaderConfig.tevStages[i] = g_gxState.tevStages[i];
  }
  config.shaderConfig.tevStageCount = g_gxState.numTevStages;
  for (u8 i = 0; i < g_gxState.numChans; ++i) {
    config.shaderConfig.colorChannels[i] = g_gxState.colorChannelConfig[i];
  }
  for (u8 i = 0; i < g_gxState.numTexGens; ++i) {
    config.shaderConfig.tcgs[i] = g_gxState.tcgs[i];
  }
  config.shaderConfig.alphaCompare = g_gxState.alphaCompare;
  config.shaderConfig.indexedAttributeCount =
      std::count_if(config.shaderConfig.vtxAttrs.begin(), config.shaderConfig.vtxAttrs.end(),
                    [](const auto type) { return type == GX::INDEX8 || type == GX::INDEX16; });
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
      const auto& lightState = g_gxState.colorChannelState[i].lightState;
      for (int li = 0; li < lightState.size(); ++li) {
        if (!lightState.test(li)) {
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
  for (int i = 0; i < info.usesTexMtx.size(); ++i) {
    if (!info.usesTexMtx.test(i)) {
      continue;
    }
    switch (info.texMtxTypes[i]) {
    case GX::TG_MTX2x4:
      if (std::holds_alternative<Mat4x2<float>>(g_gxState.texMtxs[i])) {
        buf.append(&std::get<Mat4x2<float>>(g_gxState.texMtxs[i]), 32);
      } else {
        Log.report(logvisor::Fatal, FMT_STRING("expected 2x4 mtx in idx {}"), i);
        unreachable();
      }
      break;
    case GX::TG_MTX3x4:
      if (std::holds_alternative<Mat4x4<float>>(g_gxState.texMtxs[i])) {
        const auto& mat = std::get<Mat4x4<float>>(g_gxState.texMtxs[i]);
        buf.append(&mat, 64);
      } else {
        // Log.report(logvisor::Fatal, FMT_STRING("expected 3x4 mtx in idx {}"), i);
        buf.append(&Mat4x4_Identity, 64);
      }
      break;
    default:
      Log.report(logvisor::Fatal, FMT_STRING("unhandled tex mtx type {}"), info.texMtxTypes[i]);
      unreachable();
    }
  }
  for (int i = 0; i < info.usesPTTexMtx.size(); ++i) {
    if (!info.usesPTTexMtx.test(i)) {
      continue;
    }
    buf.append(&g_gxState.ptTexMtxs[i], 64);
  }
  if (info.usesFog) {
    const auto& state = g_gxState.fog;
    struct Fog {
      zeus::CColor color = state.color;
      float a = 0.f;
      float b = 0.5f;
      float c = 0.f;
      float pad = FLT_MAX;
    } fog{};
    static_assert(sizeof(Fog) == 32);
    if (state.nearZ != state.farZ && state.startZ != state.endZ) {
      const float depthRange = state.farZ - state.nearZ;
      const float fogRange = state.endZ - state.startZ;
      fog.a = (state.farZ * state.nearZ) / (depthRange * fogRange);
      fog.b = state.farZ / depthRange;
      fog.c = state.startZ / fogRange;
    }
    buf.append(&fog, 32);
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

static absl::flat_hash_map<u32, wgpu::BindGroupLayout> sUniformBindGroupLayouts;
static absl::flat_hash_map<u32, std::pair<wgpu::BindGroupLayout, wgpu::BindGroupLayout>> sTextureBindGroupLayouts;

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
          .size = ranges.vtxDataRange.size,
      },
      // Normals
      wgpu::BindGroupEntry{
          .binding = 2,
          .buffer = g_storageBuffer,
          .size = ranges.nrmDataRange.size,
      },
      // Packed UVs
      wgpu::BindGroupEntry{
          .binding = 3,
          .buffer = g_storageBuffer,
          .size = ranges.packedTcDataRange.size,
      },
      // UVs
      wgpu::BindGroupEntry{
          .binding = 4,
          .buffer = g_storageBuffer,
          .size = ranges.tcDataRange.size,
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
          .entryCount = static_cast<uint32_t>(config.indexedAttributeCount > 0 ? uniformEntries.size() : 1),
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
  u32 uniformSizeKey = info.uniformSize + (config.indexedAttributeCount > 0 ? 1 : 0);
  const auto uniformIt = sUniformBindGroupLayouts.find(uniformSizeKey);
  if (uniformIt != sUniformBindGroupLayouts.end()) {
    out.uniformLayout = uniformIt->second;
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
        .entryCount = static_cast<uint32_t>(config.indexedAttributeCount > 0 ? uniformLayoutEntries.size() : 1),
        .entries = uniformLayoutEntries.data(),
    };
    out.uniformLayout = g_device.CreateBindGroupLayout(&uniformLayoutDescriptor);
    sUniformBindGroupLayouts.try_emplace(uniformSizeKey, out.uniformLayout);
  }

  u32 textureCount = info.sampledTextures.count();
  const auto textureIt = sTextureBindGroupLayouts.find(textureCount);
  if (textureIt != sTextureBindGroupLayouts.end()) {
    const auto& [sl, tl] = textureIt->second;
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
extern absl::flat_hash_map<ShaderRef, std::pair<wgpu::ShaderModule, gx::ShaderInfo>> g_gxCachedShaders;
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
