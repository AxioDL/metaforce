#include "CSimplePool.hpp"
#include "CStateManager.hpp"
#include "GameGlobalObjects.hpp"
#include "MP1/World/CEnergyBall.hpp"
#include "World/CPatternedInfo.hpp"
#include "World/CPlayer.hpp"

namespace urde::MP1 {
CEnergyBall::CEnergyBall(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                         CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo, s32 w1,
                         float f1, const CDamageInfo& dInfo1, float f2, const CAssetId& a1, s16 sfxId1,
                         const CAssetId& a2, const CAssetId& a3, s16 sfxId2, float f3, float f4, const CAssetId& a4,
                         const CDamageInfo& dInfo2, float f5)
: CPatterned(ECharacter::EnergyBall, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::NewFlyer, actParms, EKnockBackVariant::Medium)
, x570_(w1)
, x574_(f1)
, x578_(dInfo1)
, x594_initialTurnSpeed(pInfo.GetTurnSpeed())
, x598_(f2)
, x59c_(a1)
, x5a0_(sfxId1)
, x5a4_(a2)
, x5a8_(g_SimplePool->GetObj({FOURCC('ELSC'), a3}))
, x5b4_(sfxId2)
, x5b8_(f3)
, x5bc_(f4)
, x5c0_(g_SimplePool->GetObj({FOURCC('PART'), a4}))
, x5cc_(dInfo2)
, x5e8_(f5) {
  x460_knockBackController.SetEnableExplodeDeath(false);
  x460_knockBackController.SetAutoResetImpulse(false);
  x460_knockBackController.SetEnableBurnDeath(false);
  x460_knockBackController.SetX82_24(false);
  x460_knockBackController.SetEnableBurn(false);
  x460_knockBackController.SetEnableLaggedBurnDeath(false);
  x460_knockBackController.SetEnableShock(false);
  x460_knockBackController.SetEnableFreeze(false);
  x460_knockBackController.SetX81_31(false);
}

void CEnergyBall::Think(float dt, CStateManager& mgr) {
  float newTurnSpeed = x594_initialTurnSpeed * zeus::clamp(0.f, (x56c_ - 2.5f) * 0.125f, 1.f);
  x3b8_turnSpeed = newTurnSpeed;
  x450_bodyController->SetTurnSpeed(newTurnSpeed);
  CPatterned::Think(dt, mgr);
  GetModelData()->GetAnimationData()->GetParticleDB().SetModulationColorAllActiveEffects(
      zeus::CColor::lerp(zeus::skWhite, zeus::skRed, zeus::clamp(0.f, x428_damageCooldownTimer / 0.33f, 1.f)));

  bool r27 = false;
  if (GetActive() && IsAlive()) {
    x56c_ -= dt;
    if (x56c_ > x574_)
      r27 = true;
    if (!InMaxRange(mgr, dt))
      r27 = true;
  }

  if (!r27)
    sub8029f4a8(mgr);
}

void CEnergyBall::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::Registered) {
    SetMaterialFilter(CMaterialFilter::MakeInclude({EMaterialTypes::Player}));
    RemoveMaterial(EMaterialTypes::Solid, mgr);
  }
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
}

void CEnergyBall::Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState /* state */) {
  CHealthInfo* hInfo = HealthInfo(mgr);
  if (hInfo && hInfo->GetHP() > 0.f) {
    CPatterned::Death(mgr, direction, EScriptObjectState::Any);
  } else {
    CPatterned::Death(mgr, direction, EScriptObjectState::DeathRattle);
  }
}

void CEnergyBall::Generate(CStateManager& mgr, EStateMsg msg, float /*arg*/) {
  if (msg == EStateMsg::Activate || msg == EStateMsg::Update) {
    if (msg == EStateMsg::Activate)
      x32c_animState = EAnimState::Ready;

    TryGenerateDeactivate(mgr, 0);

    if (!x450_bodyController->GetActive())
      x450_bodyController->Activate(mgr);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}

void CEnergyBall::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Update) {
    zeus::CVector3f seekPos = x568_steeringBehaviors.Seek(*this, mgr.GetPlayer().GetEyePosition());
    x450_bodyController->FaceDirection3D(seekPos, GetTransform().basis[1], arg);
  }
}
void CEnergyBall::sub8029f4a8(CStateManager& mgr) {}
}