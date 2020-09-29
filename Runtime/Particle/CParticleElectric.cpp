#include "Runtime/Particle/CParticleElectric.hpp"

#include <algorithm>

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/Particle/CElectricDescription.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CParticleGlobals.hpp"
#include "Runtime/Particle/CParticleSwoosh.hpp"
#include "Runtime/Particle/CSwooshDescription.hpp"

#include <zeus/CQuaternion.hpp>
#include <zeus/CRelAngle.hpp>

namespace urde {

u16 CParticleElectric::g_GlobalSeed = 99;

CParticleElectric::CParticleElectric(const TToken<CElectricDescription>& token)
: x1c_elecDesc(token)
, x14c_randState(g_GlobalSeed++) {
  x1bc_allocated.resize(32);

  CElectricDescription* desc = x1c_elecDesc.GetObj();

  if (CIntElement* sseg = desc->x10_SSEG.get()) {
    sseg->GetValue(x28_currentFrame, x150_SSEG);
  }

  if (CIntElement* scnt = desc->xc_SCNT.get()) {
    scnt->GetValue(x28_currentFrame, x154_SCNT);
  }

  x154_SCNT = std::min(x154_SCNT, 32);

  if (CIntElement* life = desc->x0_LIFE.get()) {
    life->GetValue(0, x2c_LIFE);
  } else {
    x2c_LIFE = INT_MAX;
  }

  if (desc->x40_SSWH) {
    x450_27_haveSSWH = true;
    for (int i = 0; i < x154_SCNT; ++i) {
      x1e0_swooshGenerators.emplace_back(std::make_unique<CParticleSwoosh>(desc->x40_SSWH.m_token, x150_SSEG));
      x1e0_swooshGenerators.back()->DoElectricWarmup();
    }
  }

  ++x150_SSEG;
  x420_calculatedVerts.resize(x150_SSEG);
  x440_fractalOffsets.resize(x150_SSEG);
  x430_fractalMags.resize(x150_SSEG);

  if (desc->x50_GPSM) {
    x450_25_haveGPSM = true;
    x400_gpsmGenerators.reserve(x154_SCNT);
    for (int i = 0; i < x154_SCNT; ++i) {
      x400_gpsmGenerators.emplace_back(std::make_unique<CElementGen>(desc->x50_GPSM.m_token));
      x400_gpsmGenerators.back()->SetParticleEmission(false);
    }
  }

  if (desc->x60_EPSM) {
    x450_26_haveEPSM = true;
    x410_epsmGenerators.reserve(x154_SCNT);
    for (int i = 0; i < x154_SCNT; ++i) {
      x410_epsmGenerators.emplace_back(std::make_unique<CElementGen>(desc->x60_EPSM.m_token));
      x410_epsmGenerators.back()->SetParticleEmission(false);
    }
  }

  if (x1c_elecDesc->x28_LWD1 || x1c_elecDesc->x2c_LWD2 || x1c_elecDesc->x30_LWD3) {
    x450_28_haveLWD = true;
    for (int i = 0; i < x154_SCNT; ++i) {
      x2e4_lineManagers.emplace_back(std::make_unique<CLineManager>());
    }
  }
}

void CParticleElectric::RenderSwooshes() {
  for (const CParticleElectricManager& elec : x3e8_electricManagers) {
    x1e0_swooshGenerators[elec.x0_idx]->Render();
  }
}

void CParticleElectric::SetupLineGXMaterial() {
  // Konst color/alpha 0
}

void CParticleElectric::DrawLineStrip(const std::vector<zeus::CVector3f>& verts, float width,
                                      const zeus::CColor& color) {
  const size_t useIdx = m_nextLineRenderer;
  if (++m_nextLineRenderer > m_lineRenderers.size()) {
    m_lineRenderers.resize(m_nextLineRenderer);
  }
  if (!m_lineRenderers[useIdx]) {
    m_lineRenderers[useIdx] =
        std::make_unique<CLineRenderer>(CLineRenderer::EPrimitiveMode::LineStrip, x150_SSEG, nullptr, true, true);
  }
  CLineRenderer& renderer = *m_lineRenderers[useIdx];
  const zeus::CColor useColor = x1b8_moduColor * color;

  renderer.Reset();
  for (const zeus::CVector3f& vert : verts) {
    renderer.AddVertex(vert, useColor, width);
  }
  renderer.Render(g_Renderer->IsThermalVisorHotPass());
}

void CParticleElectric::RenderLines() {
  m_nextLineRenderer = 0;
  CGraphics::DisableAllLights();
  // Z-test, no write
  // Additive blend

  CGraphics::SetModelMatrix(zeus::CTransform::Translate(xa4_globalTranslation) * xb0_globalOrientation *
                            zeus::CTransform::Translate(x38_translation) * x44_orientation *
                            zeus::CTransform::Scale(xe0_globalScale) * zeus::CTransform::Scale(xec_localScale));
  // Disable culling
  SetupLineGXMaterial();
  for (CParticleElectricManager& elec : x3e8_electricManagers) {
    CLineManager& line = *x2e4_lineManagers[elec.x0_idx];
    if (x1c_elecDesc->x28_LWD1) {
      DrawLineStrip(line.x0_verts, line.x10_widths[0], line.x1c_colors[0]);
    }
    if (x1c_elecDesc->x2c_LWD2) {
      DrawLineStrip(line.x0_verts, line.x10_widths[1], line.x1c_colors[1]);
    }
    if (x1c_elecDesc->x30_LWD3) {
      DrawLineStrip(line.x0_verts, line.x10_widths[2], line.x1c_colors[2]);
    }
  }

  // Enable culling
  // Line Width 1
}

void CParticleElectric::UpdateCachedTransform() {
  xf8_cachedXf = zeus::CTransform::Translate(xa4_globalTranslation) * xb0_globalOrientation *
                 zeus::CTransform::Translate(x38_translation) * x44_orientation;
  x450_29_transformDirty = false;
}

void CParticleElectric::UpdateLine(size_t idx, int frame) {
  CLineManager& line = *x2e4_lineManagers[idx];

  if (CColorElement* lcl1 = x1c_elecDesc->x34_LCL1.get()) {
    lcl1->GetValue(frame, line.x1c_colors[0]);
  }
  if (CColorElement* lcl2 = x1c_elecDesc->x38_LCL2.get()) {
    lcl2->GetValue(frame, line.x1c_colors[1]);
  }
  if (CColorElement* lcl3 = x1c_elecDesc->x3c_LCL3.get()) {
    lcl3->GetValue(frame, line.x1c_colors[2]);
  }

  if (CRealElement* lwd1 = x1c_elecDesc->x28_LWD1.get()) {
    lwd1->GetValue(frame, line.x10_widths[0]);
  }
  if (CRealElement* lwd2 = x1c_elecDesc->x2c_LWD2.get()) {
    lwd2->GetValue(frame, line.x10_widths[1]);
  }
  if (CRealElement* lwd3 = x1c_elecDesc->x30_LWD3.get()) {
    lwd3->GetValue(frame, line.x10_widths[2]);
  }
}

void CParticleElectric::UpdateElectricalEffects() {
  for (auto it = x3e8_electricManagers.begin(); it != x3e8_electricManagers.end();) {
    CParticleElectricManager& elec = *it;
    if (elec.x4_slif <= 1) {
      x1bc_allocated[elec.x0_idx] = false;
      if (elec.x10_gpsmIdx != -1) {
        x400_gpsmGenerators[elec.x10_gpsmIdx]->SetParticleEmission(false);
      }
      if (elec.x14_epsmIdx != -1) {
        x410_epsmGenerators[elec.x14_epsmIdx]->SetParticleEmission(false);
      }
      it = x3e8_electricManagers.erase(it);
      continue;
    }

    CParticleGlobals::instance()->SetParticleLifetime(int(elec.xc_endFrame - elec.x8_startFrame));
    const int frame = x28_currentFrame - int(elec.x8_startFrame);
    CParticleGlobals::instance()->UpdateParticleLifetimeTweenValues(frame);

    if (x450_27_haveSSWH) {
      CParticleSwoosh& swoosh = *x1e0_swooshGenerators[elec.x0_idx];
      zeus::CColor color = zeus::skWhite;
      if (CColorElement* colr = x1c_elecDesc->x14_COLR.get()) {
        colr->GetValue(frame, color);
      }
      swoosh.SetModulationColor(color * x1b8_moduColor);
    }

    if (x450_28_haveLWD) {
      UpdateLine(elec.x0_idx, frame);
    }

    elec.x4_slif -= 1;
    ++it;
  }
}

void CParticleElectric::CalculateFractal(int start, int end, float ampl, float ampd) {
  const float tmp = float(end - start) / float(x430_fractalMags.size()) * ampl;
  const int storeIdx = (start + end) / 2;
  x430_fractalMags[storeIdx] = (x430_fractalMags[start] + x430_fractalMags[end]) * 0.5f + tmp * x14c_randState.Float() -
                               tmp * 0.5f + ampd * x14c_randState.Float() - ampd * 0.5f;
  if (((start + end) & 1) != 0) {
    x430_fractalMags[end - 1] = x430_fractalMags[end];
  }

  if (storeIdx - start > 1) {
    CalculateFractal(start, storeIdx, ampl, ampd);
  }
  if (end - storeIdx > 1) {
    CalculateFractal(storeIdx, end, ampl, ampd);
  }
}

void CParticleElectric::CalculatePoints() {
  zeus::CVector3f pos, vel;
  if (CEmitterElement* iemt = x1c_elecDesc->x18_IEMT.get()) {
    iemt->GetValue(x28_currentFrame, pos, vel);
  }

  if (x178_overrideIPos) {
    pos = *x178_overrideIPos;
  }
  if (x188_overrideIVel) {
    vel = *x188_overrideIVel;
  }

  rstl::reserved_vector<zeus::CVector3f, 4> points;

  if (!vel.isZero()) {
    points.push_back(pos);
    points.push_back(pos + vel);
    points.push_back(pos + vel * 2.f);
  } else {
    points.push_back(pos);
  }

  zeus::CVector3f fpos = zeus::skForward;
  zeus::CVector3f fvel;
  if (CEmitterElement* femt = x1c_elecDesc->x1c_FEMT.get()) {
    femt->GetValue(x28_currentFrame, fpos, fvel);
  }

  if (x198_overrideFPos) {
    fpos = *x198_overrideFPos;
  }
  if (x1a8_overrideFVel) {
    fvel = *x1a8_overrideFVel;
  }

  if (!fvel.isZero()) {
    if (points.size() == 3) {
      points.push_back(fpos);
      points[2] = fpos + fvel;
    } else {
      points.push_back(fpos + fvel * 2.f);
      points.push_back(fpos + fvel);
      points.push_back(fpos);
    }
  } else {
    points.push_back(fpos);
  }

  if (points.size() == 4) {
    x420_calculatedVerts[0] = points[0];
    const int segs = x150_SSEG - 1;
    const float segDiv = 1.f / float(segs);
    float curDiv = segDiv;
    for (int i = 1; i < segs; ++i) {
      const float t = segDiv * x14c_randState.Range(-0.45f, 0.45f) + curDiv;
      x420_calculatedVerts[i] = zeus::getBezierPoint(points[0], points[1], points[2], points[3], t);
      curDiv += segDiv;
    }
    x420_calculatedVerts[segs] = points[3];
  } else {
    x420_calculatedVerts[0] = pos;
    const int segs = x150_SSEG - 1;
    const float segDiv = 1.f / float(segs);
    zeus::CVector3f accum = x420_calculatedVerts[0];
    const zeus::CVector3f segDelta = (fpos - pos) * segDiv;
    for (int i = 1; i < segs; ++i) {
      float r = x14c_randState.Range(-0.45f, 0.45f);
      x420_calculatedVerts[i] = segDelta * r + accum;
      accum += segDelta;
    }
    x420_calculatedVerts[segs] = fpos;
  }

  for (int i = 0; i < x150_SSEG; ++i) {
    x430_fractalMags[i] = 0.f;
  }

  float amplVal = 1.f;
  if (CRealElement* ampl = x1c_elecDesc->x20_AMPL.get()) {
    ampl->GetValue(x28_currentFrame, amplVal);
    amplVal *= 2.f;
  }

  float ampdVal = 0.f;
  if (CRealElement* ampd = x1c_elecDesc->x24_AMPD.get()) {
    ampd->GetValue(x28_currentFrame, ampdVal);
  }

  CalculateFractal(0, x420_calculatedVerts.size() - 1, amplVal, ampdVal);

  zeus::CVector3f v0 = x420_calculatedVerts[0] - x420_calculatedVerts[1];
  zeus::CVector3f v1 = x420_calculatedVerts[x420_calculatedVerts.size() - 1] - x420_calculatedVerts[1];
  zeus::CVector3f upVec = zeus::skUp;
  if (v0.canBeNormalized() && v1.canBeNormalized()) {
    v0.normalize();
    v1.normalize();
    float dot = v0.dot(v1);
    if (dot < 0) {
      dot = -dot;
    }
    if (std::fabs(dot - 1.f) < 0.00001f) {
      upVec = zeus::lookAt(x420_calculatedVerts[0], x420_calculatedVerts[1]).basis[2];
    } else {
      upVec = v0.cross(v1).normalized();
    }
  } else if (x420_calculatedVerts[0] != x420_calculatedVerts[1]) {
    upVec = zeus::lookAt(x420_calculatedVerts[0], x420_calculatedVerts[1]).basis[2];
  }

  const float commonRand = x14c_randState.Range(0.f, 360.f);

  for (size_t i = 1; i < x420_calculatedVerts.size() - 1; ++i) {
    const zeus::CVector3f delta = x420_calculatedVerts[i] - x420_calculatedVerts[i - 1];
    if (!delta.isZero()) {
      const zeus::CRelAngle angle =
          zeus::degToRad(x430_fractalMags[i] / amplVal * 16.f * x14c_randState.Range(-1.f, 1.f) + commonRand);
      x440_fractalOffsets[i] = zeus::CQuaternion::fromAxisAngle(delta, angle).transform(x430_fractalMags[i] * upVec);
    }
  }

  for (size_t i = 1; i < x420_calculatedVerts.size() - 1; ++i) {
    x420_calculatedVerts[i] += x440_fractalOffsets[i];
  }

  if (x1c_elecDesc->x70_ZERY) {
    for (auto& calculatedVert : x420_calculatedVerts) {
      calculatedVert.y() = 0.f;
    }
  }
}

void CParticleElectric::CreateNewParticles(int count) {
  size_t allocIdx = 0;

  for (int i = 0; i < count; ++i) {
    if (x3e8_electricManagers.size() < size_t(x154_SCNT)) {
      const zeus::CTransform cachedRot = xf8_cachedXf.getRotation();

      const size_t toAdd = x1bc_allocated.size() - allocIdx;
      for (size_t j = 0; j < toAdd; ++j, ++allocIdx) {
        if (x1bc_allocated[allocIdx]) {
          continue;
        }
        x1bc_allocated[allocIdx] = true;

        int lifetime = 1;
        if (CIntElement* slif = x1c_elecDesc->x4_SLIF.get()) {
          slif->GetValue(x28_currentFrame, lifetime);
        }

        x3e8_electricManagers.emplace_back(allocIdx, lifetime, x28_currentFrame);
        CParticleElectricManager& elec = x3e8_electricManagers.back();
        CParticleGlobals::instance()->SetParticleLifetime(elec.xc_endFrame - elec.x8_startFrame);
        const int frame = x28_currentFrame - int(elec.x8_startFrame);
        CParticleGlobals::instance()->UpdateParticleLifetimeTweenValues(frame);
        CalculatePoints();

        if (x450_27_haveSSWH) {
          CParticleSwoosh& swoosh = *x1e0_swooshGenerators[allocIdx];
          swoosh.SetParticleEmission(true);
          swoosh.SetGlobalTranslation(xf8_cachedXf.origin);
          swoosh.SetGlobalOrientation(cachedRot);
          swoosh.SetGlobalScale(xe0_globalScale);
          swoosh.SetLocalScale(xec_localScale);
          zeus::CColor color = zeus::skWhite;
          if (CColorElement* colr = x1c_elecDesc->x14_COLR.get()) {
            colr->GetValue(frame, color);
          }
          swoosh.SetModulationColor(color * x1b8_moduColor);
          swoosh.DoElectricCreate(x420_calculatedVerts);
        }

        if (x450_28_haveLWD) {
          CLineManager& line = *x2e4_lineManagers[allocIdx];
          line.x0_verts = x420_calculatedVerts;
          UpdateLine(allocIdx, 0);
          if (x450_27_haveSSWH) {
            x130_buildBounds = zeus::CAABox();
            for (const zeus::CVector3f& vec : x420_calculatedVerts) {
              x130_buildBounds.accumulateBounds(vec);
            }
            line.x28_aabb = x130_buildBounds;
          }
        }

        if (x450_25_haveGPSM) {
          for (int k = 0; k < x154_SCNT; ++k) {
            CElementGen& gen = *x400_gpsmGenerators[k];
            if (!gen.GetParticleEmission()) {
              const zeus::CTransform scale =
                  zeus::CTransform::Scale(xe0_globalScale) * zeus::CTransform::Scale(xec_localScale);
              gen.SetTranslation(scale * x420_calculatedVerts.front());
              gen.SetParticleEmission(true);
              elec.x10_gpsmIdx = k;
              break;
            }
          }
        }

        if (x450_26_haveEPSM) {
          for (int k = 0; k < x154_SCNT; ++k) {
            CElementGen& gen = *x410_epsmGenerators[k];
            if (!gen.GetParticleEmission()) {
              const zeus::CTransform scale =
                  zeus::CTransform::Scale(xe0_globalScale) * zeus::CTransform::Scale(xec_localScale);
              gen.SetTranslation(scale * x420_calculatedVerts.back());
              gen.SetParticleEmission(true);
              elec.x14_epsmIdx = k;
              break;
            }
          }
        }

        break;
      }
    }
  }
}

void CParticleElectric::AddElectricalEffects() {
  float genRate = 0.f;
  if (CRealElement* grat = x1c_elecDesc->x8_GRAT.get()) {
    if (grat->GetValue(x28_currentFrame, genRate)) {
      x3e8_electricManagers.clear();
      std::fill(x1bc_allocated.begin(), x1bc_allocated.end(), false);
      return;
    } else {
      genRate = std::max(0.f, genRate);
    }
  }

  x15c_genRem += genRate;
  const float partCount = std::floor(x15c_genRem);
  x15c_genRem -= partCount;
  CreateNewParticles(int(partCount));
}

void CParticleElectric::BuildBounds() {
  if (GetParticleCount() <= 0) {
    x160_systemBounds = zeus::CAABox();
    return;
  }

  x160_systemBounds = zeus::CAABox();

  if (x450_27_haveSSWH) {
    for (const CParticleElectricManager& elec : x3e8_electricManagers) {
      const CParticleSwoosh& swoosh = *x1e0_swooshGenerators[elec.x0_idx];
      if (const auto bounds = swoosh.GetBounds()) {
        x160_systemBounds.accumulateBounds(*bounds);
      }
    }
  } else if (x450_28_haveLWD) {
    zeus::CAABox tmp = zeus::CAABox();
    for (const CParticleElectricManager& elec : x3e8_electricManagers) {
      CLineManager& line = *x2e4_lineManagers[elec.x0_idx];
      tmp.accumulateBounds(line.x28_aabb);
    }
    if (!tmp.invalid()) {
      x160_systemBounds.accumulateBounds(tmp.getTransformedAABox(
          zeus::CTransform::Translate(xa4_globalTranslation) * xb0_globalOrientation *
          zeus::CTransform::Translate(x38_translation) * x44_orientation * zeus::CTransform::Scale(xe0_globalScale)));
    }
  }

  if (x450_25_haveGPSM) {
    for (int i = 0; i < x154_SCNT; ++i) {
      if (auto bounds = x400_gpsmGenerators[i]->GetBounds()) {
        x160_systemBounds.accumulateBounds(*bounds);
      }
    }
  }

  if (x450_26_haveEPSM) {
    for (int i = 0; i < x154_SCNT; ++i) {
      if (auto bounds = x410_epsmGenerators[i]->GetBounds()) {
        x160_systemBounds.accumulateBounds(*bounds);
      }
    }
  }
}

bool CParticleElectric::Update(double dt) {
  [[maybe_unused]] CGlobalRandom gr(x14c_randState);
  bool ret = false;

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

  double evalTime = x28_currentFrame / 60.0;
  x30_curTime += dt;

  if (x450_29_transformDirty) {
    UpdateCachedTransform();
    const zeus::CTransform globalOrient = xf8_cachedXf.getRotation();
    if (x450_27_haveSSWH) {
      for (const CParticleElectricManager& elec : x3e8_electricManagers) {
        CParticleSwoosh& swoosh = *x1e0_swooshGenerators[elec.x0_idx];
        swoosh.SetGlobalTranslation(xf8_cachedXf.origin);
        swoosh.SetGlobalOrientation(globalOrient);
        swoosh.SetGlobalScale(xe0_globalScale);
        swoosh.SetLocalScale(xec_localScale);
      }
    }

    if (x450_25_haveGPSM) {
      for (const CParticleElectricManager& elec : x3e8_electricManagers) {
        CElementGen& gen = *x400_gpsmGenerators[elec.x0_idx];
        gen.SetGlobalTranslation(xf8_cachedXf.origin);
        gen.SetGlobalOrientation(globalOrient);
        gen.SetGlobalScale(xe0_globalScale);
        gen.SetLocalScale(xec_localScale);
      }
    }

    if (x450_26_haveEPSM) {
      for (const CParticleElectricManager& elec : x3e8_electricManagers) {
        CElementGen& gen = *x410_epsmGenerators[elec.x0_idx];
        gen.SetGlobalTranslation(xf8_cachedXf.origin);
        gen.SetGlobalOrientation(globalOrient);
        gen.SetGlobalScale(xe0_globalScale);
        gen.SetLocalScale(xec_localScale);
      }
    }

    ret = true;
  }

  while (evalTime < x30_curTime) {
    CParticleGlobals::instance()->SetEmitterTime(x28_currentFrame);
    UpdateElectricalEffects();
    if (emitting)
      AddElectricalEffects();

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

    ret = true;
    evalTime += (1.0 / 60.0);
    x28_currentFrame += 1;
  }

  if (ret) {
    BuildBounds();
  }

  return ret;
}

void CParticleElectric::Render(const CActorLights* lights) {
  SCOPED_GRAPHICS_DEBUG_GROUP(fmt::format(FMT_STRING("CParticleElectric::Render {}"),
    *x1c_elecDesc.GetObjectTag()).c_str(), zeus::skYellow);

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
      x400_gpsmGenerators[i]->Render(lights);
    }
  }

  if (x450_26_haveEPSM) {
    for (int i = 0; i < x154_SCNT; ++i) {
      x410_epsmGenerators[i]->Render(lights);
    }
  }
}

void CParticleElectric::SetOrientation(const zeus::CTransform& orientation) {
  x44_orientation = orientation;
  x74_invOrientation = x44_orientation.inverse();
  x450_29_transformDirty = true;
}

void CParticleElectric::SetTranslation(const zeus::CVector3f& translation) {
  x38_translation = translation;
  x450_29_transformDirty = true;
}

void CParticleElectric::SetGlobalOrientation(const zeus::CTransform& orientation) {
  xb0_globalOrientation = orientation;
  x450_29_transformDirty = true;

  if (x450_27_haveSSWH) {
    for (const CParticleElectricManager& elec : x3e8_electricManagers) {
      CParticleSwoosh& swoosh = *x1e0_swooshGenerators[elec.x0_idx];
      swoosh.SetGlobalOrientation(xb0_globalOrientation);
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

void CParticleElectric::SetGlobalTranslation(const zeus::CVector3f& translation) {
  xa4_globalTranslation = translation;
  x450_29_transformDirty = true;

  if (x450_27_haveSSWH) {
    for (const CParticleElectricManager& elec : x3e8_electricManagers) {
      CParticleSwoosh& swoosh = *x1e0_swooshGenerators[elec.x0_idx];
      swoosh.SetGlobalTranslation(xa4_globalTranslation);
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

void CParticleElectric::SetGlobalScale(const zeus::CVector3f& scale) {
  xe0_globalScale = scale;
  x450_29_transformDirty = true;
}

void CParticleElectric::SetLocalScale(const zeus::CVector3f& scale) {
  xec_localScale = scale;
  x450_29_transformDirty = true;

  if (x450_27_haveSSWH) {
    for (const CParticleElectricManager& elec : x3e8_electricManagers) {
      CParticleSwoosh& swoosh = *x1e0_swooshGenerators[elec.x0_idx];
      swoosh.SetLocalScale(xec_localScale);
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

void CParticleElectric::SetParticleEmission(bool emitting) { x450_24_emitting = emitting; }

void CParticleElectric::SetModulationColor(const zeus::CColor& color) { x1b8_moduColor = color; }

const zeus::CTransform& CParticleElectric::GetOrientation() const { return x44_orientation; }

const zeus::CVector3f& CParticleElectric::GetTranslation() const { return x38_translation; }

const zeus::CTransform& CParticleElectric::GetGlobalOrientation() const { return xb0_globalOrientation; }

const zeus::CVector3f& CParticleElectric::GetGlobalTranslation() const { return xa4_globalTranslation; }

const zeus::CVector3f& CParticleElectric::GetGlobalScale() const { return xe0_globalScale; }

const zeus::CColor& CParticleElectric::GetModulationColor() const { return x1b8_moduColor; }

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

std::optional<zeus::CAABox> CParticleElectric::GetBounds() const {
  if (GetParticleCount() <= 0) {
    return std::nullopt;
  }

  return x160_systemBounds;
}

u32 CParticleElectric::GetParticleCount() const {
  u32 ret = 0;

  for (const CParticleElectricManager& elec : x3e8_electricManagers) {
    if (x450_27_haveSSWH) {
      ret += x1e0_swooshGenerators[elec.x0_idx]->GetParticleCount();
    }
    if (x450_28_haveLWD) {
      ret += x150_SSEG;
    }
  }

  if (x450_25_haveGPSM) {
    for (int i = 0; i < x154_SCNT; ++i) {
      ret += x400_gpsmGenerators[i]->GetParticleCount();
    }
  }

  if (x450_26_haveEPSM) {
    for (int i = 0; i < x154_SCNT; ++i) {
      ret += x410_epsmGenerators[i]->GetParticleCount();
    }
  }

  return ret;
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
  return CLight::BuildLocalAmbient(GetGlobalTranslation(), zeus::skOrange);
}

bool CParticleElectric::GetParticleEmission() const { return x450_24_emitting; }

void CParticleElectric::DestroyParticles() {
  // Empty
}

} // namespace urde
