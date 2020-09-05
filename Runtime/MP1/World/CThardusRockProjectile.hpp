#pragma once

#include "Runtime/World/CPatterned.hpp"

namespace urde::MP1 {
class CThardusRockProjectile : public CPatterned {
  float x568_ = 1.f;
  u32 x56c_ = 0;
  TUniqueId x570_ = kInvalidUniqueId;
  bool x572_ = false;
  EAnimState x574_ = EAnimState::Invalid;
  u32 x578_ = 0;
  std::vector<std::unique_ptr<CModelData>> x57c_;
  std::vector<TUniqueId> x58c_;
  CAssetId x59c_stateMachine;
  u32 x5a0_ = 0;
  bool x5a4_ = true;
  float x5a8_ = 0.f;
  float x5ac_ = 0.f;
  zeus::CVector3f x5b0_ = zeus::skForward;
  bool x5bc_ = true;
  float x5c0_;
  u32 x5c4_ = 0;
  u32 x5c8_ = 0;
  u32 x5cc_ = 0;
  TUniqueId x5d0_thardusId = kInvalidUniqueId;
  u32 x5d4_ = 0;
  u32 x5d8_ = 0;
  bool x5dc_ = false;
  bool x5dd_ = false;
  void sub80203824(CStateManager& mgr, u32 w1, const zeus::CVector3f& pos, const zeus::CVector3f& scale, u32 w2) {}
public:
  DEFINE_PATTERNED(ThardusRockProjectile);
  CThardusRockProjectile(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                         CModelData&& modelData, const CActorParameters& aParms, const CPatternedInfo& patternedInfo,
                         std::vector<std::unique_ptr<CModelData>>&& mDataVec, CAssetId stateMachine, float);

  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId other, CStateManager& mgr) override;
  void Render(CStateManager& mgr) override;
  void sub80203d58() {
    x328_25_verticalMovement = false;
    x150_momentum = {0.f, 0.f, 2.f * -GetWeight()};
    x56c_= 3;
  }

  void Patrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Dead(CStateManager& mgr, EStateMsg msg, float dt) override;
  void LoopedAttack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void GetUp(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Lurk(CStateManager& mgr, EStateMsg msg, float dt) override;
  bool Delay(CStateManager& mgr, float arg) override;
  bool AnimOver(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool HitSomething(CStateManager& mgr, float arg) override;
  bool ShouldMove(CStateManager& mgr, float arg) override;
};
} // namespace urde::MP1
