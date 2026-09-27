#include "MetroidPrime/Enemies/COmegaPirate.hpp"

#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CVertexMorphEffect.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Enemies/CGrenadeLauncher.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CAuxWeapon.hpp"
#include "rstl/StringExtras.hpp"

#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSound.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"

const char* const COmegaPirate::skpGrenadeLauncher2LCTR = "grenadeLauncher2_LCTR";
const SSphereJointInfo COmegaPirate::skSphereJointList[1] = {{"lockon_target_LCTR", 1.f}};
const COmegaPirate::SOBBoxJointInfo COmegaPirate::skOBBJointList[11] = {
    {"Spine_2", "Collar", 1.f, 1.f, 1.f},          {"R_ankle", "R_toe_1", 1.f, 1.f, 1.f},
    {"L_ankle", "L_toe_1", 1.f, 1.f, 1.f},         {"R_knee", "R_ankle", 1.f, 1.f, 1.f},
    {"L_knee", "L_ankle", 1.f, 1.f, 1.f},          {"R_elbow", "R_wrist", 1.f, 1.f, 1.f},
    {"L_elbow", "L_wrist", 1.f, 1.f, 1.f},         {"R_wrist", "R_index_1", 1.f, 1.f, 1.f},
    {"L_wrist", "L_index_1", 1.f, 1.f, 1.f},       {"R_index_1", "R_index_3_SDK", 2.f, 2.f, 2.f},
    {"L_index_1", "L_index_3_SDK", 2.f, 2.f, 2.f},
};

COmegaPirate::COmegaPirate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf, const CModelData& mData,
                           const CPatternedInfo& pInfo, const CActorParameters& actParms,
                           const CElitePirateData& data, int skeletonModelId,
                           int skeletonSkinRulesId, int skeletonLayoutInfoId)
: CElitePirate(uid, name, info, xf, mData, pInfo, actParms, data)
, mLauncherId2(kInvalidUniqueId)
, mNormalFadeState(kNFS_Zero)
, mNormalFadeTime(0.f)
, mNormalAlpha(1.f)
, mVisible(true)
, mFadeIn(true)
, mLostAllHp(false)
, mScaleState(kSS_None)
, mScaleTime(0.f)
, mInitialScale(mData.ScaleCopy())
, mDecrement(false)
, mSkeletonModel(gpSimplePool->GetObj(SObjectTag('CMDL', skeletonModelId)),
                     gpSimplePool->GetObj(SObjectTag('CSKR', skeletonSkinRulesId)),
                     gpSimplePool->GetObj(SObjectTag('CINF', skeletonLayoutInfoId)),
                     CSkinnedModel::kDO_Owned)
, mSkeletonAlpha(0.f)
, mSkeletonFadeState(kSFS_None)
, mSkeletonStateTime(0.f)
, mCollisionActorMgr1(nullptr)
, mHearPlayer(false)
, mLocomotionType(pas::kLT_Relaxed)
, mTargetable(false)
, xa46_(kInvalidUniqueId)
, xa48_(kInvalidUniqueId)
, mHeartVisible(false)
, mInitialXf(CTransform4f::Identity())
, mXrayAlphaState(kXFS_None)
, mXrayAlpha(1.f)
, mXrayAlphaStateTime(0.f)
, mXrayFadeInTrigger(false)
, mXrayFadeOutTime(3.f)
, mXrayFadeInTime(1.f)
, mXrayFadeTriggerTime(1.f)
, mMaxEnergy(0.f)
, mCollisionActorMgr2(nullptr)
, xab0_(0.f)
, xac4_(0)
, xac8_(0)
, xacc_(0)
, mScaleUpTrigger(false)
, mCachedSpeed(1.f)
, mCover(false)
, mLastWaypointId(kInvalidUniqueId)
, xadc_(0)
, xadd_(0)
, mArmorPiecesDestroyed(0)
, mLauncher1FollowPlayer(true)
, mLauncher2FollowPlayer(true)
, mPlatformVuln(CDamageVulnerability::NormalVulnerability())
, mArmorPiecesHealed(0)
, mArmorPieceHealTime(0.f)
, mPlatformColor(CColor::White())
, mHealTime(2.5f)
, mHpLost(0.f)
, mHpLostInPhase(0.f)
, mStateTime(17.f)
, xb68_(0)
, mExit1Sent(false)
, mExit2Sent(false)
, mArmorPieceActivated(false)
, mThermalSpot(gpSimplePool->GetObj("Thermal_Spot_2"))
, mCodeTrigger(false)
, mBossPhaseActive(0) {
  mScriptWaypointPlatforms.reserve(3);
  mScriptEffects.reserve(24);
  mScriptPlatforms.reserve(4);
  mScriptSounds.reserve(4);
  xab4_.reserve(3);
  mThermalSpot.Lock();
  SetMass(100000.f);
  xb7c_.reserve(4);
  for (uint i = 0; i < 4; ++i) {
    xb7c_.push_back(0);
  }
  CMaterialList exclude = GetMaterialFilter().GetExcludeList();
  exclude.Add(CMaterialList(kMT_Character, kMT_CollisionActor, kMT_Platform));
  SetMaterialFilter(
      CMaterialFilter::MakeIncludeExclude(GetMaterialFilter().GetIncludeList(), exclude));
  GetSearchPath()->SetPadding(20.f);
}

void COmegaPirate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered: {
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    mLauncherId2 = mgr.AllocateUniqueId();
    CreateGrenadeLauncher(mgr, mLauncherId2);
    InitializeOmegaPirateCollisionManagers(mgr);
    mBodyController->SetLocomotionType(pas::kLT_Internal8);
    mNoXrayModel = false;
    mInitialXf = GetTransform();
    mMaxEnergy = HealthInfo(mgr)->GetHP();
    CActor* launcher1 = static_cast< CActor* >(mgr.ObjectById(GetLauncherId()));
    CActor* launcher2 = static_cast< CActor* >(mgr.ObjectById(mLauncherId2));
    if (launcher1) {
      launcher1->RemoveMaterial(kMT_Scannable, mgr);
    }
    if (launcher2) {
      launcher2->RemoveMaterial(kMT_Scannable, mgr);
    }
    KnockBackCtrl().SetAutoResetImpulse(false);
    SetupPathFindSearch();
    break;
  }
  case kSM_Activate:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    mCollisionActorMgr1->SetActive(mgr, true);
    mCollisionActorMgr2->SetActive(mgr, true);
    KnockBackCtrl().SetAutoResetImpulse(false);
    if (CEntity* entity = mgr.ObjectById(mLauncherId2)) {
      entity->SetActive(true);
    }
    break;
  case kSM_Deactivate:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    mCollisionActorMgr1->SetActive(mgr, false);
    mCollisionActorMgr2->SetActive(mgr, false);
    if (CEntity* entity = mgr.ObjectById(mLauncherId2)) {
      entity->SetActive(false);
    }
    break;
  case kSM_Deleted:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    mCollisionActorMgr1->Destroy(mgr);
    mCollisionActorMgr2->Destroy(mgr);
    mgr.DeleteObjectRequest(mLauncherId2);
    break;
  case kSM_Alert:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    break;
  case kSM_InitializedInArea: {
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    const uint count = GetConnectionList().size();
    for (uint i = 0; i < count; ++i) {
      const SConnection& conn = GetConnectionList()[i];
      const TUniqueId connId = mgr.GetIdForScript(conn.mObjId);
      if (conn.mState == kSS_Attack && connId != kInvalidUniqueId) {
        if (conn.mMsg == kSM_Activate) {
          if (CScriptEffect* effect = TCastToPtr< CScriptEffect >(mgr.ObjectById(connId))) {
            mScriptEffects.push_back(
                rstl::pair< TUniqueId, rstl::string >(connId, effect->GetDebugName()));
          } else if (CScriptPlatform* platform =
                         TCastToPtr< CScriptPlatform >(mgr.ObjectById(connId))) {
            mScriptPlatforms.push_back(
                rstl::pair< TUniqueId, rstl::string >(connId, platform->GetDebugName()));
            platform->AddMaterial(kMT_Target, mgr);
            platform->AddMaterial(kMT_Orbit, mgr);
            platform->AddMaterial(kMT_Character, mgr);
            platform->RemoveMaterial(kMT_Scannable, mgr);
            CMaterialList excludes = platform->GetMaterialFilter().GetExcludeList();
            excludes.Add(CMaterialList(kMT_Player, kMT_Character, kMT_CollisionActor));
            const CMaterialList includes = GetMaterialFilter().GetIncludeList();
            platform->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(includes, excludes));
            mPlatformVuln = *platform->GetDamageVulnerability();
            mPlatformColor = platform->GetModelFlags().GetColor();
          } else if (CScriptSound* sound = TCastToPtr< CScriptSound >(mgr.ObjectById(connId))) {
            mScriptSounds.push_back(
                rstl::pair< TUniqueId, rstl::string >(connId, sound->GetDebugName()));
          }
        } else if (conn.mMsg == kSM_Follow) {
          if (CScriptWaypoint* waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(connId))) {
            const uint waypointCount = waypoint->GetConnectionList().size();
            rstl::vector< TUniqueId > waypointPlatformIds;
            waypointPlatformIds.reserve(3);
            for (uint j = 0; j < waypointCount; ++j) {
              const TUniqueId waypointConnId =
                  mgr.GetIdForScript(waypoint->GetConnectionList()[j].mObjId);
              if (CScriptPlatform* platform =
                      TCastToPtr< CScriptPlatform >(mgr.ObjectById(waypointConnId))) {
                platform->AddMaterial(kMT_Target, mgr);
                platform->AddMaterial(kMT_Orbit, mgr);
                waypointPlatformIds.push_back(waypointConnId);
              }
            }
            mScriptWaypointPlatforms.push_back(
                rstl::pair< TUniqueId, rstl::vector< TUniqueId > >(connId, waypointPlatformIds));
          }
        }
      }
    }
    break;
  }
  case kSM_Touched: {
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    if (uid == mLauncherId2 && mLauncherId2 != kInvalidUniqueId) {
      SetShotAt(true, mgr);
    }
    CEntity* touchedEntity = mgr.ObjectById(uid);
    if (const CCollisionActor* actor = TCastToConstPtr< CCollisionActor >(touchedEntity)) {
      const TUniqueId touchedId = actor->GetLastTouchedObject();
      CEntity* playerEntity = mgr.ObjectById(touchedId);
      if (const CPlayer* player = TCastToConstPtr< CPlayer >(playerEntity)) {
        if (mCurDamageRemTime <= 0.f) {
          mgr.ApplyDamage(
              GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              CVector3f::Zero());
          mCurDamageRemTime = mDamageWaitTime;
        }
      }
    }
    break;
  }
  case kSM_Damage:
    if (uid == mLauncherId2 && mLauncherId2 != kInvalidUniqueId) {
      mBodyController->CommandMgr().DeliverCmd(
          CBCKnockBackCmd(GetTransform().GetForward(), pas::kS_Eight));
    }
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    if (uid == xa46_ && mXrayAlphaState == kXFS_WaitForTrigger) {
      mXrayAlphaState = kXFS_FadeOut;
      mXrayAlphaStateTime = 0.f;
    }
    break;
  case kSM_InvulnDamage:
    if (const CGameProjectile* projectile =
            TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(uid))) {
      if (mHeartVisible) {
        mgr.ApplyDamage(
            uid, xa46_, projectile->GetOwnerId(), projectile->GetCurrentDamageInfo(),
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
      }
    }
    SetShotAt(true, mgr);
    break;
  case kSM_Decrement:
    mDecrement = true;
    break;
  case kSM_Increment:
    SetShotAt(true, mgr);
    break;
  case kSM_SetToMax:
    mHearPlayer = true;
    break;
  case kSM_Stop:
    KillOmegaPirate(mgr);
    break;
  case kSM_Start:
    mSpeed = 1.f;
    ++mArmorPiecesDestroyed;
    if (mArmorPiecesDestroyed < 4) {
      mBodyController->CommandMgr().DeliverCmd(CBCKnockBackCmd(CVector3f::Left(), pas::kS_One));
    }
    break;
  case kSM_Action:
    mSpeed = 1.f;
    ++mArmorPiecesDestroyed;
    if (mArmorPiecesDestroyed < 4) {
      mBodyController->CommandMgr().DeliverCmd(
          CBCKnockBackCmd(CVector3f::Right(), pas::kS_One));
    }
    break;
  case kSM_Reset:
    mCodeTrigger = true;
#if VERSION >= VERSION_GM8E_01
    mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
#endif
    break;
  case kSM_Open:
    if (xb7c_[3] != 0) {
      --xb7c_[3];
    } else {
      xb7c_[3] = 0;
    }
    break;
  case kSM_SetToZero:
    if (xb7c_[2] != 0) {
      --xb7c_[2];
    } else {
      xb7c_[2] = 0;
    }
    break;
  case kSM_StopAndReset:
    if (xb7c_[1] != 0) {
      --xb7c_[1];
    } else {
      xb7c_[1] = 0;
    }
    break;
  case kSM_UNKM18:
    if (xb7c_[0] != 0) {
      --xb7c_[0];
    } else {
      xb7c_[0] = 0;
    }
    break;
  default:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    break;
  }
}

CVector3f COmegaPirate::GetOrbitPosition(const CStateManager& mgr) const {
  if (mLauncherId2 != kInvalidUniqueId &&
      mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Thermal) {
    if (const CActor* actor = static_cast< const CActor* >(mgr.GetObjectById(mLauncherId2))) {
      return GetGrenadeLaunchPos(*actor);
    }
  }
  return CElitePirate::GetOrbitPosition(mgr);
}

void COmegaPirate::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                   EUserEventType type, float dt) {
  switch (type) {
  case kUE_Projectile:
    CElitePirate::DoUserAnimEvent(mgr, node, type, dt);
    break;
  case kUE_DamageOn:
    CElitePirate::DoUserAnimEvent(mgr, node, type, dt);
    break;
  case kUE_DamageOff:
    CElitePirate::DoUserAnimEvent(mgr, node, type, dt);
    break;
  case kUE_ScreenShake:
    CElitePirate::DoUserAnimEvent(mgr, node, type, dt);
    break;
  case kUE_BeginAction:
    CElitePirate::DoUserAnimEvent(mgr, node, type, dt);
    break;
  case kUE_BecomeShootThrough:
    CElitePirate::DoUserAnimEvent(mgr, node, type, dt);
    break;
  case kUE_EggLay:
    if (mLauncherId2 != kInvalidUniqueId) {
      if (CEntity* entity = mgr.ObjectById(mLauncherId2)) {
        mgr.DeliverScriptMsg(entity, GetUniqueId(), kSM_Action);
      }
    }
    break;
  case kUE_FadeOut:
    if (mNormalFadeState != kNFS_Two && mFadeIn) {
      mNormalFadeState = kNFS_One;
      mSkeletonFadeState = kSFS_FadeIn;
    }
    break;
  case kUE_FadeIn:
    mFadeIn = true;
    break;
  case kUE_ObjectPickUp:
    xab4_.clear();
    xac8_ = 0;
    ++xacc_;
    switch (xac4_) {
    case 0:
      QueueTrooperPiratesOfOneRandomColor(2, mgr);
      break;
    case 1:
      QueueTrooperPiratesOfOneRandomColor(1, mgr);
      QueueTrooperPiratesOfOneRandomColor(1, mgr);
      break;
    case 2:
      QueueTrooperPiratesOfOneRandomColor(2, mgr);
      QueueTrooperPiratesOfOneRandomColor(1, mgr);
      break;
    case 3:
      QueueTrooperPiratesOfOneRandomColor(1, mgr);
      QueueTrooperPiratesOfOneRandomColor(1, mgr);
      QueueTrooperPiratesOfOneRandomColor(1, mgr);
      break;
    }
    SendScriptMsgs(kSS_Arrived, mgr, kSM_None);
    break;
  default:
    CElitePirate::DoUserAnimEvent(mgr, node, type, dt);
    break;
  }
}

bool COmegaPirate::ShouldFire(CStateManager& mgr, float arg) {
  return CElitePirate::ShouldFire(mgr, arg) || ShouldFireLauncher(mgr, mLauncherId2);
}

bool COmegaPirate::ShouldCallForBackup(CStateManager& mgr, float arg) {
  return CElitePirate::ShouldCallForBackup(mgr, arg) &&
         ShouldCallForBackupForLauncher(mgr, arg, mLauncherId2);
}

bool COmegaPirate::ShotAt(CStateManager& mgr, float arg) { return CElitePirate::ShotAt(mgr, arg); }

bool COmegaPirate::AggressionCheck(CStateManager& mgr, float arg) {
  return mLauncherId2 == kInvalidUniqueId && CElitePirate::AggressionCheck(mgr, arg);
}

void COmegaPirate::SetupHealthInfo(CStateManager& mgr) {
  CElitePirate::SetupHealthInfo(mgr);
  SetupHealthInfoForLauncher(mgr, mLauncherId2);
}

void COmegaPirate::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  mAlert = true;
  CElitePirate::Think(dt, mgr);
  const float highHp = 0.7f * mMaxEnergy;
  const float maxHp = mMaxEnergy;
  if (HealthInfo(mgr)->GetHP() <= 0.2f * maxHp) {
    xac4_ = 3;
  } else if (HealthInfo(mgr)->GetHP() <= highHp) {
    xac4_ = 2;
  } else if (xacc_ > 4) {
    xac4_ = 1;
  }
  UpdateGrenadeLauncher(mgr, mLauncherId2, rstl::string_l(skpGrenadeLauncher2LCTR));
  UpdateInvisibility(mgr, dt);
  UpdateVeinsModel(mgr, dt);
  UpdateVeinsModelGlowEffect(mgr, dt);
  if ((!mFadeIn || mHeartVisible) &&
      mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay && mTargetable) {
    AddMaterial(kMT_Target, mgr);
    AddMaterial(kMT_Orbit, mgr);
    if (mScaleState == kSS_WaitForTrigger) {
      mCollisionActorMgr2->SetActive(mgr, false);
      mCollisionActorMgr1->SetActive(mgr, false);
    } else {
      mCollisionActorMgr1->SetActive(mgr, true);
      mCollisionActorMgr2->SetActive(mgr, true);
      if (CEntity* entity = mgr.ObjectById(xa48_)) {
        entity->SetActive(false);
      }
    }
  } else {
    RemoveMaterial(kMT_Target, mgr);
    RemoveMaterial(kMT_Orbit, mgr);
    mCollisionActorMgr1->SetActive(mgr, false);
    if (mFadeIn) {
      mCollisionActorMgr2->SetActive(mgr, true);
      if (CEntity* entity = mgr.ObjectById(xa48_)) {
        entity->SetActive(true);
      }
    } else {
      mCollisionActorMgr2->SetActive(mgr, false);
      if (CEntity* entity = mgr.ObjectById(xa48_)) {
        entity->SetActive(false);
      }
    }
  }
  UpdateTeleportEffect(mgr, dt);
  mCollisionActorMgr1->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  mCollisionActorMgr2->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  if (CActor* actor = static_cast< CActor* >(mgr.ObjectById(xa46_))) {
    const float hp = HealthInfo(mgr)->GetHP();
    *HealthInfo(mgr) = *actor->HealthInfo(mgr);
    const float damage = hp - HealthInfo(mgr)->GetHP();
    mHpLost += damage;
    mHpLostInPhase += damage;
  }
  if (HealthInfo(mgr)->GetHP() <= 0.f) {
    KillOmegaPirate(mgr);
  } else {
    if (mHpLost > 100.f) {
      mLostAllHp = true;
    } else if (mHpLostInPhase > 20.f) {
      mBodyController->CommandMgr().DeliverCmd(
          CBCAdditiveReactionCmd(pas::kART_One, 1.f, false));
      mHpLostInPhase = 0.f;
    }
  }
  SpawnNextQueuedTrooperPirate(mgr, dt);
  const uint numPlatforms = mScriptPlatforms.size();
  for (uint i = 0; i < numPlatforms; ++i) {
    CActor* platform = static_cast< CActor* >(mgr.ObjectById(mScriptPlatforms[i].first));
    if ((mCodeTrigger || mBossPhaseActive) && !mHeartVisible) {
      platform->AddMaterial(kMT_Target, mgr);
      platform->AddMaterial(kMT_Orbit, mgr);
    } else {
      platform->RemoveMaterial(kMT_Target, mgr);
      platform->RemoveMaterial(kMT_Orbit, mgr);
    }
  }
  const CPlayerState& playerState = *mgr.GetPlayerState();
  if (mCollisionActorMgr->GetActive() && playerState.IsFiringComboBeam() &&
      playerState.GetCurrentBeam() == CPlayerState::kBI_Wave && mCover) {
    AddMaterial(kMT_Target, mgr);
    mgr.Player()->SetAimTargetId(GetUniqueId());
    const uint count = mScriptPlatforms.size();
    for (uint i = 0; i < count; ++i) {
      if (CActor* platform =
              static_cast< CActor* >(mgr.ObjectById(mScriptPlatforms[i].first))) {
        platform->RemoveMaterial(kMT_Target, mgr);
      }
    }
    mgr.Player()->PlayerGun()->AuxWeapon().SetNewTarget(GetUniqueId(), mgr);
  } else if (!mHeartVisible) {
    RemoveMaterial(kMT_Target, mgr);
    const uint count = mScriptPlatforms.size();
    for (uint i = 0; i < count; ++i) {
      if (CActor* platform =
              static_cast< CActor* >(mgr.ObjectById(mScriptPlatforms[i].first))) {
        if (mCodeTrigger || mBossPhaseActive) {
          platform->AddMaterial(kMT_Target, mgr);
        }
      }
    }
    if (mgr.Player()->PlayerGun()->AuxWeapon().HasTarget(mgr) == GetUniqueId()) {
      CPlayer& player = *mgr.Player();
      if (player.ValidateOrbitTargetId(player.GetOrbitTargetId(), mgr) == CPlayer::kOVR_OK) {
        mgr.Player()->PlayerGun()->AuxWeapon().SetNewTarget(player.GetOrbitTargetId(), mgr);
      } else {
        mgr.Player()->PlayerGun()->AuxWeapon().SetNewTarget(kInvalidUniqueId, mgr);
      }
    }
  }
  if (CGrenadeLauncher* launcher =
          static_cast< CGrenadeLauncher* >(mgr.ObjectById(GetLauncherId()))) {
    launcher->SetFollowPlayer(mLauncher1FollowPlayer);
  }
  if (CGrenadeLauncher* launcher =
          static_cast< CGrenadeLauncher* >(mgr.ObjectById(mLauncherId2))) {
    launcher->SetFollowPlayer(mLauncher2FollowPlayer);
  }
  if (mDecrement) {
    mDecrement = false;
    mStateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("JumpBack"));
  }
  if (xb68_ >= 2) {
    DoUserAnimEvent(mgr,
                    CInt32POINode(rstl::string_l(""), kPT_EmptyInt32, CCharAnimTime(0.f), -1, false,
                                  1.f, -1, 0, 0, rstl::string_l("root")),
                    kUE_ObjectPickUp, dt);
    xb68_ = 0;
  }
  if (mAvoidStaticCollisionTime > 0.f) {
    const CAABox bounds = GetBoundingBox();
    CGameCollision::AvoidStaticCollisionWithinRadius(
        mgr, *this, 8, dt, 1.f, 1.5f * (bounds.GetMaxPoint().GetX() - bounds.GetMinPoint().GetX()),
        10000.f, 0.25f);
    mAvoidStaticCollisionTime = 0.f;
  }
  mAvoidStaticCollisionTime += dt;
}

void COmegaPirate::ActivateGrenadeLauncher(CStateManager& mgr, bool val) {
  CElitePirate::ActivateGrenadeLauncher(mgr, val);
  ActivateGrenadeLauncherById(mgr, val, mLauncherId2);
}

void COmegaPirate::UpdateInvisibility(CStateManager& mgr, float dt) {
  switch (mNormalFadeState) {
  case kNFS_One: {
    const float duration = 1.25f;
    mNormalAlpha = 1.f - rstl::min_val(mNormalFadeTime, duration) / duration;
    mColor.SetAlpha(mNormalAlpha);
    if (mNormalFadeTime > duration) {
      mNormalFadeState = kNFS_Two;
      mFadeIn = false;
      mNormalFadeTime = 0.f;
    }
    mNormalFadeTime += dt;
    mVisible = true;
    break;
  }
  case kNFS_Three: {
    const float duration = 1.f;
    mNormalAlpha = rstl::min_val(mNormalFadeTime, duration) / 1.25f;
    if (mNormalFadeTime > duration) {
      mNormalFadeState = kNFS_Zero;
      mNormalFadeTime = 0.f;
    }
    mNormalFadeTime += dt;
    mVisible = true;
    break;
  }
  case kNFS_Two:
    mNormalAlpha = 0.f;
    if (mNormalFadeTime > 1.5f && mFadeIn) {
      CreateFlash(mgr, 0.f);
      mNormalFadeState = kNFS_Three;
      mNormalFadeTime = 0.f;
    }
    mNormalFadeTime += dt;
    mVisible = false;
    break;
  default:
    mNormalAlpha = 1.f;
    mVisible = true;
    break;
  }
  float alpha = mNormalAlpha;
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay) {
    alpha = 0.f;
    mNormalAlpha = 1.f;
    mVisible = true;
  }
  mColor.SetAlpha(mNormalAlpha);
  if (alpha < 1.f) {
    if (CGrenadeLauncher* launcher =
            static_cast< CGrenadeLauncher* >(mgr.ObjectById(GetLauncherId()))) {
      launcher->SetAddColor(CColor(1.f, 1.f, 1.f, alpha));
      if (alpha == 0.f) {
        launcher->SetVisible(false);
      } else {
        launcher->SetVisible(true);
      }
    }
    if (CGrenadeLauncher* launcher =
            static_cast< CGrenadeLauncher* >(mgr.ObjectById(mLauncherId2))) {
      launcher->SetAddColor(CColor(1.f, 1.f, 1.f, alpha));
      if (alpha == 0.f) {
        launcher->SetVisible(false);
      } else {
        launcher->SetVisible(true);
      }
    }
  } else {
    if (CGrenadeLauncher* launcher =
            static_cast< CGrenadeLauncher* >(mgr.ObjectById(GetLauncherId()))) {
      launcher->SetVisible(true);
      launcher->SetAddColor(CColor(0.f, 0.f, 0.f, 1.f));
    }
    if (CGrenadeLauncher* launcher =
            static_cast< CGrenadeLauncher* >(mgr.ObjectById(mLauncherId2))) {
      launcher->SetAddColor(CColor(0.f, 0.f, 0.f, 1.f));
      launcher->SetVisible(true);
    }
  }
}

void COmegaPirate::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  CElitePirate::PreRender(mgr, frustum);
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay) {
    SetModelFlags(
        CModelFlags::ColorModulate(CColor(mXrayAlpha, mXrayAlpha, mXrayAlpha, 1.f))
            .DepthCompareUpdate(true, true));
  }
}

void COmegaPirate::Render(const CStateManager& mgr) const {
  const CTransform4f xf = GetTransform() * CTransform4f::Scale(GetModelData()->ScaleCopy());
  CGraphics::SetModelMatrix(xf);
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_XRay &&
      mSkeletonAlpha > 0.f) {
    GetAnimationData()->Render(mSkeletonModel,
                               CModelFlags::AlphaBlended(CColor(1.f, 1.f, 1.f, mSkeletonAlpha))
                                   .DepthCompareUpdate(true, true),
                               rstl::optional_object< CVertexMorphEffect >(), nullptr);
  }
  if (mVisible) {
    const bool xray = mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay;
    if (xray) {
      gpRender->SetWorldFog(kRFM_None, 0.f, 1.f, CColor::Black());
    }
    if (xray) {
      GetAnimationData()->Render(
          **GetAnimationData()->GetModelData(),
          CModelFlags::AlphaBlended(CColor(1.f, 1.f, 1.f, 0.2f)).DepthCompareUpdate(true, false),
          rstl::optional_object< CVertexMorphEffect >(), nullptr);
    }
    CPatterned::Render(mgr);
    if (xray) {
      mgr.SetupFogForArea(GetCurrentAreaId());
    }
  }
}

CVector3f COmegaPirate::FindGround(const CVector3f& pos, CStateManager& mgr) {
  const CRayCastResult& result = CGameCollision::RayStaticIntersection(
      mgr, pos, CVector3f::Down(), 30.f, CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid)));
  const CVector3f point = result.GetPoint();
  if (!result.IsValid()) {
    return pos;
  }
  return point;
}

void COmegaPirate::TeleportToFurthestPlatform(CStateManager& mgr) {
  const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
  uint waypointIdx = 0;
  float maxDist = 0.f;
  CVector3f pos = CVector3f::Zero();
  const uint count = mScriptWaypointPlatforms.size();
  for (uint i = 0; i < count; ++i) {
    if (const CScriptWaypoint* const waypoint = TCastToConstPtr< CScriptWaypoint >(
            mgr.GetObjectById(mScriptWaypointPlatforms[i].first))) {
      const CVector3f waypointPos = waypoint->GetTranslation();
      const CVector3f& delta = CVector3f(playerPos - waypointPos);
      const float dist = delta.Magnitude();
      if (dist > maxDist && waypoint->GetUniqueId() != mLastWaypointId) {
        waypointIdx = i;
        maxDist = dist;
        pos = waypointPos;
      }
    }
  }
  SetTranslation(FindGround(CVector3f(pos), mgr));
  mLastWaypointId = mScriptWaypointPlatforms[waypointIdx].first;
  if (CScriptWaypoint* waypoint = TCastToPtr< CScriptWaypoint >(
          mgr.ObjectById(mScriptWaypointPlatforms[waypointIdx].first))) {
    waypoint->SendScriptMsgs(kSS_Arrived, mgr, kSM_None);
  }
  const CVector3f selfPos = GetTranslation();
  const CVector2f dir = CVector2f(playerPos.ToVec2f() - selfPos.ToVec2f()).AsNormalized();
  const CVector2f forward = GetTransform().GetForward().ToVec2f().AsNormalized();
  const CQuaternion rotation =
      CQuaternion::ShortestRotationArc(CVector3f(forward, 0.f), CVector3f(dir, 0.f));
  const CMatrix3f matrix = GetTransform().BuildMatrix3f() * rotation.BuildTransform();
  SetTransform(CTransform4f(matrix, GetTranslation()));
}

bool COmegaPirate::CoverBlown(CStateManager&, float) {
  if (mLostAllHp) {
    mLostAllHp = false;
    mHpLost = 0.f;
    return true;
  }
  return false;
}

bool COmegaPirate::HearPlayer(CStateManager&, float) { return mHearPlayer; }

bool COmegaPirate::CodeTrigger(CStateManager&, float) { return mCodeTrigger; }

void COmegaPirate::Shuffle(CStateManager&, EStateMsg, float) {}

void COmegaPirate::Skid(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kState_Zero;
    break;
  case kStateMsg_Update:
    switch (mState) {
    case kState_Zero:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Step) {
        mState = kState_Two;
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCStepCmd(pas::kSD_Forward, pas::kStep_Normal));
      }
      break;
    case kState_One:
      break;
    case kState_Two:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Step) {
        mState = kState_Over;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}
void COmegaPirate::Suck(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kState_Zero;
    mXrayAlphaState = kXFS_FadeOut;
    mXrayFadeInTrigger = true;
    break;
  case kStateMsg_Update:
    switch (mState) {
    case kState_Zero:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Step) {
        mState = kState_Two;
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCStepCmd(pas::kSD_Backward, pas::kStep_Normal));
        mBodyController->SetLocomotionType(pas::kLT_Relaxed);
      }
      break;
    case kState_One:
      break;
    case kState_Two:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Step) {
        mState = kState_Over;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate: {
    const uint count = mScriptPlatforms.size();
    for (uint i = 0; i < count; ++i) {
      if (CScriptPlatform* platform =
              static_cast< CScriptPlatform* >(mgr.ObjectById(mScriptPlatforms[i].first))) {
        platform->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerability());
        platform->RemoveMaterial(kMT_Orbit, mgr);
        platform->RemoveMaterial(kMT_Target, mgr);
        platform->SetDisableXRayAlpha(true);
        platform->SetModelFlags(
            CModelFlags::AlphaBlended(CColor(1.f, 1.f, 1.f, 0.f)).DepthCompareUpdate(true, true));
        platform->SetXRayFog(false);
      }
    }
    mArmorPieceHealTime = 0.f;
    mHealTime = 2.5f;
    mArmorPiecesHealed = 0;
    break;
  }
  }
}

void COmegaPirate::Explode(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kState_Zero;
    mScaleUpTrigger = false;
    break;
  case kStateMsg_Update:
    switch (mState) {
    case kState_Zero:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Step) {
        mState = kState_Two;
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCStepCmd(pas::kSD_Forward, pas::kStep_Dodge));
      }
      break;
    case kState_One:
      break;
    case kState_Two:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Step) {
        mState = kState_Over;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mBodyController->SetLocomotionType(mLocomotionType);
    break;
  }
}

void COmegaPirate::Growth(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mScaleState = kSS_ScaleDownY;
    mScaleUpTrigger = false;
    RemoveMaterial(kMT_RadarObject, mgr);
    RemoveMaterial(kMT_Scannable, mgr);
    mExit1Sent = false;
    mExit2Sent = false;
    ProcessSoundEvent(0xb27, 1.f, 0, 0.1f, 1000.f, 20, 127, CVector3f::Zero(), GetTranslation(),
                      mgr.GetNextAreaId().Value(), mgr, false);
    break;
  case kStateMsg_Update:
    if (xb68_ == 0) {
      if (GetStateMachineTime() > 0.3f * mStateTime && !mExit1Sent) {
        SendScriptMsgs(kSS_Exited, mgr, kSM_None);
        mExit1Sent = true;
      }
      if (GetStateMachineTime() > 0.6f * mStateTime && !mExit2Sent) {
        SendScriptMsgs(kSS_Exited, mgr, kSM_None);
        mExit2Sent = true;
      }
    } else if (GetStateMachineTime() > 0.5f * mStateTime && !mExit1Sent) {
      SendScriptMsgs(kSS_Exited, mgr, kSM_None);
      mExit1Sent = true;
    }
    break;
  case kStateMsg_Deactivate:
    TeleportToFurthestPlatform(mgr);
    mScaleUpTrigger = true;
    AddMaterial(kMT_RadarObject, mgr);
    ProcessSoundEvent(0xb28, 1.f, 0, 0.1f, 1000.f, 20, 127, CVector3f::Zero(), GetTranslation(),
                      mgr.GetNextAreaId().Value(), mgr, false);
    break;
  }
}

void COmegaPirate::Dizzy(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mTargetable = true;
    break;
  case kStateMsg_Update:
    mBodyController->CommandMgr().DeliverCmd(CBCLoopReactionCmd(pas::kRT_Two));
    break;
  case kStateMsg_Deactivate:
    mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    break;
  }
}

void COmegaPirate::Faint(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    mBodyController->CommandMgr().DeliverCmd(CBCLoopReactionCmd(pas::kRT_Zero));
    mTargetable = true;
    mHeartVisible = true;
    if (mXrayAlphaState == kXFS_WaitForTrigger) {
      mXrayFadeOutTime = 0.333f;
    }
    const uint count = mScriptPlatforms.size();
    for (uint i = 0; i < count; ++i) {
      if (CEntity* entity = mgr.ObjectById(mScriptPlatforms[i].first)) {
        entity->SetActive(true);
      }
    }
    break;
  }
  case kStateMsg_Update:
    if (mArmorPiecesHealed < 4 && mScaleState == kSS_None && mHealTime >= 2.5f) {
      const float duration = 1.f;
      const float alpha = rstl::min_val(mArmorPieceHealTime, duration);
      const float invAlpha = 1.f - alpha;
      const uint count = mScriptPlatforms.size();
      for (uint i = 0; i < count; ++i) {
        if (CScriptPlatform* platform =
                static_cast< CScriptPlatform* >(mgr.ObjectById(mScriptPlatforms[i].first))) {
          if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay) {
            if (i < mArmorPiecesHealed) {
              platform->SetModelFlags(CModelFlags::AlphaBlended(CColor(0.f, 0.f, 0.f, 1.f))
                                          .DepthCompareUpdate(true, true));
            } else if (mArmorPiecesHealed == i) {
              if (!mArmorPieceActivated) {
                SendScriptMsgs(kSS_Entered, mgr, kSM_None);
                mArmorPieceActivated = true;
              }
              platform->SetModelFlags(
                  CModelFlags::AlphaBlended(CColor(invAlpha, invAlpha, invAlpha, alpha))
                      .DepthCompareUpdate(true, true));
            }
          } else {
            platform->SetModelFlags(CModelFlags::AlphaBlended(CColor(1.f, 1.f, 1.f, 0.f))
                                        .DepthCompareUpdate(true, true));
          }
        }
      }
      if (mArmorPieceHealTime > 1.f) {
        ++mArmorPiecesHealed;
        mArmorPieceHealTime = 0.f;
        mHealTime = 0.f;
        mArmorPieceActivated = false;
      }
      mArmorPieceHealTime += dt;
    }
    mHealTime += dt;
    mBodyController->CommandMgr().DeliverCmd(CBCLoopReactionCmd(pas::kRT_Zero));
    break;
  case kStateMsg_Deactivate:
    mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    if (mHealTime >= 2.5f) {
      ++mArmorPiecesHealed;
    }
    break;
  }
}

void COmegaPirate::DoubleSnap(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    SendScriptMsgs(kSS_MaxReached, mgr, kSM_None);
    SetShotAt(false, mgr);
    mState = kState_Zero;
    mTargetable = false;
    mHeartVisible = false;
    mXrayFadeInTrigger = false;
    mXrayFadeOutTime = 3.f;
    const uint count = mScriptPlatforms.size();
    for (uint i = 0; i < count; ++i) {
      if (CScriptPlatform* platform =
              static_cast< CScriptPlatform* >(mgr.ObjectById(mScriptPlatforms[i].first))) {
        platform->SetActive(true);
        platform->SetDamageVulnerability(mPlatformVuln);
        platform->AddMaterial(kMT_Orbit, mgr);
        platform->AddMaterial(kMT_Target, mgr);
        platform->SetDisableXRayAlpha(false);
        platform->SetXRayFog(true);
      }
    }
    mStateTime = 17.f;
    AddMaterial(kMT_Scannable, mgr);
    break;
  }
  case kStateMsg_Update:
    switch (mState) {
    case kState_Zero:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Step) {
        mState = kState_Two;
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCStepCmd(pas::kSD_Backward, pas::kStep_BreakDodge));
      }
      break;
    case kState_One:
      break;
    case kState_Two:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Step) {
        mState = kState_Over;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    if (CGrenadeLauncher* launcher =
            static_cast< CGrenadeLauncher* >(mgr.ObjectById(GetLauncherId()))) {
      launcher->SetFollowPlayer(true);
    }
    if (CGrenadeLauncher* launcher =
            static_cast< CGrenadeLauncher* >(mgr.ObjectById(mLauncherId2))) {
      launcher->SetFollowPlayer(true);
    }
    break;
  }
}

void COmegaPirate::Retreat(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    SetShotAt(false, mgr);
    mState = kState_Zero;
    SendScriptMsgs(kSS_Inside, mgr, kSM_None);
    mScaleUpTrigger = false;
    mTargetable = false;
    mHeartVisible = false;
    mHpLost = 0.f;
    mHpLostInPhase = 0.f;
    mStateTime = 5.f;
    ++xb68_;
    break;
  case kStateMsg_Update:
    switch (mState) {
    case kState_Zero:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Step) {
        mState = kState_Two;
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCStepCmd(pas::kSD_Forward, pas::kStep_BreakDodge));
      }
      break;
    case kState_One:
      break;
    case kState_Two:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Step) {
        mState = kState_Over;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void COmegaPirate::JumpBack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    SetShotAt(false, mgr);
    mState = kState_Two;
    mArmorPiecesDestroyed = 0;
    mLauncher1FollowPlayer = false;
    mLauncher2FollowPlayer = false;
    xb68_ = 0;
    mLocomotionType = mBodyController->GetLocomotionType();
    mBodyController->SetLocomotionType(pas::kLT_Internal5);
    mBodyController->CommandMgr().DeliverCmd(
        CBCKnockBackCmd(GetTransform().GetForward(), pas::kS_Five));
    const uint count = mScriptPlatforms.size();
    for (uint i = 0; i < count; ++i) {
      if (CEntity* entity = mgr.ObjectById(mScriptPlatforms[i].first)) {
        entity->SetActive(false);
      }
    }
    break;
  }
  case kStateMsg_Update:
    switch (mState) {
    case kState_Zero:
      break;
    case kState_Two:
      if (mBodyController->GetCurrentStateId() != pas::kAS_KnockBack) {
        mState = kState_Over;
      }
      break;
    case kState_One:
      return;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void COmegaPirate::WallDetach(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kState_Zero;
    break;
  case kStateMsg_Update:
    switch (mState) {
    case kState_Zero:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Step) {
        mState = kState_Two;
        mBodyController->SetLocomotionType(pas::kLT_Relaxed);
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Left, pas::kStep_Dodge));
      }
      break;
    case kState_One:
      break;
    case kState_Two:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Step) {
        mState = kState_Over;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mgr.SetBossParams(GetUniqueId(), mMaxEnergy, 89);
    mBossPhaseActive = true;
    break;
  }
}

void COmegaPirate::WallHang(CStateManager&, EStateMsg, float) {}

void COmegaPirate::UpdateTeleportEffect(CStateManager& mgr, float dt) {
  CVector3f scale = GetModelData()->GetScale();
  switch (mScaleState) {
  case kSS_ScaleDownX: {
    const float duration = 0.25f;
    scale.SetX(
        mInitialScale.GetX() *
        rstl::min_val(1.f, 0.005f + (1.f - rstl::min_val(mScaleTime, duration) / duration)));
    if (mScaleTime > duration) {
      mScaleState = kSS_ScaleDownZ;
      mScaleTime = 0.f;
    }
    mScaleTime += dt;
    break;
  }
  case kSS_ScaleDownY: {
    const float duration = 0.25f;
    scale.SetY(
        mInitialScale.GetY() *
        rstl::min_val(1.f, 0.005f + (1.f - rstl::min_val(mScaleTime, duration) / duration)));
    if (mScaleTime > duration) {
      mScaleState = kSS_ScaleDownX;
      mScaleTime = 0.f;
    }
    mScaleTime += dt;
    break;
  }
  case kSS_ScaleDownZ: {
    const float duration = 0.25f;
    scale.SetZ(
        mInitialScale.GetZ() *
        rstl::min_val(1.f, 0.005f + (1.f - rstl::min_val(mScaleTime, duration) / duration)));
    if (mScaleTime > duration) {
      mScaleState = kSS_WaitForTrigger;
      mScaleTime = 0.f;
    }
    mScaleTime += dt;
    break;
  }
  case kSS_WaitForTrigger:
    if (mScaleTime > 0.1f && mScaleUpTrigger) {
      mScaleState = kSS_ScaleUpZ;
      mScaleTime = 0.f;
    }
    mScaleTime += dt;
    break;
  case kSS_ScaleUpX: {
    const float duration = 0.25f;
    scale.SetX(mInitialScale.GetX() *
               rstl::min_val(1.f, 0.005f + rstl::min_val(mScaleTime, duration) / duration));
    if (mScaleTime > duration) {
      mScaleState = kSS_ScaleUpY;
      mScaleTime = 0.f;
    }
    mScaleTime += dt;
    break;
  }
  case kSS_ScaleUpY: {
    const float duration = 0.25f;
    scale.SetY(mInitialScale.GetY() *
               rstl::min_val(1.f, 0.005f + rstl::min_val(mScaleTime, duration) / duration));
    if (mScaleTime > duration) {
      mScaleState = kSS_None;
      mScaleTime = 0.f;
    }
    mScaleTime += dt;
    break;
  }
  case kSS_ScaleUpZ: {
    const float duration = 0.25f;
    scale.SetZ(mInitialScale.GetZ() *
               rstl::min_val(1.f, 0.005f + rstl::min_val(mScaleTime, duration) / duration));
    if (mScaleTime > duration) {
      mScaleState = kSS_ScaleUpX;
      mScaleTime = 0.f;
    }
    mScaleTime += dt;
    break;
  }
  case kSS_None:
  default:
    return;
  }
  CActor* launcher1 = static_cast< CActor* >(mgr.ObjectById(GetLauncherId()));
  CActor* launcher2 = static_cast< CActor* >(mgr.ObjectById(mLauncherId2));
  ModelData()->SetScale(scale);
  if (launcher1 != nullptr) {
    launcher1->ModelData()->SetScale(scale);
  }
  if (launcher2 != nullptr) {
    launcher2->ModelData()->SetScale(scale);
  }
  const uint numPlatforms = mScriptPlatforms.size();
  for (uint i = 0; i < numPlatforms; ++i) {
    if (CActor* platform = static_cast< CActor* >(mgr.ObjectById(mScriptPlatforms[i].first))) {
      platform->ModelData()->SetScale(scale);
    }
  }
}

void COmegaPirate::UpdateVeinsModel(CStateManager& mgr, float dt) {
  switch (mSkeletonFadeState) {
  case kSFS_FadeOut: {
    const float fadeOutTime = 1.f;
    mSkeletonAlpha = 1.f - rstl::min_val(mSkeletonStateTime, fadeOutTime);
    if (mSkeletonStateTime > fadeOutTime) {
      mSkeletonFadeState = kSFS_None;
      mSkeletonStateTime = 0.f;
    }
    mSkeletonStateTime += dt;
    break;
  }
  case kSFS_FadeIn: {
    const float fadeInTime = 1.f;
    mSkeletonAlpha = rstl::min_val(mSkeletonStateTime, fadeInTime);
    if (mSkeletonStateTime > fadeInTime) {
      mSkeletonFadeState = kSFS_Flash;
      mSkeletonStateTime = 0.f;
    }
    mSkeletonStateTime += dt;
    break;
  }
  case kSFS_Flash:
    mSkeletonAlpha = 1.f;
    if (mSkeletonStateTime > 1.f) {
      mSkeletonFadeState = kSFS_FadeOut;
      mSkeletonStateTime = 0.f;
      CreateFlash(mgr, 0.75f);
    }
    mSkeletonStateTime += dt;
    break;
  default:
    mSkeletonAlpha = 0.f;
    break;
  }
}

void COmegaPirate::InitializeOmegaPirateCollisionManagers(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > spheres;
  spheres.reserve(1);
  AddOmegaPirateSphereCollisionList(skSphereJointList, 1, spheres);
  mCollisionActorMgr1 =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), spheres, true);
  SetOmegaPirateCrystalCollisionMaterialProperties(mCollisionActorMgr1, mgr);
  xa46_ = mCollisionActorMgr1->GetCollisionDescFromIndex(0).GetCollisionActorId();
  if (CActor* actor = static_cast< CActor* >(mgr.ObjectById(xa46_))) {
    *actor->HealthInfo(mgr) = *HealthInfo(mgr);
  }
  rstl::vector< CJointCollisionDescription > boxes;
  boxes.reserve(11);
  AddOBBCollisionList(skOBBJointList, 11, boxes);
  mCollisionActorMgr2 =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), boxes, true);
  SetOmegaPirateOBBCollisionMaterialProperties(mCollisionActorMgr2, mgr);
  xa48_ = mCollisionActorMgr2->GetCollisionDescFromIndex(0).GetCollisionActorId();
}

void COmegaPirate::SetOmegaPirateCrystalCollisionMaterialProperties(
    rstl::single_ptr< CCollisionActorManager >& actors, CStateManager& mgr) const {
  for (uint i = 0; i < actors->GetNumCollisionActors(); ++i) {
    const TUniqueId uid = actors->GetCollisionDescFromIndex(i).GetCollisionActorId();
    CCollisionActor* actor = static_cast< CCollisionActor* >(mgr.ObjectById(uid));
    if (actor != nullptr) {
      actor->AddMaterial(kMT_ScanPassthrough, mgr);
      actor->AddMaterial(kMT_CameraPassthrough, mgr);
      actor->AddMaterial(kMT_AIJoint, mgr);
      actor->AddMaterial(kMT_Immovable, mgr);
      const CMaterialList include = GetMaterialFilter().GetIncludeList();
      CMaterialList exclude = GetMaterialFilter().GetExcludeList();
      CMaterialList actorInclude = actor->GetMaterialFilter().GetIncludeList();
      const CMaterialList actorExclude = actor->GetMaterialFilter().GetExcludeList();
      exclude.Add(kMT_Platform);
      actor->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include.Union(actorInclude),
                                                                   exclude.Union(actorExclude)));
      actor->RemoveMaterial(kMT_ProjectilePassthrough, mgr);
    }
  }
}

void COmegaPirate::SetOmegaPirateOBBCollisionMaterialProperties(
    rstl::single_ptr< CCollisionActorManager >& actors, CStateManager& mgr) const {
  for (uint i = 0; i < actors->GetNumCollisionActors(); ++i) {
    const TUniqueId uid = actors->GetCollisionDescFromIndex(i).GetCollisionActorId();
    CCollisionActor* actor = static_cast< CCollisionActor* >(mgr.ObjectById(uid));
    if (actor != nullptr) {
      actor->AddMaterial(kMT_ScanPassthrough, mgr);
      actor->AddMaterial(kMT_CameraPassthrough, mgr);
      actor->AddMaterial(kMT_AIJoint, mgr);
      actor->AddMaterial(kMT_Immovable, mgr);
      const CMaterialList include = GetMaterialFilter().GetIncludeList();
      CMaterialList exclude = GetMaterialFilter().GetExcludeList();
      CMaterialList actorInclude = actor->GetMaterialFilter().GetIncludeList();
      const CMaterialList actorExclude = actor->GetMaterialFilter().GetExcludeList();
      actorInclude.Add(kMT_Player);
      exclude.Remove(kMT_Player);
      exclude.Add(kMT_Platform);
      actor->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include.Union(actorInclude),
                                                                   exclude.Union(actorExclude)));
      actor->RemoveMaterial(kMT_ProjectilePassthrough, mgr);
      actor->SetDamageVulnerability(CDamageVulnerability::ReflectVulnerability());
    }
  }
}

void COmegaPirate::AddOmegaPirateSphereCollisionList(
    const ::SSphereJointInfo* joints, int count,
    rstl::vector< CJointCollisionDescription >& list) const {
  const CAnimData& animData = *GetAnimationData();
  for (int i = 0; i < count; ++i) {
    const CSegId seg = animData.GetLocatorSegId(rstl::string_l(joints[i].name));
    if (seg == CSegId::Invalid()) {
      continue;
    }
    const CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
        seg, joints[i].radius, rstl::string_l(joints[i].name), 0.001f);
    list.push_back(desc);
  }
}

void COmegaPirate::AddOBBCollisionList(const SOBBoxJointInfo* joints, int count,
                                       rstl::vector< CJointCollisionDescription >& list) const {
  const CAnimData& animData = *GetAnimationData();
  for (int i = 0; i < count; ++i) {
    const CSegId from = animData.GetLocatorSegId(rstl::string_l(joints[i].mFrom));
    const CSegId to = animData.GetLocatorSegId(rstl::string_l(joints[i].mTo));
    if (from == CSegId::Invalid() || to == CSegId::Invalid()) {
      continue;
    }
    const CJointCollisionDescription desc = CJointCollisionDescription::OBBAutoSizeCollision(
        from, to,
        CVector3f::ByElementMultiply(
            CVector3f(joints[i].mBoundsX, joints[i].mBoundsY, joints[i].mBoundsZ),
            CVector3f(1.f, 1.f, 1.f)),
        CJointCollisionDescription::kOT_One,
        rstl::string("Omega_Pirate_OBB_") + CStringExtras::CreateFromInteger(i), 0.001f);
    list.push_back(desc);
  }
}

void COmegaPirate::UpdateVeinsModelGlowEffect(CStateManager&, float dt) {
  switch (mXrayAlphaState) {
  case kXFS_FadeOut:
    mXrayAlpha =
        1.f - rstl::min_val(mXrayAlphaStateTime, mXrayFadeOutTime) / mXrayFadeOutTime;
    if (mXrayAlphaStateTime > mXrayFadeOutTime) {
      mXrayAlphaState = kXFS_WaitForTrigger;
      mXrayAlphaStateTime = 0.f;
    }
    mXrayAlphaStateTime += dt;
    break;
  case kXFS_FadeIn:
    mXrayAlpha =
        rstl::min_val(mXrayAlphaStateTime, mXrayFadeInTime) / mXrayFadeInTime;
    if (mXrayAlphaStateTime > mXrayFadeInTime) {
      mXrayAlphaState = kXFS_None;
      mXrayAlphaStateTime = 0.f;
    }
    mXrayAlphaStateTime += dt;
    break;
  case kXFS_WaitForTrigger:
    mXrayAlpha = 0.f;
    if (mXrayAlphaStateTime > mXrayFadeTriggerTime && !mXrayFadeInTrigger) {
      mXrayAlphaState = kXFS_FadeIn;
      mXrayAlphaStateTime = 0.f;
    }
    mXrayAlphaStateTime += dt;
    break;
  default:
    mXrayAlpha = 1.f;
    break;
  }
}

void COmegaPirate::QueueTrooperPiratesOfOneRandomColor(uint count, CStateManager& mgr) {
  const int type = mgr.Random()->Next() % 4;
  const uint available = 3 - (xab4_.size() + GetNumActiveTrooperPirates());
  const int numToQueue = rstl::min_val(count, available);
  if (GetNumTypesOfActiveAndQueuedTrooperPirates() < 2) {
    for (int i = 0; i < numToQueue; ++i) {
      xab4_.push_back(type);
    }
  } else {
    QueueTrooperPiratesOfActiveType(numToQueue, mgr);
  }
}

void COmegaPirate::QueueTrooperPiratesOfActiveType(uint count, CStateManager& mgr) {
  rstl::vector< uchar > activeTypes;
  activeTypes.reserve(4);
  uint numEntries = xab4_.size();
  rstl::vector< uchar > queued(4, static_cast< uchar >(0));
  for (uint i = 0; i < numEntries;) {
    ++queued[xab4_[i++]];
  }
  for (uint i = 0; i < 4; ++i) {
    if (xb7c_[i] != 0 || queued[i] != 0) {
      activeTypes.push_back(i);
    }
  }
  numEntries = activeTypes.size();
  if (static_cast< int >(numEntries) > 0) {
    const uint type = activeTypes[mgr.Random()->Next() % numEntries];
    const uint available = 3 - (xab4_.size() + GetNumActiveTrooperPirates());
    const int numToQueue = rstl::min_val(count, available);
    for (int i = 0; i < numToQueue; ++i) {
      xab4_.push_back(type);
    }
  } else {
    QueueTrooperPiratesOfOneRandomColor(count, mgr);
  }
}

void COmegaPirate::SpawnNextQueuedTrooperPirate(CStateManager& mgr, float dt) {
  if (xac8_ < static_cast< int >(xab4_.size())) {
    if (xab0_ <= 0.f) {
      switch (static_cast< int >(xab4_[xac8_++])) {
      case 1:
        SendScriptMsgs(kSS_Open, mgr, kSM_None);
        ++xb7c_[1];
        break;
      case 2:
        SendScriptMsgs(kSS_CloseIn, mgr, kSM_None);
        ++xb7c_[2];
        break;
      case 0:
        SendScriptMsgs(kSS_Closed, mgr, kSM_None);
        ++xb7c_[0];
        break;
      case 3:
        SendScriptMsgs(kSS_Modify, mgr, kSM_None);
        ++xb7c_[3];
        break;
      }
      xab0_ = 1.5f;
    }
    xab0_ -= dt;
  }
}

void COmegaPirate::Run(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mCachedSpeed = mSpeed;
    mSpeed = 1.4f * mCachedSpeed;
    break;
  case kStateMsg_Update:
    if (mBodyController->GetCurrentStateId() == pas::kAS_KnockBack) {
      mSpeed = mCachedSpeed;
    } else if (mCachedSpeed == mSpeed) {
      mSpeed = 1.4f * mCachedSpeed;
    }
    break;
  case kStateMsg_Deactivate:
    mSpeed = mCachedSpeed;
    break;
  }
  CElitePirate::Run(mgr, msg, dt);
}

void COmegaPirate::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mCachedSpeed = mSpeed;
    mSpeed = 1.4f * mCachedSpeed;
    break;
  case kStateMsg_Update:
    if (mBodyController->GetCurrentStateId() == pas::kAS_KnockBack) {
      mSpeed = mCachedSpeed;
    } else if (mCachedSpeed == mSpeed) {
      mSpeed = 1.4f * mCachedSpeed;
    }
    break;
  case kStateMsg_Deactivate:
    mSpeed = mCachedSpeed;
    break;
  }
  CElitePirate::PathFind(mgr, msg, dt);
}

void COmegaPirate::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mCachedSpeed = mSpeed;
    mSpeed = 1.4f * mCachedSpeed;
    break;
  case kStateMsg_Update:
    if (mBodyController->GetCurrentStateId() == pas::kAS_KnockBack) {
      mSpeed = mCachedSpeed;
    } else if (mCachedSpeed == mSpeed) {
      mSpeed = 1.4f * mCachedSpeed;
    }
    break;
  case kStateMsg_Deactivate:
    mSpeed = mCachedSpeed;
    break;
  }
  CElitePirate::TargetPatrol(mgr, msg, dt);
}

void COmegaPirate::Cover(CStateManager& mgr, EStateMsg msg, float dt) {
  CElitePirate::Cover(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Activate:
    mCachedSpeed = mSpeed;
    mCover = true;
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    mCover = false;
    break;
  }
}

void COmegaPirate::Enraged(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = kState_Zero;
    break;
  case kStateMsg_Update:
    switch (mState) {
    case kState_Zero:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Taunt) {
        mState = kState_Two;
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Zero));
      }
      break;
    case kState_One:
      break;
    case kState_Two:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Taunt) {
        mState = kState_Over;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mLauncher1FollowPlayer = true;
    mLauncher2FollowPlayer = true;
    break;
  }
}

CShockWaveInfo COmegaPirate::GetShockWaveInfo() const {
  return CShockWaveInfo(GetData().GetShockwaveParticleDescId(), GetData().GetShockwaveDamageInfo(),
                        24.78255f, GetData().GetShockwaveWeaponDescId(),
                        GetData().GetShockwaveElectrocuteSfxId());
}

void COmegaPirate::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mIsMakingBigStrike = true;
    mDamageDur = 1.f;
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    mIsMakingBigStrike = false;
    mDamageDur = 0.f;
    break;
  }
  CElitePirate::Attack(mgr, msg, dt);
}

bool COmegaPirate::ShouldMove(CStateManager&, float) {
  return GetStateMachineTime() > mStateTime;
}

bool COmegaPirate::Landed(CStateManager&, float) { return mArmorPiecesHealed >= 4; }

uint COmegaPirate::GetNumActiveTrooperPirates() const {
  uint count = 0;
  for (uint i = 0; i < 4; ++i) {
    count += xb7c_[i];
  }
  return count;
}

uint COmegaPirate::GetNumTypesOfActiveAndQueuedTrooperPirates() const {
  const uint numQueued = xab4_.size();
  rstl::vector< uchar > queued(4, static_cast< uchar >(0));
  for (uint i = 0; i < numQueued;) {
    ++queued[xab4_[i++]];
  }
  int count = 0;
  for (uint i = 0; i < 4; ++i) {
    if (xb7c_[i] != 0 || queued[i] != 0) {
      ++count;
    }
  }
  return count;
}

void COmegaPirate::CreateFlash(CStateManager& mgr, float delay) {
  mgr.AddObject(*rs_new CFlash(
      mgr.AllocateUniqueId(),
      CEntityInfo(GetCurrentAreaId(), rstl::vector< SConnection >(), kInvalidEditorId),
      GetRenderBoundsCached().GetCenterPoint(), mThermalSpot, delay));
}

COmegaPirate::CFlash::CFlash(TUniqueId uid, const CEntityInfo& info, const CVector3f& pos,
                             const TToken< CTexture >& thermalSpot, float delay)
: CActor(uid, true, rstl::string_l("Omega Pirate Flash"), info, CTransform4f::Translate(pos),
         CModelData::CModelDataNull(), CMaterialList(), CActorParameters::None(), kInvalidUniqueId)
, mThermalSpot(thermalSpot)
, mDelay(delay)
, mTime(0.f)
, mSize(0.f) {
  CHECK_SIZEOF(CFlash, (VERSION >= VERSION_GM8E_02 ? 0x110 : 0x100))
  mThermalSpot.Lock();
}

void COmegaPirate::CFlash::AddToRenderer(const CFrustumPlanes&, const CStateManager&) const {}

void COmegaPirate::CFlash::Render(const CStateManager& mgr) const {
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_Thermal) {
    if (mThermalSpot.IsLoaded() && mThermalSpot.GetObject() != nullptr) {
      mThermalSpot.GetObject()->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
      float multiplier = 35.f;
      if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay) {
        CGX::SetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
        multiplier = 60.f;
      } else {
        CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
      }
      const CVector3f pos = GetTranslation();
      const float size = multiplier * mSize;
      const CVector3f right = size * CGraphics::GetViewMatrix().GetRight();
      const CVector3f up = size * CGraphics::GetViewMatrix().GetUp();
      CGraphics::SetModelMatrix(CTransform4f::Identity());
      const CColor color = CColor::White();
      CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
      CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
      CGraphics::SetDepthWriteMode(false, kE_Always, false);
      const float alpha = rstl::min_val(1.f, size);
      const CColor flashColor = color.WithAlphaOf(alpha);
      CGraphics::StreamColor(flashColor);
      CGraphics::StreamBegin(kP_TriangleFan);
      CGraphics::StreamTexcoord(0.f, 0.f);
      CGraphics::StreamVertex((pos - right) + up);
      CGraphics::StreamTexcoord(1.f, 0.f);
      CGraphics::StreamVertex((pos - right) - up);
      CGraphics::StreamTexcoord(1.f, 1.f);
      CGraphics::StreamVertex((pos + right) - up);
      CGraphics::StreamTexcoord(0.f, 1.f);
      CGraphics::StreamVertex((pos + right) + up);
      CGraphics::StreamEnd();
    }
  }
}

void COmegaPirate::CFlash::PreRender(CStateManager& mgr, const CFrustumPlanes&) {
  mgr.RenderLast(GetUniqueId());
  mThermalSpot.TryCache();
}

void COmegaPirate::CFlash::Think(float dt, CStateManager& mgr) {
  CEntity::Think(dt, mgr);
  mDelay -= dt;
  if (mDelay > 0.f) {
    return;
  }
  mTime += dt;
  float intensity = mTime;
  if (intensity > 0.75f) {
    intensity = 1.f - (intensity - 0.75f) / 0.25f;
  } else {
    intensity /= 0.75f;
  }
  const CGameCamera& camera = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  const CVector3f delta = GetTranslation() - camera.GetTranslation();
  const CVector3f forward = camera.GetTransform().GetForward();
  const float dot = CVector3f::Dot(delta.AsNormalized(), forward);
  const float facing = dot < 0.f ? 0.f : dot * dot;
  mSize = facing * intensity;
  if (mTime > 1.f) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

ENTITY_ACCEPT_IMPL(COmegaPirate::CFlash)

void COmegaPirate::KillOmegaPirate(CStateManager& mgr) {
  RemoveEmitter();
  SetTransform(mInitialXf);
  mFadeIn = true;
  mHeartVisible = false;
  SendScriptMsgs(kSS_DeathRattle, mgr, kSM_None);
  SendScriptMsgs(kSS_Dead, mgr, kSM_None);
  SendScriptMsgs(kSS_Inside, mgr, kSM_None);
  const uint numPlatforms = mScriptPlatforms.size();
  for (uint i = 0; i < numPlatforms; ++i) {
    if (CActor* platform = static_cast< CActor* >(mgr.ObjectById(mScriptPlatforms[i].first))) {
      platform->SetActive(false);
      platform->RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
      platform->SetEnableRender(false);
      mgr.DeleteObjectRequest(mScriptPlatforms[i].first);
    }
  }
  mScriptPlatforms.clear();
  CEntity* launcher1 = mgr.ObjectById(GetLauncherId());
  CEntity* launcher2 = mgr.ObjectById(mLauncherId2);
  if (launcher1 != nullptr) {
    launcher1->SetActive(false);
  }
  if (launcher2 != nullptr) {
    launcher2->SetActive(false);
  }
  SetActive(false);
  mgr.SetBossParams(kInvalidUniqueId, 0.f, 89);
  mCollisionActorMgr1->SetActive(mgr, false);
  mCollisionActorMgr2->SetActive(mgr, false);
}

bool COmegaPirate::IsElitePirate() const { return false; }
