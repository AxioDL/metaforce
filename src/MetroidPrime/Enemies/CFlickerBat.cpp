#include "MetroidPrime/Enemies/CFlickerBat.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CSimpleShadow.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

CFlickerBat::CFlickerBat(const TUniqueId uid, const rstl::string& name, const EFlavorType flavor,
                         const CEntityInfo& info, const CTransform4f& xf, const CModelData& mData,
                         const CPatternedInfo& pInfo, const EColliderType colType,
                         const bool startsVisible, const CActorParameters& actParms,
                         const bool enableLineOfSight)
: CPatterned(kC_FlickerBat, uid, name, flavor, info, xf, mData, pInfo, kMT_Flyer, colType,
             kBT_Pitchable, actParms, kCS_Small)
, x568_(0.f)
, x56c_(0.f)
, x570_(0.f)
, mState(startsVisible ? kFBS_Visible : kFBS_Hidden)
, mFadeRemTime(1.f)
, mFadeDuration(0.f)
, mWasInXray(false)
, mHeardShot(false)
, mInLOS(false)
, mEnableLOSCheck(enableLineOfSight) {
  SetupPlayerCollision(startsVisible);
  mXDamageThreshold = 0.f;
  mNoXrayModel = false;
}

CFlickerBat::~CFlickerBat() {}

ENTITY_ACCEPT_IMPL(CFlickerBat)

void CFlickerBat::AcceptScriptMsg(const EScriptObjectMessage msg, const TUniqueId uid,
                                  CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_Registered:
    RemoveMaterial(kMT_Solid, mgr);
    mgr.AddActiveFlickerBat(GetUniqueId());
    BodyCtrl()->Activate(mgr);
    BodyCtrl()->BodyStateInfo().SetMaximumPitch(CMath::Deg2Rad(60.f));
    break;
  case kSM_Deleted:
    mgr.RemoveActiveFlickerBat(GetUniqueId());
    break;
  default:
    break;
  }
}

void CFlickerBat::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                  const EUserEventType type, const float dt) {

  bool skipCall = false;
  switch (type) {
  case kUE_FadeIn:
    skipCall = true;
    ToggleVisible(mgr);
    break;
  default:
    break;
  }

  if (!skipCall) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CFlickerBat::ToggleVisible(CStateManager& mgr) {
  if (GetFlickerBatState() == kFBS_Visible || GetFlickerBatState() == kFBS_FadeIn) {
    SetFlickerBatState(kFBS_FadeOut, mgr);
  } else {
    SetFlickerBatState(kFBS_FadeIn, mgr);
  }

  mFadeRemTime = 1.f;
  mFadeDuration = 1.f / mFadeRemTime;
}

void CFlickerBat::SetFlickerBatState(const EFlickerBatState state, CStateManager& mgr) {
  if (state == mState) {
    return;
  }
  FlickerBatStateChanged(state, mgr);
  mState = state;
}

CFlickerBat::EFlickerBatState CFlickerBat::GetFlickerBatState() const { return mState; }

void CFlickerBat::FlickerBatStateChanged(const EFlickerBatState state, CStateManager& mgr) {
  switch (state) {
  case kFBS_Visible:
    if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::kPV_XRay) {
      SetDrawShadow(true);
    }

    AddMaterial(kMT_Target, mgr);
    break;
  case kFBS_Hidden:
    SetMuted(true);
    RemoveMaterial(kMT_Target, mgr);
    break;
  case kFBS_FadeIn:
    if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::kPV_XRay) {
      SetDrawShadow(true);
      SetMuted(false);
    }

    CheckFadeEffect(mgr);
    SetupPlayerCollision(true);
    break;
  case kFBS_FadeOut:
    if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::kPV_XRay) {
      SetDrawShadow(false);
    }
    CheckFadeEffect(mgr);
    SetupPlayerCollision(false);
    break;
  }
}

void CFlickerBat::CheckFadeEffect(CStateManager& mgr) {
  if (!mEnableLOSCheck) {
    mInLOS = false;
    return;
  }

  const CVector3f camPos = mgr.GetCameraManager()->GetCurrentCamera(mgr).GetTranslation();
  CVector3f diff = GetBoundingBox().GetCenterPoint() - camPos;
  const float mag = diff.Magnitude();
  const float invMag = 1.f / mag;
  diff *= invMag;
  mInLOS = CGameCollision::RayStaticIntersectionBool(
      mgr, camPos, diff, mag, CMaterialFilter::MakeExclude(CMaterialList(kMT_SeeThrough)));
}

void CFlickerBat::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  SetFlickerBatState(kFBS_Visible, mgr);
  SetMuted(false);
  CPatterned::Death(mgr, direction, state);
}

void CFlickerBat::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  mDrawParticles = mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_XRay;

  if (GetFlickerBatState() == kFBS_FadeIn || GetFlickerBatState() == kFBS_FadeOut) {
    mFadeRemTime -= dt;

    if (mFadeRemTime <= 0.f) {
      if (GetFlickerBatState() == kFBS_FadeIn) {
        SetFlickerBatState(kFBS_Visible, mgr);
      } else {
        SetFlickerBatState(kFBS_Hidden, mgr);
      }
    }
  }
  const bool inXRay = mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_XRay;

  if (inXRay != mWasInXray) {
    if (inXRay) {
      if (GetFlickerBatState() == kFBS_Hidden) {
        AddMaterial(kMT_Target, kMT_Orbit, mgr);
        SetMuted(false);
      }

      SetDrawShadow(false);
    } else if (GetFlickerBatState() == kFBS_Hidden) {
      RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
      SetMuted(true);
    } else {
      SetDrawShadow(true);
    }
    mWasInXray = inXRay;
  }

  float alpha = 0.f;
  if (mWasInXray || GetFlickerBatState() == kFBS_Visible) {
    alpha = 1.f;
  } else if (GetFlickerBatState() == kFBS_FadeIn || GetFlickerBatState() == kFBS_FadeOut) {
    alpha = mFadeRemTime * mFadeDuration;
    if (GetFlickerBatState() == kFBS_FadeIn) {
      alpha = 1.f - alpha;
    }
  }

  mColor.SetAlpha(alpha);
  Shadow()->SetUserAlpha(alpha);

  SetTargetable(mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_XRay ||
                (IsVisible() && mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan));

  CPatterned::Think(dt, mgr);
}

void CFlickerBat::Touch(CActor& act, CStateManager& mgr) {
  CPlayer* player = TCastToPtr< CPlayer >(act);

  if (player && mCurDamageRemTime <= 0.f) {
    mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
                    CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()));
    mCurDamageRemTime = mDamageWaitTime;
  }

  CPatterned::Touch(act, mgr);
}

bool CFlickerBat::CanBeShot(const CStateManager& mgr, int) {
  return GetFlickerBatState() == kFBS_Visible || GetFlickerBatState() == kFBS_FadeIn ||
         mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_XRay;
}

void CFlickerBat::NotifyNeighbors(CStateManager& mgr) {
  CVector3f translation = GetTranslation();
  rstl::list< TUniqueId >& flickBats = mgr.GetActiveFlickerBats();
  for (AUTO(iter, flickBats.begin()); iter != flickBats.end();) {
    CFlickerBat* flick = PATTERNED_CAST_TO(CFlickerBat, mgr.ObjectById(*iter));
    if (flick == nullptr) {
      iter = flickBats.erase(iter);
    } else {
      if ((translation - flick->GetTranslation()).MagSquared() < 100.f) {
        flick->SetHeardShot(true);
      }
      ++iter;
    }
  }
}

void CFlickerBat::SetHeardShot(const bool heardShot) { mHeardShot = heardShot; }

bool CFlickerBat::HearShot(CStateManager&, float) {
  if (mHeardShot) {
    mHeardShot = false;
    return true;
  }

  return false;
}

void CFlickerBat::Taunt(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    NotifyNeighbors(mgr);
    SetWasHit(false);
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CFlickerBat::Shuffle(CStateManager& mgr, const EStateMsg msg, const float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    CRandom16* rnd = mgr.Random();
    const CVector3f destPos =
        GetTranslation() + CVector3f(100.f * rnd->Float() - 50.f, 100.f * rnd->Float() - 50.f,
                                     100.f * rnd->Float() - 50.f);
    SetDestPos(destPos);
    break;
  }
  case kStateMsg_Update:
    ApproachDest(mgr);
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CFlickerBat::Patrol(CStateManager& mgr, const EStateMsg msg, const float arg) {
  CPatterned::Patrol(mgr, msg, arg);
  BodyCtrl()->CommandMgr().DeliverTargetVector((mDestPos - GetTranslation()).AsNormalized());
}

void CFlickerBat::Render(const CStateManager& mgr) const {
  if (!mWasInXray && mInLOS && (mState == kFBS_FadeIn || mState == kFBS_FadeOut)) {
    float strength = 0.f;
    switch (mState) {
    case kFBS_FadeIn:
      strength = 4.f * (mFadeRemTime - .75f);
      break;
    case kFBS_FadeOut:
      strength = 4.f * mFadeRemTime;
      break;
    }
    if (strength > 0.f && strength < 1.f) {
      strength = CMath::FastSinR(strength * M_PIF) * 0.3f;
      mgr.DrawSpaceWarp(GetTranslation(), strength);
    }
  }

  if (mWasInXray) {
    TAreaId areaId = GetCurrentAreaId();
    mgr.SetupFogForArea3XRange(areaId);
    CPatterned::Render(mgr);
    mgr.SetupFogForArea(areaId);
    return;
  }
  CPatterned::Render(mgr);
}

bool CFlickerBat::InPosition(CStateManager& mgr, float) {
  const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
  const CVector3f diff = aimPos - GetTranslation();
  return CVector3f::Dot(GetTransform().GetForward(), diff) > 0.f;
}

void CFlickerBat::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    break;
  case kStateMsg_Update: {
    const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    const CVector3f translation = GetTranslation();
    const CVector3f dir = (aimPos - translation);
    BodyCtrl()->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(dir.AsNormalized(), CVector3f::Zero(), 1.f));
  } break;
  case kStateMsg_Deactivate:
    break;
  }
}
