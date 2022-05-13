#include "Graphics/CCubeMaterial.hpp"

#include "GameGlobalObjects.hpp"
#include "Graphics/CCubeModel.hpp"
#include "Graphics/CCubeRenderer.hpp"
#include "Graphics/CCubeSurface.hpp"
#include "Graphics/CGX.hpp"
#include "Graphics/CModel.hpp"

namespace metaforce {
static u32 sReflectionType = 0;
static u32 sLastMaterialUnique = UINT32_MAX;
static const u8* sLastMaterialCached = nullptr;
static const CCubeModel* sLastModelCached = nullptr;
static const CCubeModel* sRenderingModel = nullptr;
static float sReflectionAlpha = 0.f;

void CCubeMaterial::SetCurrent(const CModelFlags& flags, const CCubeSurface& surface, CCubeModel& model) {
  if (sLastMaterialCached == x0_data) {
    if (sReflectionType == 1) {
      if (sLastModelCached == sRenderingModel) {
        return;
      }
    } else if (sReflectionType != 2) {
      return;
    }
  }

  if (CCubeModel::sRenderModelBlack) {
    SetCurrentBlack();
    return;
  }

  sRenderingModel = &model;
  sLastMaterialCached = x0_data;

  u32 numIndStages = 0;
  const auto matFlags = GetFlags();
  const u8* materialDataCur = x0_data;

  const bool reflection = bool(
      matFlags & (Flags(CCubeMaterialFlagBits::fSamusReflection) | CCubeMaterialFlagBits::fSamusReflectionSurfaceEye));
  if (reflection) {
    if (!(matFlags & CCubeMaterialFlagBits::fSamusReflectionSurfaceEye)) {
      EnsureViewDepStateCached(nullptr);
    } else {
      EnsureViewDepStateCached(&surface);
    }
  }

  u32 texCount = SBig(*reinterpret_cast<const u32*>(materialDataCur + 4));
  if (flags.x2_flags & CModelFlagBits::NoTextureLock) {
    materialDataCur += (2 + texCount) * 4;
  } else {
    materialDataCur += 8;
    for (u32 i = 0; i < texCount; ++i) {
      u32 texIdx = SBig(*reinterpret_cast<const u32*>(materialDataCur));
      model.GetTexture(texIdx)->Load(static_cast<GX::TexMapID>(i), EClampMode::Repeat);
      materialDataCur += 4;
    }
  }

  auto groupIdx = SBig(*reinterpret_cast<const u32*>(materialDataCur + 4));
  if (sLastMaterialUnique != UINT32_MAX && sLastMaterialUnique == groupIdx && sReflectionType == 0) {
    return;
  }
  sLastMaterialUnique = groupIdx;

  u32 vatFlags = SBig(*reinterpret_cast<const u32*>(materialDataCur));
  CGX::SetVtxDescv_Compressed(vatFlags);
  materialDataCur += 8;

  bool packedLightMaps = matFlags.IsSet(CCubeMaterialFlagBits::fLightmapUvArray);
  if (packedLightMaps != CCubeModel::sUsingPackedLightmaps) {
    model.SetUsingPackedLightmaps(packedLightMaps);
  }

  u32 finalKColorCount = 0;
  if (matFlags & CCubeMaterialFlagBits::fKonstValues) {
    u32 konstCount = SBig(*reinterpret_cast<const u32*>(materialDataCur));
    finalKColorCount = konstCount;
    materialDataCur += 4;
    for (u32 i = 0; i < konstCount; ++i) {
      u32 kColor = SBig(*reinterpret_cast<const u32*>(materialDataCur));
      materialDataCur += 4;
      CGX::SetTevKColor(static_cast<GX::TevKColorID>(i), kColor);
    }
  }

  u32 blendFactors = SBig(*reinterpret_cast<const u32*>(materialDataCur));
  materialDataCur += 4;
  if (g_Renderer->IsInAreaDraw()) {
    CGX::SetBlendMode(GX::BM_BLEND, GX::BL_ONE, GX::BL_ONE, GX::LO_CLEAR);
  } else {
    SetupBlendMode(blendFactors, flags, matFlags.IsSet(CCubeMaterialFlagBits::fAlphaTest));
  }

  bool indTex = matFlags.IsSet(CCubeMaterialFlagBits::fSamusReflectionIndirectTexture);
  u32 indTexSlot = 0;
  if (indTex) {
    indTexSlot = SBig(*reinterpret_cast<const u32*>(materialDataCur));
    materialDataCur += 4;
  }

  HandleDepth(flags.x2_flags, matFlags);

  u32 chanCount = SBig(*reinterpret_cast<const u32*>(materialDataCur));
  materialDataCur += 4;
  u32 firstChan = SBig(*reinterpret_cast<const u32*>(materialDataCur));
  materialDataCur += 4 * chanCount;
  u32 finalNumColorChans = HandleColorChannels(chanCount, firstChan);

  u32 firstTev = 0;
  if (CCubeModel::sRenderModelShadow)
    firstTev = 2;

  u32 matTevCount = SBig(*reinterpret_cast<const u32*>(materialDataCur));
  materialDataCur += 4;
  u32 finalTevCount = matTevCount;

  const u32* texMapTexCoordFlags = reinterpret_cast<const u32*>(materialDataCur + matTevCount * 20);
  const u32* tcgs = reinterpret_cast<const u32*>(texMapTexCoordFlags + matTevCount);
  bool usesTevReg2 = false;

  u32 finalCCFlags = 0;
  u32 finalACFlags = 0;

  if (g_Renderer->IsThermalVisorActive()) {
    finalTevCount = firstTev + 1;
    u32 ccFlags = SBig(*reinterpret_cast<const u32*>(materialDataCur + 8));
    finalCCFlags = ccFlags;
    auto outputReg = static_cast<GX::TevRegID>(ccFlags >> 9 & 0x3);
    if (outputReg == GX::TEVREG0) {
      materialDataCur += 20;
      texMapTexCoordFlags += 1;
      finalCCFlags = SBig(*reinterpret_cast<const u32*>(materialDataCur + 8));
      GXSetTevColor(GX::TEVREG0, 0xc0c0c0c0);
    }
    finalACFlags = SBig(*reinterpret_cast<const u32*>(materialDataCur + 12));
    HandleTev(firstTev, reinterpret_cast<const u32*>(materialDataCur), texMapTexCoordFlags,
              CCubeModel::sRenderModelShadow);
    usesTevReg2 = false;
  } else {
    finalTevCount = firstTev + matTevCount;
    for (u32 i = firstTev; i < finalTevCount; ++i) {
      HandleTev(i, reinterpret_cast<const u32*>(materialDataCur), texMapTexCoordFlags,
                CCubeModel::sRenderModelShadow && i == firstTev);
      u32 ccFlags = SBig(*reinterpret_cast<const u32*>(materialDataCur + 8));
      finalCCFlags = ccFlags;
      finalACFlags = SBig(*reinterpret_cast<const u32*>(materialDataCur + 12));
      auto outputReg = static_cast<GX::TevRegID>(ccFlags >> 9 & 0x3);
      if (outputReg == GX::TEVREG2) {
        usesTevReg2 = true;
      }
      materialDataCur += 20;
      texMapTexCoordFlags += 1;
    }
  }

  u32 tcgCount = 0;
  if (g_Renderer->IsThermalVisorActive()) {
    u32 fullTcgCount = SBig(*tcgs);
    tcgCount = std::min(fullTcgCount, 2u);
    for (u32 i = 0; i < tcgCount; ++i) {
      CGX::SetTexCoordGen(GX::TexCoordID(i), SBig(tcgs[i + 1]));
    }
    tcgs += fullTcgCount + 1;
  } else {
    tcgCount = SBig(*tcgs);
    for (u32 i = 0; i < tcgCount; ++i) {
      CGX::SetTexCoordGen(GX::TexCoordID(i), SBig(tcgs[i + 1]));
    }
    tcgs += tcgCount + 1;
  }

  const u32* uvAnim = tcgs;
  u32 animCount = SBig(uvAnim[1]);
  uvAnim += 2;
  u32 texMtx = GX::TEXMTX0;
  u32 pttTexMtx = GX::PTTEXMTX0;
  for (u32 i = 0; i < animCount; ++i) {
    u32 size = HandleAnimatedUV(uvAnim, static_cast<GX::TexMtx>(texMtx), static_cast<GX::PTTexMtx>(pttTexMtx));
    if (size == 0)
      break;
    uvAnim += size;
    texMtx += 3;
    pttTexMtx += 3;
  }

  if (flags.x0_blendMode != 0) {
    HandleTransparency(finalTevCount, finalKColorCount, flags, blendFactors, finalCCFlags, finalACFlags);
  }

  if (reflection) {
    if (sReflectionAlpha > 0.f) {
      u32 additionalTevs = 0;
      if (indTex) {
        additionalTevs = HandleReflection(usesTevReg2, indTexSlot, 0, finalTevCount, texCount, tcgCount,
                                          finalKColorCount, finalCCFlags, finalACFlags);
        numIndStages = 1;
        tcgCount += 2;
      } else {
        additionalTevs = HandleReflection(usesTevReg2, 255, 0, finalTevCount, texCount, tcgCount, finalKColorCount,
                                          finalCCFlags, finalACFlags);
        tcgCount += 1;
      }
      texCount += 1;
      finalTevCount += additionalTevs;
      finalKColorCount += 1;
    } else if (((finalCCFlags >> 9) & 0x3) != 0) {
      DoPassthru(finalTevCount);
      finalTevCount += 1;
    }
  }

  if (CCubeModel::sRenderModelShadow) {
    DoModelShadow(texCount, tcgCount);
    tcgCount += 1;
  }

  CGX::SetNumIndStages(numIndStages);
  CGX::SetNumTevStages(finalTevCount);
  CGX::SetNumTexGens(tcgCount);
  CGX::SetNumChans(finalNumColorChans);
}

void CCubeMaterial::SetCurrentBlack() {
  const auto flags = GetFlags();
  const auto vatFlags = GetVatFlags();
  if (flags.IsSet(CCubeMaterialFlagBits::fDepthSorting) || flags.IsSet(CCubeMaterialFlagBits::fAlphaTest)) {
    CGX::SetBlendMode(GX::BM_BLEND, GX::BL_ZERO, GX::BL_ONE, GX::LO_CLEAR);
  } else {
    CGX::SetBlendMode(GX::BM_BLEND, GX::BL_ONE, GX::BL_ZERO, GX::LO_CLEAR);
  }
//  CGX::SetVtxDescv_Compressed(vatFlags);
//  CGX::SetTevColorIn(GX::TEVSTAGE0, GX::CC_ZERO, GX::CC_ZERO, GX::CC_ZERO, GX::CC_ZERO /* ? CC_ONE */);
//  CGX::SetTevAlphaIn(GX::TEVSTAGE0, GX::CA_ZERO, GX::CA_ZERO, GX::CA_ZERO, GX::CA_ZERO /* ? CA_KONST */);
//  CGX::SetTevKAlphaSel(GX::TEVSTAGE0, GX::TEV_KASEL_1);
//  CGX::SetTexCoordGen(GX::TEXCOORD0, GX::TG_MTX2x4, GX::TG_POS, GX::IDENTITY, false, GX::PTIDENTITY);
//  CGX::SetStandardTevColorAlphaOp(GX::TEVSTAGE0);
//  CGX::SetTevOrder(GX::TEVSTAGE0, GX::TEXCOORD_NULL, GX::TEXMAP_NULL, GX::COLOR_NULL);
//  CGX::SetNumTevStages(1);
//  CGX::SetNumChans(0);
//  CGX::SetNumTexGens(1);
//  CGX::SetNumIndStages(0);
}

void CCubeMaterial::SetupBlendMode(u32 blendFactors, const CModelFlags& flags, bool alphaTest) {
  auto newSrcFactor = static_cast<GX::BlendFactor>(blendFactors & 0xffff);
  auto newDstFactor = static_cast<GX::BlendFactor>(blendFactors >> 16 & 0xffff);
  if (alphaTest) {
    // discard fragments with alpha < 0.25
    CGX::SetAlphaCompare(GX::GEQUAL, 64, GX::AOP_OR, GX::NEVER, 0);
    newSrcFactor = GX::BL_ONE;
    newDstFactor = GX::BL_ZERO;
  } else {
    CGX::SetAlphaCompare(GX::ALWAYS, 0, GX::AOP_OR, GX::ALWAYS, 0);
  }

  if (flags.x0_blendMode > 4 && newSrcFactor == GX::BL_ONE) {
    newSrcFactor = GX::BL_SRCALPHA;
    if (newDstFactor == GX::BL_ZERO) {
      newDstFactor = flags.x0_blendMode > 6 ? GX::BL_ONE : GX::BL_INVSRCALPHA;
    }
  }

  CGX::SetBlendMode(GX::BM_BLEND, newSrcFactor, newDstFactor, GX::LO_CLEAR);
}

void CCubeMaterial::HandleDepth(CModelFlagsFlags modelFlags, CCubeMaterialFlags matFlags) {
  GX::Compare func = GX::NEVER;
  if (!(modelFlags & CModelFlagBits::DepthTest)) {
    func = GX::ALWAYS;
  } else if (modelFlags & CModelFlagBits::DepthGreater) {
    func = modelFlags & CModelFlagBits::DepthNonInclusive ? GX::GREATER : GX::GEQUAL;
  } else {
    func = modelFlags & CModelFlagBits::DepthNonInclusive ? GX::LESS : GX::LEQUAL;
  }
  bool depthWrite = modelFlags & CModelFlagBits::DepthUpdate && matFlags & CCubeMaterialFlagBits::fDepthWrite;
  CGX::SetZMode(true, func, depthWrite);
}

void CCubeMaterial::ResetCachedMaterials() {
  KillCachedViewDepState();
  sLastMaterialUnique = UINT32_MAX;
  sRenderingModel = nullptr;
  sLastMaterialCached = nullptr;
}

void CCubeMaterial::KillCachedViewDepState() { sLastModelCached = nullptr; }

void CCubeMaterial::EnsureViewDepStateCached(const CCubeSurface* surface) {
  // TODO
  if ((surface != nullptr || sLastModelCached != sRenderingModel) && sRenderingModel != nullptr) {
    sLastModelCached = sRenderingModel;
    if (surface == nullptr) {
      sReflectionType = 1;
    } else {
      sReflectionType = 2;
    }
    if (g_Renderer->IsReflectionDirty()) {

    } else {
      g_Renderer->SetReflectionDirty(true);
    }
  }
}

u32 CCubeMaterial::HandleColorChannels(u32 chanCount, u32 firstChan) {
  if (CCubeModel::sRenderModelShadow) {
    if (chanCount != 0) {
      CGX::SetChanAmbColor(CGX::EChannelId::Channel1, zeus::skBlack);
      CGX::SetChanMatColor(CGX::EChannelId::Channel1, zeus::skWhite);

      auto chan0Lights = CGraphics::g_LightActive & ~CCubeModel::sChannel0DisableLightMask;
      CGX::SetChanCtrl(CGX::EChannelId::Channel0, firstChan, chan0Lights);
      CGX::SetChanCtrl(CGX::EChannelId::Channel1, CCubeModel::sChannel1EnableLightMask);
      if (chan0Lights.any()) {
        CGX::SetChanMatColor(CGX::EChannelId::Channel0, zeus::skWhite);
      } else {
        CGX::SetChanMatColor(CGX::EChannelId::Channel0, CGX::GetChanAmbColor(CGX::EChannelId::Channel0));
      }
    }
    return 2;
  }

  if (chanCount == 2) {
    CGX::SetChanAmbColor(CGX::EChannelId::Channel1, zeus::skBlack);
    CGX::SetChanMatColor(CGX::EChannelId::Channel1, zeus::skWhite);
  } else {
    CGX::SetChanCtrl(CGX::EChannelId::Channel1, {});
  }

  if (chanCount == 0) {
    CGX::SetChanCtrl(CGX::EChannelId::Channel0, {});
  } else {
    CGX::SetChanCtrl(CGX::EChannelId::Channel0, firstChan, CGraphics::g_LightActive);
    if (CGraphics::g_LightActive.any()) {
      CGX::SetChanMatColor(CGX::EChannelId::Channel0, zeus::skWhite);
    } else {
      CGX::SetChanMatColor(CGX::EChannelId::Channel0, CGX::GetChanAmbColor(CGX::EChannelId::Channel0));
    }
  }

  return chanCount;
}

void CCubeMaterial::HandleTev(u32 tevCur, const u32* materialDataCur, const u32* texMapTexCoordFlags,
                              bool shadowMapsEnabled) {
  const u32 colorArgs = shadowMapsEnabled ? 0x7a04f : SBig(materialDataCur[0]);
  const u32 alphaArgs = SBig(materialDataCur[1]);
  const u32 colorOps = SBig(materialDataCur[2]);
  const u32 alphaOps = SBig(materialDataCur[3]);

  const auto stage = static_cast<GX::TevStageID>(tevCur);
  CGX::SetStandardDirectTev_Compressed(stage, colorArgs, alphaArgs, colorOps, alphaOps);

  u32 tmtcFlags = SBig(*texMapTexCoordFlags);
  u32 matFlags = SBig(materialDataCur[4]);
  CGX::SetTevOrder(stage, static_cast<GX::TexCoordID>(tmtcFlags & 0xFF),
                   static_cast<GX::TexMapID>(tmtcFlags >> 8 & 0xFF), static_cast<GX::ChannelID>(matFlags & 0xFF));
  CGX::SetTevKColorSel(stage, static_cast<GX::TevKColorSel>(matFlags >> 0x8 & 0xFF));
  CGX::SetTevKAlphaSel(stage, static_cast<GX::TevKAlphaSel>(matFlags >> 0x10 & 0xFF));
}

constexpr zeus::CTransform MvPostXf{
    {zeus::CVector3f{0.5f, 0.f, 0.f}, {0.f, 0.f, 0.f}, {0.f, 0.5f, 0.f}},
    {0.5f, 0.5f, 1.f},
};

u32 CCubeMaterial::HandleAnimatedUV(const u32* uvAnim, GX::TexMtx texMtx, GX::PTTexMtx pttTexMtx) {
  u32 type = SBig(*uvAnim);
  const float* params = reinterpret_cast<const float*>(uvAnim + 1);
  switch (type) {
  case 0: {
    auto xf = CGraphics::g_ViewMatrix.inverse().multiplyIgnoreTranslation(CGraphics::g_GXModelMatrix);
    xf.origin.zeroOut();
    GXLoadTexMtxImm(&xf, texMtx, GX::MTX3x4);
    GXLoadTexMtxImm(&MvPostXf, pttTexMtx, GX::MTX3x4);
    return 1;
  }
  case 1: {
    auto xf = CGraphics::g_ViewMatrix.inverse() * CGraphics::g_GXModelMatrix;
    GXLoadTexMtxImm(&xf, texMtx, GX::MTX3x4);
    GXLoadTexMtxImm(&MvPostXf, pttTexMtx, GX::MTX3x4);
    return 1;
  }
  case 2: {
    const float f1 = SBig(params[0]);
    const float f2 = SBig(params[1]);
    const float f3 = SBig(params[2]);
    const float f4 = SBig(params[3]);
    const float seconds = CGraphics::GetSecondsMod900();
    const auto xf = zeus::CTransform::Translate(seconds * f3 + f1, seconds * f4 + f2, 0.f);
    GXLoadTexMtxImm(&xf, texMtx, GX::MTX3x4);
    return 5;
  }
  case 3: {
    const float angle = CGraphics::GetSecondsMod900() * SBig(params[1]) + SBig(params[0]);
    const float acos = std::cos(angle);
    const float asin = std::sin(angle);
    zeus::CTransform xf;
    xf.basis[0][0] = acos;
    xf.basis[0][1] = asin;
    xf.basis[1][0] = -asin;
    xf.basis[1][1] = acos;
    xf.origin[0] = (1.f - (acos - asin)) * 0.5f;
    xf.origin[1] = (1.f - (asin + acos)) * 0.5f;
    GXLoadTexMtxImm(&xf, texMtx, GX::MTX3x4);
    return 3;
  }
  case 4:
  case 5: {
    zeus::CTransform xf;
    const float value = SBig(params[0]) * SBig(params[2]) * (SBig(params[3]) + CGraphics::GetSecondsMod900());
    if (type == 4) {
      xf.origin.x() = std::trunc(SBig(params[1]) * std::fmod(value, 1.f)) * SBig(params[2]);
      xf.origin.y() = 0.f;
    } else {
      xf.origin.x() = 0.f;
      xf.origin.y() = std::trunc(SBig(params[1]) * std::fmod(value, 1.f)) * SBig(params[2]);
    }
    GXLoadTexMtxImm(&xf, texMtx, GX::MTX3x4);
    return 5;
  }
  case 6: {
    const zeus::CTransform mtx{CGraphics::g_GXModelMatrix.basis};
    const zeus::CTransform postMtx{
        {
            zeus::CVector3f{0.5f, 0.f, 0.f},
            zeus::CVector3f{0.f, 0.f, 0.f},
            zeus::CVector3f{0.f, 0.5f, 0.f},
        },
        zeus::CVector3f{
            CGraphics::g_GXModelMatrix.origin.x() * 0.05f,
            CGraphics::g_GXModelMatrix.origin.y() * 0.05f,
            1.f,
        },
    };
    GXLoadTexMtxImm(&mtx, texMtx, GX::MTX3x4);
    GXLoadTexMtxImm(&postMtx, pttTexMtx, GX::MTX3x4);
    return 1;
  }
  case 7: {
    zeus::CTransform mtx = CGraphics::g_ViewMatrix.inverse().multiplyIgnoreTranslation(CGraphics::g_GXModelMatrix);
    mtx.origin.zeroOut();
    float xy = SBig(params[1]) * (CGraphics::g_ViewMatrix.origin.x() + CGraphics::g_ViewMatrix.origin.y()) * 0.025f;
    xy = (xy - static_cast<int>(xy));
    float z = SBig(params[1]) * CGraphics::g_ViewMatrix.origin.z() * 0.05f;
    z = (z - static_cast<int>(z));
    float halfA = SBig(params[0]) * 0.5f;
    zeus::CTransform postMtx{
        {
            zeus::CVector3f{halfA, 0.f, 0.f},
            zeus::CVector3f{0.f, 0.f, 0.f},
            zeus::CVector3f{0.f, halfA, 0.f},
        },
        zeus::CVector3f{xy, z, 1.f},
    };
    GXLoadTexMtxImm(&mtx, texMtx, GX::MTX3x4);
    GXLoadTexMtxImm(&postMtx, pttTexMtx, GX::MTX3x4);
    return 3;
  }
  default:
    return 0;
  }
}

void CCubeMaterial::HandleTransparency(u32& finalTevCount, u32& finalKColorCount, const CModelFlags& modelFlags,
                                       u32 blendFactors, u32& finalCCFlags, u32& finalACFlags) {
  if (modelFlags.x0_blendMode == 2) {
    u16 dstFactor = blendFactors >> 16 & 0xffff;
    if (dstFactor == 1) {
      return;
    }
  }
  if (modelFlags.x0_blendMode == 3) {
    // Stage outputting splatted KAlpha as color to reg0
    auto stage = static_cast<GX::TevStageID>(finalTevCount);
    CGX::SetTevColorIn(stage, GX::CC_ZERO, GX::CC_ZERO, GX::CC_ZERO, GX::CC_KONST);
    CGX::SetTevAlphaIn(stage, GX::CA_ZERO, GX::CA_ZERO, GX::CA_ZERO, GX::CA_APREV);
    CGX::SetTevColorOp(stage, GX::TEV_ADD, GX::TB_ZERO, GX::CS_SCALE_1, true, GX::TEVREG0);
    CGX::SetTevKColorSel(stage, static_cast<GX::TevKColorSel>(finalKColorCount + GX::TEV_KCSEL_K0_A));
    CGX::SetTevAlphaOp(stage, GX::TEV_ADD, GX::TB_ZERO, GX::CS_SCALE_1, true, GX::TEVPREV);
    CGX::SetTevOrder(stage, GX::TEXCOORD_NULL, GX::TEXMAP_NULL, GX::COLOR_NULL);
    CGX::SetTevDirect(stage);

    // Stage interpolating from splatted KAlpha using KColor
    stage = static_cast<GX::TevStageID>(stage + 1);
    CGX::SetTevColorIn(stage, GX::CC_CPREV, GX::CC_C0, GX::CC_KONST, GX::CC_ZERO);
    CGX::SetTevAlphaIn(stage, GX::CA_ZERO, GX::CA_ZERO, GX::CA_ZERO, GX::CA_APREV);
    CGX::SetTevKColorSel(stage, static_cast<GX::TevKColorSel>(finalKColorCount + GX::TEV_KCSEL_K0));
    CGX::SetStandardTevColorAlphaOp(stage);
    CGX::SetTevDirect(stage);
    CGX::SetTevOrder(stage, GX::TEXCOORD_NULL, GX::TEXMAP_NULL, GX::COLOR_NULL);
    CGX::SetTevKColor(static_cast<GX::TevKColorID>(finalKColorCount), modelFlags.x4_color);

    finalKColorCount += 1;
    finalTevCount += 2;
  } else {
    auto stage = static_cast<GX::TevStageID>(finalTevCount);
    if (modelFlags.x0_blendMode == 8) {
      CGX::SetTevAlphaIn(stage, GX::CA_ZERO, GX::CA_ZERO, GX::CA_ZERO, GX::CA_KONST); // Set KAlpha
    } else {
      CGX::SetTevAlphaIn(stage, GX::CA_ZERO, GX::CA_KONST, GX::CA_APREV, GX::CA_ZERO); // Mul KAlpha
    }
    if (modelFlags.x0_blendMode == 2) {
      CGX::SetTevColorIn(stage, GX::CC_ZERO, GX::CC_ONE, GX::CC_CPREV, GX::CC_KONST); // Add KColor
    } else {
      CGX::SetTevColorIn(stage, GX::CC_ZERO, GX::CC_KONST, GX::CC_CPREV, GX::CC_ZERO); // Mul KColor
    }
    CGX::SetStandardTevColorAlphaOp(stage);

    finalCCFlags = 0x100; // Just clamp, output prev reg
    finalACFlags = 0x100;

    CGX::SetTevDirect(stage);
    CGX::SetTevOrder(stage, GX::TEXCOORD_NULL, GX::TEXMAP_NULL, GX::COLOR_NULL);
    CGX::SetTevKColor(static_cast<GX::TevKColorID>(finalKColorCount), modelFlags.x4_color);
    CGX::SetTevKColorSel(stage, static_cast<GX::TevKColorSel>(finalKColorCount + GX::TEV_KCSEL_K0));
    CGX::SetTevKAlphaSel(stage, static_cast<GX::TevKAlphaSel>(finalKColorCount + GX::TEV_KASEL_K0_A));

    finalTevCount += 1;
    finalKColorCount += 1;
  }
}

u32 CCubeMaterial::HandleReflection(bool usesTevReg2, u32 indTexSlot, u32 r5, u32 finalTevCount, u32 texCount,
                                    u32 tcgCount, u32 finalKColorCount, u32& finalCCFlags, u32& finalACFlags) {
  u32 out = 0;
  GX::TevColorArg colorArg = GX::CC_KONST;
  if (usesTevReg2) {
    colorArg = GX::CC_C2;
    const auto stage = static_cast<GX::TevStageID>(finalTevCount);
    CGX::SetTevColorIn(stage, GX::CC_ZERO, GX::CC_C2, GX::CC_KONST, GX::CC_ZERO);
    CGX::SetTevAlphaIn(stage, GX::CA_ZERO, GX::CA_ZERO, GX::CA_ZERO, GX::CA_A2);
    CGX::SetTevColorOp(stage, GX::TEV_ADD, GX::TB_ZERO, GX::CS_SCALE_1, true, GX::TEVREG2);
    CGX::SetTevAlphaOp(stage, GX::TEV_ADD, GX::TB_ZERO, GX::CS_SCALE_1, true, GX::TEVREG2);
    CGX::SetTevOrder(stage, GX::TEXCOORD_NULL, GX::TEXMAP_NULL, GX::COLOR_ZERO);
    out = 1;
  }
  CGX::SetTevKColor(static_cast<GX::TevKColorID>(finalKColorCount), zeus::CColor{sReflectionAlpha, sReflectionAlpha});
  CGX::SetTevKColorSel(static_cast<GX::TevStageID>(finalTevCount),
                       static_cast<GX::TevKColorSel>(GX::TEV_KCSEL_K0 + finalKColorCount));

  const auto stage = static_cast<GX::TevStageID>(finalTevCount + out);
  // tex = g_Renderer->GetRealReflection
  // tex.Load(texCount, 0)

  // TODO

  finalACFlags = 0;
  finalCCFlags = 0;

  // aurora::gfx::set_tev_order(stage, ...)

  return out; //+ 1;
}

void CCubeMaterial::DoPassthru(u32 finalTevCount) {
  const auto stage = static_cast<GX::TevStageID>(finalTevCount);
  CGX::SetTevColorIn(stage, GX::CC_ZERO, GX::CC_ZERO, GX::CC_ZERO, GX::CC_CPREV);
  CGX::SetTevAlphaIn(stage, GX::CA_ZERO, GX::CA_ZERO, GX::CA_ZERO, GX::CA_APREV);
  CGX::SetTevOrder(stage, GX::TEXCOORD_NULL, GX::TEXMAP_NULL, GX::COLOR_NULL);
  CGX::SetTevDirect(stage);
  CGX::SetStandardTevColorAlphaOp(stage);
}

void CCubeMaterial::DoModelShadow(u32 texCount, u32 tcgCount) {
  // CCubeModel::sShadowTexture->Load(texCount, EClampMode::One);
  // TODO
}

static GX::TevStageID sCurrentTevStage = GX::NULL_STAGE;
void CCubeMaterial::EnsureTevsDirect() {
  if (sCurrentTevStage == GX::NULL_STAGE) {
    return;
  }

  CGX::SetNumIndStages(0);
  CGX::SetTevDirect(sCurrentTevStage);
  sCurrentTevStage = GX::NULL_STAGE;
}
} // namespace metaforce
