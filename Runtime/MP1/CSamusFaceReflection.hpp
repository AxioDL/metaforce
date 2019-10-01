#pragma once

#include <memory>

#include "Runtime/Character/CActorLights.hpp"
#include "Runtime/Character/CModelData.hpp"

#include <zeus/CQuaternion.hpp>
#include <zeus/CVector3f.hpp>

namespace urde::MP1 {

class CSamusFaceReflection {
  CModelData x0_modelData;
  std::unique_ptr<CActorLights> x4c_lights;
  zeus::CQuaternion x50_lookRot;
  zeus::CVector3f x60_lookDir;
  u32 x6c_ = 0;
  bool x70_hidden = true;

public:
  CSamusFaceReflection(CStateManager& stateMgr);
  void PreDraw(const CStateManager& stateMgr);
  void Draw(const CStateManager& stateMgr) const;
  void Update(float dt, const CStateManager& stateMgr, CRandom16& rand);
};

} // namespace urde::MP1
