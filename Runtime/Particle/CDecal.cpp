#include "Runtime/Particle/CDecal.hpp"

#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/Graphics/Shaders/CDecalShaders.hpp"
#include "Runtime/Particle/CParticleGlobals.hpp"
#include "Runtime/Graphics/CGX.hpp"

namespace metaforce {
CRandom16 CDecal::sDecalRandom;
bool CDecal::sMoveRedToAlphaBuffer = false;

CDecal::CDecal(const TToken<CDecalDescription>& desc, const zeus::CTransform& xf)
: x0_description(desc), xc_transform(xf) {
  CGlobalRandom gr(sDecalRandom);

  CDecalDescription& desco = *x0_description;
  x5c_31_quad1Invalid = InitQuad(x3c_decalQuads[0], desco.x0_Quads[0]);
  x5c_30_quad2Invalid = InitQuad(x3c_decalQuads[1], desco.x0_Quads[1]);

  CDecalDescription* d = x0_description.GetObj();
  if (d->x38_DMDL) {
    if (d->x48_DLFT) {
      d->x48_DLFT->GetValue(0, x54_modelLifetime);
    } else {
      x54_modelLifetime = 0x7FFFFF;
    }

    if (d->x50_DMRT) {
      d->x50_DMRT->GetValue(0, x60_rotation);
    }
  } else {
    x5c_29_modelInvalid = true;
  }
}

bool CDecal::InitQuad(CQuadDecal& quad, const SQuadDescr& desc) {
  if (desc.x14_TEX) {
    if (desc.x0_LFT) {
      desc.x0_LFT->GetValue(0, quad.x4_lifetime);
    } else {
      quad.x4_lifetime = 0x7FFFFF;
    }
    if (desc.x8_ROT) {
      desc.x8_ROT->GetValue(0, quad.x8_rotation);
      quad.x0_24_invalid &= desc.x8_ROT->IsConstant();
    }

    if (desc.x4_SZE) {
      quad.x0_24_invalid &= desc.x4_SZE->IsConstant();
      if (quad.x0_24_invalid) {
        float size = 1.f;
        desc.x4_SZE->GetValue(0, size);
        quad.x0_24_invalid = size <= 1.f;
      }
    }

    if (desc.xc_OFF) {
      quad.x0_24_invalid &= desc.xc_OFF->IsFastConstant();
    }
    return false;
  }

  quad.x0_24_invalid = false;
  return true;
}

void CDecal::SetGlobalSeed(u16 seed) { sDecalRandom.SetSeed(seed); }

void CDecal::SetMoveRedToAlphaBuffer(bool move) { sMoveRedToAlphaBuffer = move; }

void CDecal::RenderQuad(CQuadDecal& decal, const SQuadDescr& desc) const {
  zeus::CColor color = zeus::skWhite;
  float size = 1.f;
  zeus::CVector3f offset;
  if (CColorElement* clr = desc.x10_CLR.get()) {
    clr->GetValue(x58_frameIdx, color);
  }
  if (CRealElement* sze = desc.x4_SZE.get()) {
    sze->GetValue(x58_frameIdx, size);
    size *= 0.5f;
  }
  if (CRealElement* rot = desc.x8_ROT.get()) {
    rot->GetValue(x58_frameIdx, decal.x8_rotation);
  }
  if (CVectorElement* off = desc.xc_OFF.get()) {
    off->GetValue(x58_frameIdx, offset);
    offset.y() = 0.f;
  }
  zeus::CTransform modXf = xc_transform;
  modXf.origin += offset;
  CGraphics::SetModelMatrix(modXf);
  CGraphics::SetAlphaCompare(ERglAlphaFunc::Always, 0, ERglAlphaOp::And, ERglAlphaFunc::Always, 0);

  bool redToAlpha = CDecal::sMoveRedToAlphaBuffer && desc.x18_ADD && desc.x14_TEX;
  if (desc.x18_ADD) {
    CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, false);
    if (redToAlpha) {
      CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::One, ERglBlendFactor::One, ERglLogicOp::Clear);
    } else {
      CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::One,
                              ERglLogicOp::Clear);
    }
  } else {
    CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, false);
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha,
                            ERglLogicOp::Clear);
  }

  SUVElementSet uvSet{0.f, 1.f, 0.f, 1.f};
  if (desc.x14_TEX) {
    TLockedToken<CTexture> tex = desc.x14_TEX->GetValueTexture(x58_frameIdx);
    tex->Load(GX_TEXMAP0, EClampMode::Repeat);
    CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::kEnvModulate);
    desc.x14_TEX->GetValueUV(x58_frameIdx, uvSet);
    if (redToAlpha) {
      CGX::SetNumTevStages(2);
      CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
      CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
      CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
      CGX::SetAlphaCompare(GX_GREATER, 0, GX_AOP_OR, GX_NEVER, 0);
      GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP1);
    } else {
      CGraphics::SetTevOp(ERglTevStage::Stage1, CTevCombiners::kEnvPassthru);
    }
  } else {
    CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::kEnvPassthru);
    CGraphics::SetTevOp(ERglTevStage::Stage1, CTevCombiners::kEnvPassthru);
  }

  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(1);
  CGX::SetNumIndStages(0);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetChanCtrl(CGX::EChannelId::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
  static const GXVtxDescList vtxDesc[4] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_CLR0, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(vtxDesc);
  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

  float y = 0.001f;
  if (decal.x8_rotation == 0.f) {
    // Vertex 0
    GXPosition3f32(-size, y, size);
    GXColor4f32(color);
    GXTexCoord2f32(uvSet.xMin, uvSet.yMin);
    // Vertex 1
    GXPosition3f32(size, y, size);
    GXColor4f32(color);
    GXTexCoord2f32(uvSet.xMax, uvSet.yMin);
    // Vertex 2
    GXPosition3f32(-size, y, -size);
    GXColor4f32(color);
    GXTexCoord2f32(uvSet.xMin, uvSet.yMax);
    // Vertex 3
    GXPosition3f32(size, y, -size);
    GXColor4f32(color);
    GXTexCoord2f32(uvSet.xMax, uvSet.yMax);
  } else {
    float ang = zeus::degToRad(decal.x8_rotation);
    float sinSize = sin(ang) * size;
    float cosSize = cos(ang) * size;
    // Vertex 0
    GXPosition3f32(sinSize - cosSize, y, cosSize + sinSize);
    GXColor4f32(color);
    GXTexCoord2f32(uvSet.xMin, uvSet.yMin);
    // Vertex 1
    GXPosition3f32(cosSize + sinSize, y, cosSize - sinSize);
    GXColor4f32(color);
    GXTexCoord2f32(uvSet.xMax, uvSet.yMin);
    // Vertex 2
    GXPosition3f32(-(cosSize + sinSize), y, -(cosSize - sinSize));
    GXColor4f32(color);
    GXTexCoord2f32(uvSet.xMin, uvSet.yMax);
    // Vertex 3
    GXPosition3f32(-sinSize + cosSize, y, -cosSize - sinSize);
    GXColor4f32(color);
    GXTexCoord2f32(uvSet.xMax, uvSet.yMax);
  }

  CGX::End();
  if (redToAlpha) {
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
    CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  }
}

void CDecal::RenderMdl() {
  CDecalDescription& desc = *x0_description;
  zeus::CColor color = zeus::skWhite;
  zeus::CVector3f dmop;
  zeus::CTransform rotXf;

  if (!desc.x5c_25_DMOO)
    rotXf = xc_transform.getRotation();

  bool dmrtIsConst = false;
  if (CVectorElement* dmrt = desc.x50_DMRT.get())
    dmrtIsConst = dmrt->IsFastConstant();

  zeus::CTransform dmrtXf;
  if (dmrtIsConst) {
    desc.x50_DMRT->GetValue(x58_frameIdx, x60_rotation);
    dmrtXf = zeus::CTransform::RotateZ(zeus::degToRad(x60_rotation.z()));
    dmrtXf.rotateLocalY(zeus::degToRad(x60_rotation.y()));
    dmrtXf.rotateLocalX(zeus::degToRad(x60_rotation.x()));
  }

  dmrtXf = rotXf * dmrtXf;

  if (CVectorElement* dmopo = desc.x4c_DMOP.get())
    dmopo->GetValue(x58_frameIdx, dmop);

  zeus::CTransform worldXf = zeus::CTransform::Translate(rotXf * dmop + xc_transform.origin);

  if (dmrtIsConst) {
    worldXf = worldXf * dmrtXf;
  } else {
    if (CVectorElement* dmrt = desc.x50_DMRT.get()) {
      zeus::CVector3f dmrtVec;
      dmrt->GetValue(x58_frameIdx, dmrtVec);
      dmrtXf = zeus::CTransform::RotateZ(zeus::degToRad(dmrtVec.z()));
      dmrtXf.rotateLocalY(zeus::degToRad(dmrtVec.y()));
      dmrtXf.rotateLocalX(zeus::degToRad(dmrtVec.x()));
      worldXf = worldXf * rotXf * dmrtXf;
    } else {
      worldXf = worldXf * dmrtXf;
    }
  }

  if (CVectorElement* dmsc = desc.x54_DMSC.get()) {
    zeus::CVector3f dmscVec;
    dmsc->GetValue(x58_frameIdx, dmscVec);
    worldXf = worldXf * zeus::CTransform::Scale(dmscVec);
  }

  if (CColorElement* dmcl = desc.x58_DMCL.get())
    dmcl->GetValue(x58_frameIdx, color);

  CGraphics::SetModelMatrix(worldXf);

  if (desc.x5c_24_DMAB) {
    const CModelFlags flags(7, 0, 1, color);
    desc.x38_DMDL->Draw(flags);
  } else {
    if (color.a() == 1.f) {
      constexpr CModelFlags flags(0, 0, 3, zeus::skWhite);
      desc.x38_DMDL->Draw(flags);
    } else {
      const CModelFlags flags(5, 0, 1, color);
      desc.x38_DMDL->Draw(flags);
    }
  }
}

void CDecal::Render() {
  SCOPED_GRAPHICS_DEBUG_GROUP("CDecal::Render", zeus::skYellow);
  CGlobalRandom gr(sDecalRandom);
  if (x5c_29_modelInvalid && x5c_30_quad2Invalid && x5c_31_quad1Invalid) {
    return;
  }

  CGraphics::DisableAllLights();
  CParticleGlobals::instance()->SetEmitterTime(x58_frameIdx);

  const CDecalDescription& desc = *x0_description;
  if (desc.x0_Quads[0].x14_TEX && !x5c_31_quad1Invalid) {
    CParticleGlobals::instance()->SetParticleLifetime(x3c_decalQuads[0].x4_lifetime);
    CParticleGlobals::instance()->UpdateParticleLifetimeTweenValues(x58_frameIdx);
    RenderQuad(x3c_decalQuads[0], desc.x0_Quads[0]);
  }
  if (desc.x0_Quads[1].x14_TEX && !x5c_30_quad2Invalid) {
    CParticleGlobals::instance()->SetParticleLifetime(x3c_decalQuads[1].x4_lifetime);
    CParticleGlobals::instance()->UpdateParticleLifetimeTweenValues(x58_frameIdx);
    RenderQuad(x3c_decalQuads[1], desc.x0_Quads[1]);
  }
  if (desc.x38_DMDL && !x5c_29_modelInvalid) {
    CParticleGlobals::instance()->SetParticleLifetime(x54_modelLifetime);
    CParticleGlobals::instance()->UpdateParticleLifetimeTweenValues(x58_frameIdx);
    RenderMdl();
  }
}

void CDecal::Update(float dt) {
  if (x58_frameIdx >= x3c_decalQuads[0].x4_lifetime)
    x5c_31_quad1Invalid = true;
  if (x58_frameIdx >= x3c_decalQuads[1].x4_lifetime)
    x5c_30_quad2Invalid = true;
  if (x58_frameIdx >= x54_modelLifetime)
    x5c_29_modelInvalid = true;
  ++x58_frameIdx;
}

} // namespace metaforce
