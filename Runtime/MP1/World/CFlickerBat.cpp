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
, x580_24_wasInXray(false)
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
    mgr.GetActiveFlickerBats().push_back(GetUniqueId());
    x450_bodyController->Activate(mgr);
    x450_bodyController->BodyStateInfo().SetMaximumPitch(zeus::degToRad(60.f));
  } else if (msg == EScriptObjectMessage::Deleted) {
    mgr.GetActiveFlickerBats().remove(GetUniqueId());
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

  bool inXray = mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::XRay;
  if (inXray != x580_24_wasInXray) {
    if (inXray) {
      if (GetFlickerBatState() == EFlickerBatState::One) {
        AddMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
        SetMuted(false);
      }
      CreateShadow(false);
    } else {
      if (GetFlickerBatState() == EFlickerBatState::One) {
        RemoveMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
        SetMuted(true);
      }
      CreateShadow(true);
    }
    x580_24_wasInXray = inXray;
  }

  float alpha = 0.f;
  if (!x580_24_wasInXray) {
    if (GetFlickerBatState() == EFlickerBatState::Zero)
      alpha = 1.f;
    else if (GetFlickerBatState() == EFlickerBatState::Two || GetFlickerBatState() == EFlickerBatState::Three) {
      alpha = x578_ * x57c_;
      if (GetFlickerBatState() == EFlickerBatState::Two)
        alpha = 1.f - alpha;
    }
  } else
    alpha = 1.f;

  x42c_color.a() = alpha;
  x94_simpleShadow->SetUserAlpha(alpha);

  bool targetable = true;
  if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::XRay &&
      (x574_state == EFlickerBatState::Zero || x574_state == EFlickerBatState::Two))
    targetable = false;
  xe7_31_targetable = targetable;
  CPatterned::Think(dt, mgr);
}

void CFlickerBat::Render(const CStateManager& mgr) const {
  if (!x580_24_wasInXray && x580_26_ &&
      (GetFlickerBatState() == EFlickerBatState::Two || GetFlickerBatState() == EFlickerBatState::Three)) {
    float strength = 0.f;
    if (GetFlickerBatState() == EFlickerBatState::Two) {
      strength = 4.f * (x578_ - .75f);
    } else if (GetFlickerBatState() == EFlickerBatState::Three) {
      strength = 4.f * x578_;
    }
    if (strength > 0.f && strength < 1.f)
      mgr.DrawSpaceWarp(GetTranslation(), 0.3f * std::sin(M_PIF * strength));
  }

  if (x580_26_) {
    mgr.SetupFogForAreaNonCurrent(GetAreaIdAlways());
    CPatterned::Render(mgr);
    mgr.SetupFogForArea(GetAreaIdAlways());
  } else
    CPatterned::Render(mgr);
}

void CFlickerBat::Touch(CActor& act, CStateManager& mgr) {
  if (TCastToPtr<CPlayer> pl = act) {
    if (x420_curDamageRemTime <= 0.f) {
      mgr.ApplyDamage(GetUniqueId(), pl->GetUniqueId(), GetUniqueId(), GetContactDamage(),
                      CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
      x420_curDamageRemTime = x424_damageWaitTime;
    }
  }
  CPatterned::Touch(act, mgr);
}

void CFlickerBat::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  if (type == EUserEventType::FadeIn)
    ToggleVisible(mgr);
  else
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

void CFlickerBat::Patrol(CStateManager& mgr, EStateMsg state, float dt)
{
  CPatterned::Patrol(mgr, state, dt);
  x450_bodyController->GetCommandMgr().DeliverFaceVector((x2e0_destPos - GetTranslation()).normalized());
}

void CFlickerBat::Attack(CStateManager&, EStateMsg msg, float) {
  if (msg == EStateMsg::Update) {
    x450_bodyController->GetCommandMgr().DeliverCmd(
      CBCLocomotionCmd((x2e0_destPos - GetTranslation()).normalized(), {}, 1.f));
  }
}

void CFlickerBat::Shuffle(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    CRandom16* rnd = mgr.GetActiveRandom();
    SetDestPos(GetTranslation() +
               zeus::CVector3f(100.f * rnd->Float() - 50.f, 100.f * rnd->Float() - 50.f, 100.f * rnd->Float() - 50.f));
  } else if (msg == EStateMsg::Update) {
    ApproachDest(mgr);
  }
}

void CFlickerBat::Taunt(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    NotifyNeighbors(mgr);
    x400_24_hitByPlayerProjectile = false;
  }
}

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

void CFlickerBat::NotifyNeighbors(CStateManager& mgr) {
  for (TUniqueId uid : mgr.GetActiveFlickerBats()) {
    if (CFlickerBat* flick = CPatterned::CastTo<CFlickerBat>(mgr.ObjectById(uid)))
      if ((GetTranslation() - flick->GetTranslation()).magnitude() < 100.f)
        flick->SetHeardShot(true);
  }
}

void CFlickerBat::ToggleVisible(CStateManager& mgr) {
  if (GetFlickerBatState() == EFlickerBatState::Zero || GetFlickerBatState() == EFlickerBatState::Two)
    SetFlickerBatState(EFlickerBatState::Three, mgr);
  else
    SetFlickerBatState(EFlickerBatState::Two, mgr);

  x578_ = 1.f;
  x57c_ = 1.f / x578_;
}
} // namespace urde::MP1