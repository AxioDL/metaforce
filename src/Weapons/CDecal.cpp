#include "Weapons/CDecal.hpp"

#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Particles/CParticleGlobals.hpp"

#include "dolphin/gx/GXTev.h"
#include "dolphin/gx/GXVert.h"

CRandom16 CDecal::sDecalRandom(99);
bool CDecal::sMoveRedToAlphaBuffer = false;

void CDecal::SetGlobalSeed(ushort seed) { sDecalRandom.SetSeed(seed); }

CDecal::CDecal(const TToken< CDecalDescription >& desc, const CTransform4f& xf)
: mDescription(desc)
, mTransform(xf)
, mModelLifetime(0)
, mFrameIdx(0)
, mFlags(0)
, mRotation(CVector3f::Zero()) {
  CGlobalRandom gr(sDecalRandom);

  InitQuad(mQuad1, mDescription->mQuad1, 1);
  InitQuad(mQuad2, mDescription->mQuad2, 2);

  if (mDescription->mDMDL) {
    if (!mDescription->mDLFT.null()) {
      mDescription->mDLFT->GetValue(0, mModelLifetime);
    } else {
      mModelLifetime = 0x7FFFFF;
    }

    if (!mDescription->mDMRT.null()) {
      mDescription->mDMRT->GetValue(0, mRotation);
    }
  } else {
    mFlags |= 4;
  }
}

void CDecal::RenderQuad(CQuadDecal& decal, const CDecalDescription::SQuadDescr& desc) const {
  CColor color = CColor::White();
  float size = 1.f;
  CVector3f offset = CVector3f::Zero();
  if (CColorElement* clr = desc.mCLR.get()) {
    clr->GetValue(mFrameIdx, color);
  }
  if (CRealElement* sze = desc.mSZE.get()) {
    sze->GetValue(mFrameIdx, size);
    size *= 0.5f;
  }
  if (CRealElement* rot = desc.mROT.get()) {
    rot->GetValue(mFrameIdx, decal.mRotation);
  }
  if (CVectorElement* off = desc.mOFF.get()) {
    off->GetValue(mFrameIdx, offset);
    offset.SetY(0.f);
  }

  CTransform4f modXf = mTransform;
  modXf.AddTranslation(offset);
  CGraphics::SetModelMatrix(modXf);
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);

  bool redToAlpha = CDecal::sMoveRedToAlphaBuffer && desc.mADD && !desc.mTEX.null();
  if (desc.mADD) {
    CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
    if (redToAlpha) {
      CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
    } else {
      CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
    }
  } else {
    CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  }

  SUVElementSet uvSet;
  uvSet.xMin = 0.f;
  uvSet.xMax = 1.f;
  uvSet.yMin = 0.f;
  uvSet.yMax = 1.f;
  if (!desc.mTEX.null()) {
    TToken< CTexture > tex = desc.mTEX->GetValueTexture(mFrameIdx);
    if (!tex.IsLoaded()) {
      return;
    }
    tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
    tex.GetObj(); // ?
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
    desc.mTEX->GetValueUV(mFrameIdx, uvSet);
    if (redToAlpha) {
      CGX::SetNumTevStages(2);
      CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
      CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
      CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
      CGX::SetAlphaCompare(GX_GREATER, 0, GX_AOP_OR, GX_NEVER, 0);
      GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP1);
    } else {
      CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    }
  } else {
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  }

  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(1);
  CGX::SetNumIndStages(0);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  static const GXVtxDescList vtxDesc[4] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_CLR0, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(vtxDesc);
  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

  if (decal.mRotation == 0.f) {
    // Vertex 0
    GXPosition3f32(-size, 0.001f, size);
    GXColor1u32(color.GetColor_u32());
    GXTexCoord2f32(uvSet.xMin, uvSet.yMax);
    // Vertex 1
    GXPosition3f32(size, 0.001f, size);
    GXColor1u32(color.GetColor_u32());
    GXTexCoord2f32(uvSet.xMax, uvSet.yMax);
    // Vertex 2
    GXPosition3f32(-size, 0.001f, -size);
    GXColor1u32(color.GetColor_u32());
    GXTexCoord2f32(uvSet.xMin, uvSet.yMin);
    // Vertex 3
    GXPosition3f32(size, 0.001f, -size);
    GXColor1u32(color.GetColor_u32());
    GXTexCoord2f32(uvSet.xMax, uvSet.yMin);
  } else {
    const CRelAngle ang = CRelAngle::FromDegrees(decal.mRotation);
    const float sinSize = sine(ang) * size;
    const float cosSize = cosine(ang) * size;
    // Vertex 0
    GXPosition3f32(sinSize - cosSize, 0.001f, cosSize + sinSize);
    GXColor1u32(color.GetColor_u32());
    GXTexCoord2f32(uvSet.xMin, uvSet.yMax);
    // Vertex 1
    GXPosition3f32(sinSize + cosSize, 0.001f, cosSize - sinSize);
    GXColor1u32(color.GetColor_u32());
    GXTexCoord2f32(uvSet.xMax, uvSet.yMax);
    // Vertex 2
    GXPosition3f32(-(sinSize + cosSize), 0.001f, -(cosSize - sinSize));
    GXColor1u32(color.GetColor_u32());
    GXTexCoord2f32(uvSet.xMin, uvSet.yMin);
    // Vertex 3
    GXPosition3f32(-sinSize + cosSize, 0.001f, -cosSize - sinSize);
    GXColor1u32(color.GetColor_u32());
    GXTexCoord2f32(uvSet.xMax, uvSet.yMin);
  }

  CGX::End();
  if (redToAlpha) {
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  }
}

void CDecal::RenderMdl() const {
  CColor color = CColor::White();
  CVector3f offset = CVector3f::Zero();
  CTransform4f rotXf(CTransform4f::Identity());
  if (!mDescription->mDMOO) {
    rotXf = mTransform.GetRotation();
  }

  bool dmrtIsConst = false;
  if (CVectorElement* off = mDescription->mDMRT.get()) {
    if (off->IsFastConstant()) {
      dmrtIsConst = true;
    }
  }

  CTransform4f dmrtXf = CTransform4f::Identity();
  if (dmrtIsConst) {
    mDescription->mDMRT->GetValue(mFrameIdx, mRotation);
    dmrtXf = CTransform4f::RotateZ(CRelAngle::FromDegrees(mRotation.GetZ()));
    dmrtXf.RotateLocalY(CRelAngle::FromDegrees(mRotation.GetY()));
    dmrtXf.RotateLocalX(CRelAngle::FromDegrees(mRotation.GetX()));
  }
  dmrtXf = rotXf * dmrtXf;

  if (CVectorElement* off = mDescription->mDMOP.get()) {
    off->GetValue(mFrameIdx, offset);
  }

  CTransform4f worldXf = CTransform4f::Translate(mTransform.GetTranslation() + rotXf * offset);
  if (dmrtIsConst) {
    worldXf *= dmrtXf;
  } else if (CVectorElement* dmrt = mDescription->mDMRT.get()) {
    CVector3f rotation(0.f, 0.f, 0.f);
    dmrt->GetValue(mFrameIdx, rotation);
    dmrtXf = CTransform4f::RotateZ(CRelAngle::FromDegrees(rotation.GetZ()));
    dmrtXf.RotateLocalY(CRelAngle::FromDegrees(rotation.GetY()));
    dmrtXf.RotateLocalX(CRelAngle::FromDegrees(rotation.GetX()));
    worldXf *= rotXf * dmrtXf;
  } else {
    worldXf *= dmrtXf;
  }

  if (CVectorElement* dmsc = mDescription->mDMSC.get()) {
    CVector3f scale(0.f, 0.f, 0.f);
    dmsc->GetValue(mFrameIdx, scale);
    worldXf *= CTransform4f::Scale(scale.GetX(), scale.GetY(), scale.GetZ());
  }

  if (CColorElement* dmcl = mDescription->mDMCL.get()) {
    dmcl->GetValue(mFrameIdx, color);
  }

  CGraphics::SetModelMatrix(worldXf);

  if (mDescription->mDMAB) {
    const CModelFlags flags = CModelFlags::Additive(color).DepthCompareUpdate(true, false);
    (*mDescription->mDMDL)->Draw(flags);
  } else if (color.GetAlpha() == 1.f) {
    (*mDescription->mDMDL)->Draw(CModelFlags::Normal());
  } else {
    (*mDescription->mDMDL)
        ->Draw(CModelFlags::AlphaBlendedDepthCompareUpdate(color, true, false));
  }

  CGraphics::SetCullMode(kCM_Front);
  CTevCombiners::ResetStates();
}

void CDecal::Render() const {
  CGlobalRandom gr(sDecalRandom);
  if (IsDone()) {
    return;
  }

  CGraphics::DisableAllLights();
  CParticleGlobals::SetEmitterTime(mFrameIdx);

  ProcessQuad(const_cast< CQuadDecal& >(mQuad1), mDescription->mQuad1, 1);
  ProcessQuad(const_cast< CQuadDecal& >(mQuad2), mDescription->mQuad2, 2);
  if (mDescription->mDMDL && (mFlags & 4) == 0) {
    CParticleGlobals::SetParticleLifetime(mModelLifetime);
    CParticleGlobals::UpdateParticleLifetimeTweenValues(mFrameIdx);
    RenderMdl();
  }
}

void CDecal::Update(float dt) {
  if (mFrameIdx >= mQuad1.GetLifetime()) {
    mFlags |= 1;
  }

  if (mFrameIdx >= mQuad2.GetLifetime()) {
    mFlags |= 2;
  }

  if (mFrameIdx >= mModelLifetime) {
    mFlags |= 4;
  }

  ++mFrameIdx;
}
