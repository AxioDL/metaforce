#pragma once

#include "World/CPatterned.hpp"
#include "Collision/CCollisionActorManager.hpp"

namespace urde::MP1 {
class CSpankWeed : public CPatterned {
  float x568_maxDetectionRange;
  float x56c_detectionHeightRange;
  float x570_maxHearingRange;
  float x574_maxSightRange;
  float x578_hideTime;
  bool x57c_canKnockBack = false;
  /* float x580_ = 0.f; unused */
  zeus::CVector3f x584_retreatOrigin;
  TUniqueId x590_ = kInvalidUniqueId;
  std::unique_ptr<CCollisionActorManager> x594_collisionMgr;
  bool x598_isHiding = true;
  zeus::CVector3f x59c_lockonOffset;
  zeus::CVector3f x5a8_lockonTarget;
  s32 x5b4_ = -1;
  s32 x5b8_ = -1;
  s32 x5bc_ = -1;

  float GetPlayerDistance(CStateManager&) const;

public:
  DEFINE_PATTERNED(SpankWeed)

  CSpankWeed(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
             const CActorParameters&, const CPatternedInfo&, float, float, float, float);

  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;
  zeus::CVector3f GetOrbitPosition(const CStateManager&) const override;
  zeus::CVector3f GetAimPosition(const CStateManager&, float) const override;
  bool AnimOver(CStateManager&, float) override { return x5bc_ == 3; }
  void Flinch(CStateManager&, EStateMsg, float) override;
  bool Delay(CStateManager&, float) override;
  bool InRange(CStateManager&, float) override;
  bool HearPlayer(CStateManager&, float) override;
  bool InDetectionRange(CStateManager&, float) override;
  void Attack(CStateManager&, EStateMsg, float) override;
  void TargetPatrol(CStateManager&, EStateMsg, float) override;
  void Lurk(CStateManager&, EStateMsg, float) override;
  void FadeOut(CStateManager&, EStateMsg, float) override;
  void FadeIn(CStateManager&, EStateMsg, float) override;
  void Patrol(CStateManager&, EStateMsg, float) override;
  void KnockBack(const zeus::CVector3f&, CStateManager&, const CDamageInfo& info, EKnockBackType type, bool inDeferred,
                 float magnitude) override;
};
} // namespace urde::MP1