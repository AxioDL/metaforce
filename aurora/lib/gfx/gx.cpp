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
  if (id < GX::TEVPREV || id > GX::TEVREG2) {
    Log.report(logvisor::Fatal, FMT_STRING("bad tevreg {}"), id);
    unreachable();
  }
  g_gxState.colorRegs[id] = color;
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
  u32 idx = id - GX::COLOR0A0;
  auto& chan = g_gxState.colorChannelConfig[idx];
  chan.lightingEnabled = lightingEnabled;
  chan.ambSrc = ambSrc;
  chan.matSrc = matSrc;
  chan.diffFn = diffFn;
  chan.attnFn = attnFn;
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

u32 GXGetTexBufferSize(u16 width, u16 height, u32 fmt, GXBool mips, u8 maxLod) noexcept {
  s32 shiftX = 0;
  s32 shiftY = 0;
  switch (fmt) {
  case GX::TF_I4:
  case GX::TF_C4:
  case GX::TF_CMPR:
  case GX::CTF_R4:
  case GX::CTF_Z4:
    shiftX = 3;
    shiftY = 3;
    break;
  case GX::TF_I8:
  case GX::TF_IA4:
  case GX::TF_C8:
  case GX::TF_Z8:
  case GX::CTF_RA4:
  case GX::CTF_A8:
  case GX::CTF_R8:
  case GX::CTF_G8:
  case GX::CTF_B8:
  case GX::CTF_Z8M:
  case GX::CTF_Z8L:
    shiftX = 3;
    shiftY = 2;
    break;
  case GX::TF_IA8:
  case GX::TF_RGB565:
  case GX::TF_RGB5A3:
  case GX::TF_RGBA8:
  case GX::TF_C14X2:
  case GX::TF_Z16:
  case GX::TF_Z24X8:
  case GX::CTF_RA8:
  case GX::CTF_RG8:
  case GX::CTF_GB8:
  case GX::CTF_Z16L:
    shiftX = 2;
    shiftY = 2;
    break;
  default:
    break;
  }
  u32 bitSize = fmt == GX::TF_RGBA8 || fmt == GX::TF_Z24X8 ? 64 : 32;
  u32 bufLen = 0;
  if (mips) {
    while (maxLod != 0) {
      const u32 tileX = ((width + (1 << shiftX) - 1) >> shiftX);
      const u32 tileY = ((height + (1 << shiftY) - 1) >> shiftY);
      bufLen += bitSize * tileX * tileY;

      if (width == 1 && height == 1) {
        return bufLen;
      }

      width = (width < 2) ? 1 : width / 2;
      height = (height < 2) ? 1 : height / 2;
      --maxLod;
    };
  } else {
    const u32 tileX = ((width + (1 << shiftX) - 1) >> shiftX);
    const u32 tileY = ((height + (1 << shiftY) - 1) >> shiftY);
    bufLen = bitSize * tileX * tileY;
  }

  return bufLen;
}

// Lighting
void GXInitLightAttn(GX::LightObj* light, float a0, float a1, float a2, float k0, float k1, float k2) noexcept {
  light->a0 = a0;
  light->a1 = a1;
  light->a2 = a2;
  light->k0 = k0;
  light->k1 = k1;
  light->k2 = k2;
}

void GXInitLightAttnA(GX::LightObj* light, float a0, float a1, float a2) noexcept {
  light->a0 = a0;
  light->a1 = a1;
  light->a2 = a2;
}

void GXInitLightAttnK(GX::LightObj* light, float k0, float k1, float k2) noexcept {
  light->k0 = k0;
  light->k1 = k1;
  light->k2 = k2;
}

void GXInitLightSpot(GX::LightObj* light, float cutoff, GX::SpotFn spotFn) noexcept {
  if (cutoff <= 0.f || cutoff > 90.f) {
    spotFn = GX::SP_OFF;
  }

  float cr = std::cos((cutoff * M_PIF) / 180.f);
  float a0 = 1.f;
  float a1 = 0.f;
  float a2 = 0.f;
  switch (spotFn) {
  default:
    break;
  case GX::SP_FLAT:
    a0 = -1000.f * cr;
    a1 = 1000.f;
    a2 = 0.f;
    break;
  case GX::SP_COS:
    a0 = -cr / (1.f - cr);
    a1 = 1.f / (1.f - cr);
    a2 = 0.f;
    break;
  case GX::SP_COS2:
    a0 = 0.f;
    a1 = -cr / (1.f - cr);
    a2 = 1.f / (1.f - cr);
    break;
  case GX::SP_SHARP: {
    const float d = (1.f - cr) * (1.f - cr);
    a0 = cr * (cr - 2.f);
    a1 = 2.f / d;
    a2 = -1.f / d;
    break;
  }
  case GX::SP_RING1: {
    const float d = (1.f - cr) * (1.f - cr);
    a0 = 4.f * cr / d;
    a1 = 4.f * (1.f + cr) / d;
    a2 = -4.f / d;
    break;
  }
  case GX::SP_RING2: {
    const float d = (1.f - cr) * (1.f - cr);
    a0 = 1.f - 2.f * cr * cr / d;
    a1 = 4.f * cr / d;
    a2 = -2.f / d;
    break;
  }
  }

  light->a0 = a0;
  light->a1 = a1;
  light->a2 = a2;
}

void GXInitLightDistAttn(GX::LightObj* light, float refDistance, float refBrightness,
                         GX::DistAttnFn distFunc) noexcept {
  if (refDistance < 0.f || refBrightness < 0.f || refBrightness >= 1.f) {
    distFunc = GX::DA_OFF;
  }
  float k0 = 1.f;
  float k1 = 0.f;
  float k2 = 0.f;
  switch (distFunc) {
  case GX::DA_GENTLE:
    k0 = 1.0f;
    k1 = (1.0f - refBrightness) / (refBrightness * refDistance);
    k2 = 0.0f;
    break;
  case GX::DA_MEDIUM:
    k0 = 1.0f;
    k1 = 0.5f * (1.0f - refBrightness) / (refBrightness * refDistance);
    k2 = 0.5f * (1.0f - refBrightness) / (refBrightness * refDistance * refDistance);
    break;
  case GX::DA_STEEP:
    k0 = 1.0f;
    k1 = 0.0f;
    k2 = (1.0f - refBrightness) / (refBrightness * refDistance * refDistance);
    break;
  case GX::DA_OFF:
    k0 = 1.0f;
    k1 = 0.0f;
    k2 = 0.0f;
    break;
  }

  light->k0 = k0;
  light->k1 = k1;
  light->k2 = k2;
}

void GXInitLightPos(GX::LightObj* light, float x, float y, float z) noexcept {
  light->px = x;
  light->py = y;
  light->pz = z;
}

void GXInitLightDir(GX::LightObj* light, float nx, float ny, float nz) noexcept {
  light->nx = -nx;
  light->ny = -ny;
  light->nz = -nz;
}

void GXInitSpecularDir(GX::LightObj* light, float nx, float ny, float nz) noexcept {
  float hx = -nx;
  float hy = -ny;
  float hz = (-nz + 1.0f);
  float mag = ((hx * hx) + (hy * hy) + (hz * hz));
  if (mag != 0.0f) {
    mag = 1.0f / sqrtf(mag);
  }
  light->px = (nx * GX::LARGE_NUMBER);
  light->py = (ny * GX::LARGE_NUMBER);
  light->pz = (nz * GX::LARGE_NUMBER);
  light->nx = hx * mag;
  light->ny = hy * mag;
  light->nz = hz * mag;
}

void GXInitSpecularDirHA(GX::LightObj* light, float nx, float ny, float nz, float hx, float hy, float hz) noexcept {
  light->px = (nx * GX::LARGE_NUMBER);
  light->py = (ny * GX::LARGE_NUMBER);
  light->pz = (nz * GX::LARGE_NUMBER);
  light->nx = hx;
  light->ny = hy;
  light->nz = hz;
}

void GXInitLightColor(GX::LightObj* light, GX::Color col) noexcept { light->color = col; }

void GXLoadLightObjImm(const GX::LightObj* light, GX::LightID id) noexcept {
  u32 idx = std::log2<u32>(id);
  aurora::gfx::Light realLight;
  realLight.pos.assign(light->px, light->py, light->pz);
  realLight.dir.assign(light->nx, light->ny, light->nz);
  realLight.cosAtt.assign(light->a0, light->a1, light->a2);
  realLight.distAtt.assign(light->k0, light->k1, light->k2);
  realLight.color.fromRGBA8(light->color.color[0], light->color.color[1], light->color.color[2], light->color.color[3]);
  g_gxState.lights[idx] = realLight;
}

/* TODO Figure out a way to implement this, requires GXSetArray */
void GXLoadLightObjIndx(u32 index, GX::LightID) noexcept {}

void GXGetLightAttnA(const GX::LightObj* light, float* a0, float* a1, float* a2) noexcept {
  *a0 = light->a0;
  *a1 = light->a1;
  *a2 = light->a2;
}

void GXGetLightAttnK(const GX::LightObj* light, float* k0, float* k1, float* k2) noexcept {
  *k0 = light->k0;
  *k1 = light->k1;
  *k2 = light->k2;
}

void GXGetLightPos(const GX::LightObj* light, float* x, float* y, float* z) noexcept {
  *x = light->px;
  *z = light->py;
  *z = light->pz;
}

void GXGetLightDir(const GX::LightObj* light, float* nx, float* ny, float* nz) noexcept {
  *nx = -light->nx;
  *ny = -light->ny;
  *nz = -light->nz;
}

void GXGetLightColor(const GX::LightObj* light, GX::Color* col) noexcept { *col = light->color; }

// Indirect Texturing
void GXSetTevIndirect(GX::TevStageID tevStage, GX::IndTexStageID indStage, GX::IndTexFormat fmt,
                      GX::IndTexBiasSel biasSel, GX::IndTexMtxID matrixSel, GX::IndTexWrap wrapS, GX::IndTexWrap wrapT,
                      GXBool addPrev, GXBool indLod, GX::IndTexAlphaSel alphaSel) noexcept {
  auto& stage = g_gxState.tevStages[tevStage];
  stage.indTexStage = indStage;
  stage.indTexFormat = fmt;
  stage.indTexBiasSel = biasSel;
  stage.indTexAlphaSel = alphaSel;
  stage.indTexMtxId = matrixSel;
  stage.indTexWrapS = wrapS;
  stage.indTexWrapT = wrapT;
  stage.indTexAddPrev = addPrev;
  stage.indTexUseOrigLOD = indLod;
}
void GXSetIndTexOrder(GX::IndTexStageID indStage, GX::TexCoordID texCoord, GX::TexMapID texMap) noexcept {
  auto& stage = g_gxState.indStages[indStage];
  stage.texCoordId = texCoord;
  stage.texMapId = texMap;
}
void GXSetIndTexCoordScale(GX::IndTexStageID indStage, GX::IndTexScale scaleS, GX::IndTexScale scaleT) noexcept {
  auto& stage = g_gxState.indStages[indStage];
  stage.scaleS = scaleS;
  stage.scaleT = scaleT;
}
void GXSetIndTexMtx(GX::IndTexMtxID id, const void* mtx, s8 scaleExp) noexcept {
  if (id < GX::ITM_0 || id > GX::ITM_2) {
    Log.report(logvisor::Fatal, FMT_STRING("invalid ind tex mtx ID {}"), id);
  }
  g_gxState.indTexMtxs[id - 1] = {*static_cast<const aurora::Mat3x2<float>*>(mtx), scaleExp};
}

void GXInitTexObj(GXTexObj* obj, const void* data, u16 width, u16 height, GX::TextureFormat format, GXTexWrapMode wrapS,
                  GXTexWrapMode wrapT, GXBool mipmap) noexcept {
  obj->data = data;
  obj->width = width;
  obj->height = height;
  obj->fmt = format;
  obj->wrapS = wrapS;
  obj->wrapT = wrapT;
  obj->hasMips = mipmap;
  // TODO default values?
  obj->minFilter = GX_LINEAR;
  obj->magFilter = GX_LINEAR;
  obj->minLod = 0.f;
  obj->maxLod = 0.f;
  obj->lodBias = 0.f;
  obj->biasClamp = false;
  obj->doEdgeLod = false;
  obj->maxAniso = GX_ANISO_4;
  obj->tlut = GX_TLUT0;
  obj->dataInvalidated = true;
}
void GXInitTexObjResolved(GXTexObj* obj, u32 bindIdx, GX::TextureFormat format, GXTexWrapMode wrapS,
                          GXTexWrapMode wrapT, GXTlut tlut) {
  const auto& ref = aurora::gfx::g_resolvedTextures[bindIdx];
  obj->ref = ref;
  obj->data = nullptr;
  obj->dataSize = 0;
  obj->width = ref->size.width;
  obj->height = ref->size.height;
  obj->fmt = format;
  obj->wrapS = wrapS;
  obj->wrapT = wrapT;
  obj->hasMips = false;
  obj->tlut = tlut;
  // TODO default values?
  obj->minFilter = GX_LINEAR;
  obj->magFilter = GX_LINEAR;
  obj->minLod = 0.f;
  obj->maxLod = 0.f;
  obj->lodBias = 0.f;
  obj->biasClamp = false;
  obj->doEdgeLod = false;
  obj->maxAniso = GX_ANISO_4;
  obj->dataInvalidated = false;
}
void GXInitTexObjLOD(GXTexObj* obj, GXTexFilter minFilt, GXTexFilter magFilt, float minLod, float maxLod, float lodBias,
                     GXBool biasClamp, GXBool doEdgeLod, GXAnisotropy maxAniso) noexcept {
  obj->minFilter = minFilt;
  obj->magFilter = magFilt;
  obj->minLod = minLod;
  obj->maxLod = maxLod;
  obj->lodBias = lodBias;
  obj->biasClamp = biasClamp;
  obj->doEdgeLod = doEdgeLod;
  obj->maxAniso = maxAniso;
}
void GXInitTexObjCI(GXTexObj* obj, void* data, u16 width, u16 height, GXCITexFmt format, GXTexWrapMode wrapS,
                    GXTexWrapMode wrapT, GXBool mipmap, u32 tlut) noexcept {
  obj->data = data;
  obj->width = width;
  obj->height = height;
  obj->fmt = static_cast<GX::TextureFormat>(format);
  obj->wrapS = wrapS;
  obj->wrapT = wrapT;
  obj->hasMips = mipmap;
  obj->tlut = static_cast<GXTlut>(tlut);
  // TODO default values?
  obj->minFilter = GX_LINEAR;
  obj->magFilter = GX_LINEAR;
  obj->minLod = 0.f;
  obj->maxLod = 0.f;
  obj->lodBias = 0.f;
  obj->biasClamp = false;
  obj->doEdgeLod = false;
  obj->maxAniso = GX_ANISO_4;
  obj->dataInvalidated = true;
}
void GXInitTexObjData(GXTexObj* obj, void* data) noexcept {
  obj->data = data;
  obj->dataInvalidated = true;
}
void GXInitTexObjWrapMode(GXTexObj* obj, GXTexWrapMode wrapS, GXTexWrapMode wrapT) noexcept {
  obj->wrapS = wrapS;
  obj->wrapT = wrapT;
}
void GXInitTexObjTlut(GXTexObj* obj, u32 tlut) noexcept { obj->tlut = static_cast<GXTlut>(tlut); }
void GXLoadTexObj(GXTexObj* obj, GX::TexMapID id) noexcept {
  if (!obj->ref) {
    obj->ref =
        aurora::gfx::new_dynamic_texture_2d(obj->width, obj->height, u32(obj->minLod) + 1, obj->fmt, "GXLoadTexObj");
  }
  if (obj->dataInvalidated) {
    aurora::gfx::write_texture(*obj->ref, {static_cast<const u8*>(obj->data), UINT32_MAX /* TODO */});
    obj->dataInvalidated = false;
  }
  g_gxState.textures[id] = {*obj};
}

void GXInitTlutObj(GXTlutObj* obj, const void* data, GXTlutFmt format, u16 entries) noexcept {
  GX::TextureFormat texFmt;
  switch (format) {
  case GX_TL_IA8:
    texFmt = GX::TF_IA8;
    break;
  case GX_TL_RGB565:
    texFmt = GX::TF_RGB565;
    break;
  case GX_TL_RGB5A3:
    texFmt = GX::TF_RGB5A3;
    break;
  default:
    Log.report(logvisor::Fatal, FMT_STRING("invalid tlut format {}"), format);
    unreachable();
  }
  obj->ref = aurora::gfx::new_static_texture_2d(
      entries, 1, 1, texFmt, aurora::ArrayRef{static_cast<const u8*>(data), static_cast<size_t>(entries) * 2},
      "GXInitTlutObj");
}
void GXLoadTlut(const GXTlutObj* obj, GXTlut idx) noexcept { g_gxState.tluts[idx] = *obj; }
void GXSetColorUpdate(GXBool enabled) noexcept { g_gxState.colorUpdate = enabled; }
void GXSetTevColorS10(GX::TevRegID id, const GXColorS10& color) noexcept {
  g_gxState.colorRegs[id] = zeus::CColor{
      static_cast<float>(color.r) / 1023.f,
      static_cast<float>(color.g) / 1023.f,
      static_cast<float>(color.b) / 1023.f,
      static_cast<float>(color.a) / 1023.f,
  };
}
void GXInvalidateTexAll() noexcept {
  // no-op?
}

namespace aurora::gfx {
static logvisor::Module Log("aurora::gfx::gx");

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

static inline wgpu::ColorWriteMask to_write_mask(bool colorUpdate, bool alphaUpdate) {
  auto writeMask = wgpu::ColorWriteMask::None;
  if (colorUpdate) {
    writeMask |= wgpu::ColorWriteMask::Red | wgpu::ColorWriteMask::Green | wgpu::ColorWriteMask::Blue;
  }
  if (alphaUpdate) {
    writeMask |= wgpu::ColorWriteMask::Alpha;
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
  wgpu::CullMode cullMode = wgpu::CullMode::None;
  switch (gx_cullMode) {
  case GX::CULL_FRONT:
    cullMode = wgpu::CullMode::Front;
    break;
  case GX::CULL_BACK:
    cullMode = wgpu::CullMode::Back;
    break;
  case GX::CULL_NONE:
    break;
  default:
    Log.report(logvisor::Fatal, FMT_STRING("Unsupported cull mode {}"), gx_cullMode);
    unreachable();
  }
  return {
      .topology = primitive,
      .frontFace = wgpu::FrontFace::CW,
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
      .writeMask = to_write_mask(config.colorUpdate, config.alphaUpdate),
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
    const auto& cc = g_gxState.colorChannelConfig[i];
    if (g_gxState.colorChannelState[i].lightState.any() && cc.lightingEnabled) {
      config.shaderConfig.colorChannels[i] = cc;
    } else {
      // Only matSrc matters when lighting disabled
      config.shaderConfig.colorChannels[i] = {
          .matSrc = cc.matSrc,
      };
    }
  }
  for (u8 i = 0; i < g_gxState.numTexGens; ++i) {
    config.shaderConfig.tcgs[i] = g_gxState.tcgs[i];
  }
  if (g_gxState.alphaCompare) {
    config.shaderConfig.alphaCompare = g_gxState.alphaCompare;
  }
  config.shaderConfig.indexedAttributeCount =
      std::count_if(config.shaderConfig.vtxAttrs.begin(), config.shaderConfig.vtxAttrs.end(),
                    [](const auto type) { return type == GX::INDEX8 || type == GX::INDEX16; });
  for (u8 i = 0; i < MaxTextures; ++i) {
    const auto& bind = g_gxState.textures[i];
    TextureConfig texConfig{};
    if (bind.texObj.ref) {
      if (requires_copy_conversion(bind.texObj)) {
        texConfig.copyFmt = bind.texObj.ref->gxFormat;
      }
      if (requires_load_conversion(bind.texObj)) {
        texConfig.loadFmt = bind.texObj.fmt;
      }
      texConfig.renderTex = bind.texObj.ref->isRenderTexture;
    }
    config.shaderConfig.textureConfig[i] = texConfig;
  }
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
      .colorUpdate = g_gxState.colorUpdate,
  };
}

Range build_uniform(const ShaderInfo& info) noexcept {
  auto [buf, range] = map_uniform(info.uniformSize);
  {
    buf.append(&g_gxState.mv, 64);
    buf.append(&g_gxState.mvInv, 64);
    buf.append(&g_gxState.proj, 64);
  }
  for (int i = 0; i < info.loadsTevReg.size(); ++i) {
    if (!info.loadsTevReg.test(i)) {
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
      int addedLights = 0;
      const auto& lightState = g_gxState.colorChannelState[i].lightState;
      u32 state = lightState.to_ulong();
      buf.append(&lightState, sizeof(u32));
      buf.append_zeroes(12); // alignment
      for (int li = 0; li < lightState.size(); ++li) {
        if (!lightState.test(li)) {
          continue;
        }
        const auto& light = g_gxState.lights[li];
        static_assert(sizeof(Light) == 80);
        buf.append(&light, sizeof(Light));
        ++addedLights;
      }
      constexpr Light emptyLight{};
      for (int li = addedLights; li < GX::MaxLights; ++li) {
        buf.append(&emptyLight, sizeof(Light));
      }
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
        Log.report(logvisor::Fatal, FMT_STRING("expected 3x4 mtx in idx {}"), i);
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
    buf.append(&tex.texObj.lodBias, 4);
  }
  return range;
}

static absl::flat_hash_map<u32, wgpu::BindGroupLayout> sUniformBindGroupLayouts;
static absl::flat_hash_map<u32, std::pair<wgpu::BindGroupLayout, wgpu::BindGroupLayout>> sTextureBindGroupLayouts;

GXBindGroups build_bind_groups(const ShaderInfo& info, const ShaderConfig& config,
                               const BindGroupRanges& ranges) noexcept {
  const auto layouts = build_bind_group_layouts(info, config);

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
  std::array<wgpu::BindGroupEntry, MaxTextures * 2> textureEntries;
  u32 samplerCount = 0;
  u32 textureCount = 0;
  for (u32 i = 0; i < info.sampledTextures.size(); ++i) {
    if (!info.sampledTextures.test(i)) {
      continue;
    }
    const auto& tex = g_gxState.textures[i];
    if (!tex) {
      Log.report(logvisor::Fatal, FMT_STRING("unbound texture {}"), i);
      unreachable();
    }
    samplerEntries[samplerCount] = {
        .binding = samplerCount,
        .sampler = sampler_ref(tex.get_descriptor()),
    };
    ++samplerCount;
    textureEntries[textureCount] = {
        .binding = textureCount,
        .textureView = tex.texObj.ref->view,
    };
    ++textureCount;
    // Load palette
    const auto& texConfig = config.textureConfig[i];
    if (is_palette_format(texConfig.loadFmt)) {
      u32 tlut = tex.texObj.tlut;
      if (tlut < GX_TLUT0 || tlut > GX_TLUT7) {
        Log.report(logvisor::Fatal, FMT_STRING("tlut out of bounds {}"), tlut);
        unreachable();
      } else if (!g_gxState.tluts[tlut].ref) {
        Log.report(logvisor::Fatal, FMT_STRING("tlut unbound {}"), tlut);
        unreachable();
      }
      textureEntries[textureCount] = {
          .binding = textureCount,
          .textureView = g_gxState.tluts[tlut].ref->view,
      };
      ++textureCount;
    }
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
          .entryCount = samplerCount,
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

  //  u32 textureCount = info.sampledTextures.count();
  //  const auto textureIt = sTextureBindGroupLayouts.find(textureCount);
  //  if (textureIt != sTextureBindGroupLayouts.end()) {
  //    const auto& [sl, tl] = textureIt->second;
  //    out.samplerLayout = sl;
  //    out.textureLayout = tl;
  //  } else {
  u32 numSamplers = 0;
  u32 numTextures = 0;
  std::array<wgpu::BindGroupLayoutEntry, MaxTextures> samplerEntries;
  std::array<wgpu::BindGroupLayoutEntry, MaxTextures * 2> textureEntries;
  for (u32 i = 0; i < info.sampledTextures.size(); ++i) {
    if (!info.sampledTextures.test(i)) {
      continue;
    }
    const auto& texConfig = config.textureConfig[i];
    bool copyAsPalette = is_palette_format(texConfig.copyFmt);
    bool loadAsPalette = is_palette_format(texConfig.loadFmt);
    samplerEntries[numSamplers] = {
        .binding = numSamplers,
        .visibility = wgpu::ShaderStage::Fragment,
        .sampler = {.type = copyAsPalette && loadAsPalette ? wgpu::SamplerBindingType::NonFiltering
                                                           : wgpu::SamplerBindingType::Filtering},
    };
    ++numSamplers;
    if (loadAsPalette) {
      textureEntries[numTextures] = {
          .binding = numTextures,
          .visibility = wgpu::ShaderStage::Fragment,
          .texture =
              {
                  .sampleType = copyAsPalette ? wgpu::TextureSampleType::Sint : wgpu::TextureSampleType::Float,
                  .viewDimension = wgpu::TextureViewDimension::e2D,
              },
      };
      ++numTextures;
      textureEntries[numTextures] = {
          .binding = numTextures,
          .visibility = wgpu::ShaderStage::Fragment,
          .texture =
              {
                  .sampleType = wgpu::TextureSampleType::Float,
                  .viewDimension = wgpu::TextureViewDimension::e2D,
              },
      };
      ++numTextures;
    } else {
      textureEntries[numTextures] = {
          .binding = numTextures,
          .visibility = wgpu::ShaderStage::Fragment,
          .texture =
              {
                  .sampleType = wgpu::TextureSampleType::Float,
                  .viewDimension = wgpu::TextureViewDimension::e2D,
              },
      };
      ++numTextures;
    }
  }
  {
    const wgpu::BindGroupLayoutDescriptor descriptor{
        .label = "GX Sampler Bind Group Layout",
        .entryCount = numSamplers,
        .entries = samplerEntries.data(),
    };
    out.samplerLayout = g_device.CreateBindGroupLayout(&descriptor);
  }
  {
    const wgpu::BindGroupLayoutDescriptor descriptor{
        .label = "GX Texture Bind Group Layout",
        .entryCount = numTextures,
        .entries = textureEntries.data(),
    };
    out.textureLayout = g_device.CreateBindGroupLayout(&descriptor);
  }
  //    sTextureBindGroupLayouts.try_emplace(textureCount, out.samplerLayout, out.textureLayout);
  //  }
  return out;
}

// TODO this is awkward
extern absl::flat_hash_map<ShaderRef, std::pair<wgpu::ShaderModule, gx::ShaderInfo>> g_gxCachedShaders;
void shutdown() noexcept {
  // TODO we should probably store this all in g_state.gx instead
  sUniformBindGroupLayouts.clear();
  sTextureBindGroupLayouts.clear();
  for (auto& item : g_gxState.textures) {
    item.texObj.ref.reset();
  }
  for (auto& item : g_gxState.tluts) {
    item.ref.reset();
  }
  g_gxCachedShaders.clear();
}

static wgpu::AddressMode wgpu_address_mode(GXTexWrapMode mode) {
  switch (mode) {
  case GX_CLAMP:
    return wgpu::AddressMode::ClampToEdge;
  case GX_REPEAT:
    return wgpu::AddressMode::Repeat;
  case GX_MIRROR:
    return wgpu::AddressMode::MirrorRepeat;
  default:
    Log.report(logvisor::Fatal, FMT_STRING("invalid wrap mode {}"), mode);
    unreachable();
  }
}
static std::pair<wgpu::FilterMode, wgpu::FilterMode> wgpu_filter_mode(GXTexFilter filter) {
  switch (filter) {
  case GX_NEAR:
    return {wgpu::FilterMode::Nearest, wgpu::FilterMode::Linear};
  case GX_LINEAR:
    return {wgpu::FilterMode::Linear, wgpu::FilterMode::Linear};
  case GX_NEAR_MIP_NEAR:
    return {wgpu::FilterMode::Nearest, wgpu::FilterMode::Nearest};
  case GX_LIN_MIP_NEAR:
    return {wgpu::FilterMode::Linear, wgpu::FilterMode::Nearest};
  case GX_NEAR_MIP_LIN:
    return {wgpu::FilterMode::Nearest, wgpu::FilterMode::Linear};
  case GX_LIN_MIP_LIN:
    return {wgpu::FilterMode::Linear, wgpu::FilterMode::Linear};
  default:
    Log.report(logvisor::Fatal, FMT_STRING("invalid filter mode {}"), filter);
    unreachable();
  }
}
static u16 wgpu_aniso(GXAnisotropy aniso) {
  switch (aniso) {
  case GX_ANISO_1:
    return 1;
  case GX_ANISO_2:
    return std::max<u16>(g_graphicsConfig.textureAnisotropy / 2, 1);
  case GX_ANISO_4:
    return std::max<u16>(g_graphicsConfig.textureAnisotropy, 1);
  default:
    Log.report(logvisor::Fatal, FMT_STRING("invalid aniso mode {}"), aniso);
    unreachable();
  }
}
wgpu::SamplerDescriptor TextureBind::get_descriptor() const noexcept {
  if (requires_copy_conversion(texObj) && is_palette_format(texObj.ref->gxFormat)) {
    return {
        .label = "Generated Non-Filtering Sampler",
        .addressModeU = wgpu_address_mode(texObj.wrapS),
        .addressModeV = wgpu_address_mode(texObj.wrapT),
        .addressModeW = wgpu::AddressMode::Repeat,
        .magFilter = wgpu::FilterMode::Nearest,
        .minFilter = wgpu::FilterMode::Nearest,
        .mipmapFilter = wgpu::FilterMode::Nearest,
        .maxAnisotropy = 1,
    };
  }
  const auto [minFilter, mipFilter] = wgpu_filter_mode(texObj.minFilter);
  const auto [magFilter, _] = wgpu_filter_mode(texObj.magFilter);
  return {
      .label = "Generated Filtering Sampler",
      .addressModeU = wgpu_address_mode(texObj.wrapS),
      .addressModeV = wgpu_address_mode(texObj.wrapT),
      .addressModeW = wgpu::AddressMode::Repeat,
      .magFilter = magFilter,
      .minFilter = minFilter,
      .mipmapFilter = mipFilter,
      .maxAnisotropy = wgpu_aniso(texObj.maxAniso),
  };
}
} // namespace gx
} // namespace aurora::gfx
