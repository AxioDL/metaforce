#include "Runtime/MP1/World/CThardusRockProjectile.hpp"

#include "Runtime/Camera/CCameraManager.hpp"
#include "Runtime/Camera/CFirstPersonCamera.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/Collision/CJointCollisionDescription.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "Runtime/MP1/World/CThardus.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path
namespace urde::MP1 {
namespace {
constexpr std::array<SSphereJointInfo, 1> skRockCollisions{{
    {"Rock_01_Collision_LCTR", 1.5f},
}};
} // namespace

CThardusRockProjectile::CThardusRockProjectile(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                               const zeus::CTransform& xf, CModelData&& modelData,
                                               const CActorParameters& aParms, const CPatternedInfo& patternedInfo,
                                               std::vector<std::unique_ptr<CModelData>>&& mDataVec,
                                               CAssetId stateMachine, float)
: CPatterned(ECharacter::ThardusRockProjectile, uid, name, EFlavorType::Zero, info, xf, std::move(modelData),
             patternedInfo, EMovementType::Flyer, EColliderType::One, EBodyType::Flyer, aParms,
             EKnockBackVariant::Medium)
, x57c_(std::move(mDataVec)) {}

void CThardusRockProjectile::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  CPatterned::Think(dt, mgr);
}

void CThardusRockProjectile::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId other, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, other, mgr);
}

void CThardusRockProjectile::Render(CStateManager& mgr) { CPatterned::Render(mgr); }

void CThardusRockProjectile::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg != EStateMsg::Update) {
    return;
  }

  GetBodyController()->GetCommandMgr().DeliverCmd(
      CBCLocomotionCmd(zeus::skZero3f, (mgr.GetPlayer().GetTranslation() - GetTranslation()).normalized(), 1.f));
}

void CThardusRockProjectile::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg != EStateMsg::Activate) {
    return;
  }

  mgr.FreeScriptObject(GetUniqueId());
  SendScriptMsgs(EScriptObjectState::MassiveDeath, mgr, EScriptObjectMessage::None);
  GenerateDeathExplosion(mgr);
}

void CThardusRockProjectile::LoopedAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x5a4_ = true;
  } else if (msg == EStateMsg::Update) {
    zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
    if (!x5bc_ || (aimPos - GetTranslation()).magSquared() <= x5c0_ * x5c0_) {
      x5bc_ = false;
    } else {
      x5b0_ = x45c_steeringBehaviors.Arrival(*this, aimPos, 0.f);
      x5bc_ = true;
    }

    zeus::CVector3f movePos = x5b0_;
    float radius = skRockCollisions[0].radius;
    auto result = mgr.RayStaticIntersection(GetTranslation(), zeus::skDown, 100.f,
                                            CMaterialFilter::MakeInclude({EMaterialTypes::Solid}));

    if (result.IsValid()) {
      movePos = (x45c_steeringBehaviors.Separation(*this, result.GetPoint(), 2.f * radius) + x5b0_).normalized();
    }
    GetBodyController()->GetCommandMgr().DeliverCmd(CBCLocomotionCmd{movePos, zeus::skZero3f, 1.f});
  }
}

void CThardusRockProjectile::GetUp(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x574_ = EAnimState::NotReady;
  } else if (msg == EStateMsg::Update) {
    auto result = mgr.RayStaticIntersection(GetTranslation(), zeus::skDown, 2.f,
                                            CMaterialFilter::MakeInclude({EMaterialTypes::Solid}));
    if (result.IsInvalid()) {
      CThardus* thardus = static_cast<CThardus*>(mgr.ObjectById(x5d0_thardusId));
      if (thardus != nullptr && !x5dc_) {
        x5dc_ = true;
        sub80203824(mgr, x5cc_, result.GetPoint(), GetModelData()->GetScale(), 0);
      }
    } else if (mgr.GetCameraManager()->GetCurrentCameraId() == mgr.GetCameraManager()->GetFirstPersonCamera()->GetUniqueId()) {

    }
  }
}

void CThardusRockProjectile::Lurk(CStateManager& mgr, EStateMsg msg, float dt) { CAi::Lurk(mgr, msg, dt); }

bool CThardusRockProjectile::Delay(CStateManager& mgr, float arg) { return x5a8_ < x330_stateMachineState.GetTime(); }

bool CThardusRockProjectile::AnimOver(CStateManager& mgr, float arg) { return x574_ == EAnimState::Over; }

bool CThardusRockProjectile::ShouldAttack(CStateManager& mgr, float arg) {
  if (x5ac_ < x330_stateMachineState.GetTime() && x56c_ != 3) {
    x56c_ = 2;
    return true;
  }

  return false;
}

bool CThardusRockProjectile::HitSomething(CStateManager& mgr, float arg) { return x572_; }

bool CThardusRockProjectile::ShouldMove(CStateManager& mgr, float arg) { return x56c_ != 0; }
} // namespace urde::MP1
