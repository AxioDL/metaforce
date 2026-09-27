#include "MetroidPrime/Enemies/CEyeBall.hpp"
#include "Collision/CMaterialList.hpp"
#include "Kyoto/Animation/CharacterCommon.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/ActorCommon.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CAiFuncMap.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CStateMachine.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/CBeamInfo.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"

#include <Kyoto/Animation/CInt32POINode.hpp>
#include <Kyoto/Audio/CSfxManager.hpp>
const char* CEyeBall::skEyeLocator = "Laser_LCTR";

CEyeBall::CEyeBall(const TUniqueId uid, const rstl::string& name, const EFlavorType flavor,
                   const CEntityInfo& info, const CTransform4f& xf, const CModelData& mData,
                   const CPatternedInfo& pInfo, const float attackDelay,
                   const float attackStartTime, const CAssetId wpscId, const CDamageInfo& dInfo,
                   const CAssetId beamContactFxId, const CAssetId beamPulseFxId,
                   const CAssetId beamTextureId, const CAssetId beamGlowTextureId, const uint anim0,
                   const uint anim1, const uint anim2, const uint anim3, const uint beamSfx,
                   const bool attackDisabled, const CActorParameters& actParams)
: CPatterned(kC_EyeBall, uid, name, flavor, info, xf, mData, pInfo, kMT_Flyer, kCT_Zero,
             kBT_Restricted, actParams, kCS_Medium)
, mAttackDelay(attackDelay)
, mAttackStartTime(attackStartTime)
, mTargetPosition(CVector3f::Zero())
, mBoneTracking(*AnimationData(), rstl::string_l("Eye"), skMaxRadianAngle, CMath::Deg2Rad(180.f),
                kBTF_NoParent)
, mProjectileInfo(wpscId, dInfo)
, mBeamContactFxId(beamContactFxId)
, mBeamPulseFxId(beamPulseFxId)
, mBeamTextureId(beamTextureId)
, mBeamGlowTextureId(beamGlowTextureId)
, mProjectileId(kInvalidUniqueId)
, mCurrentAnim(0)
, mBeamSfxId(CSfxManager::TranslateSFXID(beamSfx))
, mCanAttack(false)
, mPlayerInRange(false)
, mAlert(false)
, mAttackDisabled(attackDisabled)
, mFiringBeam(false) {
  mAnimIndices[0] = anim0;
  mAnimIndices[1] = anim1;
  mAnimIndices[2] = anim2;
  mAnimIndices[3] = anim3;
  KnockBackCtrl().SetAutoResetImpulse(false);
}

ENTITY_ACCEPT_IMPL(CEyeBall)

void CEyeBall::CreateBeam(CStateManager& mgr) {
  if (mProjectileId != kInvalidUniqueId) {
    return;
  }
  CBeamInfo beamInfo(1 | 2, mBeamContactFxId, mBeamPulseFxId, mBeamTextureId, mBeamGlowTextureId,
                     50, 0.05f, 1.f, 2.f, 20.f, 1.f, 1.f, 2.f, CColor(1.f, 1.f, 1.f, 0.f),
                     CColor(0.f, 1.f, 0.5f, 0.f), 150.f);

  mProjectileId = mgr.AllocateUniqueId();
  CEntity* proj = rs_new CPlasmaProjectile(
      mProjectileInfo.Token(), rstl::string_l("EyeBall_Beam"), kWT_AI, beamInfo,
      CTransform4f::Identity(), kMT_Character, mProjectileInfo.GetDamage(), mProjectileId,
      GetCurrentAreaId(), GetUniqueId(), CWeaponAssetInfo(), false, CWeapon::kPA_KeepInCinematic);
  mgr.AddObject(*proj);
}

void CEyeBall::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  bool skipForward = false;
  switch (msg) {
  case kSM_Damage: {
    if (const CGameProjectile* proj =
            TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(sender))) {
      if (proj->GetOwnerId() == mgr.GetPlayer()->GetUniqueId()) {
        if (static_cast< CActor* >(this)->GetDamageVulnerability()->GetVulnerability(
                proj->GetCurrentDamageInfo().GetWeaponMode(), CDamageVulnerability::kRD_No) !=
            kVN_Deflect) {
          SetWasHit(true);
        }
      }
    }
    skipForward = true;
  } break;
  case kSM_InvulnDamage: {
    if (const CGameProjectile* proj =
            TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(sender))) {
      if (proj->GetOwnerId() == mgr.GetPlayer()->GetUniqueId()) {
        if (static_cast< CActor* >(this)->GetDamageVulnerability()->GetVulnerability(
                proj->GetCurrentDamageInfo().GetWeaponMode(), CDamageVulnerability::kRD_No) !=
            kVN_Deflect) {
          SetWasHit(true);
        }
      }
    }
    skipForward = true;
  } break;
  case kSM_Alert:
    mAlert = true;
    break;
  case kSM_Registered:
    RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
    BodyCtrl()->Activate(mgr);
    StateMachineState().SetDelay(0.f);
    SetDrawShadow(false);
    CreateBeam(mgr);
    break;
  case kSM_Deleted:
    if (mProjectileId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mProjectileId);
      if (mBeamSfx) {
        CSfxManager::RemoveEmitter(mBeamSfx);
        mBeamSfx.Clear();
      }
    }
    mProjectileId = kInvalidUniqueId;
    break;
  default:
    break;
  }

  if (!skipForward) {
    CPatterned::AcceptScriptMsg(msg, sender, mgr);
  }
}

void CEyeBall::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                               const EUserEventType event, const float dt) {
  bool skipForward = false;
  switch (event) {
  case kUE_DamageOn: {
    if (mCanAttack) {
      skipForward = true;
      CTransform4f xf = GetLctrTransform(node.GetLocatorName());
      TurnLaserOn(mgr, xf);
    }
  } break;
  case kUE_DamageOff: {
    skipForward = true;
    TurnLaserOff(mgr);
  } break;
  default:
    break;
  }
  if (!skipForward) {
    CPatterned::DoUserAnimEvent(mgr, node, event, dt);
  }
}

void CEyeBall::InActive(CStateManager& mgr, const EStateMsg msg, const float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CEyeBall::Active(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    SetWasHit(false);
    BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    mCanAttack = false;
  } break;
  case kStateMsg_Update: {
    if (GetStateMachineTime() > GetAttackStartTime()) {
      mCanAttack = true;
    }
    UpdateCycleAnimation(arg);
  } break;
  case kStateMsg_Deactivate: {
    StateMachineState().SetDelay(mAttackDelay);

    if (CPlasmaProjectile* proj =
            static_cast< CPlasmaProjectile* >(mgr.ObjectById(mProjectileId))) {
      proj->ResetBeam(mgr, true);
    }

    mCanAttack = false;
    CSfxManager::RemoveEmitter(mBeamSfx);
    mBeamSfx.Clear();
  } break;
  }
}

void CEyeBall::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  CPatterned::PreRender(mgr, frustum);
  mBoneTracking.PreRender(mgr, *AnimationData(), GetTransform(), ModelData()->GetScale(),
                          *BodyCtrl());
}

void CEyeBall::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  if (!GetActive()) {
    return;
  }

  const CPlayer* player = mgr.GetPlayer();
  static float minAngle = CMath::FastCosR(skMaxRadianAngle);
  const CVector3f direction = (player->GetTranslation() - GetTranslation()).AsNormalized();
  const float angle = CVector3f::Dot(GetTransform().GetForward(), direction);

  mPlayerInRange =
      player->GetMorphballTransitionState() == CPlayer::kMS_Morphed && angle > minAngle;
  if (mPlayerInRange) {
    mBoneTracking.SetActive(true);
    mTargetPosition = player->GetTranslation() - (player->GetVelocityWR() * 0.5f);
    mBoneTracking.SetTargetPosition(mTargetPosition);
    mBoneTracking.Update(dt);
    AnimationData()->PreRender();
    // PreRenderBoneTracking(mgr, ModelData()->GetScale(), *BodyCtrl());
    mBoneTracking.PreRender(mgr, *AnimationData(), GetTransform(), ModelData()->GetScale(),
                            *BodyCtrl());
  } else {
    mBoneTracking.SetActive(false);
  }

  if (GetActive()) {
    CPlasmaProjectile* projectile =
        static_cast< CPlasmaProjectile* >(mgr.ObjectById(mProjectileId));
    if (projectile && projectile->GetActive()) {
      CTransform4f locator = GetLctrTransform(rstl::string_l(skEyeLocator));
      projectile->UpdateFx(locator, dt, mgr);
    }
  }

  if (!mFiringBeam) {
    return;
  }
  const CGameArea& area = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId());
  if (area.GetOcclusionState() != CGameArea::kOS_Visible) {
    TurnLaserOff(mgr);
  }
}

void CEyeBall::TryFlinch(CStateManager& mgr, int arg) {
  CBodyController* controller = BodyCtrl();
  controller->CommandMgr().DeliverCmd(
      CBCKnockBackCmd(GetTransform().GetColumn(kDX), static_cast< pas::ESeverity >(arg)));
}

void CEyeBall::Render(const CStateManager& mgr) const { return CPatterned::Render(mgr); }

void CEyeBall::Flinch(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    SetAnimationState(kAS_Ready);
    StateMachineState().SetDelay(mAttackDelay);
  } break;
  case kStateMsg_Update: {
    TryCommand(mgr, pas::kAS_KnockBack, static_cast< FTryCommandCallback >(&CEyeBall::TryFlinch), 0);
  } break;
  case kStateMsg_Deactivate: {
    SetAnimationState(kAS_NotReady);
  } break;
  }
}
void CEyeBall::Cover(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
    mCanAttack = false;
    StateMachineState().SetDelay(mAttackDelay);
  } break;
  case kStateMsg_Update: {
  } break;
  case kStateMsg_Deactivate: {

  } break;
  }
}
bool CEyeBall::ShouldFire(CStateManager& mgr, const float arg) { return !mAttackDisabled; }

bool CEyeBall::ShouldAttack(CStateManager&, float) { return mAlert; }

void CEyeBall::UpdateCycleAnimation(const float dt) {
  if (!close_enough(
          GetModelData()->GetAnimationData()->GetAnimTimeRemaining(rstl::string_l("Whole Body")),
          0.f)) {
    return;
  }

  int i = 0;
  mCurrentAnim = (mCurrentAnim + 1) % 4;
  for (; mAnimIndices[mCurrentAnim] == -1 && i < 4; i++) {
    mCurrentAnim = (mCurrentAnim + 1) % 4;
  }
  const int animIdx = mAnimIndices[mCurrentAnim];
  if (animIdx != -1) {
    CBodyController* controller = BodyCtrl();
    controller->CommandMgr().DeliverCmd(CBCScriptedCmd(animIdx, false, false, 0.f));
  }
}

void CEyeBall::Touch(CActor&, CStateManager&) {}

void CEyeBall::TurnLaserOn(CStateManager& mgr, const CTransform4f& xf) {
  CPlasmaProjectile* proj = static_cast< CPlasmaProjectile* >(mgr.ObjectById(mProjectileId));
  if (!proj) {
    return;
  }
  if (proj->GetActive()) {
    return;
  }

  proj->Fire(xf, mgr, false);
  mFiringBeam = true;
  if (mBeamSfx) {
    return;
  }
  TAreaId id = GetCurrentAreaId();

  CAudioSys::C3DEmitterParmData parmData(50.f, 0.1f, 0x1, 127, 20);
  parmData.mPos = GetTranslation();
  parmData.mDir = CVector3f::Zero();
  parmData.mSfxId = mBeamSfxId;
  mBeamSfx = CSfxManager::AddEmitter(parmData, true, CSfxManager::kMedPriority, true, id.Value());
}
void CEyeBall::TurnLaserOff(CStateManager& mgr) {

  if (CPlasmaProjectile* proj = static_cast< CPlasmaProjectile* >(mgr.ObjectById(mProjectileId))) {
    proj->ResetBeam(mgr, true);
  }
  mFiringBeam = false;
  if (!mBeamSfx) {
    return;
  }
  CSfxManager::RemoveEmitter(mBeamSfx);
  mBeamSfx.Clear();
}
void CEyeBall::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  const CTransform4f xf = GetTransform();
  CPatterned::Death(mgr, direction, state);
  SetTransform(xf);
}
