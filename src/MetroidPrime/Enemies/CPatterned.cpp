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
: CAi(uid, pinfo.mActive, name, info, xf, mData,
      CAABox(-pinfo.mHalfExtent + pinfo.mBodyOrigin.GetX(),
             -pinfo.mHalfExtent + pinfo.mBodyOrigin.GetY(), pinfo.mBodyOrigin.GetZ(),
             pinfo.mHalfExtent + pinfo.mBodyOrigin.GetX(),
             pinfo.mHalfExtent + pinfo.mBodyOrigin.GetY(),
             pinfo.mBodyOrigin.GetZ() + pinfo.mHeight),
      pinfo.mMass, pinfo.mHealthInfo, pinfo.mDamageVulnerability,
      movement == kMT_Flyer ? gkPatternedFlyerMaterialList : gkPatternedGroundMaterialList,
      pinfo.mStateMachineId, params, pinfo.mStepUpHeight, 0.8f)
, mPatrolState(kPS_Invalid)
, mDestObj(kInvalidUniqueId)
, mDestPos(CVector3f::Zero())
, mReflectedDestPos(CVector3f::Zero())
, mWaypointPauseRemTime(0.f)
, mMinAttackRange(pinfo.mMinAttackRange)
, mMaxAttackRange(pinfo.mMaxAttackRange)
, mAverageAttackTime(pinfo.mAverageAttackTime)
, mAttackTimeVariation(pinfo.mAttackTimeVariation)
, mBehaviourOrient(kBO_MoveDir)
, mMoveVec(CVector3f::Zero())
, mFaceVec(CVector3f::Zero())
, mInPosition(false)
, mVerticalMovement(movement == kMT_Flyer)
, mSolidCollision(false)
, mOnGround(movement != kMT_Flyer)
, mPrevOnGround(true)
, mNoPatternShagging(false)
, mLookAtDeathDir(true)
, mEnergyAttractor(false)
, x329_24_(true)
, mAnimState(kAS_NotReady)
, mStateMachineState()
, mCharacterType(character)
, mPatternStartPos(CVector3f::Zero())
, mPatternStartPlayerPos(CVector3f::Zero())
, mDestWPDelta(CVector3f::Zero())
, mPatternTranslate(kPT_RelativeStart)
, mPatternOrient(kPO_ReversePlayerForward)
, mPatternFit(kPF_One)
, mBehaviour(kB_Zero)
, mBehaviourModifiers(kBM_Zero)
, mAnim(pinfo.GetAnimationParameters().GetInitialAnimation())
, mPatterns()
, mCurPattern(0)
, mLatestLeashPosition(CVector3f::Zero())
, mLastPatrolDest(kInvalidUniqueId)
, mMoveSpeed(1.f)
, mSpeed(pinfo.mSpeed)
, mTurnSpeed(pinfo.mTurnSpeed)
, mDetectionRange(pinfo.mDetectionRange)
, mDetectionHeightRange(pinfo.mDetectionHeightRange)
, mDetectionAngle(cosf(0.017453292f * pinfo.mDectectionAngle))
, mLeashRadius(pinfo.mLeashRadius)
, mPlayerLeashRadius(pinfo.mPlayerLeashRadius)
, mPlayerLeashTime(pinfo.mPlayerLeashTime)
, mCurPlayerLeashTime(0.f)
, mXDamageThreshold(pinfo.mXDamage)
, mFrozenXDamageThreshold(pinfo.mFrozenXDamage)
, mXDamageDelay(pinfo.mXDamageDelay)
, mLastHP(0.f)
, mAlphaDelta(0.f)
, mPendingFireDamage(0.f)
, mPendingShockDamage(0.f)
, mBurnThinkRateTimer(0.f)
, mMoveState(kMS_Zero)
, mFlavor(flavor)
, mHitByPlayerProjectile(false)
, mAlive(true)
, x400_26_(false)
, mFadeToDeath(false)
, mPendingMassiveDeath(false)
, mPendingMassiveFrozenDeath(false)
, mPatternShagged(false)
, mIsFlyer(movement == kMT_Flyer)
, mPathOverCount(0)
, mDisableMove(false)
, mPhazingOut(false)
, mBurning(false)
, mLaggedBurnDeath(false)
, mPendingDeath(false)
, mNextPendingShock(false)
, mPendingShock(false)
, mLostMassiveFrozenHP(false)
, mDieIf80PercFrozen(false)
, mNoXrayModel(false)
, mIsMakingBigStrike(false)
, mDrawParticles(true)
, mUpdateThermalFrozenState(params.IsHotInThermal())
, mThawed(mUpdateThermalFrozenState)
, mKeepThermalVisorState(false)
, mEnableStateMachine(true)
, mStateControlledMassiveDeath(true)
, mContactDamage(pinfo.mContactDamageInfo)
, mCurDamageRemTime(0.f)
, mDamageWaitTime(pinfo.mDamageWaitTime)
, mDamageCooldownTimer(-1.f)
, mColor(0.f, 0.f, 0.f, 1.f)
, mDamageColor(skDamageColor)
, mPosDelta(CVector3f::Zero())
, mRotDelta(CQuaternion::NoRotation())
, mBodyController()
, mDeathSfx(pinfo.mDeathSfx)
, mIceShatterSfx(pinfo.mIceShatterSfx)
, mSteeringBehaviors()
, mKnockBackController(kbVariant)
, mLatestPredictedTranslation(CVector3f::Zero())
, mPredictedLeashTime(0.f)
, mIntoFreezeDur(pinfo.mIntoFreezeDur)
, mOutofFreezeDur(pinfo.mOutofFreezeDur)
, mFreezeDur(pinfo.mFreezeDur)
, mPreThinkDt(0.f)
, mDamageDur(0.f)
, mColliderType(collider)
, mBaseDamageMag(params.GetThermalMag())
, mVertexMorph(nullptr)
, mDeathExplosionOffset(pinfo.mParticle1Scale)
, mDeathExplosionParticle()
, mDeathExplosionElectric()
, mIceDeathExplosionOffset(pinfo.mParticle2Scale)
, mIceDeathExplosionParticle()
, mMoveScale(1.f, 1.f, 1.f) {
  if (pinfo.mParticle1 != kInvalidAssetId) {
    mDeathExplosionParticle = gpSimplePool->GetObj(SObjectTag('PART', pinfo.mParticle1));
    mDeathExplosionParticle->Lock();
  }

  if (pinfo.mElectric != kInvalidAssetId) {
    mDeathExplosionElectric = gpSimplePool->GetObj(SObjectTag('ELSC', pinfo.mElectric));
    mDeathExplosionElectric->Lock();
  }

  if (pinfo.mParticle2 != kInvalidAssetId) {
    mIceDeathExplosionParticle = gpSimplePool->GetObj(SObjectTag('PART', pinfo.mParticle2));
    mIceDeathExplosionParticle->Lock();
  }

  if (mContactDamage.GetRadius() > 0.f) {
    mContactDamage.SetRadius(0.f);
  }

  SetRenderParticleDatabaseInside(false);

  if (HasModelData()) {
    mNoXrayModel = !ModelData()->HasModel(CModelData::kWM_XRay);
    BuildBodyController(body);
  }
}

void CPatterned::BuildBodyController(EBodyType bodyType) {
  if (!mBodyController.null()) {
    return;
  }

  mBodyController = rs_new CBodyController(*this, mTurnSpeed, bodyType);

  CPASAnimParmData data(pas::kAS_AdditiveReaction, CPASAnimParm::FromEnum(0));
  const rstl::pair< float, int > bestAnim =
      mBodyController->GetPASDatabase().FindBestAnimation(data, -1);
  mKnockBackController.mEnableShock = bestAnim.first > 0.f;
}

ENTITY_ACCEPT_IMPL(CPatterned)

void CPatterned::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CAi::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case kSM_Registered: {
    if (mColliderType != kCT_One) {
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

      mVertexMorph = rstl::ncrc_ptr< CVertexMorphEffect >(
          rs_new CVertexMorphEffect(CUnitVector3f(CVector3f(1.f, 0.f, 0.f), CUnitVector3f::kN_Yes),
                                    CVector3f(0.f, 0.f, 0.f), 0.f, diagExtent, *mgr.Random()));
    }

    SetAngularEnabled(true);
    break;
  }
  case kSM_OnFloor:
    if (!mVerticalMovement) {
      SetMomentumWR(CVector3f::Zero());
      AddMaterial(kMT_GroundCollider, mgr);
    }
    mOnGround = true;
    break;
  case kSM_Falling:
    if (!mVerticalMovement) {
      if (mBodyController->GetPercentageFrozen() == 0.f) {
        SetMomentumWR(CVector3f(0.f, 0.f, -GetWeight()));
        RemoveMaterial(kMT_GroundCollider, mgr);
      }
    }
    mOnGround = false;
    break;
  case kSM_Activate:
    mLatestLeashPosition = GetTranslation();
    break;
  case kSM_Deleted: {
    CAiState* state = mStateMachineState.GetActorState();
    if (state != 0) {
      state->CallFunc(mgr, *this, kStateMsg_Deactivate, 0.f);
    }
    break;
  }
  case kSM_Damage: {
    const CGameProjectile* proj = TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(uid));
    if (proj != 0) {
      if (proj->GetCurrentDamageInfo().GetWeaponMode().GetType() == kWT_Wave) {
        if (mKnockBackController.mEnableShock &&
            proj->GetCurrentDamageInfo().GetWeaponMode().IsComboed() && HealthInfo(mgr) != 0) {
          mNextPendingShock = true;
          KnockBack(GetTransform().GetForward(), mgr, proj->GetCurrentDamageInfo(),
                    proj->GetCurrentDamageInfo().GetKnockBackPower(), true, false);
          mKnockBackController.DeferKnockBack(kWT_Wave);
        }
      } else if (proj->GetCurrentDamageInfo().GetWeaponMode().GetType() == kWT_Plasma) {
        if (mKnockBackController.mEnableBurn &&
            proj->GetCurrentDamageInfo().GetWeaponMode().IsComboed() && HealthInfo(mgr) != 0) {
          KnockBack(GetTransform().GetForward(), mgr, proj->GetCurrentDamageInfo(),
                    proj->GetCurrentDamageInfo().GetKnockBackPower(), true, false);
          mKnockBackController.DeferKnockBack(kWT_Plasma);
        }
      }

      if (proj->GetOwnerId() == mgr.Player()->GetUniqueId()) {
        mHitByPlayerProjectile = true;
      }
    }
    break;
  }
  case kSM_InvulnDamage: {
    const CGameProjectile* proj = TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(uid));
    if (proj != 0 && proj->GetOwnerId() == mgr.Player()->GetUniqueId()) {
      mHitByPlayerProjectile = true;
    }
    break;
  }
  default:
    break;
  }
}

void CPatterned::SetDestPos(const CVector3f& pos) { mDestPos = pos; }

CVector3f CPatterned::GetGunEyePos() const {
  CVector3f origin = GetTranslation();
  const CAABox& baseBox = GetBaseBoundingBox();
  origin[kDZ] += 0.6f * (baseBox.GetMaxPoint().GetZ() - baseBox.GetMinPoint().GetZ());
  return origin;
}

u8 CPatterned::ApplyBoneTracking() const {
  bool ret = false;
  if (mAlive) {
    ret = mKnockBackController.GetFlinchRemTime() <= 0.f;
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
  if (state == mBodyController->GetBodyStateInfo().GetCurrentStateId()) {
    mAnimState = kAS_Repeat;
  } else if (mAnimState == kAS_Ready) {
    (this->*cb)(mgr, arg);
  } else {
    mAnimState = kAS_Over;
  }
}

void CPatterned::TryProjectileAttack(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = mBodyController->CommandMgr();
  CBCProjectileAttackCmd cmd(static_cast< pas::ESeverity >(arg), mDestPos, false);
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryMeleeAttack(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = mBodyController->CommandMgr();
  CBCMeleeAttackCmd cmd(static_cast< pas::ESeverity >(arg));
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryMeleeAttack_TargetPos(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = mBodyController->CommandMgr();
  CBCMeleeAttackCmd cmd(static_cast< pas::ESeverity >(arg), mDestPos);
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryStep(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = mBodyController->CommandMgr();
  CBCStepCmd cmd(static_cast< pas::EStepDirection >(arg), pas::kStep_Normal);
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryDodge(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = mBodyController->CommandMgr();
  CBCStepCmd cmd(static_cast< pas::EStepDirection >(arg), pas::kStep_Dodge);
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryBreakDodge(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = mBodyController->CommandMgr();
  CBCStepCmd cmd(static_cast< pas::EStepDirection >(arg), pas::kStep_BreakDodge);
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryGenerateDeactivate(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = mBodyController->CommandMgr();
  CBCGenerateCmd cmd(static_cast< pas::EGenerateType >(arg), CVector3f::Zero());
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryGenerate(CStateManager&, int) {
  CBodyStateCmdMgr& cmdMgr = mBodyController->CommandMgr();
  CBCGenerateCmd cmd(pas::kGType_Zero, mDestPos, true);
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryJump(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = mBodyController->CommandMgr();
  CBCJumpCmd cmd(mDestPos, static_cast< pas::EJumpType >(arg), false);
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TrySlide(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = mBodyController->CommandMgr();
  CBCSlideCmd cmd(static_cast< pas::ESlideType >(arg), mDestPos - GetTranslation());
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryTaunt(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = mBodyController->CommandMgr();
  CBCTauntCmd cmd(static_cast< pas::ETauntType >(arg));
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryKnockBack_Front(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = mBodyController->CommandMgr();
  CBCKnockBackCmd cmd(GetTransform().GetForward(), static_cast< pas::ESeverity >(arg));
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryGetUp(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = mBodyController->CommandMgr();
  CBCGetupCmd cmd(static_cast< pas::EGetupType >(arg));
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryTurn(CStateManager&, int) {
  CBodyStateCmdMgr& cmdMgr = mBodyController->CommandMgr();
  CVector3f face = (mDestPos - GetTranslation()).AsNormalized();
  CBCLocomotionCmd cmd(CVector3f::Zero(), face, 1.f);
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryLoopedReaction(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = mBodyController->CommandMgr();
  CBCLoopReactionCmd cmd(static_cast< pas::EReactionType >(arg));
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryLoopedHitReaction(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = mBodyController->CommandMgr();
  CBCLoopHitReactionCmd cmd(static_cast< pas::EReactionType >(arg));
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::TryKnockBack(CStateManager&, int arg) {
  CBodyStateCmdMgr& cmdMgr = mBodyController->CommandMgr();
  CBCKnockBackCmd cmd(GetTransform().GetForward(), static_cast< pas::ESeverity >(arg));
  cmdMgr.DeliverCmd(cmd);
}

void CPatterned::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  if (mAlive) {
    if (!mBodyController->IsOnFire()) {
      mLostMassiveFrozenHP =
          (mLastHP - HealthInfo(mgr)->GetHP()) >= mFrozenXDamageThreshold;
      if (mLostMassiveFrozenHP && mIceDeathExplosionParticle.valid() &&
          mBodyController->GetPercentageFrozen() > 0.8f) {
        mPendingMassiveFrozenDeath = true;
      } else if ((mLastHP - HealthInfo(mgr)->GetHP()) >= mXDamageThreshold) {
        mPendingMassiveDeath = true;
      }
    }

    if (mPendingMassiveDeath || mPendingMassiveFrozenDeath) {
      if (mLookAtDeathDir && mXDamageDelay <= 0.f) {
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
      mStateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("Dead"));
      RemoveMaterial(kMT_GroundCollider, mgr);
      mVerticalMovement = false;
    }

    mAlive = false;
    if (mBodyController->HasBodyState(pas::kAS_Hurled) &&
        mBodyController->GetBodyType() == kBT_Flyer) {
      mBodyController->CommandMgr().DeliverCmd(CBCHurledCmd(-direction, CVector3f::Zero()));
    } else if (mBodyController->HasBodyState(pas::kAS_Fall)) {
      mBodyController->CommandMgr().DeliverCmd(CBCKnockDownCmd(-direction, pas::kS_One));
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
      mDeathExplosionElectric;

  if (deathParticle.valid() || deathElectric.valid()) {
    CTransform4f xf(GetTransform());
    const CVector3f offset =
        CVector3f::ByElementMultiply(GetModelData()->GetScale(), mDeathExplosionOffset);
    xf.SetTranslation(GetTransform() * offset);

    if (deathParticle.valid()) {
      if (CExplosion* explosion = rs_new CExplosion(
              TLockedToken< CGenDescription >(*deathParticle), mgr.AllocateUniqueId(), true,
              CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList), rstl::string_l(""), xf,
              mThawed ? 0 : 1, CVector3f(1.f, 1.f, 1.f), CColor::White())) {
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
      mIceDeathExplosionParticle;
  if (deathParticle.valid()) {
    CTransform4f xf(GetTransform());
    const CVector3f offset =
        CVector3f::ByElementMultiply(GetModelData()->GetScale(), mIceDeathExplosionOffset);
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
  const ushort sfx = mDeathSfx;
  CSfxManager::AddEmitter(sfx, GetTranslation(), CVector3f::Zero(), true, false,
                          CSfxManager::kMedPriority, CSfxManager::kAllAreas);

  if (!mBurning) {
    SendScriptMsgs(kSS_MassiveDeath, mgr, kSM_None);
    GenerateDeathExplosion(mgr);
  }

  DeathDelete(mgr);
  mPendingMassiveDeath = mPendingMassiveFrozenDeath = false;
}

void CPatterned::MassiveFrozenDeath(CStateManager& mgr) {
  if (mIceShatterSfx == CSfxManager::kInternalInvalidSfxId) {
    mIceShatterSfx = mDeathSfx;
  }

  const ushort sfx = mIceShatterSfx;
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
  mPendingMassiveDeath = mPendingMassiveFrozenDeath = false;
}

void CPatterned::KnockBack(const CVector3f& backVec, CStateManager& mgr, const CDamageInfo& info,
                           float magnitude, bool direct, bool) {
  CHealthInfo* health = HealthInfo(mgr);
  if (mPhazingOut || mBurning || health == nullptr) {
    return;
  }

  mKnockBackController.KnockBack(backVec, mgr, *this, info, magnitude, direct);

  if (mBodyController->IsFrozen() &&
      mKnockBackController.GetActiveParms().mIntoFreezeDur >= 0.f) {
    mBodyController->FrozenBreakout();
  }

  switch (mKnockBackController.GetActiveParms().mAnimFollowup) {
  case kKBAFU_Freeze: {
    CVector3f pos(0.f, 0.f, 0.f);
    CUnitVector3f dir = GetTransform().TransposeRotate(backVec);
    float dur = mKnockBackController.GetActiveParms().mFollowupDuration;
    Freeze(mgr, pos, dir, dur);
    break;
  }
  case kKBAFU_PhazeOut:
    PhazeOut(mgr);
    break;
  case kKBAFU_Shock:
    Shock(mgr, mKnockBackController.GetActiveParms().mFollowupDuration, -1.f);
    break;
  case kKBAFU_Burn:
    Burn(mKnockBackController.GetActiveParms().mFollowupDuration, 0.25f);
    break;
  case kKBAFU_LaggedBurnDeath:
    mLaggedBurnDeath = true;
  case kKBAFU_BurnDeath: {
    Burn(mKnockBackController.GetActiveParms().mFollowupDuration, -1.f);
    Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
    mPendingMassiveDeath = mPendingMassiveFrozenDeath = false;
    mFadeToDeath = mBurning = true;
    mBurnThinkRateTimer = 1.5f;
    mDrawParticles = false;
    mBodyController->DouseFlames();
    CActorModelParticles* particles = mgr.ActorModelParticles();
    particles->StopFire(*this);
    particles->StartBurnDeath(*this);
    if (!mLaggedBurnDeath) {
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
    if (GetDeathExplosionParticle().valid() || mDeathExplosionElectric.valid()) {
      MassiveDeath(mgr);
    } else if (mBodyController->IsFrozen()) {
      mBodyController->FrozenBreakout();
    }
    break;
  case kKBAFU_IceDeath:
    Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
    if (mIceDeathExplosionParticle.valid()) {
      MassiveFrozenDeath(mgr);
    } else if (mBodyController->IsFrozen()) {
      mBodyController->FrozenBreakout();
    }
    break;
  default:
    break;
  }
}

void CPatterned::UpdateAlphaDelta(float dt, CStateManager& mgr) {
  if (mAlphaDelta == 0.f) {
    return;
  }

  float alpha = dt * mAlphaDelta + mColor.GetAlpha();
  if (alpha > 1.f) {
    alpha = 1.f;
    mAlphaDelta = 0.f;
  } else if (alpha < 0.f) {
    alpha = 0.f;
    mAlphaDelta = 0.f;
    if (mFadeToDeath) {
      DeathDelete(mgr);
    }
  }

  Shadow()->SetUserAlpha(alpha);
  mColor.SetAlpha(alpha);
  ModelData()->AnimationData()->GetParticleDB().SetModulationColorAllActiveEffects(
      CColor(1.f, 1.f, 1.f, alpha));
}

void CPatterned::UpdateDamageColor(float dt) {
  if (mDamageCooldownTimer > 0.f) {
    mDamageCooldownTimer = CMath::Max(0.f, mDamageCooldownTimer - dt);
    const float t = CMath::Min(mDamageCooldownTimer / 0.33f, 1.f);
    const CColor& color = CColor::Lerp(CColor::Black(), mDamageColor, t);
    mColor.Set(color.GetRedu8(), color.GetGreenu8(), color.GetBlueu8(),
                   mColor.GetAlphau8());

    if (!mBodyController->IsFrozen()) {
      SetDamageMag(mBaseDamageMag + mDamageCooldownTimer);
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

  if (mUpdateThermalFrozenState) {
    UpdateThermalFrozenState(mBodyController->GetPercentageFrozen() == 0.f);
  }

  const CAnimData* animData = GetModelData()->GetAnimationData();
  const rstl::optional_object< TLockedToken< CSkinnedModelWithAvgNormals > >& iceModel =
      animData->GetIceModel();
  if (iceModel.valid()) {
    mVertexMorph->Update(dt);
  }

  if (mDieIf80PercFrozen && mBodyController->GetPercentageFrozen() > 0.8f) {
    mPendingMassiveFrozenDeath = true;
  }

  if (!mAlive) {
    if ((mPendingMassiveDeath || mPendingMassiveFrozenDeath) &&
        mXDamageDelay <= 0.f) {
      if (mPendingMassiveFrozenDeath) {
        SendScriptMsgs(kSS_AboutToMassivelyDie, mgr, kSM_None);
        MassiveFrozenDeath(mgr);
      } else {
        SendScriptMsgs(kSS_AboutToMassivelyDie, mgr, kSM_None);
        MassiveDeath(mgr);
      }
      return;
    }

    mXDamageDelay -= dt;

    if (mStateControlledMassiveDeath) {
      if (mStateMachineState.GetName() != 0) {
        const bool isDead = mStateMachineState.GetName() == rstl::string_l("Dead");
        if (isDead && mStateMachineState.GetTime() > 15.f) {
          MassiveDeath(mgr);
        }
      }
    }
  }

  UpdateAlphaDelta(dt, mgr);

  mLastHP = HealthInfo(mgr)->GetHP();
  if (mStateMachineState.GetActorState() == 0) {
    mStateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("Start"));
  }

  CVector3f diffVec = mLatestPredictedTranslation - GetTranslation();
  if (!mVerticalMovement) {
    diffVec.SetZ(0.f);
  }

  if (diffVec.MagSquared() > (0.1f * dt)) {
    mPredictedLeashTime += dt;
  } else {
    mPredictedLeashTime = 0.f;
  }

  if (mKnockBackController.mEnableShock) {
    if (!mNextPendingShock && mPendingShock) {
      Shock(mgr, 0.5f + mgr.Random()->Range(0.f, 0.5f), 0.2f);
    }

    mPendingShock = mNextPendingShock;
    mNextPendingShock = false;

    if (mBodyController->IsElectrocuting()) {
      mgr.ActorModelParticles()->StartElectric(*this);

      if (mPendingShockDamage > 0.f && mAlive) {
        const CDamageInfo shockDmg =
            CDamageInfo(CWeaponMode::Wave(), mPendingShockDamage, 0.f, 0.f);
        mgr.ApplyDamage(
            kInvalidUniqueId, GetUniqueId(), kInvalidUniqueId, shockDmg.MakeScaledForTime(dt),
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
      }
    } else if (mPendingShockDamage != 0.f) {
      mPendingShockDamage = 0.f;
      mBodyController->StopElectrocution();
      mgr.ActorModelParticles()->StopElectric(*this);
    }
  }

  if (mBodyController->IsOnFire()) {
    if (mAlive) {
      mgr.ActorModelParticles()->LightDudeOnFire(*this);

      const CDamageInfo fireDmg =
          CDamageInfo(CWeaponMode::Plasma(), mPendingFireDamage, 0.f, 0.f);
      mgr.ApplyDamage(
          kInvalidUniqueId, GetUniqueId(), kInvalidUniqueId, fireDmg.MakeScaledForTime(dt),
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
          CVector3f::Zero());
    }
  } else {
    if (mPendingFireDamage > 0.f) {
      mPendingFireDamage = 0.f;
    }

    if (mBodyController->IsFrozen()) {
      mgr.ActorModelParticles()->StopFire(*this);
    }
  }

  if (mPhazingOut || mBurning) {
    mAlphaDelta = -0.33333334f;
  }

  if (mPendingDeath) {
    mPendingDeath = false;
    Death(mgr, GetTransform().GetForward(), kSS_DeathRattle);
  }

  float thinkDt;
  if (mAlive) {
    thinkDt = dt;
  } else {
    thinkDt = dt * CalcDyingThinkRate();
  }

  mBodyController->Update(thinkDt, mgr);
  mBodyController->MultiplyPlaybackRate(mSpeed);

  reinterpret_cast< CAdvancementDeltas& >(mPosDelta) =
      UpdateAnimation(thinkDt, mgr, !mBodyController->IsFrozen());

  if (mEnableStateMachine && mBodyController->GetPercentageFrozen() < 1.f) {
    mStateMachineState.Update(mgr, *this, thinkDt);
  }

  ThinkAboutMove(thinkDt);

  mKnockBackController.Update(thinkDt, mgr, *this);

  mLatestPredictedTranslation = GetTranslation() + PredictMotion(thinkDt).GetTranslation();

  mSolidCollision = false;

  if (mCurDamageRemTime > 0.f) {
    mCurDamageRemTime -= dt;
  }

  if (mBurning && mBurnThinkRateTimer > dt) {
    mBurnThinkRateTimer -= dt;
  }

  SetDamageMag(mBaseDamageMag);
  UpdateDamageColor(dt);

  if (!mBodyController->IsFrozen()) {
    if (mLatestLeashPosition == CVector3f::Zero()) {
      mLatestLeashPosition = GetTranslation();
    }

    float playerLeashRadius = mPlayerLeashRadius;
    if (playerLeashRadius) {
      if ((GetTranslation() - mgr.Player()->GetTranslation()).MagSquared() >
          playerLeashRadius * playerLeashRadius) {
        mCurPlayerLeashTime += dt;
      } else {
        mCurPlayerLeashTime = 0.f;
      }
    } else {
      mCurPlayerLeashTime = 0.f;
    }
  } else {
    RemoveEmitter();
  }

  float rem = mWaypointPauseRemTime;
  if (rem > 0.f) {
    rem -= dt;
  }
  mWaypointPauseRemTime = rem;
}

void CPatterned::Touch(CActor& act, CStateManager& mgr) {
  if (mAlive) {
    CGameProjectile* proj = TCastToPtr< CGameProjectile >(act);
    if (proj != 0 && proj->GetOwnerId() == mgr.Player()->GetUniqueId()) {
      mHitByPlayerProjectile = true;
    }
  }
}

void CPatterned::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                              CStateManager& mgr) {
  if (mCurDamageRemTime <= 0.f) {
    CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(id));
    if (player != NULL) {
      bool jumpOnHead = player->GetTimeSinceJump() < 5.f && list.GetCount() != 0 &&
                        list[0].GetNormalLeft().GetZ() > 0.707f;

      if (mAlive || jumpOnHead) {
        CDamageInfo cDamage = GetContactDamage();
        if (!mAlive || mBodyController->IsFrozen()) {
          cDamage.SetDamage(0.f);
        }

        if (jumpOnHead) {
          mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetUniqueId(), cDamage,
                          CMaterialFilter::skPassEverything, -player->GetVelocityWR());
          player->SetTimeSinceJump(1000.f);
        } else if (mAlive && !mBodyController->IsFrozen()) {
          mgr.ApplyDamage(
              GetUniqueId(), player->GetUniqueId(), GetUniqueId(), cDamage,
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              CVector3f::Zero());
        }

        mCurDamageRemTime = mDamageWaitTime;
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
        if (!mIsFlyer) {
          continue;
        }
      } else {
        if (mMoveVec.IsNonZero() && CVector3f::Dot(info.GetNormalLeft(), mMoveVec) >= 0.f) {
          continue;
        }
      }

      mSolidCollision = true;
      return;
    }
  }

  CPhysicsActor::CollidedWith(id, list, mgr);
}

void CPatterned::ThinkAboutMove(float dt) {
  bool doMove = true;
  if (!mVerticalMovement && !mOnGround) {
    doMove = false;
    mMoveVec = CVector3f::Zero();
  }

  if (doMove && mCurPattern < mPatterns.size()) {
    const CVector3f forward = GetTransform().GetForward();
    CVector3f faceVec = mFaceVec;
    if (faceVec.MagSquared() > 0.1f) {
      faceVec.Normalize();
    }

    const float faceDot = CVector3f::Dot(forward, faceVec);
    switch (mMoveState) {
    case kMS_Zero:
      if (!mSolidCollision) {
        break;
      }
    case kMS_One:
      doMove = false;
      if (faceDot > 0.85f) {
        doMove = true;
        mMoveState = kMS_Two;
      } else {
        mMoveState = kMS_One;
      }
      break;
    case kMS_Two:
      mMoveState = kMS_Three;
    case kMS_Three:
      doMove = true;
      if (!mSolidCollision) {
        mMoveState = kMS_Zero;
      } else if (faceDot > 0.9f) {
        mMoveState = kMS_Four;
      }
      break;
    case kMS_Four:
      mInPosition = true;
      doMove = false;
      mMoveState = kMS_Zero;
      break;
    default:
      break;
    }
  }

  if (!mDisableMove && doMove) {
    if (mBodyController->GetBodyStateInfo().GetCurrentState()->ApplyAnimationDeltas() &&
        !close_enough(mDestPos - GetTranslation(), CVector3f::Zero())) {
      const CVector3f& scale = CVector3f(GetModelData()->GetScale());
      const float& sx = scale[kDX];
      const float& sy = scale[kDY];
      const float& sz = scale[kDZ];
      const CVector3f scaledDelta(sx * mPosDelta.GetX() * mMoveScale.GetX(),
                                  sy * mPosDelta.GetY() * mMoveScale.GetY(),
                                  sz * mPosDelta.GetZ() * mMoveScale.GetZ());
      MoveToOR(scaledDelta, dt);
    }
  }

  RotateToOR(mRotDelta, dt);
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
    if (!mAlive) {
      if (!mFadeToDeath) {
        mAlphaDelta = -1.f / 3.f;
        mFadeToDeath = true;
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
    mBodyController->SetOnFire(1.5f * duration);
    mPendingFireDamage = 1.5f * damage;
    break;
  case kVN_Normal:
    mBodyController->SetOnFire(duration);
    mPendingFireDamage = damage;
    break;
  default:
    break;
  }
}

void CPatterned::Shock(CStateManager&, float duration, float damage) {
  switch (static_cast< const CPatterned* >(this)->GetDamageVulnerability()->GetVulnerability(
      CWeaponMode::Wave(), CDamageVulnerability::kRD_No)) {
  case kVN_Weak:
    mBodyController->SetElectrocuting(1.5f * duration);
    mPendingShockDamage = 1.5f * damage;
    break;
  case kVN_Normal:
    mBodyController->SetElectrocuting(duration);
    mPendingShockDamage = damage;
    break;
  default:
    break;
  }
}

void CPatterned::Freeze(CStateManager& mgr, const CVector3f& pos, CUnitVector3f dir,
                        float frozenDur) {
  if (mLostMassiveFrozenHP) {
    mDieIf80PercFrozen = true;
  }

  bool playSfx = false;
  if (mBodyController->IsFrozen()) {
    mBodyController->Freeze(mKnockBackController.GetActiveParms().mIntoFreezeDur,
                                frozenDur, mOutofFreezeDur);
    mgr.ActorModelParticles()->DoIcePop(*this);
    playSfx = true;
  } else if (!mBodyController->IsElectrocuting() && !mBodyController->IsOnFire()) {
    mBodyController->Freeze(mIntoFreezeDur, frozenDur, mOutofFreezeDur);
    if (mVertexMorph) {
      mVertexMorph->Reset(dir, pos, mIntoFreezeDur);
    }
    playSfx = true;
  }

  if (playSfx) {
    const CVector3f& posOut = GetTranslation();
    CSfxManager::AddEmitter(
        mKnockBackController.GetCreatureSize() != kCS_Small &&
                PATTERNED_CAST_TO(CMetroid, const_cast< CEntity* >(mgr.GetObjectById(GetUniqueId()))) != nullptr
            ? (SND_FXID)0x701
            : (SND_FXID)0x708,
        posOut, CVector3f::Zero(), true, false, CSfxManager::kMedPriority, CSfxManager::kAllAreas);
  }
}

void CPatterned::PhazeOut(CStateManager& mgr) {
  if (!mFadeToDeath) {
    mFadeToDeath = true;
    SendScriptMsgs(kSS_DeathRattle, mgr, kSM_None);
  }

  mPhazingOut = true;
  mBodyController->SetPlaybackRate(0.f);

  ModelData()->AnimationData()->GetParticleDB().SetUpdatesEnabled(false);
}

float CPatterned::CalcDyingThinkRate() {
  float thinkRate;
  if (mBurning) {
    thinkRate = mBurnThinkRateTimer / 1.5f;
  } else {
    thinkRate = 1.f;
  }

  const float minThinkRate = 0.1f;
  thinkRate = CMath::Max(minThinkRate, thinkRate);
  return thinkRate;
}

CDamageInfo CPatterned::GetContactDamage() const { return mContactDamage; }

void CPatterned::DeathDelete(CStateManager& mgr) {
  SendScriptMsgs(kSS_Dead, mgr, kSM_None);

  if (mBodyController->IsElectrocuting()) {
    mPendingShockDamage = 0.f;
    mBodyController->StopElectrocution();
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

  CColor color = mColor;
  uchar alpha = GetModelAlphau8(mgr);
  if (mNoXrayModel) {
    const bool xray = mgr.GetPlayerState()->IsXRayActive(mgr);
    if (xray) {
      alpha = 76;
    }
  }

  if (alpha < 255) {
    if (color.GetRedu8() == 0 && color.GetGreenu8() == 0 && color.GetBlueu8() == 0) {
      color = CColor::White();
    }

    if (mLaggedBurnDeath) {
      // TODO: maybe this is CModelFlags::ColorModulate?
      uchar stripedAlpha = alpha > 127 ? (alpha - 128) * 2 : 255;
      SetModelFlags(
          CModelFlags(CModelFlags::kT_Three,
                      CColor(skDisintegrateColor.GetRedu8(), skDisintegrateColor.GetGreenu8(),
                             skDisintegrateColor.GetBlueu8(), (stripedAlpha * stripedAlpha) >> 8)));
    } else if (mBurning) {
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
  const bool drawParticles = mDrawParticles;
  int mask = 0;
  int target = 0;
  if (drawParticles) {
    const_cast< CStateManager& >(mgr).GetCharacterRenderMaskAndTarget(mThawed, mask, target);
    GetModelData()->GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnFirstMasked(mask,
                                                                                          target);
  }

  const EThermalDrawFlag thermalFlag = mgr.GetThermalDrawFlag();
  if ((thermalFlag == kTD_Cold && !mThawed) || (thermalFlag == kTD_Hot && mThawed) ||
      thermalFlag == kTD_Bypass) {
    if (mBurning) {
      CTexture* ashyTexture =
          const_cast< CStateManager& >(mgr).ActorModelParticles()->GetAshyTexture(
              *const_cast< CPatterned* >(this));
      const uchar alpha = GetModelAlphau8(mgr);
      if (ashyTexture != 0 && ((!mLaggedBurnDeath && alpha <= 0xff) || alpha <= 0x7f)) {
        if (GetPointGeneratorParticles()) {
          mgr.SetupParticleHook(*this);
        }

        const CColor* disintegrateColor = nullptr;
        if (mLaggedBurnDeath) {
          disintegrateColor = &skDisintegrateColor;
        } else if (mgr.GetThermalDrawFlag() == kTD_Hot) {
          disintegrateColor = &CColor::White();
        } else {
          disintegrateColor = &CColor::Black();
        }

        const CColor disColor = *disintegrateColor;
        const float t =
            (mLaggedBurnDeath ? 0.0078740157f : 0.0039215689f) * CCast::ToReal32(alpha);
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

    if (mBodyController->IsFrozen() && !mBurning) {
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
    animData->Render(model->GetSkinnedModel(), useFlags, *mVertexMorph, avgNormals);
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
  mPreThinkDt = dt;
  CEntity::PreThink(dt, mgr);
}

void CPatterned::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  if (mDrawParticles) {
    if (HasModelData()) {
      int mask;
      int target;
      const_cast< CStateManager& >(mgr).GetCharacterRenderMaskAndTarget(mThawed, mask,
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
  mKeepThermalVisorState = true;
  SetThermalFlags(static_cast< EThermalFlags >(3));
}

void CPatterned::UpdateThermalFrozenState(const bool thawed) {
  mThawed = thawed;
  if (mKeepThermalVisorState) {
    return;
  }

  SetThermalFlags(static_cast< EThermalFlags >(thawed ? kTF_Hot : kTF_Cold));
}

CVector3f CAi::GetOrigin(const CStateManager&, const CTeamAiRole&, const CVector3f&) const {
  return GetTranslation();
}

bool CAi::IsListening() const { return false; }

rstl::optional_object< CAABox > CPatterned::GetTouchBounds() const { return GetBoundingBox(); }

void CPatterned::TakeDamage(const CVector3f&, float) { mDamageCooldownTimer = 0.33f; }

bool CPatterned::Default(CStateManager&, float) { return true; }
