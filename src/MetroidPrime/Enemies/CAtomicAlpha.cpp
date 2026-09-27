#include "MetroidPrime/Enemies/CAtomicAlpha.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "float.h"
#include "rstl/math.hpp"

CAtomicAlpha::CAtomicAlpha(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf, const CModelData& mData,
                           const CActorParameters& actParms, const CPatternedInfo& pInfo,
                           CAssetId bombWeapon, const CDamageInfo& bombDamage, float bombDropDelay,
                           float bombReappearDelay, float bombRappearTime, CAssetId cmdl,
                           bool invisible, bool applyBeamAttraction)
: CPatterned(kC_AtomicAlpha, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Flyer, kCT_One,
             kBT_Flyer, actParms, kCS_Medium)
, mInRange(false)
, mInvisible(invisible)
, mApplyBeamAttraction(applyBeamAttraction)
, mBombDropDelay(bombDropDelay)
, mBombReappearDelay(bombReappearDelay)
, mBombRappearTime(bombRappearTime)
, mBombTime(0.f)
, mCurBomb(0)
, mPathFind(NULL, 3, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, mSteeringBehaviors()
, mBombProjectile(bombWeapon, bombDamage)
, mBombModel(CStaticRes(cmdl, mData.ScaleCopy())) {
  mBombProjectile.Token().Lock();
  mBombLocators.push_back(SBomb(rstl::string_l("bomb1_LCTR"), pas::kLT_Internal10, FLT_MAX));
  mBombLocators.push_back(SBomb(rstl::string_l("bomb2_LCTR"), pas::kLT_Internal11, FLT_MAX));
  mBombLocators.push_back(SBomb(rstl::string_l("bomb3_LCTR"), pas::kLT_Internal12, FLT_MAX));
  mBombLocators.push_back(SBomb(rstl::string_l("bomb4_LCTR"), pas::kLT_Internal13, FLT_MAX));
}

ENTITY_ACCEPT_IMPL(CAtomicAlpha)

void CAtomicAlpha::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case kSM_Registered:
    BodyCtrl()->Activate(mgr);
    break;
  case kSM_AddSplashInhabitant:
    if (IsAlive()) {
      mPendingDeath = true;
    }
    break;
  case kSM_InitializedInArea:
    const TAreaId aid = GetCurrentAreaId();
    mPathFind.SetArea(mgr.GetWorld()->GetArea(aid)->GetPostConstructed()->mPathArea);
    break;
  }
}

void CAtomicAlpha::Think(const float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);

  if (!GetActive()) {
    return;
  }

  mBombTime += dt;

  for (int i = 0; i < mBombLocators.size(); ++i) {
    mBombLocators[i].mScaleTime += dt;
  }
}

void CAtomicAlpha::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                                CStateManager& mgr) {
  if (IsAlive()) {
    if (TCastToConstPtr< CPlayer >(mgr.GetObjectById(id))) {
      if (mCurDamageRemTime <= 0.f) {
        mgr.PlayerState()->StaticInterference().AddSource(GetUniqueId(), 0.5f, 0.25f);

        for (int i = 0; i < mBombLocators.size(); ++i) {
          mBombLocators[i].mScaleTime = 0.f;
        }
      }
    }
  }

  CPatterned::CollidedWith(id, list, mgr);
}

void CAtomicAlpha::Render(const CStateManager& mgr) const {
  const bool xrayActive = mgr.GetPlayerState()->IsXRayActive(mgr);
  if (mInvisible && !xrayActive) {
    return;
  }
  CPatterned::Render(mgr);

  for (int i = 0; i < mBombLocators.size(); ++i) {
    const float scale = rstl::min_val(
        rstl::max_val(0.f, mBombLocators[i].mScaleTime - mBombReappearDelay) /
            mBombReappearDelay,
        1.f);

    const CTransform4f locatorXf = GetTransform() *
                                   GetScaledLocatorTransform(mBombLocators[i].mLocatorName) *
                                   CTransform4f::Scale(scale);
    mBombModel.Render(mgr, locatorXf, GetActorLights(), CModelFlags::Normal());
  }
}

void CAtomicAlpha::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  const bool xrayActive = mgr.GetPlayerState()->IsXRayActive(mgr);
  if (mInvisible && !xrayActive) {
    return;
  }

  CPatterned::AddToRenderer(frustum, mgr);
}

void CAtomicAlpha::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  CPatterned::Patrol(mgr, msg, arg);
  switch (msg) {
  case kStateMsg_Activate:
    mBombTime = 0.f;
    break;
  case kStateMsg_Update:
    if (mInRange) {
      if (CanDropBomb()) {
        BodyCtrl()->SetLocomotionType(mBombLocators[mCurBomb].mLocomotionType);
      } else {
        BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
      }

      if (Leash(mgr, arg)) {
        mInRange = false;
      }
    } else {
      BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
      if (InMaxRange(mgr, arg)) {
        mInRange = true;
      }
    }
    break;
  case kStateMsg_Deactivate:
    mInRange = false;
    break;
  }
}

bool CAtomicAlpha::Leash(CStateManager& mgr, float arg) {
  const CVector3f diff = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  return diff.MagSquared() > mPlayerLeashRadius * mPlayerLeashRadius &&
         mCurPlayerLeashTime > mPlayerLeashTime;
}

void CAtomicAlpha::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                   EUserEventType type, float dt) {

  bool skip = false;
  switch (type) {
  case kUE_Projectile:
    const CTransform4f lctrXf = GetLctrTransform(node.GetLocatorName());
    const CVector3f origin = lctrXf.GetTranslation();
    const CTransform4f xf = CTransform4f::LookAt(origin, origin + CVector3f::Down());
    LaunchProjectile(xf, mgr, 4, CWeapon::kPA_None, false, rstl::optional_object_null(),
                     CSfxManager::kInternalInvalidSfxId, false, CVector3f(1.f, 1.f, 1.f));
    mBombTime = 0.f;
    mBombLocators[mCurBomb].mScaleTime = 0.f;
    mCurBomb = (mCurBomb + 1) % mBombLocators.size();
    skip = true;
    break;
  }

  if (!skip) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

bool CAtomicAlpha::AggressionCheck(CStateManager& mgr, float arg) {
  const CPlayerGun* gun = mgr.GetPlayer()->GetPlayerGun();
  if (mApplyBeamAttraction) {
    const float factor = gun->GetChargePercentage();
    if (factor > 0.1f) {
      return true;
    }
  }
  return false;
}

void CAtomicAlpha::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Internal8);
    break;
  case kStateMsg_Update: {
    const CVector3f playerEyePos = mgr.GetPlayer()->GetEyePosition();
    const CVector3f seekVec = mSteeringBehaviors.Seek(*this, playerEyePos);
    BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(seekVec, CVector3f::Zero(), 1.f));
  } break;
  case kStateMsg_Deactivate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    break;
  }
}

EWeaponCollisionResponseTypes CAtomicAlpha::GetCollisionResponseType(const CVector3f&,
                                                                     const CVector3f&,
                                                                     const CWeaponMode& wMode,
                                                                     int attrib) const {
  return GetDamageVulnerability()->WeaponHits(wMode, CDamageVulnerability::kRD_No)
             ? kWCR_AtomicAlpha
             : kWCR_AtomicAlphaReflect;
}
