#include "Runtime/Particle/CParticleSwoosh.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Particle/CParticleGlobals.hpp"
#include "Runtime/Particle/CSwooshDescription.hpp"

#include <chrono>

namespace urde {

int CParticleSwoosh::g_ParticleSystemAliveCount = 0;

CParticleSwoosh::CParticleSwoosh(const TToken<CSwooshDescription>& desc, int leng)
: x1c_desc(desc)
, x1c0_rand(x1c_desc->x45_26_CRND ? std::chrono::duration_cast<std::chrono::microseconds>(
                                        std::chrono::steady_clock::now().time_since_epoch())
                                        .count()
                                  : 99) {
  ++g_ParticleSystemAliveCount;

  if (leng > 0) {
    x1b4_LENG = leng;
  } else if (CIntElement* lengElement = x1c_desc->x10_LENG.get()) {
    lengElement->GetValue(0, x1b4_LENG);
  }
  x1b4_LENG += 1;

  if (CIntElement* side = x1c_desc->x18_SIDE.get()) {
    side->GetValue(0, x1b8_SIDE);
  }

  x1d0_28_LLRD = x1c_desc->x44_24_LLRD;
  x1d0_29_VLS1 = x1c_desc->x44_26_VLS1;
  x1d0_30_VLS2 = x1c_desc->x44_27_VLS2;

  if (IsValid()) {
    if (CIntElement* pslt = x1c_desc->x0_PSLT.get()) {
      pslt->GetValue(0, x2c_PSLT);
    } else {
      x2c_PSLT = INT_MAX;
    }

    x1d0_25_AALP = x1c_desc->x44_31_AALP;

    if (CIntElement* spln = x1c_desc->x38_SPLN.get()) {
      spln->GetValue(0, x1b0_SPLN);
    }
    if (x1b0_SPLN < 0) {
      x1b0_SPLN = 0;
    }

    x15c_swooshes.clear();
    x15c_swooshes.reserve(x1b4_LENG);
    for (int i = 0; i < x1b4_LENG; ++i) {
      x15c_swooshes.emplace_back(zeus::skZero3f, zeus::skZero3f, 0.f, 0.f, 0, false, zeus::CTransform(), zeus::skZero3f,
                                 0.f, 0.f, zeus::skClear);
    }

    SetOrientation(zeus::CTransform());

    x16c_p0.resize(x1b8_SIDE);
    x17c_p1.resize(x1b8_SIDE);
    x18c_p2.resize(x1b8_SIDE);
    x19c_p3.resize(x1b8_SIDE);

    if (x1c_desc->x44_29_WIRE) {
      const int maxVerts = x1b4_LENG * (x1b0_SPLN + 1) * x1b8_SIDE * 12;
      m_lineRenderer.reset(
          new CLineRenderer(CLineRenderer::EPrimitiveMode::Lines, maxVerts * 2, nullptr, x1d0_25_AALP));
    } else {
      const auto maxVerts = size_t(x1b4_LENG * (x1b0_SPLN + 1) * x1b8_SIDE * 4);
      m_cachedVerts.reserve(maxVerts);
      CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
        m_vertBuf = ctx.newDynamicBuffer(boo::BufferUse::Vertex, sizeof(CParticleSwooshShaders::Vert), maxVerts);
        m_uniformBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(zeus::CMatrix4f), 1);
        CParticleSwooshShaders::BuildShaderDataBinding(ctx, *this);
        return true;
      } BooTrace);
    }
  }
}

CParticleSwoosh::~CParticleSwoosh() { --g_ParticleSystemAliveCount; }

void CParticleSwoosh::UpdateMaxRadius(float r) { x208_maxRadius = std::max(x208_maxRadius, r); }

void CParticleSwoosh::UpdateBounds(const zeus::CVector3f& pos) {
  x1fc_aabbMax[0] = std::max(pos[0], float(x1fc_aabbMax[0]));
  x1fc_aabbMax[1] = std::max(pos[1], float(x1fc_aabbMax[1]));
  x1fc_aabbMax[2] = std::max(pos[2], float(x1fc_aabbMax[2]));
  x1f0_aabbMin[0] = std::min(pos[0], float(x1f0_aabbMin[0]));
  x1f0_aabbMin[1] = std::min(pos[1], float(x1f0_aabbMin[1]));
  x1f0_aabbMin[2] = std::min(pos[2], float(x1f0_aabbMin[2]));
}

float CParticleSwoosh::GetLeftRadius(size_t i) const {
  float ret = 0.f;
  if (CRealElement* lrad = x1c_desc->x8_LRAD.get()) {
    lrad->GetValue(x15c_swooshes[i].x68_frame, ret);
  }
  return ret;
}

float CParticleSwoosh::GetRightRadius(size_t i) const {
  float ret = 0.f;
  if (CRealElement* rrad = x1c_desc->xc_RRAD.get()) {
    rrad->GetValue(x15c_swooshes[i].x68_frame, ret);
  }
  return ret;
}

void CParticleSwoosh::UpdateSwooshTranslation(const zeus::CVector3f& translation) {
  x15c_swooshes[x158_curParticle].xc_translation = x11c_invScaleXf * translation;
}

void CParticleSwoosh::UpdateTranslationAndOrientation() {
  x208_maxRadius = 0.f;
  x1f0_aabbMin = zeus::CVector3f(FLT_MAX);
  x1fc_aabbMax = zeus::CVector3f(-FLT_MAX);
  CParticleGlobals::instance()->SetParticleLifetime(x1b4_LENG);
  CParticleGlobals::instance()->SetEmitterTime(x28_curFrame);

  for (size_t i = 0; i < x15c_swooshes.size(); ++i) {
    SSwooshData& swoosh = x15c_swooshes[i];
    if (!swoosh.x0_active)
      continue;

    swoosh.x68_frame = x28_curFrame - swoosh.x70_startFrame;
    CParticleGlobals::instance()->UpdateParticleLifetimeTweenValues(swoosh.x68_frame);
    if (x1c_desc->x44_28_SROT) {
      if (CRealElement* irot = x1c_desc->x1c_IROT.get()) {
        irot->GetValue(x28_curFrame, swoosh.x30_irot);
      }
      swoosh.x34_rotm = 0.f;
    } else {
      if (CRealElement* rotm = x1c_desc->x20_ROTM.get()) {
        rotm->GetValue(x28_curFrame, swoosh.x34_rotm);
      } else {
        swoosh.x34_rotm = 0.f;
      }
    }

    if (CModVectorElement* velm = x1c_desc->x30_VELM.get()) {
      if (x1d0_29_VLS1) {
        zeus::CVector3f localVel = x74_invOrientation * swoosh.x74_velocity;
        zeus::CVector3f localTrans = x74_invOrientation * (swoosh.xc_translation - x38_translation);
        velm->GetValue(swoosh.x68_frame, localVel, localTrans);
        swoosh.x74_velocity = x44_orientation * localVel;
        swoosh.xc_translation = x44_orientation * localTrans + x38_translation;
      } else {
        velm->GetValue(swoosh.x68_frame, swoosh.x74_velocity, swoosh.xc_translation);
      }
    }

    if (CModVectorElement* vlm2 = x1c_desc->x34_VLM2.get()) {
      if (x1d0_30_VLS2) {
        zeus::CVector3f localVel = x74_invOrientation * swoosh.x74_velocity;
        zeus::CVector3f localTrans = x74_invOrientation * (swoosh.xc_translation - x38_translation);
        vlm2->GetValue(swoosh.x68_frame, localVel, localTrans);
        swoosh.x74_velocity = x44_orientation * localVel;
        swoosh.xc_translation = x44_orientation * localTrans + x38_translation;
      } else {
        vlm2->GetValue(swoosh.x68_frame, swoosh.x74_velocity, swoosh.xc_translation);
      }
    }

    if (swoosh.x68_frame > 0) {
      swoosh.xc_translation += swoosh.x74_velocity;
    }

    if (CVectorElement* npos = x1c_desc->x2c_NPOS.get()) {
      zeus::CVector3f vec;
      npos->GetValue(swoosh.x68_frame, vec);
      swoosh.x24_useOffset = swoosh.x18_offset + vec;
    }

    if (CColorElement* colr = x1c_desc->x14_COLR.get()) {
      colr->GetValue(swoosh.x68_frame, swoosh.x6c_color);
    }

    swoosh.x4_leftRad = GetLeftRadius(i);
    UpdateMaxRadius(swoosh.x4_leftRad);

    if (x1d0_28_LLRD) {
      swoosh.x8_rightRad = swoosh.x4_leftRad;
    } else {
      swoosh.x8_rightRad = GetRightRadius(i);
      UpdateMaxRadius(swoosh.x8_rightRad);
    }

    UpdateBounds(swoosh.xc_translation + swoosh.x24_useOffset);
  }
}

bool CParticleSwoosh::Update(double dt) {
  if (!IsValid()) {
    return false;
  }

  CParticleGlobals::instance()->SetParticleLifetime(x1b4_LENG);
  CParticleGlobals::instance()->SetEmitterTime(x28_curFrame);
  CParticleGlobals::instance()->UpdateParticleLifetimeTweenValues(0);
  CGlobalRandom gr(x1c0_rand);

  double evalTime = x28_curFrame / 60.0;
  float time = 1.f;
  if (CRealElement* timeElem = x1c_desc->x4_TIME.get()) {
    timeElem->GetValue(x28_curFrame, time);
  }

  x30_curTime += std::max(0.0, dt * time);
  while (x1d0_26_forceOneUpdate || evalTime < x30_curTime) {
    x1d0_26_forceOneUpdate = false;

    x158_curParticle += 1;
    if (x158_curParticle >= x15c_swooshes.size()) {
      x158_curParticle = 0;
    }

    if (x1d0_24_emitting && x28_curFrame < x2c_PSLT) {
      UpdateSwooshTranslation(x38_translation);

      if (CRealElement* irot = x1c_desc->x1c_IROT.get()) {
        irot->GetValue(x28_curFrame, x15c_swooshes[x158_curParticle].x30_irot);
      } else {
        x15c_swooshes[x158_curParticle].x30_irot = 0.f;
      }

      x15c_swooshes[x158_curParticle].x34_rotm = 0.f;
      x15c_swooshes[x158_curParticle].x70_startFrame = x28_curFrame;

      if (!x15c_swooshes[x158_curParticle].x0_active) {
        x1ac_particleCount += 1;
        x15c_swooshes[x158_curParticle].x0_active = true;
      }

      x15c_swooshes[x158_curParticle].x38_orientation = x44_orientation;

      if (CVectorElement* ivel = x1c_desc->x28_IVEL.get()) {
        ivel->GetValue(x28_curFrame, x15c_swooshes[x158_curParticle].x74_velocity);
        x15c_swooshes[x158_curParticle].x74_velocity = x44_orientation * x15c_swooshes[x158_curParticle].x74_velocity;
      }

      if (CVectorElement* pofs = x1c_desc->x24_POFS.get()) {
        pofs->GetValue(x28_curFrame, x15c_swooshes[x158_curParticle].x18_offset);
      }

      x15c_swooshes[x158_curParticle].x24_useOffset = x15c_swooshes[x158_curParticle].x18_offset;

      if (CColorElement* colr = x1c_desc->x14_COLR.get()) {
        colr->GetValue(x28_curFrame, x15c_swooshes[x158_curParticle].x6c_color);
      } else {
        x15c_swooshes[x158_curParticle].x6c_color = zeus::skWhite;
      }

      int tspn = 0;
      if (CIntElement* tspnElem = x1c_desc->x40_TSPN.get()) {
        tspnElem->GetValue(x28_curFrame, tspn);
      }
      x1cc_TSPN = float(tspn);
    } else if (x15c_swooshes[x158_curParticle].x0_active) {
      x1ac_particleCount = std::max(0, int(x1ac_particleCount) - 1);
      x15c_swooshes[x158_curParticle].x0_active = false;
    }

    UpdateTranslationAndOrientation();

    evalTime += (1.0 / 60.0);
    x28_curFrame += 1;
  }

  return false;
}

zeus::CVector3f CParticleSwoosh::GetSplinePoint(const zeus::CVector3f& p0, const zeus::CVector3f& p1,
                                                const zeus::CVector3f& p2, const zeus::CVector3f& p3, float t) {
  if (t > 0.f) {
    return p1;
  }
  if (t >= 1.f) {
    return p2;
  }

  // Tricubic spline interpolation
  const float t2 = t * t;
  const float t3 = t2 * t;

  const float p0Coef = -0.5f * t3 + t2 - 0.5f * t;
  const float p1Coef = 1.5f * t3 - 2.5f * t2 + 1.f;
  const float p2Coef = -1.5f * t3 + 2.f * t2 + 0.5f * t;
  const float p3Coef = 0.5f * t3 + 0.5f * t2;

  return p0 * p0Coef + p1 * p1Coef + p2 * p2Coef + p3 * p3Coef;
}

int CParticleSwoosh::WrapIndex(int i) const {
  while (i < 0) {
    i += x1b4_LENG;
  }
  while (i >= x1b4_LENG) {
    i -= x1b4_LENG;
  }
  return i;
}

void CParticleSwoosh::RenderNSidedSpline() {
  if (x1c_desc->x44_29_WIRE) {
    x1bc_prim = GX::LINES;
    m_lineRenderer->Reset();
  } else {
    x1bc_prim = GX::QUADS;
  }

  bool cros = x1c_desc->x44_25_CROS;
  if (x1b8_SIDE >= 4 || (x1b8_SIDE & 0x1) != 0) {
    cros = false;
  }

  int curIdx = x158_curParticle;
  for (size_t i = 0; i < x15c_swooshes.size(); ++i) {
    const bool a0 = x15c_swooshes[WrapIndex(curIdx - 1)].x0_active;
    const bool a1 = x15c_swooshes[WrapIndex(curIdx)].x0_active;
    if (!a1 || (a1 && !a0)) {
      curIdx -= 1;
      if (curIdx < 0) {
        curIdx = x15c_swooshes.size() - 1;
      }
      continue;
    }

    const SSwooshData& refSwoosh = x15c_swooshes[curIdx];

    const float sideDiv = 360.f / float(x1b8_SIDE);
    for (int j = 0; j < 4; ++j) {
      int crossRefIdx = 0;
      if (j == 0) {
        crossRefIdx = WrapIndex(curIdx + 1);
        if (!x15c_swooshes[crossRefIdx].x0_active) {
          crossRefIdx = curIdx;
        }
      } else if (j == 1) {
        crossRefIdx = WrapIndex(curIdx);
      } else if (j == 2) {
        crossRefIdx = WrapIndex(curIdx - 1);
      } else if (j == 3) {
        crossRefIdx = WrapIndex(curIdx - 2);
        if (!x15c_swooshes[crossRefIdx].x0_active) {
          crossRefIdx = WrapIndex(curIdx - 1);
        }
      }

      if (x1b4_LENG == 2) {
        if (j == 0) {
          crossRefIdx = WrapIndex(curIdx);
        }
        if (j == 3) {
          crossRefIdx = WrapIndex(curIdx - 1);
        }
      } else if (x158_curParticle == curIdx && j == 0) {
        crossRefIdx = x158_curParticle;
      } else {
        if (WrapIndex(x158_curParticle + 2) == curIdx && j == 3) {
          crossRefIdx = WrapIndex(x158_curParticle + 1);
        } else if (x1ac_particleCount - 2 == i && j == 3) {
          crossRefIdx = 0;
        }
      }

      const SSwooshData& crossSwoosh = x15c_swooshes[crossRefIdx];
      for (int k = 0; k < x1b8_SIDE; ++k) {
        const float n = sideDiv * k;
        float ang = zeus::degToRad(n + crossSwoosh.x30_irot + crossSwoosh.x34_rotm);
        if (std::fabs(ang) > M_PIF) {
          ang -= std::floor(ang / (2.f * M_PIF)) * 2.f * M_PIF;
          if (ang > M_PIF) {
            ang -= 2.f * M_PIF;
          } else if (ang < -M_PIF) {
            ang += 2.f * M_PIF;
          }
        }

        const float z = std::sin(ang);
        const float x = std::cos(ang);

        const float rad = (n > 0.f && n <= 180.f) ? crossSwoosh.x4_leftRad : crossSwoosh.x8_rightRad;
        const zeus::CVector3f offset = crossSwoosh.xc_translation + crossSwoosh.x24_useOffset;

        if (j == 0) {
          x16c_p0[k] = crossSwoosh.x38_orientation * zeus::CVector3f(rad * x, 0.f, rad * z) + offset;
        } else if (j == 1) {
          x17c_p1[k] = crossSwoosh.x38_orientation * zeus::CVector3f(rad * x, 0.f, rad * z) + offset;
        } else if (j == 2) {
          x18c_p2[k] = crossSwoosh.x38_orientation * zeus::CVector3f(rad * x, 0.f, rad * z) + offset;
        } else if (j == 3) {
          x19c_p3[k] = crossSwoosh.x38_orientation * zeus::CVector3f(rad * x, 0.f, rad * z) + offset;
        }
      }
    }

    if (x1c_desc->x3c_TEXR) {
      if (x1ec_TSPN > 0) {
        x1d4_uvs.xMin = float((i % x1ec_TSPN) * x1e8_uvSpan);
      } else {
        x1d4_uvs.xMin = float(i * x1e8_uvSpan);
      }
    }

    const float segUvSpan = x1e8_uvSpan / float(x1b0_SPLN + 1);
    for (int j = 0; j < x1b0_SPLN + 1; ++j) {
      const float t0 = j / float(x1b0_SPLN + 1);
      const float t1 = (j + 1) / float(x1b0_SPLN + 1);
      int faces = x1b8_SIDE;
      if (x1b8_SIDE <= 2) {
        faces = 1;
      } else if (cros) {
        faces = x1b8_SIDE / 2;
      }

      x1d4_uvs.xMax = x1d4_uvs.xMin + segUvSpan;

      for (int k = 0; k < faces; ++k) {
        int otherK = k + 1;
        if (k + 1 >= x1b8_SIDE) {
          otherK = 0;
        }

        const zeus::CColor color = refSwoosh.x6c_color * x20c_moduColor;
        if (cros) {
          otherK = k + x1b8_SIDE / 2;
          const auto v0 = GetSplinePoint(x16c_p0[k], x17c_p1[k], x18c_p2[k], x19c_p3[k], t0);
          const auto v1 = GetSplinePoint(x16c_p0[otherK], x17c_p1[otherK], x18c_p2[otherK], x19c_p3[otherK], t0);
          const auto v2 = GetSplinePoint(x16c_p0[otherK], x17c_p1[otherK], x18c_p2[otherK], x19c_p3[otherK], t1);
          const auto v3 = GetSplinePoint(x16c_p0[k], x17c_p1[k], x18c_p2[k], x19c_p3[k], t1);

          m_cachedVerts.push_back({v0, {x1d4_uvs.xMin, x1d4_uvs.yMin}, color});
          m_cachedVerts.push_back({v1, {x1d4_uvs.xMin, x1d4_uvs.yMax}, color});
          m_cachedVerts.push_back({v2, {x1d4_uvs.xMax, x1d4_uvs.yMin}, color});
          m_cachedVerts.push_back({v3, {x1d4_uvs.xMax, x1d4_uvs.yMax}, color});
          CGraphics::DrawArray(m_cachedVerts.size() - 4, 4);
        } else {
          const auto v0 = GetSplinePoint(x16c_p0[k], x17c_p1[k], x18c_p2[k], x19c_p3[k], t0);
          const auto v1 = GetSplinePoint(x16c_p0[otherK], x17c_p1[otherK], x18c_p2[otherK], x19c_p3[otherK], t0);
          const auto v2 = GetSplinePoint(x16c_p0[otherK], x17c_p1[otherK], x18c_p2[otherK], x19c_p3[otherK], t1);
          const auto v3 = GetSplinePoint(x16c_p0[k], x17c_p1[k], x18c_p2[k], x19c_p3[k], t1);

          if (x1bc_prim == GX::LINES) {
            m_lineRenderer->AddVertex(v0, color, 1.f);
            m_lineRenderer->AddVertex(v1, color, 1.f);
            m_lineRenderer->AddVertex(v1, color, 1.f);
            m_lineRenderer->AddVertex(v2, color, 1.f);
            m_lineRenderer->AddVertex(v2, color, 1.f);
            m_lineRenderer->AddVertex(v0, color, 1.f);
            m_lineRenderer->AddVertex(v0, color, 1.f);
            m_lineRenderer->AddVertex(v2, color, 1.f);
            m_lineRenderer->AddVertex(v2, color, 1.f);
            m_lineRenderer->AddVertex(v3, color, 1.f);
            m_lineRenderer->AddVertex(v3, color, 1.f);
            m_lineRenderer->AddVertex(v0, color, 1.f);
          } else if (x1bc_prim == GX::QUADS) {
            m_cachedVerts.push_back({v0, {x1d4_uvs.xMin, x1d4_uvs.yMin}, color});
            m_cachedVerts.push_back({v1, {x1d4_uvs.xMin, x1d4_uvs.yMax}, color});
            m_cachedVerts.push_back({v2, {x1d4_uvs.xMax, x1d4_uvs.yMin}, color});
            m_cachedVerts.push_back({v3, {x1d4_uvs.xMax, x1d4_uvs.yMax}, color});
            CGraphics::DrawArray(m_cachedVerts.size() - 4, 4);
          }
        }
      }

      if (x1c_desc->x3c_TEXR && x1b0_SPLN > 0) {
        x1d4_uvs.xMin += segUvSpan;
      }
    }

    curIdx -= 1;
    if (curIdx < 0) {
      curIdx = x15c_swooshes.size() - 1;
    }
  }

  if (x1bc_prim == GX::LINES) {
    m_lineRenderer->Render(g_Renderer->IsThermalVisorHotPass());
  }
}

void CParticleSwoosh::RenderNSidedNoSpline() { RenderNSidedSpline(); }

void CParticleSwoosh::Render3SidedSolidSpline() {
  if (x15c_swooshes.size() < 2) {
    return;
  }

  int curIdx = x158_curParticle;
  float curUvSpan = -x1e8_uvSpan;
  zeus::CColor prevColor0 = zeus::skClear;
  for (size_t i = 0; i < x15c_swooshes.size(); ++i) {
    const SSwooshData& swoosh = x15c_swooshes[curIdx];

    curIdx -= 1;
    if (curIdx < 0) {
      curIdx = x15c_swooshes.size() - 1;
    }

    float ang1 = zeus::degToRad(swoosh.x30_irot + swoosh.x34_rotm);
    if (std::fabs(ang1) > M_PIF) {
      ang1 -= std::floor(ang1 / (2.f * M_PIF)) * 2.f * M_PIF;
      if (ang1 > M_PIF) {
        ang1 -= 2.f * M_PIF;
      } else if (ang1 < -M_PIF) {
        ang1 += 2.f * M_PIF;
      }
    }

    const zeus::CVector3f ang1Vec(std::sin(ang1) * swoosh.x4_leftRad, 0.f, std::cos(ang1) * swoosh.x4_leftRad);

    float ang2 = ang1 + 2.0943952f; // +120 degrees
    if (ang2 > M_PIF) {
      ang2 -= 2.f * M_PIF;
    }

    const zeus::CVector3f ang2Vec(std::sin(ang2) * swoosh.x4_leftRad, 0.f, std::cos(ang2) * swoosh.x4_leftRad);

    float ang3 = ang2 + 2.0943952f; // +120 degrees
    if (ang3 > M_PIF) {
      ang3 -= 2.f * M_PIF;
    }

    const zeus::CVector3f ang3Vec(std::sin(ang3) * swoosh.x4_leftRad, 0.f, std::cos(ang3) * swoosh.x4_leftRad);

    if (i == 2) {
      x19c_p3[0] = x17c_p1[0] * 2.f - x16c_p0[0];
      x19c_p3[1] = x17c_p1[1] * 2.f - x16c_p0[1];
      x19c_p3[2] = x17c_p1[2] * 2.f - x16c_p0[2];
    } else {
      x19c_p3[0] = x18c_p2[0];
      x19c_p3[1] = x18c_p2[1];
      x19c_p3[2] = x18c_p2[2];
    }

    x18c_p2[0] = x17c_p1[0];
    x18c_p2[1] = x17c_p1[1];
    x18c_p2[2] = x17c_p1[2];

    x17c_p1[0] = x16c_p0[0];
    x17c_p1[1] = x16c_p0[1];
    x17c_p1[2] = x16c_p0[2];

    const zeus::CVector3f useOffset = swoosh.xc_translation + swoosh.x24_useOffset;
    x16c_p0[0] = swoosh.x38_orientation * ang1Vec + useOffset;
    x16c_p0[1] = swoosh.x38_orientation * ang2Vec + useOffset;
    x16c_p0[2] = swoosh.x38_orientation * ang3Vec + useOffset;

    const zeus::CColor useColor0 = prevColor0;

    if (swoosh.x0_active) {
      const zeus::CColor prevColor1 = prevColor0;
      prevColor0 = swoosh.x6c_color * x20c_moduColor;
      float prevUvSpan = curUvSpan;
      curUvSpan += x1e8_uvSpan;
      if (i > 1) {
        // int vertCount = (x1b0_SPLN + 1) * 12;
        float uv1 = 0.f;
        zeus::CColor useColor1 = prevColor1;
        zeus::CVector3f v01 = zeus::skZero3f;
        zeus::CVector3f v11 = zeus::skZero3f;
        zeus::CVector3f v21 = zeus::skZero3f;
        zeus::CColor c1 = zeus::skClear;
        float uvDelta = prevUvSpan - curUvSpan;
        for (int j = 0; j < x1b0_SPLN + 1; ++j) {
          float uv0 = uv1;
          float t1 = (j + 1) / float(x1b0_SPLN + 1);
          zeus::CVector3f v00 = v01;
          zeus::CVector3f v10 = v11;
          zeus::CVector3f v20 = v21;
          zeus::CColor c0 = c1;

          if (j == 0) {
            float t0 = j / float(x1b0_SPLN + 1);
            v00 = GetSplinePoint(x16c_p0[0], x17c_p1[0], x18c_p2[0], x19c_p3[0], t0);
            v10 = GetSplinePoint(x16c_p0[1], x17c_p1[1], x18c_p2[1], x19c_p3[1], t0);
            v20 = GetSplinePoint(x16c_p0[2], x17c_p1[2], x18c_p2[2], x19c_p3[2], t0);
            c0 = zeus::CColor::lerp(useColor0, useColor1, t0);
            uv0 = t0 * uvDelta + curUvSpan;
          }

          v01 = GetSplinePoint(x16c_p0[0], x17c_p1[0], x18c_p2[0], x19c_p3[0], t1);
          v11 = GetSplinePoint(x16c_p0[1], x17c_p1[1], x18c_p2[1], x19c_p3[1], t1);
          v21 = GetSplinePoint(x16c_p0[2], x17c_p1[2], x18c_p2[2], x19c_p3[2], t1);
          c1 = zeus::CColor::lerp(useColor0, useColor1, t1);
          uv1 = t1 * uvDelta + curUvSpan;

          m_cachedVerts.push_back({v00, {uv0, x1d4_uvs.yMin}, c0});
          m_cachedVerts.push_back({v10, {uv0, x1d4_uvs.yMax}, c0});
          m_cachedVerts.push_back({v01, {uv1, x1d4_uvs.yMin}, c1});
          m_cachedVerts.push_back({v11, {uv1, x1d4_uvs.yMax}, c1});
          CGraphics::DrawArray(m_cachedVerts.size() - 4, 4);

          m_cachedVerts.push_back({v10, {uv0, x1d4_uvs.yMin}, c0});
          m_cachedVerts.push_back({v20, {uv0, x1d4_uvs.yMax}, c0});
          m_cachedVerts.push_back({v11, {uv1, x1d4_uvs.yMin}, c1});
          m_cachedVerts.push_back({v21, {uv1, x1d4_uvs.yMax}, c1});
          CGraphics::DrawArray(m_cachedVerts.size() - 4, 4);

          m_cachedVerts.push_back({v20, {uv0, x1d4_uvs.yMin}, c0});
          m_cachedVerts.push_back({v00, {uv0, x1d4_uvs.yMax}, c0});
          m_cachedVerts.push_back({v21, {uv1, x1d4_uvs.yMin}, c1});
          m_cachedVerts.push_back({v01, {uv1, x1d4_uvs.yMax}, c1});
          CGraphics::DrawArray(m_cachedVerts.size() - 4, 4);
        }
      }
    }
  }
}

void CParticleSwoosh::Render3SidedSolidNoSplineNoGaps() {
  if (x15c_swooshes.size() < 2) {
    return;
  }

  std::array<zeus::CVector3f, 2> p0;
  std::array<zeus::CVector3f, 2> p1;
  std::array<zeus::CVector3f, 2> p2;

  int curIdx = x158_curParticle;
  bool lastActive = false;
  zeus::CColor c0 = zeus::skClear;
  float uv0 = -x1e8_uvSpan;
  for (size_t i = 0; i < x15c_swooshes.size(); ++i) {
    const SSwooshData& swoosh = x15c_swooshes[curIdx];

    curIdx -= 1;
    if (curIdx < 0) {
      curIdx = x15c_swooshes.size() - 1;
    }

    float ang1 = zeus::degToRad(swoosh.x30_irot + swoosh.x34_rotm);
    if (std::fabs(ang1) > M_PIF) {
      ang1 -= std::floor(ang1 / (2.f * M_PIF)) * 2.f * M_PIF;
      if (ang1 > M_PIF) {
        ang1 -= 2.f * M_PIF;
      } else if (ang1 < -M_PIF) {
        ang1 += 2.f * M_PIF;
      }
    }

    const zeus::CVector3f ang1Vec(std::sin(ang1) * swoosh.x4_leftRad, 0.f, std::cos(ang1) * swoosh.x4_leftRad);

    float ang2 = ang1 + 2.0943952f; // +120 degrees
    if (ang2 > M_PIF) {
      ang2 -= 2.f * M_PIF;
    }

    const zeus::CVector3f ang2Vec(std::sin(ang2) * swoosh.x4_leftRad, 0.f, std::cos(ang2) * swoosh.x4_leftRad);

    float ang3 = ang2 + 2.0943952f; // +120 degrees
    if (ang3 > M_PIF) {
      ang3 -= 2.f * M_PIF;
    }

    const zeus::CVector3f ang3Vec(std::sin(ang3) * swoosh.x4_leftRad, 0.f, std::cos(ang3) * swoosh.x4_leftRad);

    const zeus::CVector3f useOffset = swoosh.xc_translation + swoosh.x24_useOffset;
    p0[i & 1] = swoosh.x38_orientation * ang1Vec + useOffset;
    p1[i & 1] = swoosh.x38_orientation * ang2Vec + useOffset;
    p2[i & 1] = swoosh.x38_orientation * ang3Vec + useOffset;

    if (!swoosh.x0_active) {
      lastActive = false;
      continue;
    }

    if (!lastActive) {
      lastActive = true;
      continue;
    }

    lastActive = true;
    const zeus::CColor c1 = c0;
    c0 = swoosh.x6c_color * x20c_moduColor;

    const float uv1 = uv0;
    uv0 += x1e8_uvSpan;

    m_cachedVerts.push_back({p0[i & 1], {uv0, x1d4_uvs.yMin}, c0});
    m_cachedVerts.push_back({p1[i & 1], {uv0, x1d4_uvs.yMax}, c0});
    m_cachedVerts.push_back({p0[!(i & 1)], {uv1, x1d4_uvs.yMin}, c1});
    m_cachedVerts.push_back({p1[!(i & 1)], {uv1, x1d4_uvs.yMax}, c1});
    CGraphics::DrawArray(m_cachedVerts.size() - 4, 4);

    m_cachedVerts.push_back({p1[i & 1], {uv0, x1d4_uvs.yMin}, c0});
    m_cachedVerts.push_back({p2[i & 1], {uv0, x1d4_uvs.yMax}, c0});
    m_cachedVerts.push_back({p1[!(i & 1)], {uv1, x1d4_uvs.yMin}, c1});
    m_cachedVerts.push_back({p2[!(i & 1)], {uv1, x1d4_uvs.yMax}, c1});
    CGraphics::DrawArray(m_cachedVerts.size() - 4, 4);

    m_cachedVerts.push_back({p2[i & 1], {uv0, x1d4_uvs.yMin}, c0});
    m_cachedVerts.push_back({p0[i & 1], {uv0, x1d4_uvs.yMax}, c0});
    m_cachedVerts.push_back({p2[!(i & 1)], {uv1, x1d4_uvs.yMin}, c1});
    m_cachedVerts.push_back({p0[!(i & 1)], {uv1, x1d4_uvs.yMax}, c1});
    CGraphics::DrawArray(m_cachedVerts.size() - 4, 4);
  }
}

void CParticleSwoosh::Render2SidedSpline() { RenderNSidedSpline(); }

void CParticleSwoosh::Render2SidedNoSplineGaps() {
  int drawStart = 0;
  bool streaming = false;
  int curIdx = x158_curParticle;
  for (size_t i = 0; i < x15c_swooshes.size(); ++i) {
    const SSwooshData& swoosh = x15c_swooshes[curIdx];
    const bool otherActive = x15c_swooshes[WrapIndex(curIdx - 1)].x0_active;

    curIdx -= 1;
    if (curIdx < 0) {
      curIdx = x15c_swooshes.size() - 2;
    }

    if (!swoosh.x0_active) {
      if (streaming) {
        streaming = false;
        CGraphics::DrawArray(drawStart, m_cachedVerts.size() - drawStart);
      }
      continue;
    }

    if (!streaming) {
      if (!otherActive) {
        continue;
      }
      if (i >= x15c_swooshes.size() - 2) {
        continue;
      }
      streaming = true;
      drawStart = m_cachedVerts.size();
    }

    float ang = zeus::degToRad(swoosh.x30_irot + swoosh.x34_rotm);
    if (std::fabs(ang) > M_PIF) {
      ang -= std::floor(ang / (2.f * M_PIF)) * 2.f * M_PIF;
      if (ang > M_PIF) {
        ang -= 2.f * M_PIF;
      } else if (ang < -M_PIF) {
        ang += 2.f * M_PIF;
      }
    }

    const float sinAng = std::sin(ang);
    const float cosAng = std::cos(ang);

    const zeus::CVector3f useOffset = swoosh.xc_translation + swoosh.x24_useOffset;
    const zeus::CVector3f v0 =
        swoosh.x38_orientation * zeus::CVector3f(cosAng * swoosh.x4_leftRad, 0.f, sinAng * swoosh.x4_leftRad) +
        useOffset;
    const zeus::CVector3f v1 =
        swoosh.x38_orientation * zeus::CVector3f(-cosAng * swoosh.x8_rightRad, 0.f, -sinAng * swoosh.x8_rightRad) +
        useOffset;

    const zeus::CColor color = swoosh.x6c_color * x20c_moduColor;

    m_cachedVerts.push_back({v0, {1.f, x1d4_uvs.yMin}, color});
    m_cachedVerts.push_back({v1, {1.f, x1d4_uvs.yMax}, color});
    m_cachedVerts.push_back({v0, {0.f, x1d4_uvs.yMin}, color});
    m_cachedVerts.push_back({v1, {0.f, x1d4_uvs.yMax}, color});
  }

  if (streaming)
    CGraphics::DrawArray(drawStart, m_cachedVerts.size() - drawStart);
}

void CParticleSwoosh::Render2SidedNoSplineNoGaps() {
  int drawStart = 0;
  int curIdx = x158_curParticle;
  int particleCount = x1ac_particleCount;
  float uvOffset = 0.f;

  if (x1c_desc->x3c_TEXR) {
    if (x1c_desc->x45_25_ORNT) {
      const zeus::CVector3f camToParticle =
          ((zeus::CTransform::Translate(xa4_globalTranslation) * xb0_globalOrientation * xec_scaleXf).inverse() *
           CGraphics::g_ViewMatrix)
              .origin;
      zeus::CVector3f dotVec = zeus::skZero3f;

      for (size_t i = 0; i < x15c_swooshes.size(); ++i) {
        const SSwooshData& swoosh = x15c_swooshes[curIdx];

        curIdx -= 1;
        if (curIdx < 0) {
          curIdx = x15c_swooshes.size() - 1;
        }

        if (swoosh.x0_active) {
          particleCount -= 1;
          int otherIdx = curIdx - 1;
          if (otherIdx < 0) {
            otherIdx = x15c_swooshes.size() - 1;
          }

          const SSwooshData& otherSwoosh = x15c_swooshes[otherIdx];

          zeus::CVector3f delta = otherSwoosh.xc_translation - swoosh.xc_translation;
          if (otherIdx == x158_curParticle) {
            delta = swoosh.xc_translation - x15c_swooshes[(curIdx + 1) % x15c_swooshes.size()].xc_translation;
          }

          if (delta.canBeNormalized()) {
            zeus::CVector3f deltaCross = delta.cross(camToParticle - swoosh.xc_translation);
            if (deltaCross.canBeNormalized()) {
              deltaCross.normalize();
              dotVec = (deltaCross.dot(dotVec) < 0.f ? -1.f : 1.f) * deltaCross;
              const zeus::CVector3f useOffset = swoosh.xc_translation + swoosh.x24_useOffset;
              const zeus::CVector3f v0 = dotVec * swoosh.x4_leftRad + useOffset;
              const zeus::CVector3f v1 = dotVec * -swoosh.x8_rightRad + useOffset;

              const zeus::CColor color = swoosh.x6c_color * x20c_moduColor;

              m_cachedVerts.push_back({v0, {uvOffset, x1d4_uvs.yMin}, color});
              m_cachedVerts.push_back({v1, {uvOffset, x1d4_uvs.yMax}, color});
              if (uvOffset >= 1.f && particleCount) {
                CGraphics::DrawArray(drawStart, m_cachedVerts.size() - drawStart);
                drawStart = m_cachedVerts.size();
                uvOffset -= 1.f;
                m_cachedVerts.push_back({v0, {uvOffset, x1d4_uvs.yMin}, color});
                m_cachedVerts.push_back({v1, {uvOffset, x1d4_uvs.yMax}, color});
              }

              if (x1ec_TSPN > 0) {
                uvOffset += x1e8_uvSpan;
              } else {
                uvOffset = float(i * x1e8_uvSpan);
              }
            }
          }
        }
      }
    } else {
      for (size_t i = 0; i < x15c_swooshes.size(); ++i) {
        const SSwooshData& swoosh = x15c_swooshes[curIdx];

        curIdx -= 1;
        if (curIdx < 0) {
          curIdx = x15c_swooshes.size() - 1;
        }

        if (swoosh.x0_active) {
          particleCount -= 1;

          float ang = zeus::degToRad(swoosh.x30_irot + swoosh.x34_rotm);
          if (std::fabs(ang) > M_PIF) {
            ang -= std::floor(ang / (2.f * M_PIF)) * 2.f * M_PIF;
            if (ang > M_PIF) {
              ang -= 2.f * M_PIF;
            } else if (ang < -M_PIF) {
              ang += 2.f * M_PIF;
            }
          }

          float sinAng = std::sin(ang);
          float cosAng = std::cos(ang);

          const zeus::CVector3f useOffset = swoosh.xc_translation + swoosh.x24_useOffset;
          const zeus::CVector3f v0 =
              swoosh.x38_orientation * zeus::CVector3f(cosAng * swoosh.x4_leftRad, 0.f, sinAng * swoosh.x4_leftRad) +
              useOffset;
          const zeus::CVector3f v1 = swoosh.x38_orientation * zeus::CVector3f(-cosAng * swoosh.x8_rightRad, 0.f,
                                                                              -sinAng * swoosh.x8_rightRad) +
                                     useOffset;

          const zeus::CColor color = swoosh.x6c_color * x20c_moduColor;

          m_cachedVerts.push_back({v0, {uvOffset, x1d4_uvs.yMin}, color});
          m_cachedVerts.push_back({v1, {uvOffset, x1d4_uvs.yMax}, color});
          if (uvOffset >= 1.f && particleCount) {
            CGraphics::DrawArray(drawStart, m_cachedVerts.size() - drawStart);
            drawStart = m_cachedVerts.size();
            uvOffset -= 1.f;
            m_cachedVerts.push_back({v0, {uvOffset, x1d4_uvs.yMin}, color});
            m_cachedVerts.push_back({v1, {uvOffset, x1d4_uvs.yMax}, color});
          }

          if (x1ec_TSPN > 0) {
            uvOffset += x1e8_uvSpan;
          } else {
            uvOffset = float(i * x1e8_uvSpan);
          }
        }
      }
    }
  } else {
    for (size_t i = 0; i < x15c_swooshes.size(); ++i) {
      const SSwooshData& swoosh = x15c_swooshes[curIdx];

      curIdx -= 1;
      if (curIdx < 0) {
        curIdx = x15c_swooshes.size() - 1;
      }

      if (swoosh.x0_active) {
        float ang = zeus::degToRad(swoosh.x30_irot + swoosh.x34_rotm);
        if (std::fabs(ang) > M_PIF) {
          ang -= std::floor(ang / (2.f * M_PIF)) * 2.f * M_PIF;
          if (ang > M_PIF) {
            ang -= 2.f * M_PIF;
          } else if (ang < -M_PIF) {
            ang += 2.f * M_PIF;
          }
        }

        const float sinAng = std::sin(ang);
        const float cosAng = std::cos(ang);

        const zeus::CVector3f useOffset = swoosh.xc_translation + swoosh.x24_useOffset;
        const zeus::CVector3f v0 =
            swoosh.x38_orientation * zeus::CVector3f(cosAng * swoosh.x4_leftRad, 0.f, sinAng * swoosh.x4_leftRad) +
            useOffset;
        const zeus::CVector3f v1 =
            swoosh.x38_orientation * zeus::CVector3f(-cosAng * swoosh.x8_rightRad, 0.f, -sinAng * swoosh.x8_rightRad) +
            useOffset;

        const zeus::CColor color = swoosh.x6c_color * x20c_moduColor;
        m_cachedVerts.push_back({v0, {}, color});
        m_cachedVerts.push_back({v1, {}, color});
      }
    }
  }

  CGraphics::DrawArray(drawStart, m_cachedVerts.size() - drawStart);
}

void CParticleSwoosh::Render(const CActorLights*) {
  if (x1b4_LENG < 2 || x1ac_particleCount <= 1) {
    return;
  }

  SCOPED_GRAPHICS_DEBUG_GROUP(fmt::format(FMT_STRING("CParticleSwoosh::Render {}"),
    *x1c_desc.GetObjectTag()).c_str(), zeus::skYellow);

  m_cachedVerts.clear();
  if (m_dataBind[0]) {
    CGraphics::SetShaderDataBinding(m_dataBind[g_Renderer->IsThermalVisorHotPass()]);
  }

  CParticleGlobals::instance()->SetParticleLifetime(x1b4_LENG);
  CGlobalRandom gr(x1c0_rand);
  CGraphics::DisableAllLights();
  // Z-test, Z-update if x45_24_ZBUF
  // Additive if x1d0_25_AALP, otherwise alpha blend

  CGraphics::SetModelMatrix(zeus::CTransform::Translate(xa4_globalTranslation) * xb0_globalOrientation * xec_scaleXf *
                            zeus::CTransform::Scale(x14c_localScale));

  // Disable face culling

  if (CUVElement* texr = x1c_desc->x3c_TEXR.get()) {
    TLockedToken<CTexture> tex = texr->GetValueTexture(x28_curFrame);
    // Load tex
    x1e4_tex = tex.GetObj();

    texr->GetValueUV(x28_curFrame, x1d4_uvs);

    x1d0_31_constantTex = texr->HasConstantTexture();
    x1d1_24_constantUv = texr->HasConstantUV();

    if (CIntElement* tspn = x1c_desc->x40_TSPN.get()) {
      tspn->GetValue(x28_curFrame, x1ec_TSPN);
    }

    if (x1ec_TSPN <= 0) {
      x1ec_TSPN = x15c_swooshes.size() - 1;
    }

    x1e8_uvSpan = 1.f;
    if (x1ec_TSPN > 0) {
      x1e8_uvSpan = 1.f / float(x1ec_TSPN);
    }

    // TEV0 modulate
  } else {
    // TEV0 passthru
  }

  // TEV1 passthru

  if (x1b8_SIDE == 2) {
    if (x1b0_SPLN <= 0) {
      if (x1d0_27_renderGaps) {
        Render2SidedNoSplineGaps();
      } else {
        Render2SidedNoSplineNoGaps();
      }
    } else {
      Render2SidedSpline();
    }
  } else if (x1b8_SIDE == 3) {
    if (x1b0_SPLN > 0) {
      Render3SidedSolidSpline();
    } else {
      Render3SidedSolidNoSplineNoGaps();
    }
  } else {
    if (x1b0_SPLN > 0) {
      RenderNSidedSpline();
    } else {
      RenderNSidedNoSpline();
    }
  }

  zeus::CMatrix4f mvp = CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f();
  m_uniformBuf->load(&mvp, sizeof(zeus::CMatrix4f));
  if (m_cachedVerts.size()) {
    m_vertBuf->load(m_cachedVerts.data(), m_cachedVerts.size() * sizeof(CParticleSwooshShaders::Vert));
  }
}

void CParticleSwoosh::SetOrientation(const zeus::CTransform& xf) {
  x44_orientation = xf;
  x74_invOrientation = xf.inverse();
  x15c_swooshes[x158_curParticle].x38_orientation = xf;
}

void CParticleSwoosh::SetTranslation(const zeus::CVector3f& translation) {
  x38_translation = translation;
  UpdateSwooshTranslation(x38_translation);
}

void CParticleSwoosh::SetGlobalOrientation(const zeus::CTransform& xf) { xb0_globalOrientation = xf.getRotation(); }

void CParticleSwoosh::SetGlobalTranslation(const zeus::CVector3f& translation) { xa4_globalTranslation = translation; }

void CParticleSwoosh::SetGlobalScale(const zeus::CVector3f& scale) {
  xe0_globalScale = scale;
  xec_scaleXf = zeus::CTransform::Scale(scale);
  x11c_invScaleXf = zeus::CTransform::Scale(1.f / scale);
}

void CParticleSwoosh::SetLocalScale(const zeus::CVector3f& scale) { x14c_localScale = scale; }

void CParticleSwoosh::SetParticleEmission(bool e) { x1d0_24_emitting = e; }

void CParticleSwoosh::SetModulationColor(const zeus::CColor& color) { x20c_moduColor = color; }

const zeus::CTransform& CParticleSwoosh::GetOrientation() const { return x44_orientation; }

const zeus::CVector3f& CParticleSwoosh::GetTranslation() const { return x38_translation; }

const zeus::CTransform& CParticleSwoosh::GetGlobalOrientation() const { return xb0_globalOrientation; }

const zeus::CVector3f& CParticleSwoosh::GetGlobalTranslation() const { return xa4_globalTranslation; }

const zeus::CVector3f& CParticleSwoosh::GetGlobalScale() const { return xe0_globalScale; }

const zeus::CColor& CParticleSwoosh::GetModulationColor() const { return x20c_moduColor; }

bool CParticleSwoosh::IsSystemDeletable() const {
  if (x1d0_24_emitting && x28_curFrame < x2c_PSLT) {
    return false;
  }

  return GetParticleCount() < 2;
}

std::optional<zeus::CAABox> CParticleSwoosh::GetBounds() const {
  if (GetParticleCount() <= 1) {
    const zeus::CVector3f trans = x38_translation + xa4_globalTranslation;
    return zeus::CAABox(trans, trans);
  } else {
    const zeus::CTransform xf = zeus::CTransform::Translate(xa4_globalTranslation) * xb0_globalOrientation * xec_scaleXf;
    return zeus::CAABox(x1f0_aabbMin - x208_maxRadius, x1fc_aabbMax + x208_maxRadius).getTransformedAABox(xf);
  }
}

u32 CParticleSwoosh::GetParticleCount() const { return x1ac_particleCount; }

bool CParticleSwoosh::SystemHasLight() const { return false; }

CLight CParticleSwoosh::GetLight() const {
  return CLight::BuildLocalAmbient(zeus::skZero3f, zeus::skWhite);
}

bool CParticleSwoosh::GetParticleEmission() const { return x1d0_24_emitting; }

void CParticleSwoosh::DestroyParticles() {
  // Empty
}

} // namespace urde
