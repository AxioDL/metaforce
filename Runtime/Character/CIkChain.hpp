#pragma once

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Character/CSegId.hpp"

#include <zeus/CQuaternion.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CAnimData;
class CSegId;
class CIkChain {
  CSegId x0_bone;
  CSegId x1_p1;
  CSegId x2_p2;
  zeus::CVector3f x4_p2p1Dir = zeus::skForward;
  zeus::CVector3f x10_p1BoneDir = zeus::skForward;
  float x1c_p2p1Length = 1.f;
  float x20_p1BoneLength = 1.f;
  zeus::CQuaternion x24_holdRot;
  zeus::CVector3f x34_holdPos;
  float x40_time = 0.f;
  bool x44_24_activated : 1 = false;

public:
  CIkChain() = default;

  bool GetActive() const { return x44_24_activated; }
  void Update(float);
  void Deactivate();
  void Activate(const CAnimData&, const CSegId&, const zeus::CTransform&);
  void PreRender(CAnimData&, const zeus::CTransform&, const zeus::CVector3f&);
  void Solve(zeus::CQuaternion&, zeus::CQuaternion&, const zeus::CVector3f&);
};
} // namespace urde
