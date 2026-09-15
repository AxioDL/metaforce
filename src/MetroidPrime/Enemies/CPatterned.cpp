#include "MetroidPrime/Enemies/CPatterned.hpp"

#include <math.h>

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialList.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Animation/CVertexMorphEffect.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyState.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CActorModelParticles.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CSimpleShadow.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CCameraShakeData.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "dolphin/gx/GXStruct.h"
#include "dolphin/types.h"

const float CPatterned::skDamageHitTime = 0.33f;
const float CPatterned::skActorApproachDistance = 3.f;

const CColor CPatterned::skDamageColor = CColor(0.5f, 0.f, 0.f, 1.f);
const CColor CPatterned::skFrozenDamageColor = CColor(0.5f, 0.5f, 0.f, 1.f);

static CColor skDisintegrateColor(static_cast< u8 >(0xff), 0xff, 0xc0, 0xff);

static CMaterialList gkPatternedFlyerMaterialList(kMT_Character, kMT_Solid, kMT_Orbit, kMT_Target);
static CMaterialList gkPatternedGroundMaterialList(kMT_Character, kMT_Solid, kMT_Orbit,
                                                   kMT_GroundCollider, kMT_Target);

static EMaterialTypes skCharacterMat = kMT_Character;

class CMetroid;

CPatterned::CPatterned(const EPatternedAI character, const TUniqueId uid, const rstl::string& name,
                       const EFlavorType flavor, const CEntityInfo& info, const CTransform4f& xf,
                       const CModelData& mData, const CPatternedInfo& pinfo, EMovementType movement,
                       const EColliderType collider, const EBodyType body,
                       const CActorParameters& params, const ECreatureSize kbVariant)
: CAi(uid, pinfo.xf8_active, name, info, xf, mData,
      CAABox(-pinfo.xc4_halfExtent + pinfo.xcc_bodyOrigin.GetX(),
             -pinfo.xc4_halfExtent + pinfo.xcc_bodyOrigin.GetY(), pinfo.xcc_bodyOrigin.GetZ(),
             pinfo.xc4_halfExtent + pinfo.xcc_bodyOrigin.GetX(),
             pinfo.xc4_halfExtent + pinfo.xcc_bodyOrigin.GetY(),
             pinfo.xcc_bodyOrigin.GetZ() + pinfo.xc8_height),
      pinfo.x0_mass, pinfo.x54_healthInfo, pinfo.x5c_damageVulnerability,
      movement == kMT_Flyer ? gkPatternedFlyerMaterialList : gkPatternedGroundMaterialList,
      pinfo.xfc_stateMachineId, params, pinfo.xd8_stepUpHeight, 0.8f)
, x2d8_patrolState(kPS_Invalid)
, x2dc_destObj(kInvalidUniqueId)
, x2e0_destPos(CVector3f::Zero())
, x2ec_reflectedDestPos(CVector3f::Zero())
, x2f8_waypointPauseRemTime(0.f)
, x2fc_minAttackRange(pinfo.x18_minAttackRange)
, x300_maxAttackRange(pinfo.x1c_maxAttackRange)
, x304_averageAttackTime(pinfo.x20_averageAttackTime)
, x308_attackTimeVariation(pinfo.x24_attackTimeVariation)
, x30c_behaviourOrient(kBO_MoveDir)
, x310_moveVec(CVector3f::Zero())
, x31c_faceVec(CVector3f::Zero())
, x328_24_inPosition(false)
, x328_25_verticalMovement(movement == kMT_Flyer)
, x328_26_solidCollision(false)
, x328_27_onGround(movement != kMT_Flyer)
, x328_28_prevOnGround(true)
, x328_29_noPatternShagging(false)
, x328_30_lookAtDeathDir(true)
, x328_31_energyAttractor(false)
, x329_24_(true)
, x32c_animState(kAS_NotReady)
, x330_stateMachineState()
, x34c_characterType(character)
, x350_patternStartPos(CVector3f::Zero())
, x35c_patternStartPlayerPos(CVector3f::Zero())
, x368_destWPDelta(CVector3f::Zero())
, x374_patternTranslate(kPT_RelativeStart)
, x378_patternOrient(kPO_ReversePlayerForward)
, x37c_patternFit(kPF_One)
, x380_behaviour(kB_Zero)
, x384_behaviourModifiers(kBM_Zero)
, x388_anim(pinfo.GetAnimationParameters().GetInitialAnimation())
, x38c_patterns()
, x39c_curPattern(0)
, x3a0_latestLeashPosition(CVector3f::Zero())
, x3ac_lastPatrolDest(kInvalidUniqueId)
, x3b0_moveSpeed(1.f)
, x3b4_speed(pinfo.x4_speed)
, x3b8_turnSpeed(pinfo.x8_turnSpeed)
, x3bc_detectionRange(pinfo.xc_detectionRange)
, x3c0_detectionHeightRange(pinfo.x10_detectionHeightRange)
, x3c4_detectionAngle(cosf(0.017453292f * pinfo.x14_dectectionAngle))
, x3c8_leashRadius(pinfo.x28_leashRadius)
, x3cc_playerLeashRadius(pinfo.x2c_playerLeashRadius)
, x3d0_playerLeashTime(pinfo.x30_playerLeashTime)
, x3d4_curPlayerLeashTime(0.f)
, x3d8_xDamageThreshold(pinfo.xdc_xDamage)
, x3dc_frozenXDamageThreshold(pinfo.xe0_frozenXDamage)
, x3e0_xDamageDelay(pinfo.xe4_xDamageDelay)
, x3e4_lastHP(0.f)
, x3e8_alphaDelta(0.f)
, x3ec_pendingFireDamage(0.f)
, x3f0_pendingShockDamage(0.f)
, x3f4_burnThinkRateTimer(0.f)
, x3f8_moveState(kMS_Zero)
, x3fc_flavor(flavor)
, x400_24_hitByPlayerProjectile(false)
, x400_25_alive(true)
, x400_26_(false)
, x400_27_fadeToDeath(false)
, x400_28_pendingMassiveDeath(false)
, x400_29_pendingMassiveFrozenDeath(false)
, x400_30_patternShagged(false)
, x400_31_isFlyer(movement == kMT_Flyer)
, x401_24_pathOverCount(0)
, x401_26_disableMove(false)
, x401_27_phazingOut(false)
, x401_28_burning(false)
, x401_29_laggedBurnDeath(false)
, x401_30_pendingDeath(false)
, x401_31_nextPendingShock(false)
, x402_24_pendingShock(false)
, x402_25_lostMassiveFrozenHP(false)
, x402_26_dieIf80PercFrozen(false)
, x402_27_noXrayModel(false)
, x402_28_isMakingBigStrike(false)
, x402_29_drawParticles(true)
, x402_30_updateThermalFrozenState(params.IsHotInThermal())
, x402_31_thawed(x402_30_updateThermalFrozenState)
, x403_24_keepThermalVisorState(false)
, x403_25_enableStateMachine(true)
, x403_26_stateControlledMassiveDeath(true)
, x404_contactDamage(pinfo.x34_contactDamageInfo)
, x420_curDamageRemTime(0.f)
, x424_damageWaitTime(pinfo.x50_damageWaitTime)
, x428_damageCooldownTimer(-1.f)
, x42c_color(0.f, 0.f, 0.f, 1.f)
, x430_damageColor(skDamageColor)
, x434_posDelta(CVector3f::Zero())
, x440_rotDelta(CQuaternion::NoRotation())
, x450_bodyController()
, x454_deathSfx(pinfo.xe8_deathSfx)
, x458_iceShatterSfx(pinfo.x134_iceShatterSfx)
, x45c_steeringBehaviors()
, x460_knockBackController(kbVariant)
, x4e4_latestPredictedTranslation(CVector3f::Zero())
, x4f0_predictedLeashTime(0.f)
, x4f4_intoFreezeDur(pinfo.x100_intoFreezeDur)
, x4f8_outofFreezeDur(pinfo.x104_outofFreezeDur)
, x4fc_freezeDur(pinfo.x108_freezeDur)
, x500_preThinkDt(0.f)
, x504_damageDur(0.f)
, x508_colliderType(collider)
, x50c_baseDamageMag(params.GetThermalMag())
, x510_vertexMorph(nullptr)
, x514_deathExplosionOffset(pinfo.x110_particle1Scale)
, x520_deathExplosionParticle()
, x530_deathExplosionElectric()
, x540_iceDeathExplosionOffset(pinfo.x124_particle2Scale)
, x54c_iceDeathExplosionParticle()
, x55c_moveScale(1.f, 1.f, 1.f) {
  if (pinfo.x11c_particle1 != kInvalidAssetId) {
    x520_deathExplosionParticle = gpSimplePool->GetObj(SObjectTag('PART', pinfo.x11c_particle1));
    x520_deathExplosionParticle->Lock();
  }

  if (pinfo.x120_electric != kInvalidAssetId) {
    x530_deathExplosionElectric = gpSimplePool->GetObj(SObjectTag('ELSC', pinfo.x120_electric));
    x530_deathExplosionElectric->Lock();
  }

  if (pinfo.x130_particle2 != kInvalidAssetId) {
    x54c_iceDeathExplosionParticle = gpSimplePool->GetObj(SObjectTag('PART', pinfo.x130_particle2));
    x54c_iceDeathExplosionParticle->Lock();
  }

  if (x404_contactDamage.GetRadius() > 0.f) {
    x404_contactDamage.SetRadius(0.f);
  }

  SetRenderParticleDatabaseInside(false);

  bool buildBodyController = false;
  if (ModelData() && ModelData()->IsNotNull()) {
    buildBodyController = true;
  }
  if (buildBodyController) {
    x402_27_noXrayModel = !ModelData()->HasModel(CModelData::kWM_XRay);
    BuildBodyController(body);
  }
}

void CPatterned::BuildBodyController(EBodyType bodyType) {
  if (!x450_bodyController.null()) {
    return;
  }

  x450_bodyController = rs_new CBodyController(*this, x3b8_turnSpeed, bodyType);

  CPASAnimParmData data(pas::kAS_AdditiveReaction, CPASAnimParm::FromEnum(0));
  const rstl::pair< float, int > bestAnim =
      x450_bodyController->GetPASDatabase().FindBestAnimation(data, -1);
  x460_knockBackController.x81_26_enableShock = bestAnim.first > 0.f;
}

ENTITY_ACCEPT_IMPL(CPatterned)

void CPatterned::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CAi::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case kSM_Registered: {
    if (x508_colliderType != kCT_One) {
      CMaterialList include = GetMaterialFilter().GetIncludeList();
      CMaterialList exclude = GetMaterialFilter().GetExcludeList();
      CMaterialList charMat(skCharacterMat);
      include.Remove(charMat);
      exclude.Add(charMat);
      SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
    }

    bool hasAnimData = ModelData() && ModelData()->HasAnimation();
    if (hasAnimData && ModelData()->GetAnimationData()->GetIceModel().valid()) {
      const CAABox& baseBox = GetBaseBoundingBox();
      CVector3f extent = baseBox.GetMaxPoint() - baseBox.GetMinPoint();
      const float diagExtent = 0.5f * extent.Magnitude();

      x510_vertexMorph = rstl::ncrc_ptr< CVertexMorphEffect >(
          rs_new CVertexMorphEffect(CUnitVector3f(CVector3f(1.f, 0.f, 0.f), CUnitVector3f::kN_Yes),
                                    CVector3f(0.f, 0.f, 0.f), 0.f, diagExtent, *mgr.Random()));
    }

    SetAngularEnabled(true);
    break;
  }
  case kSM_OnFloor:
    if (!x328_25_verticalMovement) {
      SetMomentumWR(CVector3f::Zero());
      AddMaterial(kMT_GroundCollider, mgr);
    }
    x328_27_onGround = true;
    break;
  case kSM_Falling:
    if (!x328_25_verticalMovement) {
      if (x450_bodyController->GetPercentageFrozen() == 0.f) {
        SetMomentumWR(CVector3f(0.f, 0.f, -GetWeight()));
        RemoveMaterial(kMT_GroundCollider, mgr);
      }
    }
    x328_27_onGround = false;
    break;
  case kSM_Activate:
    x3a0_latestLeashPosition = GetTranslation();
    break;
  case kSM_Deleted: {
    CAiState* state = x330_stateMachineState.GetActorState();
    if (state != 0) {
      state->CallFunc(mgr, *this, kStateMsg_Deactivate, 0.f);
    }
    break;
  }
  case kSM_Damage: {
    const CGameProjectile* proj = TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(uid));
    if (proj != 0) {
      if (proj->GetCurrentDamageInfo().GetWeaponMode().GetType() == kWT_Wave) {
        if (x460_knockBackController.x81_26_enableShock &&
            proj->GetCurrentDamageInfo().GetWeaponMode().IsComboed() && HealthInfo(mgr) != 0) {
          x401_31_nextPendingShock = true;
          KnockBack(GetTransform().GetForward(), mgr, proj->GetCurrentDamageInfo(),
                    proj->GetCurrentDamageInfo().GetKnockBackPower(), true, false);
          x460_knockBackController.DeferKnockBack(kWT_Wave);
        }
      } else if (proj->GetCurrentDamageInfo().GetWeaponMode().GetType() == kWT_Plasma) {
        if (x460_knockBackController.x81_27_enableBurn &&
            proj->GetCurrentDamageInfo().GetWeaponMode().IsComboed() && HealthInfo(mgr) != 0) {
          KnockBack(GetTransform().GetForward(), mgr, proj->GetCurrentDamageInfo(),
                    proj->GetCurrentDamageInfo().GetKnockBackPower(), true, false);
          x460_knockBackController.DeferKnockBack(kWT_Plasma);
        }
      }

      if (proj->GetOwnerId() == mgr.Player()->GetUniqueId()) {
        x400_24_hitByPlayerProjectile = true;
      }
    }
    break;
  }
  case kSM_InvulnDamage: {
    const CGameProjectile* proj = TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(uid));
    if (proj != 0 && proj->GetOwnerId() == mgr.Player()->GetUniqueId()) {
      x400_24_hitByPlayerProjectile = true;
    }
    break;
  }
  default:
    break;
  }
}

void CPatterned::SetDestPos(const CVector3f& pos) { x2e0_destPos = pos; }

CVector3f CPatterned::GetGunEyePos() const {
  CVector3f origin = GetTranslation();
  const CAABox& baseBox = GetBaseBoundingBox();
  origin[kDZ] += 0.6f * (baseBox.GetMaxPoint().GetZ() - baseBox.GetMinPoint().GetZ());
  return origin;
}

u8 CPatterned::ApplyBoneTracking() const {
  bool ret = false;
  if (x400_25_alive) {
    ret = x460_knockBackController.GetFlinchRemTime() <= 0.f;
  }

  return ret;
}

float CPatterned::GetAnimationDistance(const CPASAnimParmData& data) const {
  float dist = 1.f;
  rstl::pair< float, int > bestAnim =
      GetModelData()->GetAnimationData()->GetCharacterInfo().GetPASDatabase().FindBestAnimation(
          data, -1);

  if (bestAnim.first > FLT_EPSILON) {
    const CAnimData* animData = GetModelData()->GetAnimationData();
    const float duration = animData->GetAnimationDuration(bestAnim.second);
    dist = animData->GetAverageVelocity(bestAnim.second);
    dist *= duration;
  }

  return dist;
}

static int skPlayerMat = kMT_Player;

void CPatterned::SetupPlayerCollision(const bool startsHidden) {
  if (startsHidden) {
    CMaterialList include = GetMaterialFilter().GetIncludeList();
    CMaterialList exclude = GetMaterialFilter().GetExcludeList();
    CMaterialList playerMat(static_cast< EMaterialTypes >(skPlayerMat));
    include.Add(playerMat);
    exclude.Remove(playerMat);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
  } else {
    CMaterialList include = GetMaterialFilter().GetIncludeList();
    CMaterialList exclude = GetMaterialFilter().GetExcludeList();
    CMaterialList playerMat(static_cast< EMaterialTypes >(skPlayerMat));
    include.Remove(playerMat);
    exclude.Add(playerMat);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
  }
}

CScriptCoverPoint* CPatterned::GetCoverPoint(CStateManager& mgr, TUniqueId id) const {
  CScriptCoverPoint* cp = NULL;
  if (id.value != kInvalidUniqueId.value) {
    cp = TCastToPtr< CScriptCoverPoint >(mgr.ObjectById(id));
  }
  return cp;
}

void CPatterned::ReleaseCoverPoint(CStateManager& mgr, TUniqueId& id) {
  if (CScriptCoverPoint* cp = GetCoverPoint(mgr, id)) {
    cp->SetInUse(false);
    id = kInvalidUniqueId;
  }
}

void CPatterned::SetCoverPoint(CScriptCoverPoint* cp, TUniqueId& id) {
  cp->SetInUse(true);
  id = cp->GetUniqueId();
}

void CPatterned::TryCommand(CStateManager& mgr, int state, FTryCommandCallback cb, int arg) {
  if (state == x450_bodyController->GetBodyStateInfo().GetCurrentStateId()) {
    x32c_animState = kAS_Repeat;
  } else if (x32c_animState == kAS_Ready) {
    (this->*cb)(mgr, arg);
  } else {
    x32c_animState = kAS_Over;
  }
}

void CPatterned::TryProjectileAttack(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = x450_bodyController->CommandMgr();
  CBCProjectileAttackCmd cmd(static_cast< pas::ESeverity >(arg), x2e0_destPos, false);
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryMeleeAttack(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = x450_bodyController->CommandMgr();
  CBCMeleeAttackCmd cmd(static_cast< pas::ESeverity >(arg));
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryMeleeAttack_TargetPos(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = x450_bodyController->CommandMgr();
  CBCMeleeAttackCmd cmd(static_cast< pas::ESeverity >(arg), x2e0_destPos);
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryStep(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = x450_bodyController->CommandMgr();
  CBCStepCmd cmd(static_cast< pas::EStepDirection >(arg), pas::kStep_Normal);
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryDodge(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = x450_bodyController->CommandMgr();
  CBCStepCmd cmd(static_cast< pas::EStepDirection >(arg), pas::kStep_Dodge);
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryBreakDodge(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = x450_bodyController->CommandMgr();
  CBCStepCmd cmd(static_cast< pas::EStepDirection >(arg), pas::kStep_BreakDodge);
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryGenerateDeactivate(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = x450_bodyController->CommandMgr();
  CBCGenerateCmd cmd(static_cast< pas::EGenerateType >(arg), CVector3f::Zero());
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryGenerate(CStateManager&, int) {
  CBodyStateCmdMgr& cmdMgr = x450_bodyController->CommandMgr();
  CBCGenerateCmd cmd(pas::kGType_Zero, x2e0_destPos, true);
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryJump(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = x450_bodyController->CommandMgr();
  CBCJumpCmd cmd(x2e0_destPos, static_cast< pas::EJumpType >(arg), false);
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TrySlide(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = x450_bodyController->CommandMgr();
  CBCSlideCmd cmd(static_cast< pas::ESlideType >(arg), x2e0_destPos - GetTranslation());
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryTaunt(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = x450_bodyController->CommandMgr();
  CBCTauntCmd cmd(static_cast< pas::ETauntType >(arg));
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryKnockBack_Front(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = x450_bodyController->CommandMgr();
  CBCKnockBackCmd cmd(GetTransform().GetForward(), static_cast< pas::ESeverity >(arg));
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryGetUp(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = x450_bodyController->CommandMgr();
  CBCGetupCmd cmd(static_cast< pas::EGetupType >(arg));
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryTurn(CStateManager&, int) {
  CBodyStateCmdMgr& cmdMgr = x450_bodyController->CommandMgr();
  CVector3f face = (x2e0_destPos - GetTranslation()).AsNormalized();
  CBCLocomotionCmd cmd(CVector3f::Zero(), face, 1.f);
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryLoopedReaction(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = x450_bodyController->CommandMgr();
  CBCLoopReactionCmd cmd(static_cast< pas::EReactionType >(arg));
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryLoopedHitReaction(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = x450_bodyController->CommandMgr();
  CBCLoopHitReactionCmd cmd(static_cast< pas::EReactionType >(arg));
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryKnockBack(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = x450_bodyController->CommandMgr();
  CBCKnockBackCmd cmd(GetTransform().GetForward(), static_cast< pas::ESeverity >(arg));
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  if (x400_25_alive) {
    if (!x450_bodyController->IsOnFire()) {
      x402_25_lostMassiveFrozenHP =
          (x3e4_lastHP - HealthInfo(mgr)->GetHP()) >= x3dc_frozenXDamageThreshold;
      if (x402_25_lostMassiveFrozenHP && x54c_iceDeathExplosionParticle.valid() &&
          x450_bodyController->GetPercentageFrozen() > 0.8f) {
        x400_29_pendingMassiveFrozenDeath = true;
      } else if ((x3e4_lastHP - HealthInfo(mgr)->GetHP()) >= x3d8_xDamageThreshold) {
        x400_28_pendingMassiveDeath = true;
      }
    }

    if (x400_28_pendingMassiveDeath || x400_29_pendingMassiveFrozenDeath) {
      if (x328_30_lookAtDeathDir && x3e0_xDamageDelay <= 0.f) {
        bool hasDirection = false;
        if (0.f != direction.GetX() || 0.f != direction.GetY() || 0.f != direction.GetZ()) {
          hasDirection = true;
        }

        if (hasDirection) {
          const CVector3f pos = GetTranslation();
          const CVector3f target = pos - direction;
          const CTransform4f deathXf =
              CTransform4f::LookAt(pos, target) * CTransform4f::RotateX(CRelAngle(0.7853982f));
          SetTransform(deathXf);
        }
      }
    } else {
      x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("Dead"));
      RemoveMaterial(kMT_GroundCollider, mgr);
      x328_25_verticalMovement = false;
    }

    x400_25_alive = false;
    if (x450_bodyController->HasBodyState(pas::kAS_Hurled) &&
        x450_bodyController->GetBodyType() == kBT_Flyer) {
      x450_bodyController->CommandMgr().DeliverCmd(CBCHurledCmd(-direction, CVector3f::Zero()));
    } else if (x450_bodyController->HasBodyState(pas::kAS_Fall)) {
      x450_bodyController->CommandMgr().DeliverCmd(CBCKnockDownCmd(-direction, pas::kS_One));
    }

    if (state != kSS_Any) {
      SendScriptMsgs(state, mgr, kSM_None);
    }
  }
}

void CPatterned::GenerateDeathExplosion(CStateManager& mgr) {
  const rstl::optional_object< TCachedToken< CGenDescription > >& deathParticle =
      GetDeathExplosionParticle();
  const rstl::optional_object< TCachedToken< CElectricDescription > >& deathElectric =
      x530_deathExplosionElectric;

  if (deathParticle.valid() || deathElectric.valid()) {
    CTransform4f xf(GetTransform());
    const CVector3f offset =
        CVector3f::ByElementMultiply(GetModelData()->GetScale(), x514_deathExplosionOffset);
    xf.SetTranslation(GetTransform() * offset);

    if (deathParticle.valid()) {
      if (CExplosion* explosion = rs_new CExplosion(
              TLockedToken< CGenDescription >(*deathParticle), mgr.AllocateUniqueId(), true,
              CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList), rstl::string_l(""), xf,
              x402_31_thawed ? 0 : 1, CVector3f(1.f, 1.f, 1.f), CColor::White())) {
        mgr.AddObject(explosion);
      }
    }

    if (deathElectric.valid()) {
      if (CExplosion* explosion = rs_new CExplosion(
              TLockedToken< CElectricDescription >(*deathElectric), mgr.AllocateUniqueId(), true,
              CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList), rstl::string_l(""), xf,
              0, CVector3f(1.f, 1.f, 1.f), CColor::White())) {
        mgr.AddObject(explosion);
      }
    }
  }
}

void CPatterned::GenerateIceDeathExplosion(CStateManager& mgr) {
  const rstl::optional_object< TCachedToken< CGenDescription > >& deathParticle =
      x54c_iceDeathExplosionParticle;
  if (deathParticle.valid()) {
    CTransform4f xf(GetTransform());
    const CVector3f offset =
        CVector3f::ByElementMultiply(GetModelData()->GetScale(), x540_iceDeathExplosionOffset);
    const CVector3f position = GetTransform() * offset;
    const bool valid = deathParticle.valid();
    xf.SetTranslation(position);

    if (valid) {
      if (CExplosion* explosion = rs_new CExplosion(
              TLockedToken< CGenDescription >(*deathParticle), mgr.AllocateUniqueId(), true,
              CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList), rstl::string_l(""), xf,
              1, CVector3f(1.f, 1.f, 1.f), CColor::White())) {
        mgr.AddObject(explosion);
      }
    }
  }
}

void CPatterned::MassiveDeath(CStateManager& mgr) {
  const ushort sfx = x454_deathSfx;
  CSfxManager::AddEmitter(sfx, GetTranslation(), CVector3f::Zero(), true, false,
                          CSfxManager::kMedPriority, CSfxManager::kAllAreas);

  if (!x401_28_burning) {
    SendScriptMsgs(kSS_MassiveDeath, mgr, kSM_None);
    GenerateDeathExplosion(mgr);
  }

  DeathDelete(mgr);
  x400_28_pendingMassiveDeath = x400_29_pendingMassiveFrozenDeath = false;
}

void CPatterned::MassiveFrozenDeath(CStateManager& mgr) {
  if (x458_iceShatterSfx == CSfxManager::kInternalInvalidSfxId) {
    x458_iceShatterSfx = x454_deathSfx;
  }

  const ushort sfx = x458_iceShatterSfx;
  CSfxManager::AddEmitter(sfx, GetTranslation(), CVector3f::Zero(), true, false,
                          CSfxManager::kMedPriority, CSfxManager::kAllAreas);
  SendScriptMsgs(kSS_MassiveFrozenDeath, mgr, kSM_None);
  GenerateIceDeathExplosion(mgr);

  const CVector3f playerDelta = mgr.Player()->GetTranslation() - GetTranslation();
  const float toPlayerDist = playerDelta.Magnitude();
  if (toPlayerDist < 40.f) {
    mgr.CameraManager()->AddCameraShaker(
        CCameraShakeData::HardHorizShakeDistance(GetTranslation(), 0.25f, 0.3f, 40.f), true);
  }

  DeathDelete(mgr);
  x400_28_pendingMassiveDeath = x400_29_pendingMassiveFrozenDeath = false;
}

void CPatterned::KnockBack(const CVector3f& backVec, CStateManager& mgr, const CDamageInfo& info,
                           float magnitude, bool direct, bool) {
  CHealthInfo* health = HealthInfo(mgr);
  if (x401_27_phazingOut || x401_28_burning || health == nullptr) {
    return;
  }

  x460_knockBackController.KnockBack(backVec, mgr, *this, info, magnitude, direct);

  if (x450_bodyController->IsFrozen() &&
      x460_knockBackController.GetActiveParms().xc_intoFreezeDur >= 0.f) {
    x450_bodyController->FrozenBreakout();
  }

  switch (x460_knockBackController.GetActiveParms().x4_animFollowup) {
  case kKBAFU_Freeze: {
    CVector3f pos(0.f, 0.f, 0.f);
    CUnitVector3f dir = GetTransform().TransposeRotate(backVec);
    float dur = x460_knockBackController.GetActiveParms().x8_followupDuration;
    Freeze(mgr, pos, dir, dur);
    break;
  }
  case kKBAFU_PhazeOut:
    PhazeOut(mgr);
    break;
  case kKBAFU_Shock:
    Shock(mgr, x460_knockBackController.GetActiveParms().x8_followupDuration, -1.f);
    break;
  case kKBAFU_Burn:
    Burn(x460_knockBackController.GetActiveParms().x8_followupDuration, 0.25f);
    break;
  case kKBAFU_LaggedBurnDeath:
    x401_29_laggedBurnDeath = true;
  case kKBAFU_BurnDeath: {
    Burn(x460_knockBackController.GetActiveParms().x8_followupDuration, -1.f);
    Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
    x400_28_pendingMassiveDeath = x400_29_pendingMassiveFrozenDeath = false;
    x400_27_fadeToDeath = x401_28_burning = true;
    x3f4_burnThinkRateTimer = 1.5f;
    x402_29_drawParticles = false;
    x450_bodyController->DouseFlames();
    CActorModelParticles* particles = mgr.ActorModelParticles();
    particles->StopFire(*this);
    particles->StartBurnDeath(*this);
    if (!x401_29_laggedBurnDeath) {
      particles->DoFirePop(*this);
      particles->StartAsh(*this);
    }
    break;
  }
  case kKBAFU_Death:
    Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
    break;
  case kKBAFU_ExplodeDeath:
    Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
    if (GetDeathExplosionParticle().valid() || x530_deathExplosionElectric.valid()) {
      MassiveDeath(mgr);
    } else if (x450_bodyController->IsFrozen()) {
      x450_bodyController->FrozenBreakout();
    }
    break;
  case kKBAFU_IceDeath:
    Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
    if (x54c_iceDeathExplosionParticle.valid()) {
      MassiveFrozenDeath(mgr);
    } else if (x450_bodyController->IsFrozen()) {
      x450_bodyController->FrozenBreakout();
    }
    break;
  default:
    break;
  }
}

void CPatterned::UpdateAlphaDelta(float dt, CStateManager& mgr) {
  if (x3e8_alphaDelta == 0.f) {
    return;
  }

  float alpha = dt * x3e8_alphaDelta + x42c_color.GetAlpha();
  if (alpha > 1.f) {
    alpha = 1.f;
    x3e8_alphaDelta = 0.f;
  } else if (alpha < 0.f) {
    alpha = 0.f;
    x3e8_alphaDelta = 0.f;
    if (x400_27_fadeToDeath) {
      DeathDelete(mgr);
    }
  }

  Shadow()->SetUserAlpha(alpha);
  x42c_color.SetAlpha(alpha);
  ModelData()->AnimationData()->GetParticleDB().SetModulationColorAllActiveEffects(
      CColor(1.f, 1.f, 1.f, alpha));
}

void CPatterned::UpdateDamageColor(float dt) {
  if (x428_damageCooldownTimer > 0.f) {
    x428_damageCooldownTimer = CMath::Max(0.f, x428_damageCooldownTimer - dt);
    const float t = CMath::Min(x428_damageCooldownTimer / 0.33f, 1.f);
    const CColor& color = CColor::Lerp(CColor::Black(), x430_damageColor, t);
    x42c_color.Set(color.GetRedu8(), color.GetGreenu8(), color.GetBlueu8(),
                   x42c_color.GetAlphau8());

    if (!x450_bodyController->IsFrozen()) {
      SetDamageMag(x50c_baseDamageMag + x428_damageCooldownTimer);
    }
  }
}

// TODO: Move to rstl/string.hpp once header inlining preserves the helper and Think codegen.
bool rstl::operator==(const char* lhs, const rstl::string& rhs) {
  return rhs.compare(lhs, -1) == 0;
}

void CPatterned::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (x402_30_updateThermalFrozenState) {
    UpdateThermalFrozenState(x450_bodyController->GetPercentageFrozen() == 0.f);
  }

  const CAnimData* animData = GetModelData()->GetAnimationData();
  const rstl::optional_object< TLockedToken< CSkinnedModelWithAvgNormals > >& iceModel =
      animData->GetIceModel();
  if (iceModel.valid()) {
    x510_vertexMorph->Update(dt);
  }

  if (x402_26_dieIf80PercFrozen && x450_bodyController->GetPercentageFrozen() > 0.8f) {
    x400_29_pendingMassiveFrozenDeath = true;
  }

  if (!x400_25_alive) {
    if ((x400_28_pendingMassiveDeath || x400_29_pendingMassiveFrozenDeath) &&
        x3e0_xDamageDelay <= 0.f) {
      if (x400_29_pendingMassiveFrozenDeath) {
        SendScriptMsgs(kSS_AboutToMassivelyDie, mgr, kSM_None);
        MassiveFrozenDeath(mgr);
      } else {
        SendScriptMsgs(kSS_AboutToMassivelyDie, mgr, kSM_None);
        MassiveDeath(mgr);
      }
      return;
    }

    x3e0_xDamageDelay -= dt;

    if (x403_26_stateControlledMassiveDeath) {
      if (x330_stateMachineState.GetName() != 0) {
        const bool isDead = x330_stateMachineState.GetName() == rstl::string_l("Dead");
        if (isDead && x330_stateMachineState.GetTime() > 15.f) {
          MassiveDeath(mgr);
        }
      }
    }
  }

  UpdateAlphaDelta(dt, mgr);

  x3e4_lastHP = HealthInfo(mgr)->GetHP();
  if (x330_stateMachineState.GetActorState() == 0) {
    x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("Start"));
  }

  CVector3f diffVec = x4e4_latestPredictedTranslation - GetTranslation();
  if (!x328_25_verticalMovement) {
    diffVec.SetZ(0.f);
  }

  if (diffVec.MagSquared() > (0.1f * dt)) {
    x4f0_predictedLeashTime += dt;
  } else {
    x4f0_predictedLeashTime = 0.f;
  }

  if (x460_knockBackController.x81_26_enableShock) {
    if (!x401_31_nextPendingShock && x402_24_pendingShock) {
      Shock(mgr, 0.5f + mgr.Random()->Range(0.f, 0.5f), 0.2f);
    }

    x402_24_pendingShock = x401_31_nextPendingShock;
    x401_31_nextPendingShock = false;

    if (x450_bodyController->IsElectrocuting()) {
      mgr.ActorModelParticles()->StartElectric(*this);

      if (x3f0_pendingShockDamage > 0.f && x400_25_alive) {
        const CDamageInfo shockDmg =
            CDamageInfo(CWeaponMode::Wave(), x3f0_pendingShockDamage, 0.f, 0.f);
        mgr.ApplyDamage(
            kInvalidUniqueId, GetUniqueId(), kInvalidUniqueId, shockDmg.MakeScaledForTime(dt),
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
      }
    } else if (x3f0_pendingShockDamage != 0.f) {
      x3f0_pendingShockDamage = 0.f;
      x450_bodyController->StopElectrocution();
      mgr.ActorModelParticles()->StopElectric(*this);
    }
  }

  if (x450_bodyController->IsOnFire()) {
    if (x400_25_alive) {
      mgr.ActorModelParticles()->LightDudeOnFire(*this);

      const CDamageInfo fireDmg =
          CDamageInfo(CWeaponMode::Plasma(), x3ec_pendingFireDamage, 0.f, 0.f);
      mgr.ApplyDamage(
          kInvalidUniqueId, GetUniqueId(), kInvalidUniqueId, fireDmg.MakeScaledForTime(dt),
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
          CVector3f::Zero());
    }
  } else {
    if (x3ec_pendingFireDamage > 0.f) {
      x3ec_pendingFireDamage = 0.f;
    }

    if (x450_bodyController->IsFrozen()) {
      mgr.ActorModelParticles()->StopFire(*this);
    }
  }

  if (x401_27_phazingOut || x401_28_burning) {
    x3e8_alphaDelta = -0.33333334f;
  }

  if (x401_30_pendingDeath) {
    x401_30_pendingDeath = false;
    Death(mgr, GetTransform().GetForward(), kSS_DeathRattle);
  }

  float thinkDt;
  if (x400_25_alive) {
    thinkDt = dt;
  } else {
    thinkDt = dt * CalcDyingThinkRate();
  }

  x450_bodyController->Update(thinkDt, mgr);
  x450_bodyController->MultiplyPlaybackRate(x3b4_speed);

  reinterpret_cast< CAdvancementDeltas& >(x434_posDelta) =
      UpdateAnimation(thinkDt, mgr, !x450_bodyController->IsFrozen());

  if (x403_25_enableStateMachine && x450_bodyController->GetPercentageFrozen() < 1.f) {
    x330_stateMachineState.Update(mgr, *this, thinkDt);
  }

  ThinkAboutMove(thinkDt);

  x460_knockBackController.Update(thinkDt, mgr, *this);

  x4e4_latestPredictedTranslation = GetTranslation() + PredictMotion(thinkDt).GetTranslation();

  x328_26_solidCollision = false;

  if (x420_curDamageRemTime > 0.f) {
    x420_curDamageRemTime -= dt;
  }

  if (x401_28_burning && x3f4_burnThinkRateTimer > dt) {
    x3f4_burnThinkRateTimer -= dt;
  }

  SetDamageMag(x50c_baseDamageMag);
  UpdateDamageColor(dt);

  if (!x450_bodyController->IsFrozen()) {
    if (x3a0_latestLeashPosition == CVector3f::Zero()) {
      x3a0_latestLeashPosition = GetTranslation();
    }

    float playerLeashRadius = x3cc_playerLeashRadius;
    if (playerLeashRadius) {
      if ((GetTranslation() - mgr.Player()->GetTranslation()).MagSquared() >
          playerLeashRadius * playerLeashRadius) {
        x3d4_curPlayerLeashTime += dt;
      } else {
        x3d4_curPlayerLeashTime = 0.f;
      }
    } else {
      x3d4_curPlayerLeashTime = 0.f;
    }
  } else {
    RemoveEmitter();
  }

  float rem = x2f8_waypointPauseRemTime;
  if (rem > 0.f) {
    rem -= dt;
  }
  x2f8_waypointPauseRemTime = rem;
}

void CPatterned::Touch(CActor& act, CStateManager& mgr) {
  if (x400_25_alive) {
    CGameProjectile* proj = TCastToPtr< CGameProjectile >(act);
    if (proj != 0 && proj->GetOwnerId() == mgr.Player()->GetUniqueId()) {
      x400_24_hitByPlayerProjectile = true;
    }
  }
}

void CPatterned::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                              CStateManager& mgr) {
  if (x420_curDamageRemTime <= 0.f) {
    CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(id));
    if (player != NULL) {
      bool jumpOnHead = player->GetTimeSinceJump() < 5.f && list.GetCount() != 0 &&
                        list[0].GetNormalLeft().GetZ() > 0.707f;

      if (x400_25_alive || jumpOnHead) {
        CDamageInfo cDamage = GetContactDamage();
        if (!x400_25_alive || x450_bodyController->IsFrozen()) {
          cDamage.SetDamage(0.f);
        }

        if (jumpOnHead) {
          mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetUniqueId(), cDamage,
                          CMaterialFilter::skPassEverything, -player->GetVelocityWR());
          player->SetTimeSinceJump(1000.f);
        } else if (x400_25_alive && !x450_bodyController->IsFrozen()) {
          mgr.ApplyDamage(
              GetUniqueId(), player->GetUniqueId(), GetUniqueId(), cDamage,
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              CVector3f::Zero());
        }

        x420_curDamageRemTime = x424_damageWaitTime;
      }
    }
  }

  static CMaterialList skSolidTypes(kMT_Solid, kMT_Ceiling, kMT_Wall, kMT_Floor, kMT_Character);

  const int listCount = list.GetCount();
  const CCollisionInfo* infos = &list[0];

  for (int i = 0; i < listCount; ++i) {
    const CCollisionInfo& info = infos[i];
    if (info.GetMaterialLeft().SharesMaterials(skSolidTypes)) {
      if (info.GetMaterialLeft().HasMaterial(kMT_Floor)) {
        if (!x400_31_isFlyer) {
          continue;
        }
      } else {
        if (x310_moveVec.IsNonZero() && CVector3f::Dot(info.GetNormalLeft(), x310_moveVec) >= 0.f) {
          continue;
        }
      }

      x328_26_solidCollision = true;
      return;
    }
  }

  CPhysicsActor::CollidedWith(id, list, mgr);
}

void CPatterned::ThinkAboutMove(float dt) {
  bool doMove = true;
  if (!x328_25_verticalMovement && !x328_27_onGround) {
    doMove = false;
    x310_moveVec = CVector3f::Zero();
  }

  if (doMove && x39c_curPattern < x38c_patterns.size()) {
    const CVector3f forward = GetTransform().GetForward();
    CVector3f faceVec = x31c_faceVec;
    if (faceVec.MagSquared() > 0.1f) {
      faceVec.Normalize();
    }

    const float faceDot = CVector3f::Dot(forward, faceVec);
    switch (x3f8_moveState) {
    case kMS_Zero:
      if (!x328_26_solidCollision) {
        break;
      }
    case kMS_One:
      doMove = false;
      if (faceDot > 0.85f) {
        doMove = true;
        x3f8_moveState = kMS_Two;
      } else {
        x3f8_moveState = kMS_One;
      }
      break;
    case kMS_Two:
      x3f8_moveState = kMS_Three;
    case kMS_Three:
      doMove = true;
      if (!x328_26_solidCollision) {
        x3f8_moveState = kMS_Zero;
      } else if (faceDot > 0.9f) {
        x3f8_moveState = kMS_Four;
      }
      break;
    case kMS_Four:
      x328_24_inPosition = true;
      doMove = false;
      x3f8_moveState = kMS_Zero;
      break;
    default:
      break;
    }
  }

  if (!x401_26_disableMove && doMove) {
    if (x450_bodyController->GetBodyStateInfo().GetCurrentState()->ApplyAnimationDeltas() &&
        !close_enough(x2e0_destPos - GetTranslation(), CVector3f::Zero())) {
      const CVector3f& scale = CVector3f(GetModelData()->GetScale());
      const float& sx = scale[kDX];
      const float& sy = scale[kDY];
      const float& sz = scale[kDZ];
      const CVector3f scaledDelta(sx * x434_posDelta.GetX() * x55c_moveScale.GetX(),
                                  sy * x434_posDelta.GetY() * x55c_moveScale.GetY(),
                                  sz * x434_posDelta.GetZ() * x55c_moveScale.GetZ());
      MoveToOR(scaledDelta, dt);
    }
  }

  RotateToOR(x440_rotDelta, dt);
}

void CPatterned::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                                 float dt) {
  switch (type) {
  case kUE_Projectile: {
    const CTransform4f lctrXf = GetLctrTransform(node.GetLocatorName());
    const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    const CVector3f forward = lctrXf.GetForward();

    if (CVector3f::Dot(forward, (aimPos - lctrXf.GetTranslation()).AsNormalized()) > 0.f) {
      const CTransform4f lookAtXf = CTransform4f::LookAt(lctrXf.GetTranslation(), aimPos);
      LaunchProjectile(lookAtXf, mgr, 1, CWeapon::kPA_None, false,
                       rstl::optional_object< TLockedToken< CGenDescription > >(),
                       CSfxManager::kInternalInvalidSfxId, false, CVector3f(1.f, 1.f, 1.f));
    } else {
      LaunchProjectile(lctrXf, mgr, 1, CWeapon::kPA_None, false,
                       rstl::optional_object< TLockedToken< CGenDescription > >(),
                       CSfxManager::kInternalInvalidSfxId, false, CVector3f(1.f, 1.f, 1.f));
    }
    break;
  }
  case kUE_DamageOn: {
    const CVector3f scale = GetModelData()->GetScale();
    const CTransform4f& lctrXf = GetLocatorTransform(node.GetLocatorName());
    CVector3f xfOrigin = CVector3f::ByElementMultiply(scale, lctrXf.GetTranslation());
    xfOrigin = GetTransform() * xfOrigin;
    const CVector3f margin = CVector3f::ByElementMultiply(scale, CVector3f(1.f, 1.f, 0.5f));
    const CAABox touchBounds(xfOrigin - margin, xfOrigin + margin);

    if (touchBounds.DoBoundsOverlap(mgr.GetPlayer()->GetBoundingBox())) {
      mgr.ApplyDamage(
          GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), GetContactDamage(),
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
          CVector3f::Zero());
    }
    break;
  }
  case kUE_Delete:
    if (!x400_25_alive) {
      if (!x400_27_fadeToDeath) {
        x3e8_alphaDelta = -1.f / 3.f;
        x400_27_fadeToDeath = true;
      }
      RemoveMaterial(kMT_Character, kMT_Solid, kMT_Target, kMT_Orbit, mgr);
      AddMaterial(kMT_ProjectilePassthrough, mgr);
    } else {
      DeathDelete(mgr);
    }
    break;
  case kUE_BreakLockOn:
    RemoveMaterial(kMT_Character, kMT_Target, kMT_Orbit, mgr);
    break;
  case kUE_BecomeShootThrough:
    AddMaterial(kMT_ProjectilePassthrough, mgr);
    break;
  case kUE_RemoveCollision:
    RemoveMaterial(kMT_Solid, mgr);
    break;
  default:
    break;
  }

  CActor::DoUserAnimEvent(mgr, node, type, dt);
}

void CPatterned::Burn(float duration, float damage) {
  switch (static_cast< const CPatterned* >(this)->GetDamageVulnerability()->GetVulnerability(
      CWeaponMode::Plasma(), CDamageVulnerability::kRD_No)) {
  case kVN_Weak:
    x450_bodyController->SetOnFire(1.5f * duration);
    x3ec_pendingFireDamage = 1.5f * damage;
    break;
  case kVN_Normal:
    x450_bodyController->SetOnFire(duration);
    x3ec_pendingFireDamage = damage;
    break;
  default:
    break;
  }
}

void CPatterned::Shock(CStateManager&, float duration, float damage) {
  switch (static_cast< const CPatterned* >(this)->GetDamageVulnerability()->GetVulnerability(
      CWeaponMode::Wave(), CDamageVulnerability::kRD_No)) {
  case kVN_Weak:
    x450_bodyController->SetElectrocuting(1.5f * duration);
    x3f0_pendingShockDamage = 1.5f * damage;
    break;
  case kVN_Normal:
    x450_bodyController->SetElectrocuting(duration);
    x3f0_pendingShockDamage = damage;
    break;
  default:
    break;
  }
}

void CPatterned::Freeze(CStateManager& mgr, const CVector3f& pos, CUnitVector3f dir,
                        float frozenDur) {
  if (x402_25_lostMassiveFrozenHP) {
    x402_26_dieIf80PercFrozen = true;
  }

  bool playSfx = false;
  if (x450_bodyController->IsFrozen()) {
    x450_bodyController->Freeze(x460_knockBackController.GetActiveParms().xc_intoFreezeDur,
                                frozenDur, x4f8_outofFreezeDur);
    mgr.ActorModelParticles()->DoIcePop(*this);
    playSfx = true;
  } else if (!x450_bodyController->IsElectrocuting() && !x450_bodyController->IsOnFire()) {
    x450_bodyController->Freeze(x4f4_intoFreezeDur, frozenDur, x4f8_outofFreezeDur);
    if (x510_vertexMorph) {
      x510_vertexMorph->Reset(dir, pos, x4f4_intoFreezeDur);
    }
    playSfx = true;
  }

  if (playSfx) {
    const CVector3f& posOut = GetTranslation();
    CSfxManager::AddEmitter(
        x460_knockBackController.GetCreatureSize() != kCS_Small &&
                PATTERNED_CAST_TO(CMetroid, const_cast< CEntity* >(mgr.GetObjectById(GetUniqueId()))) != nullptr
            ? (SND_FXID)0x701
            : (SND_FXID)0x708,
        posOut, CVector3f::Zero(), true, false, CSfxManager::kMedPriority, CSfxManager::kAllAreas);
  }
}

void CPatterned::PhazeOut(CStateManager& mgr) {
  if (!x400_27_fadeToDeath) {
    x400_27_fadeToDeath = true;
    SendScriptMsgs(kSS_DeathRattle, mgr, kSM_None);
  }

  x401_27_phazingOut = true;
  x450_bodyController->SetPlaybackRate(0.f);

  ModelData()->AnimationData()->GetParticleDB().SetUpdatesEnabled(false);
}

float CPatterned::CalcDyingThinkRate() {
  float thinkRate;
  if (x401_28_burning) {
    thinkRate = x3f4_burnThinkRateTimer / 1.5f;
  } else {
    thinkRate = 1.f;
  }

  const float minThinkRate = 0.1f;
  thinkRate = CMath::Max(minThinkRate, thinkRate);
  return thinkRate;
}

CDamageInfo CPatterned::GetContactDamage() const { return x404_contactDamage; }

void CPatterned::DeathDelete(CStateManager& mgr) {
  SendScriptMsgs(kSS_Dead, mgr, kSM_None);

  if (x450_bodyController->IsElectrocuting()) {
    x3f0_pendingShockDamage = 0.f;
    x450_bodyController->StopElectrocution();
    mgr.ActorModelParticles()->StopElectric(*this);
  }

  mgr.DeleteObjectRequest(GetUniqueId());
}

CTransform4f CPatterned::GetLctrTransform(const rstl::string& name) const {
  return GetTransform() * GetScaledLocatorTransform(name);
}

CTransform4f CPatterned::GetLctrTransform(const CSegId& id) const {
  const CAnimData* animData = GetModelData()->GetAnimationData();
  const CTransform4f locatorXf = animData->GetLocatorTransform(id, 0);
  const CVector3f scaledOrigin =
      CVector3f::ByElementMultiply(GetModelData()->GetScale(), locatorXf.GetTranslation());
  return GetTransform() * CTransform4f(locatorXf.BuildMatrix3f(), scaledOrigin);
}

CVector3f CPatterned::GetAimPosition(const CStateManager&, float dt) const {
  CVector3f offset = CVector3f::Zero();
  if (dt > 0.f) {
    offset = PredictMotion(dt).GetTranslation();
  }

  const CAnimData* animData = GetModelData()->GetAnimationData();
  const CSegId segId = animData->GetLocatorSegId(rstl::string_l("lockon_target_LCTR"));

  if (segId.val() != 0xff) {
    const CTransform4f locatorXf = animData->GetLocatorTransform(segId, 0);
    const CVector3f scale = GetModelData()->GetScale();
    const CVector3f scaledOrigin = CVector3f::ByElementMultiply(scale, locatorXf.GetTranslation());

    if (GetTouchBounds()) {
      offset += GetTouchBounds()->ClampToBox(GetTransform() * scaledOrigin);
    } else {
      const CAABox& baseBox = GetBaseBoundingBox();
      const CAABox primBox(baseBox.GetMinPoint() + GetPrimitiveOffset(),
                           baseBox.GetMaxPoint() + GetPrimitiveOffset());
      offset += GetTransform() * primBox.ClampToBox(scaledOrigin);
    }
  } else {
    offset += GetBoundingBox().GetCenterPoint();
  }

  return offset;
}

CVector3f CPatterned::GetOrbitPosition(const CStateManager& mgr) const {
  return GetAimPosition(mgr, 0.f);
}

void CPatterned::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Thermal) {
    SetCalculateLighting(false);
    ActorLights()->BuildConstantAmbientLighting(CColor::White());
  } else {
    SetCalculateLighting(true);
  }

  CColor color = x42c_color;
  uchar alpha = GetModelAlphau8(mgr);
  if (x402_27_noXrayModel) {
    const bool xray = mgr.GetPlayerState()->IsXRayActive(mgr);
    if (xray) {
      alpha = 76;
    }
  }

  if (alpha < 255) {
    if (color.GetRedu8() == 0 && color.GetGreenu8() == 0 && color.GetBlueu8() == 0) {
      color = CColor::White();
    }

    if (x401_29_laggedBurnDeath) {
      // TODO: maybe this is CModelFlags::ColorModulate?
      uchar stripedAlpha = alpha > 127 ? (alpha - 128) * 2 : 255;
      SetModelFlags(
          CModelFlags(CModelFlags::kT_Three,
                      CColor(skDisintegrateColor.GetRedu8(), skDisintegrateColor.GetGreenu8(),
                             skDisintegrateColor.GetBlueu8(), (stripedAlpha * stripedAlpha) >> 8)));
    } else if (x401_28_burning) {
      SetModelFlags(CModelFlags::AlphaBlended(CColor(static_cast< uchar >(0), 0, 0, 255)));
    } else {
      SetModelFlags(CModelFlags::AlphaBlended(
          CColor(color.GetRedu8(), color.GetGreenu8(), color.GetBlueu8(), alpha)));
    }
  } else if (color.GetRedu8() != 0 || color.GetGreenu8() != 0 || color.GetBlueu8() != 0) {
    SetModelFlags(CModelFlags(
        CModelFlags::kT_Two, CColor(color.GetRedu8(), color.GetGreenu8(), color.GetBlueu8(), 255)));
  } else {
    SetModelFlags(CModelFlags::Normal());
  }

  CActor::PreRender(mgr, frustum);
}

bool CPatterned::CanRenderUnsorted(const CStateManager& mgr) const {
  if (GetModelData()->GetAnimationData()->GetParticleDB().AreAnySystemsDrawnWithModel()) {
    return false;
  }

  return CActor::CanRenderUnsorted(mgr);
}

void CPatterned::Render(const CStateManager& mgr) const {
  const bool drawParticles = x402_29_drawParticles;
  int mask = 0;
  int target = 0;
  if (drawParticles) {
    const_cast< CStateManager& >(mgr).GetCharacterRenderMaskAndTarget(x402_31_thawed, mask, target);
    GetModelData()->GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnFirstMasked(mask,
                                                                                          target);
  }

  const EThermalDrawFlag thermalFlag = mgr.GetThermalDrawFlag();
  if ((thermalFlag == kTD_Cold && !x402_31_thawed) || (thermalFlag == kTD_Hot && x402_31_thawed) ||
      thermalFlag == kTD_Bypass) {
    if (x401_28_burning) {
      CTexture* ashyTexture =
          const_cast< CStateManager& >(mgr).ActorModelParticles()->GetAshyTexture(
              *const_cast< CPatterned* >(this));
      const uchar alpha = GetModelAlphau8(mgr);
      if (ashyTexture != 0 && ((!x401_29_laggedBurnDeath && alpha <= 0xff) || alpha <= 0x7f)) {
        if (GetPointGeneratorParticles()) {
          mgr.SetupParticleHook(*this);
        }

        const CColor* disintegrateColor = nullptr;
        if (x401_29_laggedBurnDeath) {
          disintegrateColor = &skDisintegrateColor;
        } else if (mgr.GetThermalDrawFlag() == kTD_Hot) {
          disintegrateColor = &CColor::White();
        } else {
          disintegrateColor = &CColor::Black();
        }

        const CColor disColor = *disintegrateColor;
        const float t =
            (x401_29_laggedBurnDeath ? 0.0078740157f : 0.0039215689f) * CCast::ToReal32(alpha);
        GetModelData()->DisintegrateDraw(mgr, GetTransform(), *ashyTexture, disColor, t);

        if (GetPointGeneratorParticles()) {
          CSkinnedModel::ClearPointGeneratorFunc();
          mgr.GetActorModelParticles()->Render(mgr, *this);
        }
      } else {
        CPhysicsActor::Render(mgr);
      }
    } else {
      CPhysicsActor::Render(mgr);
    }

    if (x450_bodyController->IsFrozen() && !x401_28_burning) {
      CModelFlags flags(CModelFlags::kT_Opaque, 1.f);
      RenderIceModelWithFlags(flags);
    }
  }

  if (drawParticles) {
    GetModelData()->GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnLastMasked(mask,
                                                                                         target);
  }
}

void CPatterned::RenderIceModelWithFlags(const CModelFlags& flags) const {
  const CAnimData* animData = GetModelData()->GetAnimationData();
  CModelFlags useFlags = flags.UseShaderSet(0);
  const rstl::optional_object< TLockedToken< CSkinnedModelWithAvgNormals > >& iceModel =
      animData->GetIceModel();

  if (iceModel.valid()) {
    const float* avgNormals = (**iceModel)->GetAvgNormals();
    const CSkinnedModelWithAvgNormals* model = **iceModel;
    animData->Render(model->GetSkinnedModel(), useFlags, *x510_vertexMorph, avgNormals);
  }
}

CEnergyProjectile* CPatterned::LaunchProjectile(
    const CTransform4f& xf, CStateManager& mgr, const int maxAllowed,
    const CWeapon::EProjectileAttrib attrib, const bool playerHoming,
    const rstl::optional_object< TLockedToken< CGenDescription > >& visorParticle,
    const ushort visorSfx, const bool sendCollideMsg, const CVector3f& scale) {
  CEnergyProjectile* projectile = 0;
  CProjectileInfo* projectileInfo = ProjectileInfo();
  if (projectileInfo->Token().TryCache()) {
    if (mgr.CanCreateProjectile(GetUniqueId(), kWT_AI, maxAllowed)) {
      projectile = rs_new CEnergyProjectile(
          true, ProjectileInfo()->Token(), kWT_AI, xf, kMT_Character, ProjectileInfo()->GetDamage(),
          mgr.AllocateUniqueId(), GetCurrentAreaId(), GetUniqueId(),
          playerHoming ? mgr.GetPlayer()->GetUniqueId() : kInvalidUniqueId, attrib, false, scale,
          visorParticle, visorSfx, sendCollideMsg);

      if (projectile != 0) {
        mgr.AddObject(projectile);
      }
    }
  }

  return projectile;
}

EWeaponCollisionResponseTypes CPatterned::GetCollisionResponseType(const CVector3f& pos,
                                                                   const CVector3f& dir,
                                                                   const CWeaponMode& mode,
                                                                   int attrib) const {
  if (GetBodyCtrl()->IsFrozen() && mode.GetType() == kWT_Ice) {
    return kWCR_None;
  }

  return CAi::GetCollisionResponseType(pos, dir, mode, attrib);
}

void CPatterned::PreThink(float dt, CStateManager& mgr) {
  x500_preThinkDt = dt;
  CEntity::PreThink(dt, mgr);
}

void CPatterned::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  if (x402_29_drawParticles) {
    if (HasModelData()) {
      int mask;
      int target;
      const_cast< CStateManager& >(mgr).GetCharacterRenderMaskAndTarget(x402_31_thawed, mask,
                                                                        target);
      const CAnimData* animData = GetModelData()->GetAnimationData();
      if (animData != 0) {
        animData->GetParticleDB().AddToRendererClippedMasked(frustum, mask, target);
      }
    }
  }

  CActor::AddToRenderer(frustum, mgr);
}

void CPatterned::MakeThermalColdAndHot() {
  x403_24_keepThermalVisorState = true;
  SetThermalFlags(static_cast< EThermalFlags >(3));
}

void CPatterned::UpdateThermalFrozenState(const bool thawed) {
  x402_31_thawed = thawed;
  if (x403_24_keepThermalVisorState) {
    return;
  }

  SetThermalFlags(static_cast< EThermalFlags >(thawed ? kTF_Hot : kTF_Cold));
}

CVector3f CAi::GetOrigin(const CStateManager&, const CTeamAiRole&, const CVector3f&) const {
  return GetTranslation();
}

bool CAi::IsListening() const { return false; }

rstl::optional_object< CAABox > CPatterned::GetTouchBounds() const { return GetBoundingBox(); }

void CPatterned::TakeDamage(const CVector3f&, float) { x428_damageCooldownTimer = 0.33f; }

bool CPatterned::Default(CStateManager&, float) { return true; }
