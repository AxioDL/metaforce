#include "Runtime/Graphics/CRainSplashGenerator.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/World/CWorld.hpp"

namespace urde {

CRainSplashGenerator::CRainSplashGenerator(const zeus::CVector3f& scale, u32 maxSplashes, u32 genRate, float minZ,
                                           float alpha)
: x14_scale(scale), x2c_minZ(minZ) {
  x30_alpha = std::min(1.f, alpha);
  x44_genRate = std::min(maxSplashes, genRate);
  x0_rainSplashes.reserve(maxSplashes);
  CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
    for (u32 i = 0; i < maxSplashes; ++i)
      x0_rainSplashes.emplace_back(ctx);
    return true;
  } BooTrace);
}

void CRainSplashGenerator::SSplashLine::Draw(float alpha, float dt, const zeus::CVector3f& pos) {
  if (x0_t > 0.f) {
    float delta = dt * xc_speed;
    float vt = std::max(0.f, x0_t - delta * x15_length);
    auto vertCount = u32((x0_t - vt) / delta + 1.f);
    m_renderer.Reset();
    for (u32 i = 0; i < vertCount; ++i) {
      float vertAlpha = vt * alpha;
      zeus::CVector3f vec(vt * x4_xEnd, vt * x8_yEnd, -4.f * vt * (vt - 1.f) * x10_zParabolaHeight);
      vec += pos;
      vt += delta;
      m_renderer.AddVertex(vec, zeus::CColor(1.f, vertAlpha), 1);
    }
    m_renderer.Render(g_Renderer->IsThermalVisorHotPass());
  }
}

void CRainSplashGenerator::SRainSplash::Draw(float alpha, float dt, const zeus::CVector3f& pos) {
  for (SSplashLine& line : x0_lines) {
    line.Draw(alpha, dt, pos);
  }
}

void CRainSplashGenerator::DoDraw(const zeus::CTransform& xf) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CRainSplashGenerator::DoDraw", zeus::skYellow);
  CGraphics::SetModelMatrix(xf);
  if (x40_queueSize > 0) {
    if (x38_queueTail <= x3c_queueHead) {
      for (size_t i = x3c_queueHead; i < x0_rainSplashes.size(); ++i) {
        SRainSplash& splash = x0_rainSplashes[i];
        splash.Draw(x30_alpha, x28_dt, splash.x64_pos);
      }
      for (size_t i = 0; i < x38_queueTail; ++i) {
        SRainSplash& splash = x0_rainSplashes[i];
        splash.Draw(x30_alpha, x28_dt, splash.x64_pos);
      }
    } else {
      for (size_t i = x3c_queueHead; i < x38_queueTail; ++i) {
        SRainSplash& splash = x0_rainSplashes[i];
        splash.Draw(x30_alpha, x28_dt, splash.x64_pos);
      }
    }
  }
}

void CRainSplashGenerator::Draw(const zeus::CTransform& xf) {
  if (!x48_25_raining) {
    return;
  }

  DoDraw(xf);
}

CRainSplashGenerator::SSplashLine::SSplashLine(boo::IGraphicsDataFactory::Context& ctx)
: m_renderer(ctx, CLineRenderer::EPrimitiveMode::LineStrip, 3, nullptr, false) {}

CRainSplashGenerator::SRainSplash::SRainSplash(boo::IGraphicsDataFactory::Context& ctx) {
  for (size_t i = 0; i < x0_lines.capacity(); ++i) {
    x0_lines.emplace_back(ctx);
  }
}

void CRainSplashGenerator::SSplashLine::Update(float dt, CStateManager& mgr) {
  if (!x16_active)
    return;
  if (x0_t <= 0.8f) {
    x14_ = u8(5.f * (1.f - x0_t) + 3.f * x0_t);
    x0_t += dt * xc_speed;
  } else if (x15_length != 0) {
    x15_length -= 1;
  } else {
    x16_active = false;
    xc_speed = mgr.GetActiveRandom()->Range(4.0f, 8.0f);
    x10_zParabolaHeight = mgr.GetActiveRandom()->Range(0.015625f, 0.03125f);
    x4_xEnd = mgr.GetActiveRandom()->Range(-0.125f, 0.125f);
    x8_yEnd = mgr.GetActiveRandom()->Range(-0.125f, 0.125f);
    x15_length = u8(mgr.GetActiveRandom()->Range(1, 2));
  }
}

void CRainSplashGenerator::SRainSplash::Update(float dt, CStateManager& mgr) {
  for (SSplashLine& point : x0_lines)
    point.Update(dt, mgr);
}

bool CRainSplashGenerator::SRainSplash::IsActive() const {
  bool ret = false;
  for (const SSplashLine& line : x0_lines)
    ret |= line.x16_active;
  return ret;
}

void CRainSplashGenerator::UpdateRainSplashRange(CStateManager& mgr, int start, int end, float dt) {
  for (int i = start; i < end; ++i) {
    SRainSplash& set = x0_rainSplashes[i];
    set.Update(dt, mgr);
    if (!set.IsActive()) {
      x40_queueSize -= 1;
      x3c_queueHead += 1;
      if (x3c_queueHead >= x0_rainSplashes.size())
        x3c_queueHead = 0;
    }
  }
}

void CRainSplashGenerator::UpdateRainSplashes(CStateManager& mgr, float magnitude, float dt) {
  x20_generateTimer += dt;
  x24_generateInterval = 1.f / (70.f * magnitude);
  if (x40_queueSize > 0) {
    if (x38_queueTail <= x3c_queueHead) {
      UpdateRainSplashRange(mgr, x3c_queueHead, int(x0_rainSplashes.size()), dt);
      UpdateRainSplashRange(mgr, 0, x38_queueTail, dt);
    } else {
      UpdateRainSplashRange(mgr, x3c_queueHead, x38_queueTail, dt);
    }
  }
}

void CRainSplashGenerator::Update(float dt, CStateManager& mgr) {
  EEnvFxType neededFx = mgr.GetWorld()->GetNeededEnvFx();
  x28_dt = dt;
  x48_25_raining = false;
  if (neededFx != EEnvFxType::None && mgr.GetEnvFxManager()->IsSplashActive() &&
      mgr.GetEnvFxManager()->GetRainMagnitude() != 0.f && neededFx == EEnvFxType::Rain) {
    UpdateRainSplashes(mgr, mgr.GetEnvFxManager()->GetRainMagnitude(), dt);
    x48_25_raining = true;
  }
}

u32 CRainSplashGenerator::GetNextBestPt(u32 pt, const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn,
                                        CRandom16& rand, float minZ) {
  auto& refVert = vn[pt];
  float maxDist = 0.f;
  u32 nextPt = pt;
  for (int i = 0; i < 3; ++i) {
    auto idx = u32(rand.Range(0, int(vn.size() - 1)));
    auto& vert = vn[idx];
    float distSq = (refVert.first - vert.first).magSquared();
    if (distSq > maxDist && vert.second.dot(zeus::skUp) >= 0.f &&
        (vert.first.z() <= 0.f || vert.first.z() > minZ)) {
      nextPt = idx;
      maxDist = distSq;
    }
  }
  return nextPt;
}

void CRainSplashGenerator::SRainSplash::SetPoint(const zeus::CVector3f& pos) {
  for (SSplashLine& line : x0_lines)
    line.SetActive();
  x64_pos = pos;
}

void CRainSplashGenerator::AddPoint(const zeus::CVector3f& pos) {
  if (x38_queueTail >= x0_rainSplashes.size())
    x38_queueTail = 0;
  x0_rainSplashes[x38_queueTail].SetPoint(pos);
  x40_queueSize += 1;
  x38_queueTail += 1;
}

void CRainSplashGenerator::GeneratePoints(const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn) {
  if (!x48_25_raining)
    return;

  if (x20_generateTimer > x24_generateInterval) {
    for (u32 i = 0; i < x44_genRate; ++i) {
      if (x40_queueSize >= x0_rainSplashes.size())
        break;
      x34_curPoint = GetNextBestPt(x34_curPoint, vn, x10_random, x2c_minZ);
      AddPoint(x14_scale * vn[x34_curPoint].first);
    }
    x20_generateTimer = 0.f;
  }
}

} // namespace urde
