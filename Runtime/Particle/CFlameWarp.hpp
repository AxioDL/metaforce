#pragma once

#include "Runtime/rstl.hpp"
#include "Runtime/Particle/CWarp.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CStateManager;

class CFlameWarp : public CWarp {
  rstl::reserved_vector<zeus::CVector3f, 9> x4_collisionPoints;
  zeus::CVector3f x74_warpPoint;
  zeus::CVector3f x80_floatingPoint;
  float x8c_maxDistSq = 0.f;
  float x90_minSize = FLT_MAX;
  float x94_maxSize = FLT_MIN;
  float x98_maxInfluenceDistSq;
  CStateManager* x9c_stateMgr = nullptr;
  bool xa0_24_activated : 1 = false;
  bool xa0_25_collisionWarp : 1;
  bool xa0_26_processed : 1 = false;

public:
  CFlameWarp(float maxInfluenceDist, const zeus::CVector3f& warpPoint, bool collisionWarp)
  : x74_warpPoint(warpPoint)
  , x80_floatingPoint(warpPoint)
  , x98_maxInfluenceDistSq(maxInfluenceDist * maxInfluenceDist)
  , xa0_25_collisionWarp{collisionWarp} {
    x4_collisionPoints.resize(9, warpPoint);
  }

  const rstl::reserved_vector<zeus::CVector3f, 9>& GetCollisionPoints() const { return x4_collisionPoints; }
  float GetMinSize() const { return x90_minSize; }
  float GetMaxSize() const { return x94_maxSize; }
  void SetWarpPoint(const zeus::CVector3f& p) { x74_warpPoint = p; }
  void SetFloatingPoint(const zeus::CVector3f& p) { x80_floatingPoint = p; }
  const zeus::CVector3f& GetFloatingPoint() const { return x80_floatingPoint; }
  void SetMaxDistSq(float d) { x8c_maxDistSq = d; }
  void SetStateManager(CStateManager& mgr) { x9c_stateMgr = &mgr; }
  bool UpdateWarp() override { return xa0_24_activated; }
  void ModifyParticles(std::vector<CParticle>& particles) override;
  void Activate(bool val) override { xa0_24_activated = val; }
  bool IsActivated() override { return xa0_24_activated; }
  bool IsProcessed() const { return xa0_26_processed; }
  FourCC Get4CharID() override { return FOURCC('FWRP'); }
  void ResetPosition(const zeus::CVector3f& pos);
  zeus::CAABox CalculateBounds() const;
};
} // namespace urde
