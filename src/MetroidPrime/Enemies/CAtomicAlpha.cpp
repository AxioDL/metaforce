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
, x568_24_inRange(false)
, x568_25_invisible(invisible)
, x568_26_applyBeamAttraction(applyBeamAttraction)
, x56c_bombDropDelay(bombDropDelay)
, x570_bombReappearDelay(bombReappearDelay)
, x574_bombRappearTime(bombRappearTime)
, x578_bombTime(0.f)
, x57c_curBomb(0)
, x580_pathFind(NULL, 3, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x664_steeringBehaviors()
, x668_bombProjectile(bombWeapon, bombDamage)
, x690_bombModel(CStaticRes(cmdl, mData.ScaleCopy())) {
  x668_bombProjectile.Token().Lock();
  x6dc_bombLocators.push_back(SBomb(rstl::string_l("bomb1_LCTR"), pas::kLT_Internal10, FLT_MAX));
  x6dc_bombLocators.push_back(SBomb(rstl::string_l("bomb2_LCTR"), pas::kLT_Internal11, FLT_MAX));
  x6dc_bombLocators.push_back(SBomb(rstl::string_l("bomb3_LCTR"), pas::kLT_Internal12, FLT_MAX));
  x6dc_bombLocators.push_back(SBomb(rstl::string_l("bomb4_LCTR"), pas::kLT_Internal13, FLT_MAX));
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
      x401_30_pendingDeath = true;
    }
    break;
  case kSM_InitializedInArea:
    const TAreaId aid = GetCurrentAreaId();
    x580_pathFind.SetArea(mgr.GetWorld()->GetArea(aid)->GetPostConstructed()->x10bc_pathArea);
    break;
  }
}

void CAtomicAlpha::Think(const float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);

  if (!GetActive()) {
    return;
  }

  x578_bombTime += dt;

  for (int i = 0; i < x6dc_bombLocators.size(); ++i) {
    x6dc_bombLocators[i].x14_scaleTime += dt;
  }
}

void CAtomicAlpha::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                                CStateManager& mgr) {
  if (IsAlive()) {
    if (TCastToConstPtr< CPlayer >(mgr.GetObjectById(id))) {
      if (x420_curDamageRemTime <= 0.f) {
        mgr.PlayerState()->StaticInterference().AddSource(GetUniqueId(), 0.5f, 0.25f);

        for (int i = 0; i < x6dc_bombLocators.size(); ++i) {
          x6dc_bombLocators[i].x14_scaleTime = 0.f;
        }
      }
    }
  }

  CPatterned::CollidedWith(id, list, mgr);
}

void CAtomicAlpha::Render(const CStateManager& mgr) const {
  const bool xrayActive = mgr.GetPlayerState()->IsXRayActive(mgr);
  if (x568_25_invisible && !xrayActive) {
    return;
  }
  CPatterned::Render(mgr);

  for (int i = 0; i < x6dc_bombLocators.size(); ++i) {
    const float scale = rstl::min_val(
        rstl::max_val(0.f, x6dc_bombLocators[i].x14_scaleTime - x570_bombReappearDelay) /
            x570_bombReappearDelay,
        1.f);

    const CTransform4f locatorXf = GetTransform() *
                                   GetScaledLocatorTransform(x6dc_bombLocators[i].x0_locatorName) *
                                   CTransform4f::Scale(scale);
    x690_bombModel.Render(mgr, locatorXf, GetActorLights(), CModelFlags::Normal());
  }
}

void CAtomicAlpha::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  const bool xrayActive = mgr.GetPlayerState()->IsXRayActive(mgr);
  if (x568_25_invisible && !xrayActive) {
    return;
  }

  CPatterned::AddToRenderer(frustum, mgr);
}

void CAtomicAlpha::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  CPatterned::Patrol(mgr, msg, arg);
  switch (msg) {
  case kStateMsg_Activate:
    x578_bombTime = 0.f;
    break;
  case kStateMsg_Update:
    if (x568_24_inRange) {
      if (CanDropBomb()) {
        BodyCtrl()->SetLocomotionType(x6dc_bombLocators[x57c_curBomb].x10_locomotionType);
      } else {
        BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
      }

      if (Leash(mgr, arg)) {
        x568_24_inRange = false;
      }
    } else {
      BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
      if (InMaxRange(mgr, arg)) {
        x568_24_inRange = true;
      }
    }
    break;
  case kStateMsg_Deactivate:
    x568_24_inRange = false;
    break;
  }
}

bool CAtomicAlpha::Leash(CStateManager& mgr, float arg) {
  const CVector3f diff = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  return diff.MagSquared() > x3cc_playerLeashRadius * x3cc_playerLeashRadius &&
         x3d4_curPlayerLeashTime > x3d0_playerLeashTime;
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
    x578_bombTime = 0.f;
    x6dc_bombLocators[x57c_curBomb].x14_scaleTime = 0.f;
    x57c_curBomb = (x57c_curBomb + 1) % x6dc_bombLocators.size();
    skip = true;
    break;
  }

  if (!skip) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

bool CAtomicAlpha::AggressionCheck(CStateManager& mgr, float arg) {
  const CPlayerGun* gun = mgr.GetPlayer()->GetPlayerGun();
  if (x568_26_applyBeamAttraction) {
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
    const CVector3f seekVec = x664_steeringBehaviors.Seek(*this, playerEyePos);
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
