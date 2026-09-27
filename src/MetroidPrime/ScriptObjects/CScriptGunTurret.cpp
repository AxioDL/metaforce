#include "MetroidPrime/ScriptObjects/CScriptGunTurret.hpp"

#include "Collision/CRayCastResult.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Enemies/CAiFuncMap.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"

#include <Kyoto/Animation/CPASAnimParmData.hpp>
#include <Kyoto/Audio/CSfxManager.hpp>
#include <Kyoto/Basics/CCast.hpp>
#include <Kyoto/Math/CRelAngle.hpp>
#include <Kyoto/Particles/CElementGen.hpp>
#include <Kyoto/Particles/CGenDescription.hpp>

const uint CScriptGunTurret::skStateToLocoTypeLookup[] = {
    5, 7, 9, 0, 1, 0, 1, 2, 3, 1, 1, 1, 1,
};

const SBurst CScriptGunTurret::skBurst2InfoTemplate[] = {
    {3, {1, 2, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {3, {7, 6, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {4, {3, 5, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {60, {16, 4, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {30, {4, 4, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};

const SBurst CScriptGunTurret::skBurst3InfoTemplate[] = {
    {30, {4, 5, 4, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {30, {2, 3, 4, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {30, {3, 4, 5, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {5, {16, 1, 2, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {5, {8, 7, 6, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};

const SBurst CScriptGunTurret::skBurst4InfoTemplate[] = {
    {5, {16, 1, 2, 3, 0, 0, 0, 0}, 0.150000, 0.050000},
    {5, {9, 8, 7, 6, 0, 0, 0, 0}, 0.150000, 0.050000},
    {15, {2, 3, 4, 5, 0, 0, 0, 0}, 0.150000, 0.050000},
    {15, {5, 4, 3, 2, 0, 0, 0, 0}, 0.150000, 0.050000},
    {15, {10, 11, 4, 13, 0, 0, 0, 0}, 0.150000, 0.050000},
    {15, {14, 13, 4, 11, 0, 0, 0, 0}, 0.150000, 0.050000},
    {30, {2, 4, 4, 6, 0, 0, 0, 0}, 0.150000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};

const SBurst CScriptGunTurret::skOOVBurst2InfoTemplate[] = {
    {20, {16, 15, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {20, {8, 9, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {20, {13, 11, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {20, {2, 6, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {20, {3, 4, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};

const SBurst CScriptGunTurret::skOOVBurst3InfoTemplate[] = {
    {10, {14, 4, 10, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {10, {15, 13, 4, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {10, {9, 11, 4, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {35, {15, 13, 11, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {35, {9, 11, 13, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};

const SBurst CScriptGunTurret::skOOVBurst4InfoTemplate[] = {
    {10, {14, 13, 4, 11, 0, 0, 0, 0}, 0.150000, 0.050000},
    {30, {1, 15, 13, 11, 0, 0, 0, 0}, 0.150000, 0.050000},
    {20, {16, 15, 14, 13, 0, 0, 0, 0}, 0.150000, 0.050000},
    {10, {8, 9, 11, 4, 0, 0, 0, 0}, 0.150000, 0.050000},
    {10, {1, 15, 13, 4, 0, 0, 0, 0}, 0.150000, 0.050000},
    {20, {8, 9, 10, 11, 0, 0, 0, 0}, 0.150000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};

const int CScriptGunTurretData::skMinProperties = 43;
const float CScriptGunTurret::skExtensionOverlapMaxPer = 0.89999999f;
const char* const CScriptGunTurret::skGunLCTRName = "Gun_SDK";
const char* const CScriptGunTurret::skBlastLCTRName = "Blast_LCTR";
const char* const CScriptGunTurret::skLightLCTRName = "light_LCTR";
const char* const CScriptGunTurret::skLockonTargetLCTRName = "lockon_target_LCTR";

const SBurst* CScriptGunTurret::skBursts[] = {
    skBurst2InfoTemplate,
    skBurst3InfoTemplate,
    skBurst4InfoTemplate,
    skOOVBurst2InfoTemplate,
    skOOVBurst3InfoTemplate,
    skOOVBurst4InfoTemplate,
    nullptr,
};

CScriptGunTurretData::CScriptGunTurretData(CInputStream& in, const int propCount)
: mIntoDeactivateDelay(in.Get< float >())
, mIntoActivateDelay(in.Get< float >())
, mReloadTime(in.Get< float >())
, mReloadTimeVariance(in.Get< float >())
, mPanStartTime(in.Get< float >())
, mPanHoldTime(in.Get< float >())
, mTotalPanSearchTime(30.f)
, mLeftMaxAngle(CRelAngle::FromDegrees(in.Get< float >()).AsRadians())
, mRightMaxAngle(CRelAngle::FromDegrees(in.Get< float >()).AsRadians())
, mDownMaxAngle(CRelAngle::FromDegrees(in.Get< float >()).AsRadians())
, mTurnSpeed(CRelAngle::FromDegrees(in.Get< float >()).AsRadians())
, mDetectionRange(in.Get< float >())
, mDetectionZRange(in.Get< float >())
, mFreezeDuration(in.Get< float >())
, mFreezeVariance(in.Get< float >())
, mFreezeTimeout(propCount >= 48 ? in.Get< bool >() : false)
, mProjectileRes(in.Get< CAssetId >())
, mProjectileDamage(in)
, mIdleLightRes(in.Get< CAssetId >())
, mDeactivateLightRes(in.Get< CAssetId >())
, mTargettingLightRes(in.Get< CAssetId >())
, mFrozenEffectRes(in.Get< CAssetId >())
, mChargingEffectRes(in.Get< CAssetId >())
, mPanningEffectRes(in.Get< CAssetId >())
, mVisorEffectRes(propCount >= 44 ? in.Get< CAssetId >() : kInvalidAssetId)
, mTrackingSoundId(CSfxManager::TranslateSFXID(in.Get< int >()))
, mLockOnSoundId(CSfxManager::TranslateSFXID(in.Get< int >()))
, mUnfreezeSoundId(CSfxManager::TranslateSFXID(in.Get< int >()))
, mStopClankSoundId(CSfxManager::TranslateSFXID(in.Get< int >()))
, mChargingSoundId(CSfxManager::TranslateSFXID(in.Get< int >()))
, mVisorSoundId(propCount >= 45 ? CSfxManager::TranslateSFXID(in.Get< int >())
                                   : CSfxManager::kInternalInvalidSfxId)
, mExtensionModelResId(in.Get< CAssetId >())
, mExtensionDropDownDist(in.Get< float >())
, mNumInitialShots(in.Get< int >())
, mInitialShotTableIndex(in.Get< int >())
, mNumSubsequentShots(in.Get< int >())
, mFrenzyDuration(propCount >= 47 ? in.Get< float >() : 3.f)
, mScriptedStartOnly(propCount >= 46 ? in.Get< bool >() : false) {}

static const CMaterialList skTurretMaterialList(kMT_Character);
static const CMaterialList skGunMaterialList(kMT_Solid, kMT_Character, kMT_Orbit, kMT_Target);

CScriptGunTurret::CScriptGunTurret(TUniqueId uid, const rstl::string& name, ETurretComponent comp,
                                   const CEntityInfo& info, const CTransform4f& xf,
                                   const CModelData& mData, const CAABox& aabb,
                                   const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                                   const CActorParameters& aParms,
                                   const CScriptGunTurretData& turretData)
: CPhysicsActor(uid, true, name, info, xf, mData,
                comp == kTC_Base ? skTurretMaterialList : skGunMaterialList, aabb,
                SMoverData(1000.f), aParms, 0.3f, 0.1f)
, mType(comp)
, mGunId(kInvalidUniqueId)
, mLastGunHP(0.f)
, mHealthInfo(hInfo)
, mDamageVuln(dVuln)
, mData(turretData)
, x378_(kInvalidUniqueId)
, mProjectileInfo(turretData.GetProjectileRes(), turretData.GetProjectileDamage())
, mBurstFire(skBursts, 1)
, mTargetPosition(CVector3f::Zero())
, mIdleLightDesc(gpSimplePool->GetObj(SObjectTag('PART', turretData.GetIdleLightRes())))
, mDeactivateLightDesc(
      gpSimplePool->GetObj(SObjectTag('PART', turretData.GetDeactivateLightRes())))
, mTargettingLightDesc(
      gpSimplePool->GetObj(SObjectTag('PART', turretData.GetTargettingLightRes())))
, mFrozenEffectDesc(gpSimplePool->GetObj(SObjectTag('PART', turretData.GetFrozenEffectRes())))
, mChargingEffectDesc(
      gpSimplePool->GetObj(SObjectTag('PART', turretData.GetChargingEffectRes())))
, mPanningEffectDesc(gpSimplePool->GetObj(SObjectTag('PART', turretData.GetPanningEffectRes())))
, mVisorEffectDesc(
      turretData.GetVisorEffectRes() != kInvalidAssetId
          ? rstl::optional_object< TLockedToken< CGenDescription > >(
                gpSimplePool->GetObj(SObjectTag('PART', turretData.GetVisorEffectRes())))
          : rstl::optional_object_null())
, mIdleLight(rs_new CElementGen(mIdleLightDesc))
, mDeactivateLight(rs_new CElementGen(mDeactivateLightDesc))
, mTargettingLight(rs_new CElementGen(mTargettingLightDesc))
, mFrozenEffect(rs_new CElementGen(mFrozenEffectDesc))
, mChargingEffect(rs_new CElementGen(mChargingEffectDesc))
, mPanningEffect(rs_new CElementGen(mPanningEffectDesc))
, mLightId(kInvalidUniqueId)
, mCollisionActor(kInvalidUniqueId)
, mExtensionModel(rstl::optional_object< CModelData >())
, mExtensionRange(0.f)
, mExtensionT(0.f)
, mExtensionOffset(xf.GetTranslation())
, mGunSDKSeg(CSegId::Invalid())
, mTimeSinceLastTargetSfx(0.f)
, mLastFrontVector(xf.GetForward())
, mState(kTS_Invalid)
, mCurStateTime(0.f)
, mCurInactiveTime(0.f)
, mCurActiveTime(0.f)
, mCurPanTime(0.f)
, mFireCycleRemTime(0.f)
, mHalfFireCycleDur(0.f)
, mFreezeRemTime(0.f)
, mTurretAnim(-1)
, mOriginalFrontVec(xf.GetForward())
, mOriginalRightVec(xf.GetRight())
, mAdditiveChargeAnim(-1)
, mDead(false)
, mFrozen(false)
, mFiredWithSetBurst(false)
, mBurstSet(false)
, mHasBeenActivated(false)
, mScriptedStart(false)
, mNeedsStopClankSound(true)
, mFrenzyReverse(false) {

  if (comp == kTC_Base && HasAnimation()) {
    ModelData()->EnableLooping(true);
  }
  mProjectileInfo.Token().Lock();
}

CScriptGunTurret::~CScriptGunTurret() {}

void CScriptGunTurret::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  switch (mType) {
  case kTC_Base:
    if (!mFrozen) {
      ProcessGunStateMachine(dt, mgr);
      UpdateTurretAnimation();
      UpdateGunOrientation(dt, mgr);
      CVector3f vec = UpdateExtensionModelState(dt);
      const CAdvancementDeltas deltas = UpdateAnimation(dt, mgr, true);
      SetTranslation((GetTranslation() + deltas.GetOffsetDelta()) + vec);
      RotateToOR(deltas.GetOrientationDelta(), dt);
    } else {
      Stop();
    }

    UpdateTargettingSound(dt);
    break;
  case kTC_Gun:
    UpdateGunParticles(dt, mgr);
    const CAdvancementDeltas deltas = UpdateAnimation(dt, mgr, true);
    MoveToOR(deltas.GetOffsetDelta(), dt);
    RotateToOR(deltas.GetOrientationDelta(), dt);
    UpdateGunCollisionManager(dt, mgr);
    UpdateFrozenState(dt, mgr);
    break;
  }

  UpdateHealthInfo(mgr);
}

void CScriptGunTurret::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                       CStateManager& mgr) {
  CPhysicsActor::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case kSM_Activate: {
    if (!mCollisionManager.null()) {
      mCollisionManager->SetActive(mgr, true);
    }
    break;
  }
  case kSM_Deactivate: {
    if (!mCollisionManager.null()) {
      mCollisionManager->SetActive(mgr, false);
    }
    break;
  }
  case kSM_Registered: {
    if (mType == kTC_Gun && mTargettingLight->SystemHasLight()) {
      mLightId = mgr.AllocateUniqueId();
      mgr.AddObject(rs_new CGameLight(mLightId, GetCurrentAreaId(), GetActive(),
                                      rstl::string_l("ParticleLight_") + GetDebugName(),
                                      GetTransform(), GetUniqueId(),
                                      mTargettingLight->GetLight(), 0, 1, 0.f));
      SetupCollisionManager(mgr);
    } else if (mType == kTC_Base) {
      if (mData.GetExtensionModelResId() != kInvalidAssetId) {
        mExtensionModel =
            rstl::optional_object< CModelData >(SetupExtensionModel(GetModelScale()));
        mExtensionRange = mExtensionModel->GetBounds().GetDepth();
      }

      SetTurretState(kTS_Inactive, mgr);
    }
    break;
  }

  case kSM_Deleted: {
    if (mType == kTC_Gun && mLightId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mLightId);
    }

    if (mTargetingEmitter) {
      CSfxManager::RemoveEmitter(mTargetingEmitter);
    }

    if (!mCollisionManager.null()) {
      mCollisionManager->Destroy(mgr);
    }
    break;
  }
  case kSM_Start:
    if (mType != kTC_Base || mState != kTS_Inactive) {
      break;
    }
    mScriptedStart = true;
    break;
  case kSM_Stop: {
    if (mType != kTC_Base) {
      break;
    }

    if (mState != kTS_Deactivate && mState != kTS_DeactivateFromReady &&
        mState != kTS_Deactivating) {
      SetTurretState(mHasBeenActivated ? kTS_DeactivatingFromReady : kTS_Deactivating, mgr);
    }
  } break;
  case kSM_Action: {
    if (mType == kTC_Gun) {
      LaunchProjectile(mgr);
    } else if (mType == kTC_Base) {
      PlayAdditiveFlinchAnimation(mgr);
    }
  } break;
  case kSM_SetToMax: {
    mFrozen = false;
    SetMuted(false);
  } break;
  case kSM_SetToZero: {
    mFrozen = true;
    SetMuted(true);
  } break;
  case kSM_InitializedInArea: {
    if (mType != kTC_Base) {
      break;
    }

    for (AUTO(conn, GetConnectionList().begin()); conn != GetConnectionList().end(); ++conn) {
      if (conn->mState != kSS_Play || conn->mMsg != kSM_Activate) {
        continue;
      }

      TUniqueId gunId = mgr.GetIdForScript(conn->mObjId);

      if (CScriptGunTurret* gun = TCastToPtr< CScriptGunTurret >(mgr.ObjectById(gunId))) {
        mGunId = gunId;
        mLastGunHP = gun->GetHealthInfo(mgr)->GetHP();
        break;
      }
    }
  } break;
  case kSM_Damage: {
    if (mType != kTC_Gun) {
      break;
    }
    if (!(GetHealthInfo(mgr)->GetHP() <= 0.f)) {
      break;
    }

    if (const CGameProjectile* proj = TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(uid))) {
      if ((proj->GetAttribField() & CWeapon::kPA_Wave) != CWeapon::kPA_Wave) {
        break;
      }

      mState = kTS_Frenzy;
      RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
      mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
      mFreezeRemTime = 0.f;
    }
  } break;
  default:
    break;
  }
}

ENTITY_ACCEPT_IMPL(CScriptGunTurret)

void CScriptGunTurret::Render(const CStateManager& mgr) const {
  CPhysicsActor::Render(mgr);

  if (mType == kTC_Gun) {
    if (!mFrozen) {
      switch (mState) {
      case kTS_Deactivate:
      case kTS_DeactivateFromReady:
      case kTS_Deactivating:
      case kTS_DeactivatingFromReady:
        mDeactivateLight->Render();
        break;
      case kTS_Inactive:
        mIdleLight->Render();
        break;
      case kTS_PanningA:
      case kTS_PanningB:
        mPanningEffect->Render();
        break;
      case kTS_Ready:
      case kTS_Targeting:
      case kTS_Firing:
      case kTS_ExitTargeting:
      case kTS_Frenzy:
        mTargettingLight->Render();
        if (mState == kTS_Firing) {
          mChargingEffect->Render();
        }
        break;
      default:
        break;
      }
    } else {
      mFrozenEffect->Render();
    }
  } else if (mType == kTC_Base && mExtensionModel && mExtensionT > 0.f) {
    CTransform4f xf = GetTransform();
    xf.SetTranslation(mExtensionOffset + mExtensionRange * (CVector3f::Down() * 0.5f));
    mExtensionModel->Render(mgr, xf, GetActorLights(), CModelFlags::Normal());
  }
}

void CScriptGunTurret::AddToRenderer(const CFrustumPlanes& frustum,
                                     const CStateManager& mgr) const {
  CActor::AddToRenderer(frustum, mgr);

  if (mType != kTC_Gun) {
    return;
  }

  if (!mFrozen) {
    switch (mState) {
    case kTS_Deactivate:
    case kTS_DeactivateFromReady:
    case kTS_Deactivating:
    case kTS_DeactivatingFromReady:
      gpRender->AddParticleGen(*mDeactivateLight);
      break;
    case kTS_Inactive:
      gpRender->AddParticleGen(*mIdleLight);
      break;
    case kTS_PanningA:
    case kTS_PanningB:
      gpRender->AddParticleGen(*mPanningEffect);
      break;
    case kTS_Ready:
    case kTS_Targeting:
    case kTS_Firing:
    case kTS_ExitTargeting:
    case kTS_Frenzy:
      gpRender->AddParticleGen(*mTargettingLight);
      if (mState == kTS_Firing || mState == kTS_Frenzy) {
        gpRender->AddParticleGen(*mChargingEffect);
      }
      break;
    default:
      break;
    }
  } else {
    gpRender->AddParticleGen(*mFrozenEffect);
  }
}

rstl::optional_object< CAABox > CScriptGunTurret::GetTouchBounds() const {
  if (GetActive() && GetMaterialList().HasMaterial(kMT_Solid)) {
    return GetBoundingBox();
  }
  return rstl::optional_object_null();
}

void CScriptGunTurret::Touch(CActor& actor, CStateManager& mgr) {
  if (mType != kTC_Gun) {
    return;
  }

  if (const CGameProjectile* proj = TCastToConstPtr< CGameProjectile >(actor)) {
    const CPlayer* player = mgr.GetPlayer();
    if (proj->GetOwnerId() == player->GetUniqueId()) {
      const CDamageVulnerability* dVuln = GetDamageVulnerability();
      if (!mDead && mState != kTS_Frenzy &&
          (proj->GetAttribField() & CWeapon::kPA_Ice) == CWeapon::kPA_Ice &&
          dVuln->WeaponHits(CWeaponMode::Ice(), CDamageVulnerability::kRD_No)) {
        mFrozen = true;
        SendScriptMsgs(kSS_Zero, mgr, kSM_None);
        mFreezeRemTime =
            mgr.Random()->Float() * mData.GetFreezeVariance() + mData.GetFreezeDuration();
        SetMuted(true);
      }

      SendScriptMsgs(kSS_Damage, mgr, kSM_None);
    }
  }
}

CVector3f CScriptGunTurret::GetAimPosition(const CStateManager& mgr, float dt) const {
  if (mType == kTC_Gun) {
    CTransform4f lctrXf(GetLocatorTransform(rstl::string_l(skGunLCTRName)));
    return GetTranslation() + GetTransform().Rotate(lctrXf.GetTranslation());
  }

  return GetTranslation();
}

CVector3f CScriptGunTurret::GetOrbitPosition(const CStateManager& mgr) const {
  return GetAimPosition(mgr, 0.f);
}

void CScriptGunTurret::SetTurretState(const ETurretState state, CStateManager& mgr) {
  if (state >= kTS_Destroyed && state <= kTS_Frenzy) {
    if (mState != kTS_Invalid) {
      ProcessCurrentState(kStateMsg_Deactivate, 0.f, mgr);
    }

    mState = state;
    mCurStateTime = 0.f;
    ProcessCurrentState(kStateMsg_Activate, 0.f, mgr);
  }
}

void CScriptGunTurret::ProcessGunStateMachine(float dt, CStateManager& mgr) {
  ProcessCurrentState(kStateMsg_Update, dt, mgr);
  mCurStateTime += dt;
  PlayAdditiveChargingAnimation(mgr);

  if (mGunId == kInvalidUniqueId) {
    return;
  }

  CScriptGunTurret* gun = TCastToPtr< CScriptGunTurret >(mgr.ObjectById(mGunId));
  if (!gun) {
    return;
  }

  if (gun->mState != kTS_Frenzy) {
    gun->mState = mState;
    return;
  }

  if (mState == kTS_Frenzy) {
    return;
  }

  SetTurretState(kTS_Frenzy, mgr);
  gun->RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
  mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
}

void CScriptGunTurret::ProcessCurrentState(EStateMsg msg, float dt, CStateManager& mgr) {
  switch (mState) {
  case kTS_Destroyed:
    break;
  case kTS_Deactivate:
    break;
  case kTS_Deactivating:
  case kTS_DeactivatingFromReady:
    ProcessDeactivatingState(msg, dt, mgr);
    break;
  case kTS_Inactive:
    ProcessInactiveState(msg, dt, mgr);
    break;
  case kTS_Ready:
    ProcessReadyState(msg, dt, mgr);
    break;
  case kTS_PanningA:
  case kTS_PanningB:
    ProcessPanningState(msg, dt, mgr);
    break;
  case kTS_Targeting:
  case kTS_Firing:
    ProcessTargettingState(msg, dt, mgr);
    break;
  case kTS_ExitTargeting:
    ProcessExitTargettingState(msg, dt, mgr);
    break;
  case kTS_Frenzy:
    ProcessFrenzyState(msg, dt, mgr);
    break;
  default:
    break;
  }
}

void CScriptGunTurret::ProcessDeactivatingState(EStateMsg msg, float dt, CStateManager& mgr) {
  switch (msg) {
  case kStateMsg_Update:
    if (mCurStateTime >= mData.GetIntoDeactivateDelay()) {
      SetTurretState(mHasBeenActivated ? kTS_DeactivateFromReady : kTS_Deactivate, mgr);
    }
    break;
  default:
    break;
  }
}

void CScriptGunTurret::ProcessInactiveState(EStateMsg msg, float dt, CStateManager& mgr) {
  switch (msg) {
  case kStateMsg_Activate:
    mCurInactiveTime = 0.f;
    mBurstSet = false;
    if (CScriptGunTurret* gun = TCastToPtr< CScriptGunTurret >(mgr.ObjectById(mGunId))) {
      mLastGunHP = gun->GetHealthInfo(mgr)->GetHP();
    }
    break;
  case kStateMsg_Update: {
    bool forceActivate = false;
    if (mGunId != kInvalidUniqueId) {
      if (CScriptGunTurret* gun = TCastToPtr< CScriptGunTurret >(mgr.ObjectById(mGunId))) {
        forceActivate = gun->GetHealthInfo(mgr)->GetHP() < mLastGunHP;
      }
    }

    if (mData.IsScriptedStartOnly()
            ? (forceActivate || mScriptedStart)
            : (forceActivate || mScriptedStart || InDetectionRange(mgr))) {
      mCurInactiveTime += dt;
      if (forceActivate || mCurInactiveTime >= mData.GetIntoActivateDelay()) {
        SetTurretState(kTS_Ready, mgr);
      }
    } else {
      mIdleLight->SetParticleEmission(true);
    }
  } break;
  case kStateMsg_Deactivate:
    mHasBeenActivated = true;
    mIdleLight->SetParticleEmission(false);
    if (CScriptGunTurret* gun = TCastToPtr< CScriptGunTurret >(mgr.ObjectById(mGunId))) {
      mLastGunHP = gun->GetHealthInfo(mgr)->GetHP();
    }
    break;
  }
}

void CScriptGunTurret::ProcessReadyState(EStateMsg msg, float dt, CStateManager& mgr) {
  switch (msg) {
  case kStateMsg_Activate:
    mCurActiveTime = 0.f;
    break;
  case kStateMsg_Update:
    mCurActiveTime += dt;
    if (mCurActiveTime >= mData.GetPanStartTime()) {
      if (IsPlayerInFiringRange(mgr) && InDetectionRange(mgr)) {
        SetTurretState(kTS_Targeting, mgr);
        CSfxManager::AddEmitter(mData.GetLockOnSoundId(), GetTranslation(), CVector3f::Up(),
                                false, false, CSfxManager::kMedPriority,
                                GetCurrentAreaId().Value());
      } else {
        SetTurretState(kTS_PanningA, mgr);
        mCurPanTime = 0.f;
      }
    }
    break;
  default:
    break;
  }
}

void CScriptGunTurret::ProcessPanningState(EStateMsg msg, float dt, CStateManager& mgr) {
  switch (msg) {
  case kStateMsg_Activate:
    mCurActiveTime = 0.f;
    break;
  case kStateMsg_Update:
    if (IsPlayerInFiringRange(mgr) && InDetectionRange(mgr)) {
      SetTurretState(kTS_Targeting, mgr);
      CSfxManager::AddEmitter(mData.GetLockOnSoundId(), GetTranslation(), CVector3f::Up(),
                              false, false, CSfxManager::kMedPriority, GetCurrentAreaId().Value());
    } else {
      mCurActiveTime += dt;
      mCurPanTime += dt;
      if (mCurPanTime >= mData.GetPanTotalSearchTime() && !mExtensionModel &&
          !mData.IsScriptedStartOnly()) {
        SetTurretState(kTS_Inactive, mgr);
        mScriptedStart = mHasBeenActivated = false;
      } else if (mCurActiveTime >= mData.GetPanHoldTime()) {
        SetTurretState(mState == kTS_PanningA ? kTS_PanningB : kTS_PanningA, mgr);
      }
    }
    break;
  default:
    break;
  }
}

void CScriptGunTurret::ProcessTargettingState(EStateMsg msg, float dt, CStateManager& mgr) {
  switch (msg) {
  case kStateMsg_Activate:
    mCurActiveTime = 0.f;
    break;
  case kStateMsg_Update:
    if (mFiredWithSetBurst || InDetectionRange(mgr)) {
      UpdateTargettingMode(dt, mgr);
      if (mGunId != kInvalidUniqueId) {
        if (CScriptGunTurret* const gun =
                TCastToPtr< CScriptGunTurret >(mgr.ObjectById(mGunId))) {
          CVector3f intercept = mTargetPosition;
          if (IsPlayerInFiringRange(mgr)) {
            const CTransform4f blastXf = gun->GetLocatorTransform(rstl::string_l(skBlastLCTRName));
            const CVector3f blastPos =
                gun->GetTranslation() + gun->GetTransform().Rotate(blastXf.GetTranslation());
            mTargetPosition = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
            intercept = mProjectileInfo.PredictInterceptPos(
                blastPos, mgr.GetPlayer()->GetAimPosition(mgr, 0.f), *mgr.GetPlayer(), false, dt);
          }

          const CVector3f targetDelta = mTargetPosition - gun->GetTranslation();
          const CVector3f error = gun->GetTransform().Rotate(
              mBurstFire.GetDistanceCompensatedError(targetDelta.Magnitude(), 20.f));
          gun->SetTargetPosition(mTargetPosition + (intercept - mTargetPosition + error));
        }
      }

      CVector3f targetDelta = mTargetPosition - GetTranslation();
      targetDelta.SetZ(0.f);
      if (targetDelta.CanBeNormalized()) {
        const CVector3f targetDir = targetDelta.AsNormalized();
        const CVector3f front = GetTransform().GetForward();
        const float rotationAngle =
            rstl::min_val(dt * mData.GetTurnSpeed(), CVector3f::GetAngleDiff(targetDir, front));
        const CQuaternion rot = CQuaternion::LookAt(CUnitVector3f(front, CUnitVector3f::kN_No),
                                                    CUnitVector3f(targetDir, CUnitVector3f::kN_No),
                                                    CRelAngle::FromRadians(rotationAngle));
        const CQuaternion localRot = CQuaternion::ScalarVector(
            rot.GetScalar(), GetTransform().TransposeRotate(rot.GetVector()));
        RotateInOneFrameOR(localRot, dt);
      }

      if (ShouldFire(mgr)) {
        SendScriptMsgs(kSS_Attack, mgr, kSM_None);
        mFiredWithSetBurst = true;
      }
      mCurActiveTime = 0.f;
    } else {
      mCurActiveTime += dt;
      if (mCurActiveTime >= 10.f) {
        SetTurretState(kTS_ExitTargeting, mgr);
      }
    }
    break;
  case kStateMsg_Deactivate:
    mNeedsStopClankSound = true;
    break;
  }
}

void CScriptGunTurret::ProcessExitTargettingState(EStateMsg msg, float dt, CStateManager& mgr) {
  switch (msg) {
  case kStateMsg_Update:
    if (mGunId == kInvalidUniqueId) {
      break;
    }
    if (CScriptGunTurret* gun = TCastToPtr< CScriptGunTurret >(mgr.ObjectById(mGunId))) {
      CTransform4f gunXf = GetTransform() * GetLocatorTransform(rstl::string_l(skGunLCTRName));
      const CVector3f front = gun->GetTransform().GetForward();
      const CVector3f originalFront = mOriginalFrontVec;
      if (CVector3f::GetAngleDiff(front, originalFront) < (M_PIF / 180.f)) {
        SetTurretState(kTS_Ready, mgr);
      }
    }
    break;
  default:
    break;
  }
}

void CScriptGunTurret::ProcessFrenzyState(EStateMsg msg, float dt, CStateManager& mgr) {
  switch (msg) {
  case kStateMsg_Activate:
    mFrenzyReverse = mgr.Random()->Float() < 0.5f;
    mFireCycleRemTime = 0.15f;
    RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
    mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
    break;
  case kStateMsg_Update: {
    if (mCurStateTime >= mData.GetFrenzyDuration()) {
      SetTurretState(kTS_Destroyed, mgr);
      if (CScriptGunTurret* gun = TCastToPtr< CScriptGunTurret >(mgr.ObjectById(mGunId))) {
        gun->mState = kTS_Destroyed;
      }
      break;
    }

    const CVector3f frontVec = GetTransform().GetForward();
    if (mFrenzyReverse) {
      if (CVector3f::Dot(mOriginalRightVec, frontVec) < 0.f &&
          CVector3f::GetAngleDiff(mOriginalFrontVec, frontVec) >= CMath::Deg2Rad(45.f)) {
        mFrenzyReverse = false;
      }
    } else {
      if (CVector3f::Dot(mOriginalRightVec, frontVec) > 0.f &&
          CVector3f::GetAngleDiff(mOriginalFrontVec, frontVec) >= CMath::Deg2Rad(45.f)) {
        mFrenzyReverse = true;
      }
    }

    if (CScriptGunTurret* gun = TCastToPtr< CScriptGunTurret >(mgr.ObjectById(mGunId))) {
      mFireCycleRemTime -= dt;
      if (mFireCycleRemTime < 0.f) {
        mTargetPosition = gun->GetTranslation() + 100.f * gun->GetTransform().GetForward();
        SendScriptMsgs(kSS_Attack, mgr, kSM_None);
        mFireCycleRemTime = 0.15f;
      }
    }
    break;
  }
  default:
    break;
  }
}

void CScriptGunTurret::UpdateTurretAnimation() {
  if (HasAnimation() && mState >= kTS_Destroyed && mState <= kTS_Frenzy) {
    const CPASAnimParmData parms(pas::kAS_Locomotion, CPASAnimParm::FromEnum(0),
                                 CPASAnimParm::FromEnum(skStateToLocoTypeLookup[mState]));
    const rstl::pair< float, int > best =
        GetAnimationData()->GetPASDatabase().FindBestAnimation(parms, -1);
    if (best.first > 0.f && best.second != mTurretAnim) {
      AnimationData()->SetAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), false);
      ModelData()->EnableLooping(true);
      mTurretAnim = best.second;
    }
  }
}

void CScriptGunTurret::UpdateTargettingMode(float dt, CStateManager& mgr) {
  if (mgr.GetCameraManager()->IsInCinematicCamera()) {
    mFireCycleRemTime =
        mgr.Random()->Float() * mData.GetReloadTimeVariance() + mData.GetReloadTime();
    mHalfFireCycleDur = 0.5f * mFireCycleRemTime;
  }
  if (mFireCycleRemTime > 0.f) {
    mFireCycleRemTime -= dt;
    if (mFireCycleRemTime < mHalfFireCycleDur && mState != kTS_Firing) {
      CSfxManager::AddEmitter(mData.GetChargingSoundId(), GetTranslation(), CVector3f::Up(),
                              false, false, CSfxManager::kMedPriority, GetCurrentAreaId().Value());
      SetTurretState(kTS_Firing, mgr);
    }
  } else {
    if (mState != kTS_Targeting) {
      SetTurretState(kTS_Targeting, mgr);
    }
    if (!mBurstFire.IsBurstSet()) {
      UpdateBurstType(mgr);
      mFireCycleRemTime =
          mgr.Random()->Float() * mData.GetReloadTimeVariance() + mData.GetReloadTime();
      mHalfFireCycleDur = 0.5f * mFireCycleRemTime;
    } else {
      mBurstFire.Update(mgr, dt);
    }
  }
}

void CScriptGunTurret::UpdateGunOrientation(float dt, CStateManager& mgr) {
  if (mGunId != kInvalidUniqueId) {
    if (CScriptGunTurret* gun = TCastToPtr< CScriptGunTurret >(mgr.ObjectById(mGunId))) {
      const CTransform4f gunXf =
          GetTransform() * GetLocatorTransform(rstl::string_l(skGunLCTRName));
      switch (mState) {
      case kTS_Targeting:
      case kTS_Firing: {
        const CVector3f gunPos = gunXf.GetTranslation();
        float yaw = 0.f;
        const float horizontalMag =
            sqrt(gunXf.Get11() * gunXf.Get11() + gunXf.Get01() * gunXf.Get01());
        if (horizontalMag > 0.001f) {
          const double angle = -atan2(gunXf.Get01(), gunXf.Get11());
          yaw = angle;
        }
        const float oldPitch = gun->GetPitch();
        float pitch = 0.f;
        if (!gun->PlayerInsideTurretSphere(mgr)) {
          const CVector3f targetDelta = mTargetPosition - gunPos;
          const CTransform4f targetXf = targetDelta.CanBeNormalized()
                                            ? CTransform4f::LookAt(gunPos, mTargetPosition)
                                            : GetTransform();
          const float targetHorizontalMag = CMath::SqrtF(targetXf.Get11() * targetXf.Get11() +
                                                         targetXf.Get01() * targetXf.Get01());
          const float newPitch = CCast::ToReal32(-atan2(-targetXf.Get21(), targetHorizontalMag));
          const float delta = newPitch - oldPitch;
          const float step =
              delta > 0.f ? dt * mData.GetTurnSpeed() : dt * -mData.GetTurnSpeed();
          pitch = fabsf(delta) <= fabsf(step) ? newPitch : oldPitch + step;
          pitch = rstl::max_val(pitch, -mData.GetDownMaxAngle());
        }
        const CQuaternion rotation =
            CQuaternion::YXZRotation(CRelAngle::FromRadians(0.f), CRelAngle::FromRadians(pitch),
                                     CRelAngle::FromRadians(yaw));
        gun->SetTransform(CTransform4f(rotation.BuildTransform(), gunPos));
        break;
      }
      case kTS_ExitTargeting: {
        const float angle = 0.3f * (dt * mData.GetTurnSpeed());
        const CVector3f front = GetTransform().GetForward();
        const CVector3f gunFront = gun->GetTransform().GetForward();
        const CQuaternion gunRotation = CQuaternion::LookAt(
            CUnitVector3f(gunFront.GetX(), gunFront.GetY(), gunFront.GetZ()),
            CUnitVector3f(front.GetX(), front.GetY(), front.GetZ()), CRelAngle::FromRadians(angle));
        const CQuaternion localGunRotation = CQuaternion::ScalarVector(
            gunRotation.GetScalar(), gun->GetTransform().TransposeRotate(gunRotation.GetVector()));
        gun->RotateInOneFrameOR(localGunRotation, dt);
        const CVector3f originalFront = mOriginalFrontVec;
        const CVector3f currentFront = GetTransform().GetForward();
        const CQuaternion rotation = CQuaternion::LookAt(
            CUnitVector3f(currentFront.GetX(), currentFront.GetY(), currentFront.GetZ()),
            CUnitVector3f(originalFront.GetX(), originalFront.GetY(), originalFront.GetZ()),
            CRelAngle::FromRadians(angle));
        const CQuaternion localRotation = CQuaternion::ScalarVector(
            rotation.GetScalar(), GetTransform().TransposeRotate(rotation.GetVector()));
        RotateInOneFrameOR(localRotation, dt);
        break;
      }
      case kTS_Frenzy: {
        const CVector3f gunPos = gunXf.GetTranslation();
        float yaw = 0.f;
        const float horizontalMag =
            sqrt(gunXf.Get11() * gunXf.Get11() + gunXf.Get01() * gunXf.Get01());
        if (horizontalMag > 0.001f) {
          const double angle = -atan2(gunXf.Get01(), gunXf.Get11());
          yaw = angle;
        }
        const float turnSpeed = mData.GetTurnSpeed();
        const float newPitch = -0.5f * mData.GetDownMaxAngle() *
                               (1.f - CMath::FastCosR(2.f * mCurStateTime * turnSpeed));
        const float oldPitch = gun->GetPitch();
        const float delta = newPitch - oldPitch;
        const float angle = turnSpeed * dt;
        const float step = delta > 0.f ? angle : -angle;
        float pitch = fabsf(delta) <= fabsf(step) ? newPitch : oldPitch + step;
        pitch = rstl::max_val(pitch, -mData.GetDownMaxAngle());
        const CQuaternion gunRotation =
            CQuaternion::YXZRotation(CRelAngle::FromRadians(0.f), CRelAngle::FromRadians(pitch),
                                     CRelAngle::FromRadians(yaw));
        gun->SetTransform(CTransform4f(gunRotation.BuildTransform(), gunPos));
        const CVector3f targetDir =
            mFrenzyReverse ? -mOriginalRightVec : mOriginalRightVec;
        const CVector3f currentFront = GetTransform().GetForward();
        const CQuaternion rotation = CQuaternion::LookAt(
            CUnitVector3f(currentFront.GetX(), currentFront.GetY(), currentFront.GetZ()),
            CUnitVector3f(targetDir.GetX(), targetDir.GetY(), targetDir.GetZ()),
            CRelAngle::FromRadians(angle));
        const CQuaternion localRotation = CQuaternion::ScalarVector(
            rotation.GetScalar(), GetTransform().TransposeRotate(rotation.GetVector()));
        RotateInOneFrameOR(localRotation, dt);
        break;
      }
      default:
        gun->SetTransform(gunXf);
        break;
      }
    }
  }
}

CVector3f CScriptGunTurret::UpdateExtensionModelState(float dt) {
  CVector3f offset = CVector3f::Zero();
  if (mExtensionModel) {
    const float change = 1.5f * dt;
    switch (mState) {
    case kTS_PanningA:
    case kTS_PanningB:
    case kTS_Targeting:
    case kTS_Firing:
    case kTS_ExitTargeting:
      mExtensionT = rstl::min_val(mExtensionT + change, skExtensionOverlapMaxPer);
      break;
    case kTS_Ready:
    case kTS_Deactivating:
    case kTS_DeactivatingFromReady:
    case kTS_Frenzy:
      break;
    default:
      mExtensionT = rstl::max_val(0.f, mExtensionT - change);
      break;
    }
    offset = mExtensionOffset +
             mData.GetExtensionDropDownDist() * (mExtensionT * CVector3f::Down()) -
             GetTranslation();
  }
  return offset;
}

void CScriptGunTurret::UpdateHealthInfo(CStateManager& mgr) {
  switch (mType) {
  case kTC_Base:
    if (mGunId != kInvalidUniqueId) {
      if (!TCastToPtr< CScriptGunTurret >(mgr.ObjectById(mGunId))) {
        SetTurretState(kTS_Destroyed, mgr);
        mFrozen = false;
        mGunId = kInvalidUniqueId;
        if (mTargetingEmitter) {
          CSfxManager::RemoveEmitter(mTargetingEmitter);
          mTargetingEmitter.Clear();
        }
      }
    } else {
      SetTurretState(kTS_Destroyed, mgr);
    }
    break;
  case kTC_Gun:
    if (!mDead && mState != kTS_Frenzy && HealthInfo(mgr)->GetHP() <= 0.f) {
      mDead = true;
      SendScriptMsgs(kSS_Dead, mgr, kSM_None);
      mgr.DeleteObjectRequest(GetUniqueId());
    }
    break;
  default:
    break;
  }
}

bool CScriptGunTurret::IsStopped(const float dt) const {
  const CVector2f thisForward(GetTransform().GetForward().ToVec2f());
  const CVector2f lastForward(mLastFrontVector.ToVec2f());

  return CVector2f::GetAngleDiff(lastForward, thisForward) < dt * (2 * (M_PIF / 180));
}

bool CScriptGunTurret::IsPlayerInFiringRange(CStateManager& mgr) const {
  const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  const CVector3f horizontalDelta(delta.ToVec2f(), 0.f);
  bool inHorizontalRange;
  if (CVector3f::Dot(mOriginalRightVec, delta) >= 0.f) {
    inHorizontalRange = CVector3f::GetAngleDiff(mOriginalFrontVec, horizontalDelta) <=
                        mData.GetRightMaxAngle();
  } else {
    inHorizontalRange = CVector3f::GetAngleDiff(mOriginalFrontVec, horizontalDelta) <=
                        mData.GetLeftMaxAngle();
  }
  if (inHorizontalRange) {
    const float pitch = CVector3f::GetAngleDiff(CVector3f::Up(), delta) - M_PIF / 2.f;
    inHorizontalRange = pitch >= -20.f * (M_PIF / 180.f) && pitch <= mData.GetDownMaxAngle();
  }
  return inHorizontalRange;
}

bool CScriptGunTurret::InDetectionRange(CStateManager& mgr) const {
  const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  if ((CVector3f::Dot(delta, CVector3f::Down()) >= 0.f ||
       CVector3f::GetAngleDiff(GetTransform().GetForward(), delta) <= 20.f * (M_PIF / 180.f)) &&
      delta.MagSquared() <= mData.GetDetectionRange() * mData.GetDetectionRange() &&
      (mData.GetDetectionZRange() == 0.f ||
       CMath::AbsF(delta.GetZ()) < mData.GetDetectionZRange())) {
    return LineOfSightTest(mgr);
  }
  return false;
}

bool CScriptGunTurret::ShouldFire(CStateManager& mgr) const {
  if (mState == kTS_Targeting && mFireCycleRemTime <= 0.f && mBurstFire.ShouldFire()) {
    return IsPlayerInFiringRange(mgr);
  }
  return false;
}

void CScriptGunTurret::LaunchProjectile(CStateManager& mgr) {
  if (mProjectileInfo.Token().TryCache() && mgr.CanCreateProjectile(GetUniqueId(), kWT_AI, 8)) {
    const CTransform4f blastXf = GetLocatorTransform(rstl::string_l(skBlastLCTRName));
    const CVector3f projectilePos =
        GetTranslation() + GetTransform().Rotate(blastXf.GetTranslation());
    CVector3f lookPos = mTargetPosition;
    const CVector3f front = GetTransform().GetForward();
    const CVector3f delta = mTargetPosition - projectilePos;
    if (CVector3f::GetAngleDiff(front, delta) > 20.f * (M_PIF / 180.f)) {
      if (delta.CanBeNormalized()) {
        const CVector3f dir =
            CVector3f::Slerp(front, delta.AsNormalized(), CRelAngle(20.f * (M_PIF / 180.f)));
        lookPos = projectilePos + delta.Magnitude() * dir;
      } else {
        lookPos = projectilePos + front;
      }
    } else if (!delta.CanBeNormalized()) {
      lookPos = projectilePos + front;
    }
    const CTransform4f projectileXf = CTransform4f::LookAt(projectilePos, lookPos);
    CEnergyProjectile* projectile = rs_new CEnergyProjectile(
        true, mProjectileInfo.Token(), kWT_AI, projectileXf, kMT_Character,
        mProjectileInfo.GetDamage(), mgr.AllocateUniqueId(), GetCurrentAreaId(), GetUniqueId(),
        kInvalidUniqueId, 0, false, CVector3f(1.f, 1.f, 1.f), mVisorEffectDesc,
        mData.GetVisorSoundId(), false);
    if (projectile) {
      mgr.AddObject(projectile);
      const CPASAnimParmData parms(pas::kAS_ProjectileAttack, CPASAnimParm::FromEnum(1),
                                   CPASAnimParm::FromReal32(90.f),
                                   CPASAnimParm::FromEnum(skStateToLocoTypeLookup[mState]));
      const rstl::pair< float, int > anim =
          GetAnimationData()->GetCharacterInfo().GetPASDatabase().FindBestAnimation(parms, -1);
      if (anim.first > 0.f) {
        ModelData()->EnableLooping(false);
        AnimationData()->SetAnimation(CAnimPlaybackParms(anim.second, -1, 1.f, true), false);
      }
    }
  }
}

void CScriptGunTurret::PlayAdditiveFlinchAnimation(CStateManager& mgr) {
  const CPASAnimParmData parms(pas::kAS_AdditiveFlinch);
  const rstl::pair< float, int > best =
      GetAnimationData()->GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
  if (best.first > 0.f) {
    AnimationData()->AddAdditiveAnimation(best.second, 1.f, false, true);
  }
}

void CScriptGunTurret::PlayAdditiveChargingAnimation(CStateManager& mgr) {
  if (mState == kTS_Firing) {
    if (mAdditiveChargeAnim == -1) {
      const CPASAnimParmData parms(pas::kAS_AdditiveReaction, CPASAnimParm::FromEnum(2));
      CAnimData* animData = AnimationData();
      const rstl::pair< float, int > best =
          animData->GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
      if (best.first > 0.f) {
        mAdditiveChargeAnim = best.second;
        animData->AddAdditiveAnimation(mAdditiveChargeAnim, 1.f, true, false);
      }
    }
  } else if (mAdditiveChargeAnim != -1) {
    AnimationData()->DelAdditiveAnimation(mAdditiveChargeAnim);
    mAdditiveChargeAnim = -1;
  }
}

void CScriptGunTurret::UpdateGunParticles(float dt, CStateManager& mgr) {
  CGameLight* light = nullptr;
  if (mLightId != kInvalidUniqueId) {
    light = TCastToPtr< CGameLight >(mgr.ObjectById(mLightId));
  }
  if (!mFrozen) {
    const CTransform4f lightXf = GetLocatorTransform(rstl::string_l(skLightLCTRName));
    const CVector3f lightPos = GetTranslation() + GetTransform().Rotate(lightXf.GetTranslation());
    if (light) {
      light->SetActive(true);
    }
    switch (mState) {
    case kTS_Deactivate:
    case kTS_DeactivateFromReady:
    case kTS_Deactivating:
    case kTS_DeactivatingFromReady:
      mIdleLight->SetParticleEmission(false);
      mDeactivateLight->SetParticleEmission(true);
      mTargettingLight->SetParticleEmission(false);
      mFrozenEffect->SetParticleEmission(false);
      mChargingEffect->SetParticleEmission(false);
      mPanningEffect->SetParticleEmission(false);
      mDeactivateLight->SetOrientation(GetTransform().GetRotation());
      mDeactivateLight->SetGlobalTranslation(lightPos);
      mDeactivateLight->SetGlobalScale(GetModelScale());
      mDeactivateLight->Update(dt);
      if (light) {
        if (mDeactivateLight->SystemHasLight()) {
          light->SetLight(mDeactivateLight->GetLight());
        } else {
          light->SetActive(false);
        }
      }
      break;
    case kTS_Inactive:
      mIdleLight->SetParticleEmission(true);
      mDeactivateLight->SetParticleEmission(false);
      mTargettingLight->SetParticleEmission(false);
      mFrozenEffect->SetParticleEmission(false);
      mChargingEffect->SetParticleEmission(false);
      mPanningEffect->SetParticleEmission(false);
      mIdleLight->SetOrientation(GetTransform().GetRotation());
      mIdleLight->SetGlobalTranslation(lightPos);
      mIdleLight->SetGlobalScale(GetModelScale());
      mIdleLight->Update(dt);
      if (light) {
        light->SetActive(false);
      }
      break;
    case kTS_PanningA:
    case kTS_PanningB:
      mIdleLight->SetParticleEmission(false);
      mDeactivateLight->SetParticleEmission(false);
      mTargettingLight->SetParticleEmission(false);
      mFrozenEffect->SetParticleEmission(false);
      mChargingEffect->SetParticleEmission(false);
      mPanningEffect->SetParticleEmission(true);
      mPanningEffect->SetOrientation(GetTransform().GetRotation());
      mPanningEffect->SetGlobalTranslation(lightPos);
      mPanningEffect->SetGlobalScale(GetModelScale());
      mPanningEffect->Update(dt);
      if (light) {
        light->SetActive(false);
      }
      break;
    case kTS_Ready:
    case kTS_Targeting:
    case kTS_Firing:
    case kTS_ExitTargeting:
    case kTS_Frenzy: {
      const bool charging = mState == kTS_Firing || mState == kTS_Frenzy;
      mIdleLight->SetParticleEmission(false);
      mDeactivateLight->SetParticleEmission(false);
      mTargettingLight->SetParticleEmission(true);
      mFrozenEffect->SetParticleEmission(false);
      mChargingEffect->SetParticleEmission(charging);
      mTargettingLight->SetOrientation(GetTransform().GetRotation());
      mTargettingLight->SetGlobalTranslation(lightPos);
      mTargettingLight->SetGlobalScale(GetModelScale());
      mTargettingLight->Update(dt);
      if (mTargettingLight->SystemHasLight()) {
        light->SetLight(mTargettingLight->GetLight());
      } else {
        light->SetActive(false);
      }
      if (charging) {
        const CTransform4f blastXf = GetLocatorTransform(rstl::string_l(skBlastLCTRName));
        const CVector3f blastPos =
            GetTranslation() + GetTransform().Rotate(blastXf.GetTranslation());
        mChargingEffect->SetOrientation(GetTransform().GetRotation());
        mChargingEffect->SetGlobalTranslation(blastPos);
        mChargingEffect->SetGlobalScale(GetModelScale());
        mChargingEffect->Update(dt);
      }
      break;
    }
    case kTS_Destroyed:
    default:
      mIdleLight->SetParticleEmission(false);
      mDeactivateLight->SetParticleEmission(false);
      mTargettingLight->SetParticleEmission(false);
      mFrozenEffect->SetParticleEmission(false);
      mChargingEffect->SetParticleEmission(false);
      mPanningEffect->SetParticleEmission(false);
      mPanningEffect->Update(dt);
      if (light) {
        light->SetActive(false);
      }
      break;
    }
  } else {
    mIdleLight->SetParticleEmission(false);
    mDeactivateLight->SetParticleEmission(false);
    mTargettingLight->SetParticleEmission(false);
    mFrozenEffect->SetParticleEmission(true);
    mChargingEffect->SetParticleEmission(false);
    mPanningEffect->SetParticleEmission(false);
    mFrozenEffect->SetOrientation(GetTransform().GetRotation());
    mFrozenEffect->SetGlobalTranslation(GetTranslation());
    mFrozenEffect->SetGlobalScale(GetModelScale());
    mFrozenEffect->Update(dt);
    if (light) {
      light->SetActive(false);
    }
  }
}

void CScriptGunTurret::UpdateBurstType(CStateManager& mgr) {
  if (mBurstSet) {
    const bool inView = mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Morphed ||
                        CVector3f::Dot(GetTransform().GetForward(),
                                       mgr.GetPlayer()->GetTransform().GetForward()) < 0.f;
    const int viewOffset = inView ? 0 : 3;
    const int shots = mgr.Random()->Range(0, 3) + 2;
    int type;
    if (shots <= 2 || static_cast< int >(mData.GetNumSubsequentShots()) < 3) {
      type = 0;
    } else if (shots >= 5 && static_cast< int >(mData.GetNumSubsequentShots()) > 3) {
      type = 2;
    } else {
      type = 1;
    }
    mBurstFire.SetBurstType(type + viewOffset);
  } else {
    mBurstFire.SetBurstType(mData.GetNumInitialShots() - 2);
    mBurstFire.SetFirstBurstIndex(mData.GetInitialShotTableIndex());
  }
  mBurstFire.Start(mgr);
  mFiredWithSetBurst = false;
  mBurstSet = true;
}

void CScriptGunTurret::UpdateTargettingSound(float dt) {
  mTimeSinceLastTargetSfx += dt;
  const CVector3f& front = GetTransform().GetForward();
  const CVector2f front2d = front.ToVec2f();
  const CVector2f lastFront2d = mLastFrontVector.ToVec2f();
  const float angle = CVector2f::GetAngleDiff(lastFront2d, front2d);
  if (mNeedsStopClankSound && angle < 20.f * (M_PIF / 180.f) &&
      (mState == kTS_Targeting || mState == kTS_Firing)) {
    if (!mFrozen) {
      CSfxManager::AddEmitter(mData.GetStopClankSoundId(), GetTranslation(), CVector3f::Up(),
                              false, false, CSfxManager::kMedPriority, GetCurrentAreaId().Value());
    }
    mNeedsStopClankSound = false;
  }
  if (mTimeSinceLastTargetSfx >= 0.5f && !mFrozen) {
    if (mState == kTS_Targeting || mState == kTS_Firing || mState == kTS_Frenzy) {
      const bool stopped = IsStopped(dt);
      if (!stopped && !mTargetingEmitter) {
        mTargetingEmitter = CSfxManager::AddEmitter(
            mData.GetTrackingSoundId(), GetTranslation(), CVector3f::Zero(), false, true,
            CSfxManager::kMedPriority, GetCurrentAreaId().Value());
        mTimeSinceLastTargetSfx = 0.f;
      } else if (stopped && mTargetingEmitter) {
        CSfxManager::RemoveEmitter(mTargetingEmitter);
        mTargetingEmitter.Clear();
        mTimeSinceLastTargetSfx = 0.f;
      }
      if (mTargetingEmitter) {
        const float maxAngle = dt * mData.GetTurnSpeed();
        const float ratio = maxAngle > 0.f ? angle / maxAngle : 0.f;
        const float pitch = rstl::min_val(8192.f * ratio, 1.f);
        CSfxManager::PitchBend(mTargetingEmitter, 0x2000 + static_cast< int >(pitch));
      }
    } else if (mTargetingEmitter) {
      CSfxManager::RemoveEmitter(mTargetingEmitter);
      mTargetingEmitter.Clear();
      mTimeSinceLastTargetSfx = 0.f;
    }
  } else if (mFrozen && mTargetingEmitter) {
    CSfxManager::RemoveEmitter(mTargetingEmitter);
    mTargetingEmitter.Clear();
  }
  mLastFrontVector = GetTransform().GetForward();
}

bool CScriptGunTurret::LineOfSightTest(CStateManager& mgr) const {
  if (mGunId != kInvalidUniqueId) {
    if (const CScriptGunTurret* const gun =
            TCastToPtr< CScriptGunTurret >(mgr.ObjectById(mGunId))) {
      if (mBurstSet || (mState == kTS_Inactive && mExtensionModel)) {
        return true;
      }
      const CTransform4f blastXf = gun->GetLocatorTransform(rstl::string_l(skBlastLCTRName));
      const CVector3f muzzlePos =
          gun->GetTranslation() + gun->GetTransform().Rotate(blastXf.GetTranslation());
      CVector3f dir = mgr.GetPlayer()->GetAimPosition(mgr, 0.f) - muzzlePos;
      const float distance = dir.Magnitude();
      dir /= distance;
      TEntityList nearList;
      const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
          CMaterialList(kMT_Solid), CMaterialList(kMT_Player, kMT_CollisionActor));
      mgr.BuildNearList(nearList, muzzlePos, dir, distance, filter, gun);
      TUniqueId id = kInvalidUniqueId;
      return mgr.RayWorldIntersection(id, muzzlePos, dir, distance, filter, nearList).IsInvalid();
    }
  }
  return false;
}

void CScriptGunTurret::SetupCollisionManager(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > jointDescs;
  jointDescs.reserve(2);
  const CAnimData* animData = GetAnimationData();
  mGunSDKSeg = animData->GetLocatorSegId(rstl::string_l(skGunLCTRName));
  const CSegId blastSeg = animData->GetLocatorSegId(rstl::string_l(skBlastLCTRName));
  const CJointCollisionDescription gunDesc = CJointCollisionDescription::SphereSubdivideCollision(
      mGunSDKSeg, blastSeg, 0.6f, 1.f, CJointCollisionDescription::kOT_One,
      rstl::string_l(skGunLCTRName), 1000.f);
  jointDescs.push_back(gunDesc);
  const CJointCollisionDescription blastDesc = CJointCollisionDescription::SphereCollision(
      blastSeg, 0.3f, rstl::string_l(skBlastLCTRName), 1000.f);
  jointDescs.push_back(blastDesc);
  mCollisionManager =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), jointDescs, true);
  mCollisionManager->SetActive(mgr, GetActive());
  for (uint i = 0; i < mCollisionManager->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = mCollisionManager->GetCollisionDescFromIndex(i);
    const TUniqueId actorId = desc.GetCollisionActorId();
    if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(actorId))) {
      actor->AddMaterial(kMT_ProjectilePassthrough, mgr);
      actor->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
          CMaterialList(kMT_Player),
          CMaterialList(kMT_Character, kMT_NoStaticCollision, kMT_NoPlatformCollision)));
      if (desc.GetName() == rstl::string_l(skBlastLCTRName)) {
        mCollisionActor = actorId;
      }
    }
  }
}

void CScriptGunTurret::UpdateGunCollisionManager(float dt, CStateManager& mgr) {
  if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(mCollisionActor))) {
    actor->SetActive(mgr.GetPlayer()->GetMorphballTransitionState() != CPlayer::kMS_Morphed);
  }
  mCollisionManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
}

void CScriptGunTurret::UpdateFrozenState(float dt, CStateManager& mgr) {
  if (mFrozen) {
    if (mFreezeRemTime <= 0.f) {
      mFrozen = false;
      SendScriptMsgs(kSS_UnFrozen, mgr, kSM_None);
      CSfxManager::AddEmitter(mData.GetUnFreezeSoundId(), GetTranslation(), CVector3f::Up(),
                              false, false, CSfxManager::kMedPriority, GetCurrentAreaId().Value());
      SetMuted(false);
    } else if (mData.UseFreezeTimeout()) {
      mFreezeRemTime -= dt;
    }
  } else {
    mFreezeRemTime = 0.f;
  }
}

bool CScriptGunTurret::PlayerInsideTurretSphere(CStateManager& mgr) {
  if (const CCollisionActor* actor =
          TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(mCollisionActor))) {
    if (actor->GetActive()) {
      const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
      const CVector3f delta = aimPos - GetTranslation();
      if (delta.GetZ() < 0.f) {
        const CVector3f actorDelta = actor->GetTranslation() - GetTranslation();
        const float radius = actor->GetSphereRadius() * 2.f + actorDelta.Magnitude();
        return delta.MagSquared() < radius * radius;
      }
    }
  }
  return false;
}
