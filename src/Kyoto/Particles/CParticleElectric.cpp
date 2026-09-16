#include "Kyoto/Particles/CParticleElectric.hpp"

#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElectricDescription.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleGlobals.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"
#include "Kyoto/Particles/IElement.hpp"
#include "dolphin/gx/GXVert.h"
#include "float.h"
#include "math.h"
#include "rstl/math.hpp"

ushort CParticleElectric::sSeed = 99;

CParticleElectric::CParticleElectric(TToken< CElectricDescription > desc)
: x1c_elecDesc(desc)
, x28_currentFrame(0)
, x2c_LIFE(0)
, x30_curTime(0.0)
, x38_translation(CVector3f::Zero())
, x44_orientation(CTransform4f::Identity())
, x74_invOrientation(CTransform4f::Identity())
, xa4_globalTranslation(CVector3f::Zero())
, xb0_globalOrientation(CTransform4f::Identity())
, xe0_globalScale(1.f, 1.f, 1.f)
, xec_localScale(1.f, 1.f, 1.f)
, xf8_cachedXf(CTransform4f::Identity())
, x128_(0.f)
, x12c_renderTime(0.f)
, x130_minBounds(CVector3f::Zero())
, x13c_maxBounds(CVector3f::Zero())
, x148_(0.f)
, x14c_randState(sSeed++)
, x150_SSEG(8)
, x154_SCNT(1)
, x158_(0)
, x15c_genRem(0.f)
, x160_systemBounds(CAABox::MakeMaxInvertedBox())
, x1b8_moduColor(CColor::White())
, x1bc_allocated(false)
, x450_24_emitting(true)
, x450_25_haveGPSM(false)
, x450_26_haveEPSM(false)
, x450_27_haveSSWH(false)
, x450_28_haveLWD(false)
, x450_29_transformDirty(true) {
  if (x1c_elecDesc->x10_SSEG) {
    x1c_elecDesc->x10_SSEG->GetValue(x28_currentFrame, x150_SSEG);
  }
  if (x1c_elecDesc->xc_SCNT) {
    x1c_elecDesc->xc_SCNT->GetValue(x28_currentFrame, x154_SCNT);
  }
  if (x154_SCNT > 32) {
    x154_SCNT = 32;
  }
  if (x1c_elecDesc->x0_LIFE) {
    x1c_elecDesc->x0_LIFE->GetValue(0, x2c_LIFE);
  } else {
    x2c_LIFE = 0x7fffff;
  }
  if (x1c_elecDesc->x40_SSWH) {
    x450_27_haveSSWH = true;
    for (int i = 0; i < x154_SCNT; ++i) {
      x1e0_swooshGenerators.push_back(
          rs_new CParticleSwoosh(x1c_elecDesc->x40_SSWH->GetToken(), x150_SSEG));
      CParticleSwoosh& swoosh = *x1e0_swooshGenerators.back();
      const int count = swoosh.GetSwooshCount();
      for (int j = 0; j < count; ++j) {
        swoosh.SetWarmUp();
        swoosh.Update(0.0);
      }
    }
  }
  ++x150_SSEG;
  x420_calculatedVerts =
      rstl::vector< CVector3f, rstl::aligned_allocator >(x150_SSEG, CVector3f::Zero(),
                                                        rstl::aligned_allocator());
  x440_fractalOffsets =
      rstl::vector< CVector3f >(x150_SSEG, CVector3f::Zero(), rstl::rmemory_allocator());
  x430_fractalMags = rstl::vector< float >(x150_SSEG, 0.f, rstl::rmemory_allocator());
  if (x1c_elecDesc->x50_GPSM) {
    x450_25_haveGPSM = true;
    x400_gpsmGenerators.reserve(x154_SCNT);
    for (int i = 0; i < x154_SCNT; ++i) {
      x400_gpsmGenerators.push_back(rs_new CElementGen(x1c_elecDesc->x50_GPSM->GetToken()));
      x400_gpsmGenerators.back()->SetParticleEmission(false);
    }
  }
  if (x1c_elecDesc->x60_EPSM) {
    x450_26_haveEPSM = true;
    x410_epsmGenerators.reserve(x154_SCNT);
    for (int i = 0; i < x154_SCNT; ++i) {
      x410_epsmGenerators.push_back(rs_new CElementGen(x1c_elecDesc->x60_EPSM->GetToken()));
      x410_epsmGenerators.back()->SetParticleEmission(false);
    }
  }
  if (x1c_elecDesc->x28_LWD1 || x1c_elecDesc->x2c_LWD2 || x1c_elecDesc->x30_LWD3) {
    x450_28_haveLWD = true;
    for (int i = 0; i < x154_SCNT; ++i) {
      x2e4_lineManagers.push_back(rs_new CLineManager);
    }
  }
}

CParticleElectric::~CParticleElectric() {}

void CParticleElectric::CalculatePoints() {
  rstl::reserved_vector< CVector3f, 4 > points;
  CVector3f pos = CVector3f::Zero();
  CVector3f vel = CVector3f::Zero();
  if (x1c_elecDesc->x18_IEMT) {
    x1c_elecDesc->x18_IEMT->GetValue(x28_currentFrame, pos, vel);
  }
  if (x178_overrideIPos) {
    pos = *x178_overrideIPos;
  }
  if (x188_overrideIVel) {
    vel = *x188_overrideIVel;
  }
  if (vel.IsNonZero()) {
    points.push_back(pos);
    points.push_back(pos + vel);
    points.push_back(pos + vel * 2.f);
  } else {
    points.push_back(pos);
  }
  CVector3f fpos(0.f, 1.f, 0.f);
  CVector3f fvel = CVector3f::Zero();
  if (x1c_elecDesc->x1c_FEMT) {
    x1c_elecDesc->x1c_FEMT->GetValue(x28_currentFrame, fpos, fvel);
  }
  if (x198_overrideFPos) {
    fpos = *x198_overrideFPos;
  }
  if (x1a8_overrideFVel) {
    fvel = *x1a8_overrideFVel;
  }
  if (fvel.IsNonZero()) {
    if (points.size() == 3) {
      points[2] = fpos + fvel;
      points.push_back(fpos);
    } else {
      points.push_back(fpos + fvel * 2.f);
      points.push_back(fpos + fvel);
      points.push_back(fpos);
    }
  } else {
    points.push_back(fpos);
  }
  if (points.size() == 4) {
    const int segments = x150_SSEG - 1;
    const float segDiv = 1.f / static_cast< float >(segments);
    x420_calculatedVerts[0] = points[0];
    float curDiv = segDiv;
    for (int i = 1; i < segments; ++i) {
      const float t = segDiv * x14c_randState.Range(-0.45f, 0.45f) + curDiv;
      const CVector3f point = CMath::GetBezierPoint(points[0], points[1], points[2], points[3], t);
      x420_calculatedVerts[i] = point;
      curDiv += segDiv;
    }
    x420_calculatedVerts[segments] = points[3];
  } else {
    x420_calculatedVerts[0] = pos;
    const int segments = x150_SSEG - 1;
    const float segDiv = 1.f / static_cast< float >(segments);
    CVector3f accum = x420_calculatedVerts[0];
    const CVector3f segDelta = (fpos - pos) * segDiv;
    for (int i = 1; i < segments; ++i) {
      const float r = x14c_randState.Range(-0.45f, 0.45f);
      x420_calculatedVerts[i] = accum + segDelta * r;
      accum += segDelta;
    }
    x420_calculatedVerts[segments] = fpos;
  }
  for (int i = 0; i < x150_SSEG; ++i) {
    x430_fractalMags[i] = 0.f;
  }
  float ampl = 1.f;
  if (x1c_elecDesc->x20_AMPL) {
    x1c_elecDesc->x20_AMPL->GetValue(x28_currentFrame, ampl);
    ampl *= 2.f;
  }
  float ampd = 0.f;
  if (x1c_elecDesc->x24_AMPD) {
    x1c_elecDesc->x24_AMPD->GetValue(x28_currentFrame, ampd);
  }
  CalculateFractal(0, x420_calculatedVerts.size() - 1, ampl, ampd);
  CVector3f v0 = x420_calculatedVerts[0] - x420_calculatedVerts[1];
  CVector3f v1 = x420_calculatedVerts[x420_calculatedVerts.size() - 1] - x420_calculatedVerts[1];
  CVector3f up = CVector3f::Up();
  if (v0.CanBeNormalized() && v1.CanBeNormalized()) {
    v0.Normalize();
    v1.Normalize();
    float dot = CVector3f::Dot(v0, v1);
    dot = dot < 0.f ? -dot : dot;
    if (close_enough(dot, 1.f)) {
      up = CTransform4f::LookAt(x420_calculatedVerts[0], x420_calculatedVerts[1]).GetUp();
    } else {
      up = CVector3f::Cross(v0, v1).AsNormalized();
    }
  } else if (!(x420_calculatedVerts[0] == x420_calculatedVerts[1])) {
    up = CTransform4f::LookAt(x420_calculatedVerts[0], x420_calculatedVerts[1]).GetUp();
  }
  const float commonRand = x14c_randState.Range(0.f, 360.f);
  for (int i = 1; i < x420_calculatedVerts.size() - 1; ++i) {
    const CVector3f delta = x420_calculatedVerts[i] - x420_calculatedVerts[i - 1];
    if (delta.IsNonZero()) {
      const float randomAngle = x430_fractalMags[i] / ampl * 16.f * x14c_randState.Range(-1.f, 1.f);
      const CRelAngle angle = CRelAngle::FromDegrees(randomAngle + commonRand);
      const CQuaternion rot = CQuaternion::AxisAngle(delta, angle);
      x440_fractalOffsets[i] = rot.Transform(x430_fractalMags[i] * up);
    }
  }
  for (int i = 1; i < x420_calculatedVerts.size() - 1; ++i) {
    x420_calculatedVerts[i] += x440_fractalOffsets[i];
  }
  if (x1c_elecDesc->x70_ZERY) {
    for (int i = 0; i < x420_calculatedVerts.size(); ++i) {
      x420_calculatedVerts[i].SetY(0.f);
    }
  }
}

void CParticleElectric::CalculateFractal(int start, int end, float ampl, float ampd) {
  float mag;
  const float ratio =
      static_cast< float >(end - start) / static_cast< float >(x430_fractalMags.size());
  const float startMag = x430_fractalMags[start];
  const int mid = (start + end) / 2;
  mag = ratio * ampl;
  const float endMag = x430_fractalMags[end];
  const float base = (startMag + endMag) / 2.f + mag * x14c_randState.Float() - mag / 2.f;
  x430_fractalMags[mid] = base + (ampd * x14c_randState.Float() - ampd / 2.f);
  if ((start + end) % 2 == 1) {
    x430_fractalMags[end - 1] = x430_fractalMags[end];
  }
  if (mid - start > 1) {
    CalculateFractal(start, mid, ampl, ampd);
  }
  if (end - mid > 1) {
    CalculateFractal(mid, end, ampl, ampd);
  }
}

const bool CParticleElectric::Update(double dt) {
  CGlobalRandom random(x14c_randState);
  bool updated = false;
  if (x450_25_haveGPSM) {
    for (int i = 0; i < x154_SCNT; ++i) {
      if (!x400_gpsmGenerators[i]->IsSystemDeletable()) {
        break;
      }
    }
  }
  if (x450_26_haveEPSM) {
    for (int i = 0; i < x154_SCNT; ++i) {
      if (!x410_epsmGenerators[i]->IsSystemDeletable()) {
        break;
      }
    }
  }
  const bool emitting = x450_24_emitting && x28_currentFrame < x2c_LIFE;
  double evalTime = x28_currentFrame * (1.0 / 60.0);
  x30_curTime += dt;
  if (x450_29_transformDirty) {
    UpdateCachedTransform();
    const CTransform4f orientation = xf8_cachedXf.GetRotation();
    const CVector3f translation = xf8_cachedXf.GetTranslation();
    if (x450_27_haveSSWH) {
      for (AUTO(it, x3e8_electricManagers.begin()); it != x3e8_electricManagers.end(); ++it) {
        x1e0_swooshGenerators[it->x0_idx]->SetGlobalTranslation(translation);
        x1e0_swooshGenerators[it->x0_idx]->SetGlobalOrientation(orientation);
        x1e0_swooshGenerators[it->x0_idx]->SetGlobalScale(xe0_globalScale);
        x1e0_swooshGenerators[it->x0_idx]->SetLocalScale(xec_localScale);
      }
    }
    if (x450_25_haveGPSM) {
      for (int i = 0; i < x154_SCNT; ++i) {
        x400_gpsmGenerators[i]->SetGlobalTranslation(translation);
        x400_gpsmGenerators[i]->SetGlobalOrientation(orientation);
        x400_gpsmGenerators[i]->SetGlobalScale(xe0_globalScale);
        x400_gpsmGenerators[i]->SetLocalScale(xec_localScale);
      }
    }
    if (x450_26_haveEPSM) {
      for (int i = 0; i < x154_SCNT; ++i) {
        x410_epsmGenerators[i]->SetGlobalTranslation(translation);
        x410_epsmGenerators[i]->SetGlobalOrientation(orientation);
        x410_epsmGenerators[i]->SetGlobalScale(xe0_globalScale);
        x410_epsmGenerators[i]->SetLocalScale(xec_localScale);
      }
    }
    updated = true;
  }
  while (evalTime < x30_curTime) {
    CParticleGlobals::SetEmitterTime(x28_currentFrame);
    UpdateElectricalEffects();
    if (emitting) {
      AddElectricalEffects();
    }
    if (x450_25_haveGPSM) {
      if (x28_currentFrame >= x2c_LIFE) {
        for (int i = 0; i < x154_SCNT; ++i) {
          x400_gpsmGenerators[i]->EndLifetime();
        }
      }
      for (int i = 0; i < x154_SCNT; ++i) {
        x400_gpsmGenerators[i]->Update(1.0 / 60.0);
      }
    }
    if (x450_26_haveEPSM) {
      if (x28_currentFrame >= x2c_LIFE) {
        for (int i = 0; i < x154_SCNT; ++i) {
          x410_epsmGenerators[i]->EndLifetime();
        }
      }
      for (int i = 0; i < x154_SCNT; ++i) {
        x410_epsmGenerators[i]->Update(1.0 / 60.0);
      }
    }
    updated = true;
    evalTime += 1.0 / 60.0;
    ++x28_currentFrame;
  }
  if (updated) {
    BuildBounds();
  }
  return updated;
}

void CParticleElectric::Render() {
  const float startTime = CStopwatch::GetGlobalTime();
  if (!x3e8_electricManagers.empty()) {
    if (x450_29_transformDirty) {
      UpdateCachedTransform();
    }
    if (x450_27_haveSSWH) {
      RenderSwooshes();
    }
    if (x450_28_haveLWD) {
      RenderLines();
    }
  }
  if (x450_25_haveGPSM) {
    for (int i = 0; i < x154_SCNT; ++i) {
      x400_gpsmGenerators[i]->Render();
    }
  }
  if (x450_26_haveEPSM) {
    for (int i = 0; i < x154_SCNT; ++i) {
      x410_epsmGenerators[i]->Render();
    }
  }
  x12c_renderTime = CStopwatch::GetGlobalTime() - startTime;
}

void CParticleElectric::RenderSwooshes() {
  for (AUTO(it, x3e8_electricManagers.begin()); it != x3e8_electricManagers.end(); ++it) {
    x1e0_swooshGenerators[it->x0_idx]->Render();
  }
}

void CParticleElectric::RenderLines() {
  CGraphics::DisableAllLights();
  CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
  CGraphics::SetModelMatrix(CTransform4f::Translate(xa4_globalTranslation) * xb0_globalOrientation *
                            CTransform4f::Translate(x38_translation) * x44_orientation *
                            CTransform4f::Scale(xe0_globalScale) *
                            CTransform4f::Scale(xec_localScale));
  CGraphics::SetCullMode(kCM_None);
  SetupLineGXMaterial();
  for (AUTO(it, x3e8_electricManagers.begin()); it != x3e8_electricManagers.end(); ++it) {
    CLineManager& line = *x2e4_lineManagers[it->x0_idx];
    if (x1c_elecDesc->x28_LWD1) {
      DrawLineStrip(line.x0_verts, line.x10_width1, line.x1c_color1);
    }
    if (x1c_elecDesc->x2c_LWD2) {
      DrawLineStrip(line.x0_verts, line.x14_width2, line.x20_color2);
    }
    if (x1c_elecDesc->x30_LWD3) {
      DrawLineStrip(line.x0_verts, line.x18_width3, line.x24_color3);
    }
  }
  CGraphics::SetCullMode(kCM_Front);
  CGraphics::SetLineWidth(1.f, kTO_Zero);
}

void CParticleElectric::SetupLineGXMaterial() {
  static const GXVtxDescList vtxDesc[] = {{GX_VA_POS, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
  CGX::SetVtxDescv(vtxDesc);
  CGX::SetNumChans(1);
  CGX::SetNumTexGens(0);
  CGX::SetNumTevStages(1);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
}

void CParticleElectric::DrawLineStrip(
    const rstl::vector< CVector3f, rstl::aligned_allocator >& verts, float width,
    const CColor& color) {
  CGraphics::SetLineWidth(width, kTO_Zero);
  CGX::SetTevKColor(GX_KCOLOR0, CColor::Modulate(GetModulationColor(), color).GetGXColor());
  const int count = verts.size();
  CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, count);
  for (int i = 0; i < count; ++i) {
    const CVector3f& vert = verts[i];
    GXPosition3f32(vert[kDX], vert[kDY], vert[kDZ]);
  }
  CGX::End();
}

void CParticleElectric::AddElectricalEffects() {
  float rate = 0.f;
  if (x1c_elecDesc->x8_GRAT) {
    if (x1c_elecDesc->x8_GRAT->GetValue(x28_currentFrame, rate)) {
      x3e8_electricManagers.clear();
      for (int i = 0; i < x1bc_allocated.size(); ++i) {
        x1bc_allocated[i] = false;
      }
      return;
    } else {
      rate = rstl::max_val(0.f, rate);
    }
  }
  x15c_genRem += rate;
  const int count = static_cast< int >(floor(x15c_genRem));
  x15c_genRem -= count;
  CreateNewParticles(count);
}

void CParticleElectric::CreateNewParticles(int count) {
  int allocIdx = 0;
  for (int i = 0; i < count; ++i) {
    if (x3e8_electricManagers.size() >= x154_SCNT) {
      return;
    }
    const CTransform4f orientation = xf8_cachedXf.GetRotation();
    const CVector3f translation = xf8_cachedXf.GetTranslation();
    for (; allocIdx < x1bc_allocated.size(); ++allocIdx) {
      if (x1bc_allocated[allocIdx]) {
        continue;
      }
      x1bc_allocated[allocIdx] = true;
      int lifetime = 1;
      if (x1c_elecDesc->x4_SLIF) {
        x1c_elecDesc->x4_SLIF->GetValue(x28_currentFrame, lifetime);
      }
      x3e8_electricManagers.push_back(
          CParticleElectricManager(allocIdx, lifetime, x28_currentFrame));
      CParticleElectricManager& manager = x3e8_electricManagers.back();
      int frame = x28_currentFrame - manager.x8_startFrame;
      CParticleGlobals::SetParticleLifetime(manager.xc_endFrame - manager.x8_startFrame);
      CParticleGlobals::UpdateParticleLifetimeTweenValues(frame);
      CalculatePoints();
      if (x450_27_haveSSWH) {
        CParticleSwoosh& swoosh = *x1e0_swooshGenerators[allocIdx];
        swoosh.SetParticleEmission(true);
        swoosh.SetGlobalTranslation(translation);
        swoosh.SetGlobalOrientation(orientation);
        swoosh.SetGlobalScale(xe0_globalScale);
        swoosh.SetLocalScale(xec_localScale);
        CColor color = CColor::White();
        if (x1c_elecDesc->x14_COLR) {
          x1c_elecDesc->x14_COLR->GetValue(frame, color);
        }
        swoosh.SetModulationColor(CColor::Modulate(color, x1b8_moduColor));
        int curParticle = swoosh.x158_curParticle;
        for (int j = 0; j < swoosh.x15c_swooshes.size(); ++j) {
          curParticle = (curParticle + 1) % swoosh.x15c_swooshes.size();
          CParticleSwoosh::SSwooshData& data = swoosh.x15c_swooshes[curParticle];
          data.mTranslation = x420_calculatedVerts[j];
        }
      }
      if (x450_28_haveLWD) {
        CLineManager& line = *x2e4_lineManagers[allocIdx];
        line.x0_verts = x420_calculatedVerts;
        UpdateLine(allocIdx, 0);
        if (!x450_27_haveSSWH) {
          x130_minBounds = CVector3f(FLT_MAX, FLT_MAX, FLT_MAX);
          x13c_maxBounds = CVector3f(-FLT_MAX, -FLT_MAX, -FLT_MAX);
          for (int j = 0; j < x420_calculatedVerts.size(); ++j) {
            const CVector3f& vert = x420_calculatedVerts[j];
            x13c_maxBounds = CVector3f(rstl::max_val(vert.GetX(), x13c_maxBounds.GetX()),
                                       rstl::max_val(vert.GetY(), x13c_maxBounds.GetY()),
                                       rstl::max_val(vert.GetZ(), x13c_maxBounds.GetZ()));
            x130_minBounds = CVector3f(rstl::min_val(vert.GetX(), x130_minBounds.GetX()),
                                       rstl::min_val(vert.GetY(), x130_minBounds.GetY()),
                                       rstl::min_val(vert.GetZ(), x130_minBounds.GetZ()));
          }
          line.x28_bounds = CAABox(x130_minBounds, x13c_maxBounds);
        }
      }
      if (x450_25_haveGPSM) {
        for (int j = 0; j < x154_SCNT; ++j) {
          if (!x400_gpsmGenerators[j]->GetParticleEmission()) {
            const CTransform4f scale =
                CTransform4f::Scale(xe0_globalScale) * CTransform4f::Scale(xec_localScale);
            x400_gpsmGenerators[j]->SetTranslation(scale * x420_calculatedVerts.front());
            x400_gpsmGenerators[j]->SetParticleEmission(true);
            manager.x10_gpsmIdx = j;
            break;
          }
        }
      }
      if (x450_26_haveEPSM) {
        for (int j = 0; j < x154_SCNT; ++j) {
          if (!x410_epsmGenerators[j]->GetParticleEmission()) {
            const CTransform4f scale =
                CTransform4f::Scale(xe0_globalScale) * CTransform4f::Scale(xec_localScale);
            x410_epsmGenerators[j]->SetTranslation(scale * x420_calculatedVerts.back());
            x410_epsmGenerators[j]->SetParticleEmission(true);
            manager.x14_epsmIdx = j;
            break;
          }
        }
      }
      break;
    }
  }
}

void CParticleElectric::UpdateElectricalEffects() {
  for (AUTO(it, x3e8_electricManagers.begin()); it != x3e8_electricManagers.end();) {
    CParticleElectricManager& manager = *it;
    if (manager.x4_slif <= 1) {
      x1bc_allocated[manager.x0_idx] = false;
      if (manager.x10_gpsmIdx != -1) {
        x400_gpsmGenerators[manager.x10_gpsmIdx]->SetParticleEmission(false);
      }
      if (manager.x14_epsmIdx != -1) {
        x410_epsmGenerators[manager.x14_epsmIdx]->SetParticleEmission(false);
      }
      it = x3e8_electricManagers.erase(it);
      continue;
    }
    int frame = x28_currentFrame - manager.x8_startFrame;
    CParticleGlobals::SetParticleLifetime(manager.xc_endFrame - manager.x8_startFrame);
    CParticleGlobals::UpdateParticleLifetimeTweenValues(frame);
    if (x450_27_haveSSWH) {
      CParticleSwoosh& swoosh = *x1e0_swooshGenerators[manager.x0_idx];
      CColor color = CColor::White();
      if (x1c_elecDesc->x14_COLR) {
        x1c_elecDesc->x14_COLR->GetValue(frame, color);
      }
      swoosh.SetModulationColor(CColor::Modulate(color, x1b8_moduColor));
    }
    if (x450_28_haveLWD) {
      UpdateLine(manager.x0_idx, frame);
    }
    --manager.x4_slif;
    ++it;
  }
}

void CParticleElectric::UpdateLine(int idx, int frame) {
  CColorElement* element = x1c_elecDesc->x34_LCL1;
  CLineManager& line = *x2e4_lineManagers[idx];
  if (element) {
    element->GetValue(frame, line.x1c_color1);
  }
  if (CColorElement* element = x1c_elecDesc->x38_LCL2) {
    element->GetValue(frame, line.x20_color2);
  }
  if (CColorElement* element = x1c_elecDesc->x3c_LCL3) {
    element->GetValue(frame, line.x24_color3);
  }
  if (CRealElement* element = x1c_elecDesc->x28_LWD1) {
    element->GetValue(frame, line.x10_width1);
  }
  if (CRealElement* element = x1c_elecDesc->x2c_LWD2) {
    element->GetValue(frame, line.x14_width2);
  }
  if (CRealElement* element = x1c_elecDesc->x30_LWD3) {
    element->GetValue(frame, line.x18_width3);
  }
}

void CParticleElectric::BuildBounds() {
  if (GetParticleCount() <= 0) {
    x160_systemBounds = CAABox::MakeMaxInvertedBox();
    return;
  }
  x160_systemBounds = CAABox::MakeMaxInvertedBox();
  if (x450_27_haveSSWH) {
    for (AUTO(it, x3e8_electricManagers.begin()); it != x3e8_electricManagers.end(); ++it) {
      rstl::optional_object< CAABox > bounds = x1e0_swooshGenerators[it->x0_idx]->GetBounds();
      if (bounds) {
        x160_systemBounds.Include(*bounds);
      }
    }
  } else if (x450_28_haveLWD) {
    CAABox bounds = CAABox::MakeMaxInvertedBox();
    for (AUTO(it, x3e8_electricManagers.begin()); it != x3e8_electricManagers.end(); ++it) {
      bounds.Include(x2e4_lineManagers[it->x0_idx]->x28_bounds);
    }
    if (!bounds.Invalid()) {
      x160_systemBounds.Include(bounds.GetTransformedAABox(
          CTransform4f::Translate(xa4_globalTranslation) * xb0_globalOrientation *
          CTransform4f::Translate(x38_translation) * x44_orientation *
          CTransform4f::Scale(xe0_globalScale)));
    }
  }
  if (x450_25_haveGPSM) {
    for (int i = 0; i < x154_SCNT; ++i) {
      rstl::optional_object< CAABox > bounds = x400_gpsmGenerators[i]->GetBounds();
      if (bounds) {
        x160_systemBounds.Include(*bounds);
      }
    }
  }
  if (x450_26_haveEPSM) {
    for (int i = 0; i < x154_SCNT; ++i) {
      rstl::optional_object< CAABox > bounds = x410_epsmGenerators[i]->GetBounds();
      if (bounds) {
        x160_systemBounds.Include(*bounds);
      }
    }
  }
}

rstl::optional_object< CAABox > CParticleElectric::GetBounds() const {
  if (GetParticleCount() <= 0) {
    return rstl::optional_object_null();
  }
  return x160_systemBounds;
}

void CParticleElectric::SetModulationColor(const CColor& color) { x1b8_moduColor = color; }

bool CParticleElectric::IsSystemDeletable() const {
  if (x450_24_emitting && x28_currentFrame < x2c_LIFE) {
    return false;
  }
  if (!x3e8_electricManagers.empty()) {
    return false;
  }
  if (x450_25_haveGPSM) {
    for (int i = 0; i < x154_SCNT; ++i) {
      if (!x400_gpsmGenerators[i]->IsSystemDeletable()) {
        return false;
      }
    }
  }
  if (x450_26_haveEPSM) {
    for (int i = 0; i < x154_SCNT; ++i) {
      if (!x410_epsmGenerators[i]->IsSystemDeletable()) {
        return false;
      }
    }
  }
  return true;
}

int CParticleElectric::GetParticleCount() const {
  int count = 0;
  if (!x3e8_electricManagers.empty()) {
    for (AUTO(it, x3e8_electricManagers.begin()); it != x3e8_electricManagers.end(); ++it) {
      if (x450_27_haveSSWH) {
        count += x1e0_swooshGenerators[it->x0_idx]->GetParticleCount();
      }
      if (x450_28_haveLWD) {
        count += x150_SSEG;
      }
    }
  }
  if (x450_25_haveGPSM) {
    for (int i = 0; i < x154_SCNT; ++i) {
      count += x400_gpsmGenerators[i]->GetParticleCount();
    }
  }
  if (x450_26_haveEPSM) {
    for (int i = 0; i < x154_SCNT; ++i) {
      count += x410_epsmGenerators[i]->GetParticleCount();
    }
  }
  return count;
}

void CParticleElectric::ForceParticleCreation(int count) {
  CGlobalRandom random(x14c_randState);
  CreateNewParticles(count);
}

void CParticleElectric::SetTranslation(const CVector3f& translation) {
  x38_translation = translation;
  x450_29_transformDirty = true;
}

void CParticleElectric::SetOrientation(const CTransform4f& orientation) {
  x44_orientation = orientation;
  x74_invOrientation = x44_orientation.GetQuickInverse();
  x450_29_transformDirty = true;
}

void CParticleElectric::SetGlobalTranslation(const CVector3f& translation) {
  xa4_globalTranslation = translation;
  x450_29_transformDirty = true;
  if (x450_27_haveSSWH) {
    for (AUTO(it, x3e8_electricManagers.begin()); it != x3e8_electricManagers.end(); ++it) {
      x1e0_swooshGenerators[it->x0_idx]->SetGlobalTranslation(xa4_globalTranslation);
    }
  }
  if (x450_25_haveGPSM) {
    for (int i = 0; i < x154_SCNT; ++i) {
      x400_gpsmGenerators[i]->SetGlobalTranslation(xa4_globalTranslation);
    }
  }
  if (x450_26_haveEPSM) {
    for (int i = 0; i < x154_SCNT; ++i) {
      x410_epsmGenerators[i]->SetGlobalTranslation(xa4_globalTranslation);
    }
  }
}

void CParticleElectric::SetGlobalOrientation(const CTransform4f& orientation) {
  xb0_globalOrientation = orientation.GetRotation();
  x450_29_transformDirty = true;
  if (x450_27_haveSSWH) {
    for (AUTO(it, x3e8_electricManagers.begin()); it != x3e8_electricManagers.end(); ++it) {
      x1e0_swooshGenerators[it->x0_idx]->SetGlobalOrientation(xb0_globalOrientation);
    }
  }
  if (x450_25_haveGPSM) {
    for (int i = 0; i < x154_SCNT; ++i) {
      x400_gpsmGenerators[i]->SetGlobalOrientation(xb0_globalOrientation);
    }
  }
  if (x450_26_haveEPSM) {
    for (int i = 0; i < x154_SCNT; ++i) {
      x410_epsmGenerators[i]->SetGlobalOrientation(xb0_globalOrientation);
    }
  }
}

void CParticleElectric::SetGlobalScale(const CVector3f& scale) {
  xe0_globalScale = scale;
  x450_29_transformDirty = true;
}

void CParticleElectric::SetLocalScale(const CVector3f& scale) {
  xec_localScale = scale;
  x450_29_transformDirty = true;
  if (x450_27_haveSSWH) {
    for (AUTO(it, x3e8_electricManagers.begin()); it != x3e8_electricManagers.end(); ++it) {
      x1e0_swooshGenerators[it->x0_idx]->SetLocalScale(xec_localScale);
    }
  }
  if (x450_25_haveGPSM) {
    for (int i = 0; i < x154_SCNT; ++i) {
      x400_gpsmGenerators[i]->SetLocalScale(xec_localScale);
    }
  }
  if (x450_26_haveEPSM) {
    for (int i = 0; i < x154_SCNT; ++i) {
      x410_epsmGenerators[i]->SetLocalScale(xec_localScale);
    }
  }
}

void CParticleElectric::UpdateCachedTransform() {
  xf8_cachedXf = CTransform4f::Translate(xa4_globalTranslation) * xb0_globalOrientation *
                 CTransform4f::Translate(x38_translation) * x44_orientation;
  x450_29_transformDirty = false;
}

bool CParticleElectric::SystemHasLight() const {
  if (x450_25_haveGPSM) {
    return x400_gpsmGenerators.front()->SystemHasLight();
  }
  if (x450_26_haveEPSM) {
    return x410_epsmGenerators.front()->SystemHasLight();
  }
  return false;
}

CLight CParticleElectric::GetLight() const {
  if (x450_25_haveGPSM) {
    return x400_gpsmGenerators.front()->GetLight();
  }
  if (x450_26_haveEPSM) {
    return x410_epsmGenerators.front()->GetLight();
  }
  return CLight::BuildLocalAmbient(GetGlobalTranslation(), CColor::Orange());
}

void CParticleElectric::SetParticleEmission(const bool emission) { x450_24_emitting = emission; }
uint CParticleElectric::Get4CharId() const { return 'ELSC'; }

void CParticleElectric::DestroyParticles() {}

const CTransform4f& CParticleElectric::GetGlobalOrientation() const {
  return xb0_globalOrientation;
}
const CVector3f& CParticleElectric::GetGlobalTranslation() const { return xa4_globalTranslation; }
const CTransform4f& CParticleElectric::GetOrientation() const { return x44_orientation; }
const CVector3f& CParticleElectric::GetTranslation() const { return x38_translation; }
