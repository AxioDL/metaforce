#include "Runtime/Particle/CDecal.hpp"

#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/Graphics/Shaders/CDecalShaders.hpp"
#include "Runtime/Particle/CParticleGlobals.hpp"

namespace urde {
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
    if (d->x48_DLFT)
      d->x48_DLFT->GetValue(0, x54_modelLifetime);
    else
      x54_modelLifetime = 0x7FFFFF;

    if (d->x50_DMRT)
      d->x50_DMRT->GetValue(0, x60_rotation);
  } else {
    x5c_29_modelInvalid = true;
  }

  CGraphics::CommitResources([this](boo::IGraphicsDataFactory::Context& ctx) {
    for (auto& decal : x3c_decalQuads) {
      if (decal.m_desc->x14_TEX) {
        decal.m_instBuf = ctx.newDynamicBuffer(boo::BufferUse::Vertex, sizeof(SParticleInstanceTex), 1);
      } else {
        decal.m_instBuf = ctx.newDynamicBuffer(boo::BufferUse::Vertex, sizeof(SParticleInstanceNoTex), 1);
      }
      decal.m_uniformBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(SParticleUniforms), 1);
      CDecalShaders::BuildShaderDataBinding(ctx, decal);
    }
    return true;
  } BooTrace);
}

bool CDecal::InitQuad(CQuadDecal& quad, const SQuadDescr& desc) {
  quad.m_desc = &desc;
  if (desc.x14_TEX) {
    if (desc.x0_LFT)
      desc.x0_LFT->GetValue(0, quad.x4_lifetime);
    else
      quad.x4_lifetime = 0x7FFFFF;
    if (desc.x8_ROT) {
      desc.x8_ROT->GetValue(0, quad.x8_rotation);
      quad.x0_24_invalid = desc.x8_ROT->IsConstant();
    }

    if (desc.x4_SZE) {
      quad.x0_24_invalid = desc.x4_SZE->IsConstant();
      float size = 1.f;
      desc.x4_SZE->GetValue(0, size);
      quad.x0_24_invalid = size <= 1.f;
    }

    if (desc.xc_OFF)
      quad.x0_24_invalid = desc.xc_OFF->IsFastConstant();
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
  if (CColorElement* clr = desc.x10_CLR.get())
    clr->GetValue(x58_frameIdx, color);
  if (CRealElement* sze = desc.x4_SZE.get()) {
    sze->GetValue(x58_frameIdx, size);
    size *= 0.5f;
  }
  if (CRealElement* rot = desc.x8_ROT.get())
    rot->GetValue(x58_frameIdx, decal.x8_rotation);
  if (CVectorElement* off = desc.xc_OFF.get()) {
    off->GetValue(x58_frameIdx, offset);
    offset.y() = 0.f;
  }
  zeus::CTransform modXf = xc_transform;
  modXf.origin += offset;
  CGraphics::SetModelMatrix(modXf);

  SParticleUniforms uniformData = {
      CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f(), {1.f, 1.f, 1.f, 1.f}};
  decal.m_uniformBuf->load(&uniformData, sizeof(SParticleUniforms));

  bool redToAlpha = sMoveRedToAlphaBuffer && desc.x18_ADD && desc.x14_TEX;

  SUVElementSet uvSet = {0.f, 1.f, 0.f, 1.f};
  if (CUVElement* tex = desc.x14_TEX.get()) {
    TLockedToken<CTexture> texObj = tex->GetValueTexture(x58_frameIdx);
    if (!texObj.IsLoaded())
      return;
    tex->GetValueUV(x58_frameIdx, uvSet);
    if (redToAlpha)
      CGraphics::SetShaderDataBinding(decal.m_redToAlphaDataBind);
    else
      CGraphics::SetShaderDataBinding(decal.m_normalDataBind);

    g_instTexData.clear();
    g_instTexData.reserve(1);

    SParticleInstanceTex& inst = g_instTexData.emplace_back();
    if (decal.x8_rotation == 0.f) {
      inst.pos[0] = zeus::CVector3f(-size, 0.001f, size);
      inst.pos[1] = zeus::CVector3f(size, 0.001f, size);
      inst.pos[2] = zeus::CVector3f(-size, 0.001f, -size);
      inst.pos[3] = zeus::CVector3f(size, 0.001f, -size);
    } else {
      float ang = zeus::degToRad(decal.x8_rotation);
      float sinSize = std::sin(ang) * size;
      float cosSize = std::cos(ang) * size;
      inst.pos[0] = zeus::CVector3f(sinSize - cosSize, 0.001f, cosSize + sinSize);
      inst.pos[1] = zeus::CVector3f(cosSize + sinSize, 0.001f, cosSize - sinSize);
      inst.pos[2] = zeus::CVector3f(-(cosSize + sinSize), 0.001f, -(cosSize - sinSize));
      inst.pos[3] = zeus::CVector3f(-sinSize + cosSize, 0.001f, -cosSize - sinSize);
    }
    inst.color = color;
    inst.uvs[0] = zeus::CVector2f(uvSet.xMin, uvSet.yMin);
    inst.uvs[1] = zeus::CVector2f(uvSet.xMax, uvSet.yMin);
    inst.uvs[2] = zeus::CVector2f(uvSet.xMin, uvSet.yMax);
    inst.uvs[3] = zeus::CVector2f(uvSet.xMax, uvSet.yMax);

    decal.m_instBuf->load(g_instTexData.data(), g_instTexData.size() * sizeof(SParticleInstanceTex));
    CGraphics::DrawInstances(0, 4, g_instTexData.size());
  } else {
    g_instNoTexData.clear();
    g_instNoTexData.reserve(1);

    SParticleInstanceNoTex& inst = g_instNoTexData.emplace_back();
    if (decal.x8_rotation == 0.f) {
      inst.pos[0] = zeus::CVector3f(-size, 0.001f, size);
      inst.pos[1] = zeus::CVector3f(size, 0.001f, size);
      inst.pos[2] = zeus::CVector3f(-size, 0.001f, -size);
      inst.pos[3] = zeus::CVector3f(size, 0.001f, -size);
    } else {
      float ang = zeus::degToRad(decal.x8_rotation);
      float sinSize = std::sin(ang) * size;
      float cosSize = std::cos(ang) * size;
      inst.pos[0] = zeus::CVector3f(sinSize - cosSize, 0.001f, cosSize + sinSize);
      inst.pos[1] = zeus::CVector3f(cosSize + sinSize, 0.001f, cosSize - sinSize);
      inst.pos[2] = zeus::CVector3f(-(cosSize + sinSize), 0.001f, -(cosSize - sinSize));
      inst.pos[3] = zeus::CVector3f(-sinSize + cosSize, 0.001f, -cosSize - sinSize);
    }
    inst.color = color;

    decal.m_instBuf->load(g_instNoTexData.data(), g_instNoTexData.size() * sizeof(SParticleInstanceNoTex));
    CGraphics::DrawInstances(0, 4, g_instNoTexData.size());
  }
}

void CDecal::RenderMdl() {
  const CDecalDescription& desc = *x0_description;
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
    desc.x38_DMDL.m_token->Draw(flags);
  } else {
    if (color.a() == 1.f) {
      constexpr CModelFlags flags(0, 0, 3, zeus::skWhite);
      desc.x38_DMDL.m_token->Draw(flags);
    } else {
      const CModelFlags flags(5, 0, 1, color);
      desc.x38_DMDL.m_token->Draw(flags);
    }
  }
}

void CDecal::Render() {
  SCOPED_GRAPHICS_DEBUG_GROUP("CDecal::Render", zeus::skYellow);
  CGlobalRandom gr(sDecalRandom);
  if (x5c_29_modelInvalid && x5c_30_quad2Invalid && x5c_31_quad1Invalid)
    return;

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

} // namespace urde
