#include "Runtime/MP1/World/CFlickerBat.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Camera/CCameraManager.hpp"
#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/Collision/CGameCollision.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {

CFlickerBat::CFlickerBat(TUniqueId uid, std::string_view name, CPatterned::EFlavorType flavor, const CEntityInfo& info,
                         const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                         EColliderType colType, bool startsHidden, const CActorParameters& actParms,
                         bool enableLineOfSight)
: CPatterned(ECharacter::FlickerBat, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer,
             colType, EBodyType::Pitchable, actParms, EKnockBackVariant::Small)
, x574_state(EFlickerBatState(startsHidden))
, x580_27_enableLOSCheck(enableLineOfSight) {
  SetupPlayerCollision(startsHidden);
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

  if (GetFlickerBatState() == EFlickerBatState::FadeIn || GetFlickerBatState() == EFlickerBatState::FadeOut) {
    x578_fadeRemTime -= dt;
    if (x578_fadeRemTime <= 0.f) {
      if (GetFlickerBatState() == EFlickerBatState::FadeIn)
        SetFlickerBatState(EFlickerBatState::Visible, mgr);
      else
        SetFlickerBatState(EFlickerBatState::Hidden, mgr);
    }
  }

  bool inXray = mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::XRay;
  if (inXray != x580_24_wasInXray) {
    if (inXray) {
      if (GetFlickerBatState() == EFlickerBatState::Hidden) {
        AddMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
        SetMuted(false);
      }
      CreateShadow(false);
    } else {
      if (GetFlickerBatState() == EFlickerBatState::Hidden) {
        RemoveMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
        SetMuted(true);
      }
      CreateShadow(true);
    }
    x580_24_wasInXray = inXray;
  }

  float alpha = 0.f;
  if (!x580_24_wasInXray) {
    if (GetFlickerBatState() == EFlickerBatState::Visible)
      alpha = 1.f;
    else if (GetFlickerBatState() == EFlickerBatState::FadeIn || GetFlickerBatState() == EFlickerBatState::FadeOut) {
      alpha = x578_fadeRemTime * x57c_ooFadeDur;
      if (GetFlickerBatState() == EFlickerBatState::FadeIn)
        alpha = 1.f - alpha;
    }
  } else
    alpha = 1.f;

  x42c_color.a() = alpha;
  x94_simpleShadow->SetUserAlpha(alpha);

  xe7_31_targetable = (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::XRay
                           ? true
                           : x574_state == EFlickerBatState::Visible || x574_state == EFlickerBatState::FadeIn);
  CPatterned::Think(dt, mgr);
}

void CFlickerBat::Render(CStateManager& mgr) {
  if (!x580_24_wasInXray && x580_26_inLOS &&
      (GetFlickerBatState() == EFlickerBatState::FadeIn || GetFlickerBatState() == EFlickerBatState::FadeOut)) {
    float strength = 0.f;
    if (GetFlickerBatState() == EFlickerBatState::FadeIn) {
      strength = 4.f * (x578_fadeRemTime - .75f);
    } else if (GetFlickerBatState() == EFlickerBatState::FadeOut) {
      strength = 4.f * x578_fadeRemTime;
    }
    if (strength > 0.f && strength < 1.f)
      mgr.DrawSpaceWarp(GetTranslation(), 0.3f * std::sin(M_PIF * strength));
  }

  if (x580_26_inLOS) {
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
  SetFlickerBatState(EFlickerBatState::Visible, mgr);
  SetMuted(false);
  CPatterned::Death(mgr, direction, state);
}

bool CFlickerBat::CanBeShot(const CStateManager& mgr, int) {
  return (GetFlickerBatState() == EFlickerBatState::Visible || GetFlickerBatState() == EFlickerBatState::FadeIn ||
          mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::XRay);
}

void CFlickerBat::Patrol(CStateManager& mgr, EStateMsg state, float dt) {
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
  if (state == EFlickerBatState::Visible) {
    if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::XRay)
      CreateShadow(true);

    AddMaterial(EMaterialTypes::Target, mgr);
  } else if (state == EFlickerBatState::Hidden) {
    SetMuted(true);
    RemoveMaterial(EMaterialTypes::Target, mgr);
  } else if (state == EFlickerBatState::FadeIn) {
    if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::XRay) {
      CreateShadow(true);
      SetMuted(false);
    }

    CheckStaticIntersection(mgr);
    SetupPlayerCollision(true);
  } else if (state == EFlickerBatState::FadeOut) {
    if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::XRay)
      CreateShadow(true);

    CheckStaticIntersection(mgr);
    SetupPlayerCollision(false);
  }
}

void CFlickerBat::CheckStaticIntersection(CStateManager& mgr) {
  if (!x580_27_enableLOSCheck) {
    x580_26_inLOS = false;
    return;
  }

  zeus::CVector3f camPos = mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTranslation();
  zeus::CVector3f diff = GetBoundingBox().center() - camPos;
  float mag = diff.magnitude();
  diff *= zeus::CVector3f(1.f / mag);
  x580_26_inLOS = CGameCollision::RayStaticIntersectionBool(mgr, camPos, diff, mag,
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
  if (GetFlickerBatState() == EFlickerBatState::Visible || GetFlickerBatState() == EFlickerBatState::FadeIn)
    SetFlickerBatState(EFlickerBatState::FadeOut, mgr);
  else
    SetFlickerBatState(EFlickerBatState::FadeIn, mgr);

  x578_fadeRemTime = 1.f;
  x57c_ooFadeDur = 1.f / x578_fadeRemTime;
}
} // namespace urde::MP1