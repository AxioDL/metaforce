#pragma once

#include <memory>
#include <string_view>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/World/CPatterned.hpp"

#include <zeus/CVector3f.hpp>

namespace urde::MP1 {
class CFlaahgraTentacle : public CPatterned {
  s32 x568_ = -1;
  std::unique_ptr<CCollisionActorManager> x56c_collisionManager;
  float x570_ = 0.f;
  float x574_ = 0.f;
  float x578_ = 0.f;
  TUniqueId x57c_tentacleTipAct = kInvalidUniqueId;
  zeus::CVector3f x580_forceVector;
  TUniqueId x58c_triggerId = kInvalidUniqueId;
  bool x58e_24_ : 1 = false;

  void AddSphereCollisionList(const SSphereJointInfo* sphereJoints, size_t jointCount,
                              std::vector<CJointCollisionDescription>& outJoints);
  void SetupCollisionManager(CStateManager&);
  void ExtractTentacle(CStateManager&);
  void RetractTentacle(CStateManager&);
  void SaveBombSlotInfo(CStateManager&);
public:
  DEFINE_PATTERNED(FlaahgraTentacle);
  CFlaahgraTentacle(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                    const CPatternedInfo&, const CActorParameters&);

  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;

  zeus::CVector3f GetAimPosition(const CStateManager&, float) const override;

  bool Inside(CStateManager&, float) override {
    return x450_bodyController->GetLocomotionType() == pas::ELocomotionType::Crouch;
  }
  bool AnimOver(CStateManager&, float) override { return x568_ == 3; }
  bool ShouldAttack(CStateManager&, float) override;

  void Dead(CStateManager&, EStateMsg, float) override {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
  }
  void Attack(CStateManager&, EStateMsg, float) override;
  void Retreat(CStateManager&, EStateMsg, float) override;
  void InActive(CStateManager&, EStateMsg, float) override;
};
}