#pragma once

#include <utility>
#include <vector>

#include "Runtime/CRandom16.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Graphics/CLineRenderer.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {
class CStateManager;

class CRainSplashGenerator {
  struct SSplashLine {
    float x0_t = 0.f;
    float x4_xEnd = 0.f;
    float x8_yEnd = 0.f;
    float xc_speed = 4.f;
    float x10_zParabolaHeight = 0.015625f;
    u8 x14_ = 3;
    u8 x15_length = 1;
    bool x16_active = true; // used to be one-bit bitfield
    CLineRenderer m_renderer;
    explicit SSplashLine(boo::IGraphicsDataFactory::Context& ctx);
    void Update(float dt, CStateManager& mgr);
    void Draw(float alpha, float dt, const zeus::CVector3f& pos);
    void SetActive() { x16_active = true; }
  };
  struct SRainSplash {
    rstl::reserved_vector<SSplashLine, 4> x0_lines;
    zeus::CVector3f x64_pos;
    float x70_ = 0.f;
    explicit SRainSplash(boo::IGraphicsDataFactory::Context& ctx);
    SRainSplash(const SRainSplash&) = delete;
    SRainSplash& operator=(const SRainSplash&) = delete;
    SRainSplash(SRainSplash&&) = default;
    SRainSplash& operator=(SRainSplash&&) = default;
    void Update(float dt, CStateManager& mgr);
    bool IsActive() const;
    void Draw(float alpha, float dt, const zeus::CVector3f& pos);
    void SetPoint(const zeus::CVector3f& pos);
  };
  std::vector<SRainSplash> x0_rainSplashes;
  CRandom16 x10_random{99};
  zeus::CVector3f x14_scale;
  float x20_generateTimer = 0.0f;
  float x24_generateInterval = 0.0f;
  float x28_dt = 0.0f;
  float x2c_minZ;
  float x30_alpha;
  u32 x34_curPoint = 0;
  u32 x38_queueTail = 0;
  u32 x3c_queueHead = 0;
  u32 x40_queueSize = 0;
  u32 x44_genRate;
  bool x48_24 : 1;
  bool x48_25_raining : 1;
  void UpdateRainSplashRange(CStateManager& mgr, int start, int end, float dt);
  void UpdateRainSplashes(CStateManager& mgr, float magnitude, float dt);
  void DoDraw(const zeus::CTransform& xf);
  static u32 GetNextBestPt(u32 pt, const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn, CRandom16& rand,
                           float minZ);
  void AddPoint(const zeus::CVector3f& pos);

public:
  CRainSplashGenerator(const zeus::CVector3f& scale, u32 maxSplashes, u32 genRate, float minZ, float alpha);
  void Update(float dt, CStateManager& mgr);
  void GeneratePoints(const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn);
  void Draw(const zeus::CTransform& xf);
  bool IsRaining() const { return x48_25_raining; }
};

} // namespace urde
