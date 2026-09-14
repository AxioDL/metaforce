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
, x990_launcherId2(kInvalidUniqueId)
, x994_normalFadeState(kNFS_Zero)
, x998_normalFadeTime(0.f)
, x99c_normalAlpha(1.f)
, x9a0_visible(true)
, x9a1_fadeIn(true)
, x9b4_lostAllHp(false)
, x9c8_scaleState(kSS_None)
, x9cc_scaleTime(0.f)
, x9d0_initialScale(mData.ScaleCopy())
, x9ec_decrement(false)
, x9f0_skeletonModel(gpSimplePool->GetObj(SObjectTag('CMDL', skeletonModelId)),
                     gpSimplePool->GetObj(SObjectTag('CSKR', skeletonSkinRulesId)),
                     gpSimplePool->GetObj(SObjectTag('CINF', skeletonLayoutInfoId)),
                     CSkinnedModel::kDO_Owned)
, xa2c_skeletonAlpha(0.f)
, xa30_skeletonFadeState(kSFS_None)
, xa34_skeletonStateTime(0.f)
, xa38_collisionActorMgr1(nullptr)
, xa3c_hearPlayer(false)
, xa40_locomotionType(pas::kLT_Relaxed)
, xa44_targetable(false)
, xa46_(kInvalidUniqueId)
, xa48_(kInvalidUniqueId)
, xa4a_heartVisible(false)
, xa4c_initialXf(CTransform4f::Identity())
, xa7c_xrayAlphaState(kXFS_None)
, xa80_xrayAlpha(1.f)
, xa84_xrayAlphaStateTime(0.f)
, xa88_xrayFadeInTrigger(false)
, xa8c_xrayFadeOutTime(3.f)
, xa90_xrayFadeInTime(1.f)
, xa94_xrayFadeTriggerTime(1.f)
, xa98_maxEnergy(0.f)
, xa9c_collisionActorMgr2(nullptr)
, xab0_(0.f)
, xac4_(0)
, xac8_(0)
, xacc_(0)
, xad0_scaleUpTrigger(false)
, xad4_cachedSpeed(1.f)
, xad8_cover(false)
, xada_lastWaypointId(kInvalidUniqueId)
, xadc_(0)
, xadd_(0)
, xade_armorPiecesDestroyed(0)
, xadf_launcher1FollowPlayer(true)
, xae0_launcher2FollowPlayer(true)
, xae4_platformVuln(CDamageVulnerability::NormalVulnerability())
, xb4c_armorPiecesHealed(0)
, xb50_armorPieceHealTime(0.f)
, xb54_platformColor(CColor::White())
, xb58_healTime(2.5f)
, xb5c_hpLost(0.f)
, xb60_hpLostInPhase(0.f)
, xb64_stateTime(17.f)
, xb68_(0)
, xb6c_exit1Sent(false)
, xb6d_exit2Sent(false)
, xb6e_armorPieceActivated(false)
, xb70_thermalSpot(gpSimplePool->GetObj("Thermal_Spot_2"))
, xb78_codeTrigger(false)
, xb79_bossPhaseActive(0) {
  x9a4_scriptWaypointPlatforms.reserve(3);
  x9b8_scriptEffects.reserve(24);
  x9dc_scriptPlatforms.reserve(4);
  xaa0_scriptSounds.reserve(4);
  xab4_.reserve(3);
  xb70_thermalSpot.Lock();
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
    x990_launcherId2 = mgr.AllocateUniqueId();
    CreateGrenadeLauncher(mgr, x990_launcherId2);
    InitializeOmegaPirateCollisionManagers(mgr);
    x450_bodyController->SetLocomotionType(pas::kLT_Internal8);
    x402_27_noXrayModel = false;
    xa4c_initialXf = GetTransform();
    xa98_maxEnergy = HealthInfo(mgr)->GetHP();
    CActor* launcher1 = static_cast< CActor* >(mgr.ObjectById(GetLauncherId()));
    CActor* launcher2 = static_cast< CActor* >(mgr.ObjectById(x990_launcherId2));
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
    xa38_collisionActorMgr1->SetActive(mgr, true);
    xa9c_collisionActorMgr2->SetActive(mgr, true);
    KnockBackCtrl().SetAutoResetImpulse(false);
    if (CEntity* entity = mgr.ObjectById(x990_launcherId2)) {
      entity->SetActive(true);
    }
    break;
  case kSM_Deactivate:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    xa38_collisionActorMgr1->SetActive(mgr, false);
    xa9c_collisionActorMgr2->SetActive(mgr, false);
    if (CEntity* entity = mgr.ObjectById(x990_launcherId2)) {
      entity->SetActive(false);
    }
    break;
  case kSM_Deleted:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    xa38_collisionActorMgr1->Destroy(mgr);
    xa9c_collisionActorMgr2->Destroy(mgr);
    mgr.DeleteObjectRequest(x990_launcherId2);
    break;
  case kSM_Alert:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    break;
  case kSM_InitializedInArea: {
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    const uint count = GetConnectionList().size();
    for (uint i = 0; i < count; ++i) {
      const SConnection& conn = GetConnectionList()[i];
      const TUniqueId connId = mgr.GetIdForScript(conn.x8_objId);
      if (conn.x0_state == kSS_Attack && connId != kInvalidUniqueId) {
        if (conn.x4_msg == kSM_Activate) {
          if (CScriptEffect* effect = TCastToPtr< CScriptEffect >(mgr.ObjectById(connId))) {
            x9b8_scriptEffects.push_back(
                rstl::pair< TUniqueId, rstl::string >(connId, effect->GetDebugName()));
          } else if (CScriptPlatform* platform =
                         TCastToPtr< CScriptPlatform >(mgr.ObjectById(connId))) {
            x9dc_scriptPlatforms.push_back(
                rstl::pair< TUniqueId, rstl::string >(connId, platform->GetDebugName()));
            platform->AddMaterial(kMT_Target, mgr);
            platform->AddMaterial(kMT_Orbit, mgr);
            platform->AddMaterial(kMT_Character, mgr);
            platform->RemoveMaterial(kMT_Scannable, mgr);
            CMaterialList excludes = platform->GetMaterialFilter().GetExcludeList();
            excludes.Add(CMaterialList(kMT_Player, kMT_Character, kMT_CollisionActor));
            const CMaterialList includes = GetMaterialFilter().GetIncludeList();
            platform->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(includes, excludes));
            xae4_platformVuln = *platform->GetDamageVulnerability();
            xb54_platformColor = platform->GetModelFlags().GetColor();
          } else if (CScriptSound* sound = TCastToPtr< CScriptSound >(mgr.ObjectById(connId))) {
            xaa0_scriptSounds.push_back(
                rstl::pair< TUniqueId, rstl::string >(connId, sound->GetDebugName()));
          }
        } else if (conn.x4_msg == kSM_Follow) {
          if (CScriptWaypoint* waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(connId))) {
            const uint waypointCount = waypoint->GetConnectionList().size();
            rstl::vector< TUniqueId > waypointPlatformIds;
            waypointPlatformIds.reserve(3);
            for (uint j = 0; j < waypointCount; ++j) {
              const TUniqueId waypointConnId =
                  mgr.GetIdForScript(waypoint->GetConnectionList()[j].x8_objId);
              if (CScriptPlatform* platform =
                      TCastToPtr< CScriptPlatform >(mgr.ObjectById(waypointConnId))) {
                platform->AddMaterial(kMT_Target, mgr);
                platform->AddMaterial(kMT_Orbit, mgr);
                waypointPlatformIds.push_back(waypointConnId);
              }
            }
            x9a4_scriptWaypointPlatforms.push_back(
                rstl::pair< TUniqueId, rstl::vector< TUniqueId > >(connId, waypointPlatformIds));
          }
        }
      }
    }
    break;
  }
  case kSM_Touched: {
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    if (uid == x990_launcherId2 && x990_launcherId2 != kInvalidUniqueId) {
      SetShotAt(true, mgr);
    }
    CEntity* touchedEntity = mgr.ObjectById(uid);
    if (const CCollisionActor* actor = TCastToConstPtr< CCollisionActor >(touchedEntity)) {
      const TUniqueId touchedId = actor->GetLastTouchedObject();
      CEntity* playerEntity = mgr.ObjectById(touchedId);
      if (const CPlayer* player = TCastToConstPtr< CPlayer >(playerEntity)) {
        if (x420_curDamageRemTime <= 0.f) {
          mgr.ApplyDamage(
              GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              CVector3f::Zero());
          x420_curDamageRemTime = x424_damageWaitTime;
        }
      }
    }
    break;
  }
  case kSM_Damage:
    if (uid == x990_launcherId2 && x990_launcherId2 != kInvalidUniqueId) {
      x450_bodyController->CommandMgr().DeliverCmd(
          CBCKnockBackCmd(GetTransform().GetForward(), pas::kS_Eight));
    }
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    if (uid == xa46_ && xa7c_xrayAlphaState == kXFS_WaitForTrigger) {
      xa7c_xrayAlphaState = kXFS_FadeOut;
      xa84_xrayAlphaStateTime = 0.f;
    }
    break;
  case kSM_InvulnDamage:
    if (const CGameProjectile* projectile =
            TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(uid))) {
      if (xa4a_heartVisible) {
        mgr.ApplyDamage(
            uid, xa46_, projectile->GetOwnerId(), projectile->GetCurrentDamageInfo(),
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
      }
    }
    SetShotAt(true, mgr);
    break;
  case kSM_Decrement:
    x9ec_decrement = true;
    break;
  case kSM_Increment:
    SetShotAt(true, mgr);
    break;
  case kSM_SetToMax:
    xa3c_hearPlayer = true;
    break;
  case kSM_Stop:
    KillOmegaPirate(mgr);
    break;
  case kSM_Start:
    x3b4_speed = 1.f;
    ++xade_armorPiecesDestroyed;
    if (xade_armorPiecesDestroyed < 4) {
      x450_bodyController->CommandMgr().DeliverCmd(CBCKnockBackCmd(CVector3f::Left(), pas::kS_One));
    }
    break;
  case kSM_Action:
    x3b4_speed = 1.f;
    ++xade_armorPiecesDestroyed;
    if (xade_armorPiecesDestroyed < 4) {
      x450_bodyController->CommandMgr().DeliverCmd(
          CBCKnockBackCmd(CVector3f::Right(), pas::kS_One));
    }
    break;
  case kSM_Reset:
    xb78_codeTrigger = true;
#if VERSION >= VERSION_GM8E_01
    x450_bodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
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

void COmegaPirate::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  x988_28_alert = true;
  CElitePirate::Think(dt, mgr);
  const float highHp = 0.7f * xa98_maxEnergy;
  const float maxHp = xa98_maxEnergy;
  if (HealthInfo(mgr)->GetHP() <= 0.2f * maxHp) {
    xac4_ = 3;
  } else if (HealthInfo(mgr)->GetHP() <= highHp) {
    xac4_ = 2;
  } else if (xacc_ > 4) {
    xac4_ = 1;
  }
  UpdateGrenadeLauncher(mgr, x990_launcherId2, rstl::string_l(skpGrenadeLauncher2LCTR));
  UpdateInvisibility(mgr, dt);
  UpdateVeinsModel(mgr, dt);
  UpdateVeinsModelGlowEffect(mgr, dt);
  if ((!x9a1_fadeIn || xa4a_heartVisible) &&
      mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay && xa44_targetable) {
    AddMaterial(kMT_Target, mgr);
    AddMaterial(kMT_Orbit, mgr);
    if (x9c8_scaleState == kSS_WaitForTrigger) {
      xa9c_collisionActorMgr2->SetActive(mgr, false);
      xa38_collisionActorMgr1->SetActive(mgr, false);
    } else {
      xa38_collisionActorMgr1->SetActive(mgr, true);
      xa9c_collisionActorMgr2->SetActive(mgr, true);
      if (CEntity* entity = mgr.ObjectById(xa48_)) {
        entity->SetActive(false);
      }
    }
  } else {
    RemoveMaterial(kMT_Target, mgr);
    RemoveMaterial(kMT_Orbit, mgr);
    xa38_collisionActorMgr1->SetActive(mgr, false);
    if (x9a1_fadeIn) {
      xa9c_collisionActorMgr2->SetActive(mgr, true);
      if (CEntity* entity = mgr.ObjectById(xa48_)) {
        entity->SetActive(true);
      }
    } else {
      xa9c_collisionActorMgr2->SetActive(mgr, false);
      if (CEntity* entity = mgr.ObjectById(xa48_)) {
        entity->SetActive(false);
      }
    }
  }
  UpdateTeleportEffect(mgr, dt);
  xa38_collisionActorMgr1->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  xa9c_collisionActorMgr2->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  if (CActor* actor = static_cast< CActor* >(mgr.ObjectById(xa46_))) {
    const float hp = HealthInfo(mgr)->GetHP();
    *HealthInfo(mgr) = *actor->HealthInfo(mgr);
    const float damage = hp - HealthInfo(mgr)->GetHP();
    xb5c_hpLost += damage;
    xb60_hpLostInPhase += damage;
  }
  if (HealthInfo(mgr)->GetHP() <= 0.f) {
    KillOmegaPirate(mgr);
  } else {
    if (xb5c_hpLost > 100.f) {
      x9b4_lostAllHp = true;
    } else if (xb60_hpLostInPhase > 20.f) {
      x450_bodyController->CommandMgr().DeliverCmd(
          CBCAdditiveReactionCmd(pas::kART_One, 1.f, false));
      xb60_hpLostInPhase = 0.f;
    }
  }
  SpawnNextQueuedTrooperPirate(mgr, dt);
  const uint numPlatforms = x9dc_scriptPlatforms.size();
  for (uint i = 0; i < numPlatforms; ++i) {
    CActor* platform = static_cast< CActor* >(mgr.ObjectById(x9dc_scriptPlatforms[i].first));
    if ((xb78_codeTrigger || xb79_bossPhaseActive) && !xa4a_heartVisible) {
      platform->AddMaterial(kMT_Target, mgr);
      platform->AddMaterial(kMT_Orbit, mgr);
    } else {
      platform->RemoveMaterial(kMT_Target, mgr);
      platform->RemoveMaterial(kMT_Orbit, mgr);
    }
  }
  const CPlayerState& playerState = *mgr.GetPlayerState();
  if (x5d4_collisionActorMgr->GetActive() && playerState.IsFiringComboBeam() &&
      playerState.GetCurrentBeam() == CPlayerState::kBI_Wave && xad8_cover) {
    AddMaterial(kMT_Target, mgr);
    mgr.Player()->SetAimTargetId(GetUniqueId());
    const uint count = x9dc_scriptPlatforms.size();
    for (uint i = 0; i < count; ++i) {
      if (CActor* platform =
              static_cast< CActor* >(mgr.ObjectById(x9dc_scriptPlatforms[i].first))) {
        platform->RemoveMaterial(kMT_Target, mgr);
      }
    }
    mgr.Player()->PlayerGun()->AuxWeapon().SetNewTarget(GetUniqueId(), mgr);
  } else if (!xa4a_heartVisible) {
    RemoveMaterial(kMT_Target, mgr);
    const uint count = x9dc_scriptPlatforms.size();
    for (uint i = 0; i < count; ++i) {
      if (CActor* platform =
              static_cast< CActor* >(mgr.ObjectById(x9dc_scriptPlatforms[i].first))) {
        if (xb78_codeTrigger || xb79_bossPhaseActive) {
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
    launcher->SetFollowPlayer(xadf_launcher1FollowPlayer);
  }
  if (CGrenadeLauncher* launcher =
          static_cast< CGrenadeLauncher* >(mgr.ObjectById(x990_launcherId2))) {
    launcher->SetFollowPlayer(xae0_launcher2FollowPlayer);
  }
  if (x9ec_decrement) {
    x9ec_decrement = false;
    x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("JumpBack"));
  }
  if (xb68_ >= 2) {
    DoUserAnimEvent(mgr,
                    CInt32POINode(rstl::string_l(""), kPT_EmptyInt32, CCharAnimTime(0.f), -1, false,
                                  1.f, -1, 0, 0, rstl::string_l("root")),
                    kUE_ObjectPickUp, dt);
    xb68_ = 0;
  }
  if (xb8c_avoidStaticCollisionTime > 0.f) {
    const CAABox bounds = GetBoundingBox();
    CGameCollision::AvoidStaticCollisionWithinRadius(
        mgr, *this, 8, dt, 1.f, 1.5f * (bounds.GetMaxPoint().GetX() - bounds.GetMinPoint().GetX()),
        10000.f, 0.25f);
    xb8c_avoidStaticCollisionTime = 0.f;
  }
  xb8c_avoidStaticCollisionTime += dt;
}

void COmegaPirate::UpdateInvisibility(CStateManager& mgr, float dt) {
  switch (x994_normalFadeState) {
  case kNFS_One: {
    const float duration = 1.25f;
    x99c_normalAlpha = 1.f - rstl::min_val(x998_normalFadeTime, duration) / duration;
    x42c_color.SetAlpha(x99c_normalAlpha);
    if (x998_normalFadeTime > duration) {
      x994_normalFadeState = kNFS_Two;
      x9a1_fadeIn = false;
      x998_normalFadeTime = 0.f;
    }
    x998_normalFadeTime += dt;
    x9a0_visible = true;
    break;
  }
  case kNFS_Three: {
    const float duration = 1.f;
    x99c_normalAlpha = rstl::min_val(x998_normalFadeTime, duration) / 1.25f;
    if (x998_normalFadeTime > duration) {
      x994_normalFadeState = kNFS_Zero;
      x998_normalFadeTime = 0.f;
    }
    x998_normalFadeTime += dt;
    x9a0_visible = true;
    break;
  }
  case kNFS_Two:
    x99c_normalAlpha = 0.f;
    if (x998_normalFadeTime > 1.5f && x9a1_fadeIn) {
      CreateFlash(mgr, 0.f);
      x994_normalFadeState = kNFS_Three;
      x998_normalFadeTime = 0.f;
    }
    x998_normalFadeTime += dt;
    x9a0_visible = false;
    break;
  default:
    x99c_normalAlpha = 1.f;
    x9a0_visible = true;
    break;
  }
  float alpha = x99c_normalAlpha;
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay) {
    alpha = 0.f;
    x99c_normalAlpha = 1.f;
    x9a0_visible = true;
  }
  x42c_color.SetAlpha(x99c_normalAlpha);
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
            static_cast< CGrenadeLauncher* >(mgr.ObjectById(x990_launcherId2))) {
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
            static_cast< CGrenadeLauncher* >(mgr.ObjectById(x990_launcherId2))) {
      launcher->SetAddColor(CColor(0.f, 0.f, 0.f, 1.f));
      launcher->SetVisible(true);
    }
  }
}

void COmegaPirate::ActivateGrenadeLauncher(CStateManager& mgr, bool val) {
  CElitePirate::ActivateGrenadeLauncher(mgr, val);
  ActivateGrenadeLauncherById(mgr, val, x990_launcherId2);
}

bool COmegaPirate::CoverBlown(CStateManager&, float) {
  if (x9b4_lostAllHp) {
    x9b4_lostAllHp = false;
    xb5c_hpLost = 0.f;
    return true;
  }
  return false;
}

bool COmegaPirate::HearPlayer(CStateManager&, float) { return xa3c_hearPlayer; }

bool COmegaPirate::CodeTrigger(CStateManager&, float) { return xb78_codeTrigger; }

void COmegaPirate::Shuffle(CStateManager&, EStateMsg, float) {}

void COmegaPirate::WallHang(CStateManager&, EStateMsg, float) {}

void COmegaPirate::SetupHealthInfo(CStateManager& mgr) {
  CElitePirate::SetupHealthInfo(mgr);
  SetupHealthInfoForLauncher(mgr, x990_launcherId2);
}

CShockWaveInfo COmegaPirate::GetShockWaveInfo() const {
  return CShockWaveInfo(GetData().GetShockwaveParticleDescId(), GetData().GetShockwaveDamageInfo(),
                        24.78255f, GetData().GetShockwaveWeaponDescId(),
                        GetData().GetShockwaveElectrocuteSfxId());
}

bool COmegaPirate::ShouldFire(CStateManager& mgr, float arg) {
  return CElitePirate::ShouldFire(mgr, arg) || ShouldFireLauncher(mgr, x990_launcherId2);
}

bool COmegaPirate::ShouldCallForBackup(CStateManager& mgr, float arg) {
  return CElitePirate::ShouldCallForBackup(mgr, arg) &&
         ShouldCallForBackupForLauncher(mgr, arg, x990_launcherId2);
}

bool COmegaPirate::ShotAt(CStateManager& mgr, float arg) { return CElitePirate::ShotAt(mgr, arg); }

bool COmegaPirate::AggressionCheck(CStateManager& mgr, float arg) {
  return x990_launcherId2 == kInvalidUniqueId && CElitePirate::AggressionCheck(mgr, arg);
}

CVector3f COmegaPirate::GetOrbitPosition(const CStateManager& mgr) const {
  if (x990_launcherId2 != kInvalidUniqueId &&
      mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Thermal) {
    if (const CActor* actor = static_cast< const CActor* >(mgr.GetObjectById(x990_launcherId2))) {
      return GetGrenadeLaunchPos(*actor);
    }
  }
  return CElitePirate::GetOrbitPosition(mgr);
}

void COmegaPirate::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  CElitePirate::PreRender(mgr, frustum);
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay) {
    SetModelFlags(
        CModelFlags::ColorModulate(CColor(xa80_xrayAlpha, xa80_xrayAlpha, xa80_xrayAlpha, 1.f))
            .DepthCompareUpdate(true, true));
  }
}

void COmegaPirate::Render(const CStateManager& mgr) const {
  const CTransform4f xf = GetTransform() * CTransform4f::Scale(GetModelData()->ScaleCopy());
  CGraphics::SetModelMatrix(xf);
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_XRay &&
      xa2c_skeletonAlpha > 0.f) {
    GetAnimationData()->Render(x9f0_skeletonModel,
                               CModelFlags::AlphaBlended(CColor(1.f, 1.f, 1.f, xa2c_skeletonAlpha))
                                   .DepthCompareUpdate(true, true),
                               rstl::optional_object< CVertexMorphEffect >(), nullptr);
  }
  if (x9a0_visible) {
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
  const uint count = x9a4_scriptWaypointPlatforms.size();
  for (uint i = 0; i < count; ++i) {
    if (const CScriptWaypoint* const waypoint = TCastToConstPtr< CScriptWaypoint >(
            mgr.GetObjectById(x9a4_scriptWaypointPlatforms[i].first))) {
      const CVector3f waypointPos = waypoint->GetTranslation();
      const CVector3f& delta = CVector3f(playerPos - waypointPos);
      const float dist = delta.Magnitude();
      if (dist > maxDist && waypoint->GetUniqueId() != xada_lastWaypointId) {
        waypointIdx = i;
        maxDist = dist;
        pos = waypointPos;
      }
    }
  }
  SetTranslation(FindGround(CVector3f(pos), mgr));
  xada_lastWaypointId = x9a4_scriptWaypointPlatforms[waypointIdx].first;
  if (CScriptWaypoint* waypoint = TCastToPtr< CScriptWaypoint >(
          mgr.ObjectById(x9a4_scriptWaypointPlatforms[waypointIdx].first))) {
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

void COmegaPirate::Suck(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = kState_Zero;
    xa7c_xrayAlphaState = kXFS_FadeOut;
    xa88_xrayFadeInTrigger = true;
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case kState_Zero:
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_Step) {
        x568_state = kState_Two;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCStepCmd(pas::kSD_Backward, pas::kStep_Normal));
        x450_bodyController->SetLocomotionType(pas::kLT_Relaxed);
      }
      break;
    case kState_One:
      break;
    case kState_Two:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_Step) {
        x568_state = kState_Over;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate: {
    const uint count = x9dc_scriptPlatforms.size();
    for (uint i = 0; i < count; ++i) {
      if (CScriptPlatform* platform =
              static_cast< CScriptPlatform* >(mgr.ObjectById(x9dc_scriptPlatforms[i].first))) {
        platform->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerability());
        platform->RemoveMaterial(kMT_Orbit, mgr);
        platform->RemoveMaterial(kMT_Target, mgr);
        platform->SetDisableXRayAlpha(true);
        platform->SetModelFlags(
            CModelFlags::AlphaBlended(CColor(1.f, 1.f, 1.f, 0.f)).DepthCompareUpdate(true, true));
        platform->SetXRayFog(false);
      }
    }
    xb50_armorPieceHealTime = 0.f;
    xb58_healTime = 2.5f;
    xb4c_armorPiecesHealed = 0;
    break;
  }
  }
}

void COmegaPirate::Explode(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = kState_Zero;
    xad0_scaleUpTrigger = false;
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case kState_Zero:
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_Step) {
        x568_state = kState_Two;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCStepCmd(pas::kSD_Forward, pas::kStep_Dodge));
      }
      break;
    case kState_One:
      break;
    case kState_Two:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_Step) {
        x568_state = kState_Over;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    x450_bodyController->SetLocomotionType(xa40_locomotionType);
    break;
  }
}

void COmegaPirate::Growth(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x9c8_scaleState = kSS_ScaleDownY;
    xad0_scaleUpTrigger = false;
    RemoveMaterial(kMT_RadarObject, mgr);
    RemoveMaterial(kMT_Scannable, mgr);
    xb6c_exit1Sent = false;
    xb6d_exit2Sent = false;
    ProcessSoundEvent(0xb27, 1.f, 0, 0.1f, 1000.f, 20, 127, CVector3f::Zero(), GetTranslation(),
                      mgr.GetNextAreaId().Value(), mgr, false);
    break;
  case kStateMsg_Update:
    if (xb68_ == 0) {
      if (GetStateMachineTime() > 0.3f * xb64_stateTime && !xb6c_exit1Sent) {
        SendScriptMsgs(kSS_Exited, mgr, kSM_None);
        xb6c_exit1Sent = true;
      }
      if (GetStateMachineTime() > 0.6f * xb64_stateTime && !xb6d_exit2Sent) {
        SendScriptMsgs(kSS_Exited, mgr, kSM_None);
        xb6d_exit2Sent = true;
      }
    } else if (GetStateMachineTime() > 0.5f * xb64_stateTime && !xb6c_exit1Sent) {
      SendScriptMsgs(kSS_Exited, mgr, kSM_None);
      xb6c_exit1Sent = true;
    }
    break;
  case kStateMsg_Deactivate:
    TeleportToFurthestPlatform(mgr);
    xad0_scaleUpTrigger = true;
    AddMaterial(kMT_RadarObject, mgr);
    ProcessSoundEvent(0xb28, 1.f, 0, 0.1f, 1000.f, 20, 127, CVector3f::Zero(), GetTranslation(),
                      mgr.GetNextAreaId().Value(), mgr, false);
    break;
  }
}

void COmegaPirate::Skid(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = kState_Zero;
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case kState_Zero:
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_Step) {
        x568_state = kState_Two;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCStepCmd(pas::kSD_Forward, pas::kStep_Normal));
      }
      break;
    case kState_One:
      break;
    case kState_Two:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_Step) {
        x568_state = kState_Over;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void COmegaPirate::Retreat(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    SetShotAt(false, mgr);
    x568_state = kState_Zero;
    SendScriptMsgs(kSS_Inside, mgr, kSM_None);
    xad0_scaleUpTrigger = false;
    xa44_targetable = false;
    xa4a_heartVisible = false;
    xb5c_hpLost = 0.f;
    xb60_hpLostInPhase = 0.f;
    xb64_stateTime = 5.f;
    ++xb68_;
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case kState_Zero:
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_Step) {
        x568_state = kState_Two;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCStepCmd(pas::kSD_Forward, pas::kStep_BreakDodge));
      }
      break;
    case kState_One:
      break;
    case kState_Two:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_Step) {
        x568_state = kState_Over;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void COmegaPirate::Faint(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    x450_bodyController->CommandMgr().DeliverCmd(CBCLoopReactionCmd(pas::kRT_Zero));
    xa44_targetable = true;
    xa4a_heartVisible = true;
    if (xa7c_xrayAlphaState == kXFS_WaitForTrigger) {
      xa8c_xrayFadeOutTime = 0.333f;
    }
    const uint count = x9dc_scriptPlatforms.size();
    for (uint i = 0; i < count; ++i) {
      if (CEntity* entity = mgr.ObjectById(x9dc_scriptPlatforms[i].first)) {
        entity->SetActive(true);
      }
    }
    break;
  }
  case kStateMsg_Update:
    if (xb4c_armorPiecesHealed < 4 && x9c8_scaleState == kSS_None && xb58_healTime >= 2.5f) {
      const float duration = 1.f;
      const float alpha = rstl::min_val(xb50_armorPieceHealTime, duration);
      const float invAlpha = 1.f - alpha;
      const uint count = x9dc_scriptPlatforms.size();
      for (uint i = 0; i < count; ++i) {
        if (CScriptPlatform* platform =
                static_cast< CScriptPlatform* >(mgr.ObjectById(x9dc_scriptPlatforms[i].first))) {
          if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay) {
            if (i < xb4c_armorPiecesHealed) {
              platform->SetModelFlags(CModelFlags::AlphaBlended(CColor(0.f, 0.f, 0.f, 1.f))
                                          .DepthCompareUpdate(true, true));
            } else if (xb4c_armorPiecesHealed == i) {
              if (!xb6e_armorPieceActivated) {
                SendScriptMsgs(kSS_Entered, mgr, kSM_None);
                xb6e_armorPieceActivated = true;
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
      if (xb50_armorPieceHealTime > 1.f) {
        ++xb4c_armorPiecesHealed;
        xb50_armorPieceHealTime = 0.f;
        xb58_healTime = 0.f;
        xb6e_armorPieceActivated = false;
      }
      xb50_armorPieceHealTime += dt;
    }
    xb58_healTime += dt;
    x450_bodyController->CommandMgr().DeliverCmd(CBCLoopReactionCmd(pas::kRT_Zero));
    break;
  case kStateMsg_Deactivate:
    x450_bodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    if (xb58_healTime >= 2.5f) {
      ++xb4c_armorPiecesHealed;
    }
    break;
  }
}

void COmegaPirate::DoubleSnap(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    SendScriptMsgs(kSS_MaxReached, mgr, kSM_None);
    SetShotAt(false, mgr);
    x568_state = kState_Zero;
    xa44_targetable = false;
    xa4a_heartVisible = false;
    xa88_xrayFadeInTrigger = false;
    xa8c_xrayFadeOutTime = 3.f;
    const uint count = x9dc_scriptPlatforms.size();
    for (uint i = 0; i < count; ++i) {
      if (CScriptPlatform* platform =
              static_cast< CScriptPlatform* >(mgr.ObjectById(x9dc_scriptPlatforms[i].first))) {
        platform->SetActive(true);
        platform->SetDamageVulnerability(xae4_platformVuln);
        platform->AddMaterial(kMT_Orbit, mgr);
        platform->AddMaterial(kMT_Target, mgr);
        platform->SetDisableXRayAlpha(false);
        platform->SetXRayFog(true);
      }
    }
    xb64_stateTime = 17.f;
    AddMaterial(kMT_Scannable, mgr);
    break;
  }
  case kStateMsg_Update:
    switch (x568_state) {
    case kState_Zero:
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_Step) {
        x568_state = kState_Two;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCStepCmd(pas::kSD_Backward, pas::kStep_BreakDodge));
      }
      break;
    case kState_One:
      break;
    case kState_Two:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_Step) {
        x568_state = kState_Over;
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
            static_cast< CGrenadeLauncher* >(mgr.ObjectById(x990_launcherId2))) {
      launcher->SetFollowPlayer(true);
    }
    break;
  }
}

void COmegaPirate::Dizzy(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    xa44_targetable = true;
    break;
  case kStateMsg_Update:
    x450_bodyController->CommandMgr().DeliverCmd(CBCLoopReactionCmd(pas::kRT_Two));
    break;
  case kStateMsg_Deactivate:
    x450_bodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
    break;
  }
}

void COmegaPirate::JumpBack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    SetShotAt(false, mgr);
    x568_state = kState_Two;
    xade_armorPiecesDestroyed = 0;
    xadf_launcher1FollowPlayer = false;
    xae0_launcher2FollowPlayer = false;
    xb68_ = 0;
    xa40_locomotionType = x450_bodyController->GetLocomotionType();
    x450_bodyController->SetLocomotionType(pas::kLT_Internal5);
    x450_bodyController->CommandMgr().DeliverCmd(
        CBCKnockBackCmd(GetTransform().GetForward(), pas::kS_Five));
    const uint count = x9dc_scriptPlatforms.size();
    for (uint i = 0; i < count; ++i) {
      if (CEntity* entity = mgr.ObjectById(x9dc_scriptPlatforms[i].first)) {
        entity->SetActive(false);
      }
    }
    break;
  }
  case kStateMsg_Update:
    switch (x568_state) {
    case kState_Zero:
      break;
    case kState_Two:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_KnockBack) {
        x568_state = kState_Over;
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
    x568_state = kState_Zero;
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case kState_Zero:
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_Step) {
        x568_state = kState_Two;
        x450_bodyController->SetLocomotionType(pas::kLT_Relaxed);
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Left, pas::kStep_Dodge));
      }
      break;
    case kState_One:
      break;
    case kState_Two:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_Step) {
        x568_state = kState_Over;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mgr.SetBossParams(GetUniqueId(), xa98_maxEnergy, 89);
    xb79_bossPhaseActive = true;
    break;
  }
}

void COmegaPirate::UpdateTeleportEffect(CStateManager& mgr, float dt) {
  CVector3f scale = GetModelData()->GetScale();
  switch (x9c8_scaleState) {
  case kSS_ScaleDownX: {
    const float duration = 0.25f;
    scale.SetX(
        x9d0_initialScale.GetX() *
        rstl::min_val(1.f, 0.005f + (1.f - rstl::min_val(x9cc_scaleTime, duration) / duration)));
    if (x9cc_scaleTime > duration) {
      x9c8_scaleState = kSS_ScaleDownZ;
      x9cc_scaleTime = 0.f;
    }
    x9cc_scaleTime += dt;
    break;
  }
  case kSS_ScaleDownY: {
    const float duration = 0.25f;
    scale.SetY(
        x9d0_initialScale.GetY() *
        rstl::min_val(1.f, 0.005f + (1.f - rstl::min_val(x9cc_scaleTime, duration) / duration)));
    if (x9cc_scaleTime > duration) {
      x9c8_scaleState = kSS_ScaleDownX;
      x9cc_scaleTime = 0.f;
    }
    x9cc_scaleTime += dt;
    break;
  }
  case kSS_ScaleDownZ: {
    const float duration = 0.25f;
    scale.SetZ(
        x9d0_initialScale.GetZ() *
        rstl::min_val(1.f, 0.005f + (1.f - rstl::min_val(x9cc_scaleTime, duration) / duration)));
    if (x9cc_scaleTime > duration) {
      x9c8_scaleState = kSS_WaitForTrigger;
      x9cc_scaleTime = 0.f;
    }
    x9cc_scaleTime += dt;
    break;
  }
  case kSS_WaitForTrigger:
    if (x9cc_scaleTime > 0.1f && xad0_scaleUpTrigger) {
      x9c8_scaleState = kSS_ScaleUpZ;
      x9cc_scaleTime = 0.f;
    }
    x9cc_scaleTime += dt;
    break;
  case kSS_ScaleUpX: {
    const float duration = 0.25f;
    scale.SetX(x9d0_initialScale.GetX() *
               rstl::min_val(1.f, 0.005f + rstl::min_val(x9cc_scaleTime, duration) / duration));
    if (x9cc_scaleTime > duration) {
      x9c8_scaleState = kSS_ScaleUpY;
      x9cc_scaleTime = 0.f;
    }
    x9cc_scaleTime += dt;
    break;
  }
  case kSS_ScaleUpY: {
    const float duration = 0.25f;
    scale.SetY(x9d0_initialScale.GetY() *
               rstl::min_val(1.f, 0.005f + rstl::min_val(x9cc_scaleTime, duration) / duration));
    if (x9cc_scaleTime > duration) {
      x9c8_scaleState = kSS_None;
      x9cc_scaleTime = 0.f;
    }
    x9cc_scaleTime += dt;
    break;
  }
  case kSS_ScaleUpZ: {
    const float duration = 0.25f;
    scale.SetZ(x9d0_initialScale.GetZ() *
               rstl::min_val(1.f, 0.005f + rstl::min_val(x9cc_scaleTime, duration) / duration));
    if (x9cc_scaleTime > duration) {
      x9c8_scaleState = kSS_ScaleUpX;
      x9cc_scaleTime = 0.f;
    }
    x9cc_scaleTime += dt;
    break;
  }
  case kSS_None:
  default:
    return;
  }
  CActor* launcher1 = static_cast< CActor* >(mgr.ObjectById(GetLauncherId()));
  CActor* launcher2 = static_cast< CActor* >(mgr.ObjectById(x990_launcherId2));
  ModelData()->SetScale(scale);
  if (launcher1 != nullptr) {
    launcher1->ModelData()->SetScale(scale);
  }
  if (launcher2 != nullptr) {
    launcher2->ModelData()->SetScale(scale);
  }
  const uint numPlatforms = x9dc_scriptPlatforms.size();
  for (uint i = 0; i < numPlatforms; ++i) {
    if (CActor* platform = static_cast< CActor* >(mgr.ObjectById(x9dc_scriptPlatforms[i].first))) {
      platform->ModelData()->SetScale(scale);
    }
  }
}

void COmegaPirate::UpdateVeinsModel(CStateManager& mgr, float dt) {
  switch (xa30_skeletonFadeState) {
  case kSFS_FadeOut: {
    const float fadeOutTime = 1.f;
    xa2c_skeletonAlpha = 1.f - rstl::min_val(xa34_skeletonStateTime, fadeOutTime);
    if (xa34_skeletonStateTime > fadeOutTime) {
      xa30_skeletonFadeState = kSFS_None;
      xa34_skeletonStateTime = 0.f;
    }
    xa34_skeletonStateTime += dt;
    break;
  }
  case kSFS_FadeIn: {
    const float fadeInTime = 1.f;
    xa2c_skeletonAlpha = rstl::min_val(xa34_skeletonStateTime, fadeInTime);
    if (xa34_skeletonStateTime > fadeInTime) {
      xa30_skeletonFadeState = kSFS_Flash;
      xa34_skeletonStateTime = 0.f;
    }
    xa34_skeletonStateTime += dt;
    break;
  }
  case kSFS_Flash:
    xa2c_skeletonAlpha = 1.f;
    if (xa34_skeletonStateTime > 1.f) {
      xa30_skeletonFadeState = kSFS_FadeOut;
      xa34_skeletonStateTime = 0.f;
      CreateFlash(mgr, 0.75f);
    }
    xa34_skeletonStateTime += dt;
    break;
  default:
    xa2c_skeletonAlpha = 0.f;
    break;
  }
}

void COmegaPirate::InitializeOmegaPirateCollisionManagers(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > spheres;
  spheres.reserve(1);
  AddOmegaPirateSphereCollisionList(skSphereJointList, 1, spheres);
  xa38_collisionActorMgr1 =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), spheres, true);
  SetOmegaPirateCrystalCollisionMaterialProperties(xa38_collisionActorMgr1, mgr);
  xa46_ = xa38_collisionActorMgr1->GetCollisionDescFromIndex(0).GetCollisionActorId();
  if (CActor* actor = static_cast< CActor* >(mgr.ObjectById(xa46_))) {
    *actor->HealthInfo(mgr) = *HealthInfo(mgr);
  }
  rstl::vector< CJointCollisionDescription > boxes;
  boxes.reserve(11);
  AddOBBCollisionList(skOBBJointList, 11, boxes);
  xa9c_collisionActorMgr2 =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), boxes, true);
  SetOmegaPirateOBBCollisionMaterialProperties(xa9c_collisionActorMgr2, mgr);
  xa48_ = xa9c_collisionActorMgr2->GetCollisionDescFromIndex(0).GetCollisionActorId();
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
    const CSegId from = animData.GetLocatorSegId(rstl::string_l(joints[i].x0_from));
    const CSegId to = animData.GetLocatorSegId(rstl::string_l(joints[i].x4_to));
    if (from == CSegId::Invalid() || to == CSegId::Invalid()) {
      continue;
    }
    const CJointCollisionDescription desc = CJointCollisionDescription::OBBAutoSizeCollision(
        from, to,
        CVector3f::ByElementMultiply(
            CVector3f(joints[i].x8_boundsX, joints[i].xc_boundsY, joints[i].x10_boundsZ),
            CVector3f(1.f, 1.f, 1.f)),
        CJointCollisionDescription::kOT_One,
        rstl::string("Omega_Pirate_OBB_") + CStringExtras::CreateFromInteger(i), 0.001f);
    list.push_back(desc);
  }
}

void COmegaPirate::UpdateVeinsModelGlowEffect(CStateManager&, float dt) {
  switch (xa7c_xrayAlphaState) {
  case kXFS_FadeOut:
    xa80_xrayAlpha =
        1.f - rstl::min_val(xa84_xrayAlphaStateTime, xa8c_xrayFadeOutTime) / xa8c_xrayFadeOutTime;
    if (xa84_xrayAlphaStateTime > xa8c_xrayFadeOutTime) {
      xa7c_xrayAlphaState = kXFS_WaitForTrigger;
      xa84_xrayAlphaStateTime = 0.f;
    }
    xa84_xrayAlphaStateTime += dt;
    break;
  case kXFS_FadeIn:
    xa80_xrayAlpha =
        rstl::min_val(xa84_xrayAlphaStateTime, xa90_xrayFadeInTime) / xa90_xrayFadeInTime;
    if (xa84_xrayAlphaStateTime > xa90_xrayFadeInTime) {
      xa7c_xrayAlphaState = kXFS_None;
      xa84_xrayAlphaStateTime = 0.f;
    }
    xa84_xrayAlphaStateTime += dt;
    break;
  case kXFS_WaitForTrigger:
    xa80_xrayAlpha = 0.f;
    if (xa84_xrayAlphaStateTime > xa94_xrayFadeTriggerTime && !xa88_xrayFadeInTrigger) {
      xa7c_xrayAlphaState = kXFS_FadeIn;
      xa84_xrayAlphaStateTime = 0.f;
    }
    xa84_xrayAlphaStateTime += dt;
    break;
  default:
    xa80_xrayAlpha = 1.f;
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

void COmegaPirate::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    xad4_cachedSpeed = x3b4_speed;
    x3b4_speed = 1.4f * xad4_cachedSpeed;
    break;
  case kStateMsg_Update:
    if (x450_bodyController->GetCurrentStateId() == pas::kAS_KnockBack) {
      x3b4_speed = xad4_cachedSpeed;
    } else if (xad4_cachedSpeed == x3b4_speed) {
      x3b4_speed = 1.4f * xad4_cachedSpeed;
    }
    break;
  case kStateMsg_Deactivate:
    x3b4_speed = xad4_cachedSpeed;
    break;
  }
  CElitePirate::TargetPatrol(mgr, msg, dt);
}

void COmegaPirate::Cover(CStateManager& mgr, EStateMsg msg, float dt) {
  CElitePirate::Cover(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Activate:
    xad4_cachedSpeed = x3b4_speed;
    xad8_cover = true;
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    xad8_cover = false;
    break;
  }
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
    if (x990_launcherId2 != kInvalidUniqueId) {
      if (CEntity* entity = mgr.ObjectById(x990_launcherId2)) {
        mgr.DeliverScriptMsg(entity, GetUniqueId(), kSM_Action);
      }
    }
    break;
  case kUE_FadeOut:
    if (x994_normalFadeState != kNFS_Two && x9a1_fadeIn) {
      x994_normalFadeState = kNFS_One;
      xa30_skeletonFadeState = kSFS_FadeIn;
    }
    break;
  case kUE_FadeIn:
    x9a1_fadeIn = true;
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

void COmegaPirate::Enraged(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_state = kState_Zero;
    break;
  case kStateMsg_Update:
    switch (x568_state) {
    case kState_Zero:
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_Taunt) {
        x568_state = kState_Two;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Zero));
      }
      break;
    case kState_One:
      break;
    case kState_Two:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_Taunt) {
        x568_state = kState_Over;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    xadf_launcher1FollowPlayer = true;
    xae0_launcher2FollowPlayer = true;
    break;
  }
}

void COmegaPirate::Run(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    xad4_cachedSpeed = x3b4_speed;
    x3b4_speed = 1.4f * xad4_cachedSpeed;
    break;
  case kStateMsg_Update:
    if (x450_bodyController->GetCurrentStateId() == pas::kAS_KnockBack) {
      x3b4_speed = xad4_cachedSpeed;
    } else if (xad4_cachedSpeed == x3b4_speed) {
      x3b4_speed = 1.4f * xad4_cachedSpeed;
    }
    break;
  case kStateMsg_Deactivate:
    x3b4_speed = xad4_cachedSpeed;
    break;
  }
  CElitePirate::Run(mgr, msg, dt);
}

void COmegaPirate::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    xad4_cachedSpeed = x3b4_speed;
    x3b4_speed = 1.4f * xad4_cachedSpeed;
    break;
  case kStateMsg_Update:
    if (x450_bodyController->GetCurrentStateId() == pas::kAS_KnockBack) {
      x3b4_speed = xad4_cachedSpeed;
    } else if (xad4_cachedSpeed == x3b4_speed) {
      x3b4_speed = 1.4f * xad4_cachedSpeed;
    }
    break;
  case kStateMsg_Deactivate:
    x3b4_speed = xad4_cachedSpeed;
    break;
  }
  CElitePirate::PathFind(mgr, msg, dt);
}

void COmegaPirate::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x402_28_isMakingBigStrike = true;
    x504_damageDur = 1.f;
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    x402_28_isMakingBigStrike = false;
    x504_damageDur = 0.f;
    break;
  }
  CElitePirate::Attack(mgr, msg, dt);
}

bool COmegaPirate::ShouldMove(CStateManager&, float) {
  return GetStateMachineTime() > xb64_stateTime;
}

bool COmegaPirate::Landed(CStateManager&, float) { return xb4c_armorPiecesHealed >= 4; }

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
      GetRenderBoundsCached().GetCenterPoint(), xb70_thermalSpot, delay));
}

COmegaPirate::CFlash::CFlash(TUniqueId uid, const CEntityInfo& info, const CVector3f& pos,
                             const TToken< CTexture >& thermalSpot, float delay)
: CActor(uid, true, rstl::string_l("Omega Pirate Flash"), info, CTransform4f::Translate(pos),
         CModelData::CModelDataNull(), CMaterialList(), CActorParameters::None(), kInvalidUniqueId)
, xe8_thermalSpot(thermalSpot)
, xf4_delay(delay)
, xf8_time(0.f)
, xfc_size(0.f) {
  CHECK_SIZEOF(CFlash, 0x100)
  xe8_thermalSpot.Lock();
}

void COmegaPirate::CFlash::AddToRenderer(const CFrustumPlanes&, const CStateManager&) const {}

void COmegaPirate::CFlash::Render(const CStateManager& mgr) const {
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_Thermal) {
    if (xe8_thermalSpot.IsLoaded() && xe8_thermalSpot.GetObject() != nullptr) {
      xe8_thermalSpot.GetObject()->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
      float multiplier = 35.f;
      if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay) {
        CGX::SetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
        multiplier = 60.f;
      } else {
        CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
      }
      const CVector3f pos = GetTranslation();
      const float size = multiplier * xfc_size;
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
  xe8_thermalSpot.TryCache();
}

void COmegaPirate::CFlash::Think(float dt, CStateManager& mgr) {
  CEntity::Think(dt, mgr);
  xf4_delay -= dt;
  if (xf4_delay > 0.f) {
    return;
  }
  xf8_time += dt;
  float intensity = xf8_time;
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
  xfc_size = facing * intensity;
  if (xf8_time > 1.f) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

ENTITY_ACCEPT_IMPL(COmegaPirate::CFlash)

void COmegaPirate::KillOmegaPirate(CStateManager& mgr) {
  RemoveEmitter();
  SetTransform(xa4c_initialXf);
  x9a1_fadeIn = true;
  xa4a_heartVisible = false;
  SendScriptMsgs(kSS_DeathRattle, mgr, kSM_None);
  SendScriptMsgs(kSS_Dead, mgr, kSM_None);
  SendScriptMsgs(kSS_Inside, mgr, kSM_None);
  const uint numPlatforms = x9dc_scriptPlatforms.size();
  for (uint i = 0; i < numPlatforms; ++i) {
    if (CActor* platform = static_cast< CActor* >(mgr.ObjectById(x9dc_scriptPlatforms[i].first))) {
      platform->SetActive(false);
      platform->RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
      platform->SetEnableRender(false);
      mgr.DeleteObjectRequest(x9dc_scriptPlatforms[i].first);
    }
  }
  x9dc_scriptPlatforms.clear();
  CEntity* launcher1 = mgr.ObjectById(GetLauncherId());
  CEntity* launcher2 = mgr.ObjectById(x990_launcherId2);
  if (launcher1 != nullptr) {
    launcher1->SetActive(false);
  }
  if (launcher2 != nullptr) {
    launcher2->SetActive(false);
  }
  SetActive(false);
  mgr.SetBossParams(kInvalidUniqueId, 0.f, 89);
  xa38_collisionActorMgr1->SetActive(mgr, false);
  xa9c_collisionActorMgr2->SetActive(mgr, false);
}
