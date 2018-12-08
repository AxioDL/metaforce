#include "CFlickerBat.hpp"
#include "CStateManager.hpp"
#include "Camera/CCameraManager.hpp"
#include "Camera/CGameCamera.hpp"
#include "Collision/CGameCollision.hpp"
#include "World/CPlayer.hpp"
#include "TCastTo.hpp"

namespace urde::MP1 {

CFlickerBat::CFlickerBat(TUniqueId uid, std::string_view name, CPatterned::EFlavorType flavor, const CEntityInfo& info,
                         const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                         EColliderType colType, bool b1, const CActorParameters& actParms, bool b2)
: CPatterned(ECharacter::FlickerBat, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer,
             colType, EBodyType::Pitchable, actParms, EKnockBackVariant::Small)
, x580_24_(false)
, x580_25_heardShot(false)
, x580_26_(false)
, x580_27_(b2)
, x574_state(EFlickerBatState(b1)) {

  SetupPlayerCollision(b1);
  x3d8_xDamageThreshold = 0.f;
  x402_27_noXrayModel = false;
}

void CFlickerBat::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CFlickerBat::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);

  if (msg == EScriptObjectMessage::Registered) {
    RemoveMaterial(EMaterialTypes::Solid, mgr);
    /* TODO: Implement xf3c_ in CStateManager (skipping 801311B8 - 80131224) */
    x450_bodyController->Activate(mgr);
    x450_bodyController->BodyStateInfo().SetMaximumPitch(zeus::degToRad(60.f));
  } else if (msg == EScriptObjectMessage::Deleted) {
    /* sub80125D88(mgr.xf3c_, uid) */
  }
}

void CFlickerBat::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  x402_29_drawParticles = mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::EPlayerVisor::XRay;

  if (GetFlickerBatState() == EFlickerBatState::Two || GetFlickerBatState() == EFlickerBatState::Three) {
    x578_ -= dt;
    if (x578_ <= 0.f) {
      if (GetFlickerBatState() == EFlickerBatState::Two)
        SetFlickerBatState(EFlickerBatState::Zero, mgr);
      else
        SetFlickerBatState(EFlickerBatState::One, mgr);
    }
  }

  CPatterned::Think(dt, mgr);
}

void CFlickerBat::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }

void CFlickerBat::Touch(CActor& act, CStateManager& mgr) { CPatterned::Touch(act, mgr); }

void CFlickerBat::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void CFlickerBat::Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) {
  SetFlickerBatState(EFlickerBatState::Zero, mgr);
  CPatterned::Death(mgr, direction, state);
}

bool CFlickerBat::CanBeShot(CStateManager& mgr, int) {
  return (GetFlickerBatState() == EFlickerBatState::Zero || GetFlickerBatState() == EFlickerBatState::Two ||
          mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::XRay);
}

void CFlickerBat::Patrol(CStateManager& mgr, EStateMsg state, float dt) { CPatterned::Patrol(mgr, state, dt); }

void CFlickerBat::Attack(CStateManager&, EStateMsg, float) {}

void CFlickerBat::Shuffle(CStateManager&, EStateMsg, float) {}

void CFlickerBat::Taunt(CStateManager&, EStateMsg, float) {}

bool CFlickerBat::InPosition(CStateManager& mgr, float arg) {
  return GetTransform().frontVector().dot(mgr.GetPlayer().GetAimPosition(mgr, 0.f) - GetTranslation()) > 0.f;
}

bool CFlickerBat::HearShot(CStateManager&, float) {
  if (x580_25_heardShot) {
    x580_25_heardShot = false;
    return true;
  }

  return false;
}

void CFlickerBat::SetFlickerBatState(EFlickerBatState state, CStateManager& mgr) {
  if (state == x574_state)
    return;

  FlickerBatStateChanged(state, mgr);
  x574_state = state;
}

void CFlickerBat::FlickerBatStateChanged(EFlickerBatState state, CStateManager& mgr) {
  if (state == EFlickerBatState::Zero) {
    if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::XRay)
      CreateShadow(true);

    AddMaterial(EMaterialTypes::Target, mgr);
  } else if (state == EFlickerBatState::One) {
    SetMuted(true);
    RemoveMaterial(EMaterialTypes::Target, mgr);
  } else if (state == EFlickerBatState::Two) {
    if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::XRay) {
      CreateShadow(true);
      SetMuted(false);
    }

    CheckStaticIntersection(mgr);
    SetupPlayerCollision(true);
  } else if (state == EFlickerBatState::Three) {
    if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::XRay)
      CreateShadow(true);

    CheckStaticIntersection(mgr);
    SetupPlayerCollision(false);
  }
}

void CFlickerBat::CheckStaticIntersection(CStateManager& mgr) {
  if (!x580_27_) {
    x580_26_ = false;
    return;
  }

  zeus::CVector3f camPos = mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTranslation();
  zeus::CVector3f diff = GetBoundingBox().center() - camPos;
  float mag = diff.magnitude();
  diff *= zeus::CVector3f(1.f / mag);
  x580_26_ = CGameCollision::RayStaticIntersectionBool(mgr, camPos, diff, mag,
                                                       CMaterialFilter::MakeExclude({EMaterialTypes::SeeThrough}));
}

} // namespace urde::MP1