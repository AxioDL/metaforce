#include "MetroidPrime/Enemies/CWarWasp.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateInfo.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CTeamAiMgr.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"

static const float skSeekMagTable[] = {0.4f, 0.6f, 1.f};
static const float skMinAngle = 0.17453292f;

static const char* const kStingLctrName = "LCTR_WARTAIL";

CWarWasp::CWarWasp(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                   const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
                   CPatterned::EFlavorType flavor, CPatterned::EColliderType collider,
                   const CDamageInfo& dInfo, const CActorParameters& actParms,
                   CAssetId projectileWeapon, CDamageInfo projectileDamage,
                   CAssetId projectileVisorParticle, uint projectileVisorSfx)
: CPatterned(kC_WarWasp, uid, name, flavor, info, xf, mData, pInfo, kMT_Flyer, collider, kBT_Flyer,
             actParms, kCS_Small)
, mStateProg(-1)
, mCSphere(CSphere(CVector3f(0.f, 0.f, 1.8f), 1.f), GetMaterialList())
, mPfSearch(nullptr, 3, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, mAiMgr(kInvalidUniqueId)
, mTargetPos(CVector3f::Zero())
, mContactDamage(dInfo)
, mInitialRot(CQuaternion::NoRotation())
, mCircleBurstPos(CVector3f::Zero())
, mCircleBurstDir(CVector3f(0.f, 0.f, 0.f))
, mCircleBurstRight(CVector3f(0.f, 0.f, 0.f))
, mProjectileInfo(projectileWeapon, projectileDamage)
, mInitialSpeed(mSpeed)
, mAttackRemTime(0.f)
, mDodgeDir(pas::kSD_Invalid)
, mCircleAttackTeam(-1)
, mInitialCircleAttackTeam(-1)
, mInitialCircleAttackTeamUnit(-1)
, mCircleTelegraphSeekHeight(0.f)
, mCircleBurstOffTotemAngle(1.5707964f)
, mProjectileVisorSfx(CSfxManager::TranslateSFXID(projectileVisorSfx))
, mJumpBackRepeat(true)
, mCanApplyDamage(false)
, mInitiallyInactive(!pInfo.GetActive())
, mTeamMatesMelee(false)
, mInProjectileAttack(false)
, mPathObstructed(false)
, mIsRetreating(false)
, mHeardNoise(false) {
  if (flavor == kFT_Two) {
    mProjectileInfo.Token().Lock();
  }
  UpdateTouchBounds();
  SetCoefficientOfRestitutionModifier(0.1f);
  if (projectileVisorParticle != kInvalidAssetId) {
    mProjectileVisorParticle =
        gpSimplePool->GetObj(SObjectTag('PART', projectileVisorParticle));
  }
  mNoPatternShagging = true;
  mKnockBackController.SetAnimationStateRange(kAR_KnockBack, kAR_KnockBack);
}

CWarWasp::~CWarWasp() {}

ENTITY_ACCEPT_IMPL(CWarWasp)

void CWarWasp::AcceptScriptMsg(const EScriptObjectMessage msg, const TUniqueId uid,
                               CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_Deleted:
  case kSM_Deactivate:
    SwarmRemove(mgr);
    break;
  case kSM_InitializedInArea: {
    if (mAiMgr == kInvalidUniqueId) {
      mAiMgr = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
    }

    const TAreaId aid = GetCurrentAreaId();
    mPfSearch.SetArea(mgr.World()->GetAreaAlways(aid).GetPostConstructed()->mPathArea);
    if (!mCircleBurstPos.IsNonZero()) {
      SetUpCircleBurstWaypoint(mgr);
    }
  } break;
  default:
    break;
  }
}

void CWarWasp::Death(CStateManager& mgr, const CVector3f& direction,
                     const EScriptObjectState state) {
  CPatterned::Death(mgr, direction, state);
  mVerticalMovement = false;
  AddMaterial(kMT_GroundCollider, mgr);
  SwarmRemove(mgr);
}

void CWarWasp::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (mAttackRemTime > 0.f) {
    const CPlayer* player = mgr.GetPlayer();
    float multiplier = 1.f;
    if (player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
      const CVector3f distance = GetTranslation() - player->GetTranslation();
      const CVector3f playerForward = player->GetTransform().GetForward();
      multiplier = 1.f - (CVector2f::GetAngleDiff(playerForward.ToVec2f(), distance.ToVec2f()) /
                          M_PIF * 0.666f);
    }

    mAttackRemTime -= dt * multiplier;
  }

  ApplyDamage(mgr);
  CPatterned::Think(dt, mgr);
}

CProjectileInfo* CWarWasp::ProjectileInfo() { return &mProjectileInfo; }

rstl::optional_object< CAABox > CWarWasp::GetTouchBounds() const {
  return mCSphere.CalculateAABox(GetTransform());
}

const CCollisionPrimitive* CWarWasp::GetCollisionPrimitive() const { return &mCSphere; }

void CWarWasp::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                               float dt) {
  bool handled = false;
  switch (type) {
  case kUE_Projectile: {
    const CTransform4f xf = GetLctrTransform(node.GetLocatorName());
    CVector3f aimPos = GetProjectileAimPos(mgr, -0.07f);
    if (mgr.GetPlayer()->GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
      const CVector3f delta = aimPos - xf.GetTranslation();
      if (delta.CanBeNormalized()) {
        TEntityList nearList;
        TUniqueId bestId = kInvalidUniqueId;
        const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
        const CVector3f dir = delta.AsNormalized();
        const float length = delta.Magnitude();
        CRayCastResult res =
            mgr.RayWorldIntersection(bestId, xf.GetTranslation(), dir, length, filter, nearList);
        if (res.IsValid()) {
          aimPos = res.GetPoint();
        }
      }
    }
    CVector3f intercept = ProjectileInfo()->PredictInterceptPos(xf.GetTranslation(), aimPos,
                                                                *mgr.GetPlayer(), true, dt);
    CTransform4f projectileXf = CTransform4f::LookAt(xf.GetTranslation(), intercept);
    LaunchProjectile(projectileXf, mgr, 4, CWeapon::kPA_None, false, mProjectileVisorParticle,
                     mProjectileVisorSfx, true, CVector3f(1.f, 1.f, 1.f));
    handled = true;
    break;
  }
  case kUE_DeGenerate:
    SendScriptMsgs(kSS_DeactivateState, mgr, kSM_None);
    mgr.DeleteObjectRequest(GetUniqueId());
    handled = true;
    break;
  case kUE_GenerateEnd:
    AddMaterial(kMT_Character, kMT_Solid, mgr);
    handled = true;
    break;
  case kUE_DamageOn:
    mCanApplyDamage = true;
    break;
  case kUE_DamageOff:
    mCanApplyDamage = false;
    break;
  case kUE_BeginAction:
  default:
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

bool CWarWasp::Listen(const CVector3f& pos, EListenNoiseType type) {
  switch (type) {
  case kLNT_PlayerFire:
  case kLNT_BombExplode:
  case kLNT_ProjectileExplode: {
    const CVector3f diff = GetTranslation() - pos;
    if (diff.MagSquared() < mDetectionRange * mDetectionRange) {
      mHeardNoise = true;
      return true;
    }
  } break;
  default:
    break;
  }

  return false;
}

CVector3f CWarWasp::GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                              const CVector3f& aimPos) const {
  if (!mCircleBurstPos.IsNonZero()) {
    const CVector3f ret = GetCloseInPos(mgr, aimPos);
    return ret;
  }
  return GetTranslation();
}

bool CWarWasp::PathShagged(CStateManager& mgr, float arg) {
  if (GetSearchPath()) {
    return GetSearchPath()->IsShagged();
  }
  return false;
}

bool CWarWasp::HearShot(CStateManager& mgr, float arg) {
  if (mHeardNoise || mHitByPlayerProjectile) {
    return true;
  }

  if (const CTeamAiMgr* teamMgr = TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(mAiMgr))) {
    return teamMgr->GetNumRoles() != 0;
  }

  return false;
}

bool CWarWasp::InAttackPosition(CStateManager& mgr, float arg) {
  CVector3f pos = GetTranslation();
  CVector3f aimPos = GetProjectileAimPos(mgr, -1.25f);
  CVector3f delta = aimPos - pos;
  float distanceSq = delta.MagSquared();
  float minRange = mMinAttackRange - 5.f;
  bool ret = distanceSq > minRange * minRange &&
             distanceSq < (5.f + mMaxAttackRange) * (5.f + mMaxAttackRange) &&
             !ShouldTurn(mgr, 45.f);
  if (ret) {
    const CVector3f& currentPos = GetTranslation();
    ret = currentPos.GetZ() > mgr.GetPlayer()->GetTranslation().GetZ() &&
          currentPos.GetZ() < 2.5f + aimPos.GetZ();
    if (ret && delta.CanBeNormalized()) {
      const CMaterialFilter filter =
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList(kMT_Player));
      float length = delta.Magnitude();
      CVector3f dir = (1.f / length) * delta;
      ret = !mgr.RayStaticIntersection(pos, dir, length, filter).IsValid();
    }
  }
  return ret;
}

bool CWarWasp::ShouldAttack(CStateManager& mgr, float arg) {
  if (mAttackRemTime <= 0.f) {
    if (const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, mAiMgr, GetUniqueId())) {
      if (role->GetTeamAiRole() == CTeamAiRole::kTAR_Melee && !mgr.GetPlayer()->IsInsideFluid()) {
        if (CTeamAiMgr* teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mAiMgr))) {
          if (!teamMgr->HasMeleeAttackers()) {
            return !IsPatternObstructed(mgr, GetTranslation(), GetProjectileAimPos(mgr, -1.25f));
          }
        }
      }
    } else if (mFlavor != kFT_Two && !mgr.GetPlayer()->IsInsideFluid()) {
      return !IsPatternObstructed(mgr, GetTranslation(), GetProjectileAimPos(mgr, -1.25f));
    }
  }
  return false;
}

bool CWarWasp::ShouldFire(CStateManager& mgr, float arg) {
  if (mAttackRemTime <= 0.f) {
    if (const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, mAiMgr, GetUniqueId())) {
      if (role->GetTeamAiRole() == CTeamAiRole::kTAR_Projectile) {
        CVector3f aimPos = GetProjectileAimPos(mgr, -1.25f);
        CVector3f delta = aimPos - GetTranslation();
        CVector3f forward = GetTransform().GetForward();
        if (delta.CanBeNormalized() && CVector3f::Dot(forward, delta.AsNormalized()) >= 0.906f) {
          if (CTeamAiMgr* teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mAiMgr))) {
            return !teamMgr->HasProjectileAttackers();
          }
        }
      }
    } else if (mFlavor == kFT_Two) {
      CVector3f aimPos = GetProjectileAimPos(mgr, -1.25f);
      CVector3f delta = aimPos - GetTranslation();
      CVector3f forward = GetTransform().GetForward();
      if (delta.CanBeNormalized()) {
        return CVector3f::Dot(forward, delta.AsNormalized()) >= 0.906f;
      }
    }
  }
  return false;
}

bool CWarWasp::ShouldSpecialAttack(CStateManager& mgr, float arg) {
  if (mCircleAttackTeam == 0 && !mgr.GetPlayer()->IsInsideFluid()) {
    if (const CTeamAiMgr* teamMgr = TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(mAiMgr))) {
      if (CheckCircleAttackSpread(mgr, mCircleAttackTeam)) {
        TUniqueId leaderId = GetAttackTeamLeader(mgr, mCircleAttackTeam);
        if (leaderId == GetUniqueId()) {
          if (mAttackRemTime <= 0.f) {
            CVector2f delta =
                mgr.GetPlayer()->GetTranslation().DropZ() - mCircleBurstPos.DropZ();
            if (delta.MagSquared() < 90.25f) {
              CVector3f fromCenter = GetTranslation() - mCircleBurstPos;
              CVector3f threshold = mCircleBurstDir;
              if (mCircleBurstOffTotemAngle <= M_PIF / 2.f) {
                threshold = CVector3f::Slerp(mCircleBurstRight, mCircleBurstDir,
                                             CRelAngle::FromRadians(mCircleBurstOffTotemAngle));
              } else {
                threshold = CVector3f::Slerp(
                    mCircleBurstDir, -mCircleBurstRight,
                    CRelAngle::FromRadians(mCircleBurstOffTotemAngle - M_PIF / 2.f));
              }
              if (CVector3f::GetAngleDiff(threshold, fromCenter) <
                  CRelAngle::FromDegrees(10.f).AsRadians()) {
                return CTeamAiMgr::AddAttacker(kAT_Melee, mgr, mAiMgr, GetUniqueId());
              }
            }
          }
        } else {
          if (const CWarWasp* leader = PATTERNED_CAST_TO(CWarWasp, const_cast< CEntity* >(mgr.GetObjectById(leaderId)))) {
            if (leader->mTeamMatesMelee) {
              return CTeamAiMgr::AddAttacker(kAT_Melee, mgr, mAiMgr, GetUniqueId());
            }
          }
        }
      }
    }
  }
  return false;
}

bool CWarWasp::ShouldTurn(CStateManager& mgr, float arg) {
  CVector2f delta = (mgr.GetPlayer()->GetTranslation() - GetTranslation()).DropZ();
  CVector2f forward = GetTransform().GetForward().DropZ();
  return CVector2f::GetAngleDiff(forward, delta) > CRelAngle::FromDegrees(arg).AsRadians();
}

bool CWarWasp::ShouldDodge(CStateManager& mgr, float arg) {
  CVector3f pos = GetTranslation();
  CAABox bounds(pos - CVector3f(7.5f, 7.5f, 7.5f), pos + CVector3f(7.5f, 7.5f, 7.5f));
  TEntityList nearList;
  const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Projectile));
  mgr.BuildNearList(nearList, bounds, filter, nullptr);
  if (nearList.size() > 0) {
    CVector3f forward = GetTransform().GetForward();
    for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
      if (const CGameProjectile* proj =
              TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(*it))) {
        if (proj->GetOwnerId() == mgr.GetPlayer()->GetUniqueId()) {
          CVector3f delta = proj->GetTranslation() - GetTranslation();
          if (CVector3f::GetAngleDiff(forward, delta) < M_PIF / 4.f) {
            CVector3f right = GetTransform().GetRight();
            mDodgeDir = CVector3f::Dot(right, delta) > 0.f ? pas::kSD_Right : pas::kSD_Left;
            return true;
          }
        }
      }
    }
  }
  return false;
}

bool CWarWasp::Leash(CStateManager& mgr, float arg) {
  const CVector3f leashDelta = mLatestLeashPosition - GetTranslation();
  if (leashDelta.MagSquared() > mLeashRadius * mLeashRadius) {
    const CVector3f playerDelta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    return playerDelta.MagSquared() > mPlayerLeashRadius * mPlayerLeashRadius &&
           mCurPlayerLeashTime > mPlayerLeashTime;
  }
  return false;
}

bool CWarWasp::InPosition(CStateManager& mgr, float arg) {
  if (GetSearchPath()) {
    return GetSearchPath()->IsOver();
  }
  CVector3f delta = mTargetPos - GetTranslation();
  return delta.MagSquared() < 1.f;
}

bool CWarWasp::AnimOver(CStateManager& mgr, float arg) { return mStateProg == 3; }

void CWarWasp::Generate(CStateManager& mgr, EStateMsg msg, float dt) {
  CBodyController* const bc = BodyCtrl();
  switch (msg) {
  case kStateMsg_Activate:
    bc->Activate(mgr);
    if (mInitiallyInactive) {
      RemoveMaterial(kMT_Character, kMT_Solid, mgr);
      mInitialRot = CQuaternion::FromMatrix(GetTransform());
      CQuaternion rot = mInitialRot;
      rot *= CQuaternion::ZRotation(
          CRelAngle::FromRadians((M_PIF / 4.f) * mgr.Random()->Float() - M_PIF / 8.f));
      SetRotation(rot.BuildNormalized());
      mStateProg = 0;
    } else {
      mStateProg = 3;
    }
    break;
  case kStateMsg_Update:
    switch (mStateProg) {
    case 0:
      if (bc->GetCurrentStateId() == pas::kAS_Generate) {
        mStateProg = 2;
      } else {
        const int anim = mAnim;
        CBCGenerateCmd cmd(pas::kGType_Zero, anim);
        bc->CommandMgr().DeliverCmd(cmd);
      }
      break;
    case 2:
      if (bc->GetCurrentStateId() != pas::kAS_Generate) {
        mStateProg = 3;
      } else {
        CVector3f move = CVector3f::Zero();
        const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
        for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
          if (const CPatterned* other = TCastToPtr< CPatterned >(const_cast< CEntity* >(list[i]))) {
            if (other != this && other->GetCurrentAreaId() == GetCurrentAreaId()) {
              move += (5.f * dt) *
                      mSteeringBehaviors.Separation(*this, other->GetTranslation(), 5.f);
            }
          }
        }
        if (move.IsNonZero()) {
          CVector3f impulse = GetMoveToORImpulseWR(move, dt);
          ApplyImpulseWR(impulse, CAxisAngle::Identity());
        }
      }
      break;
    default:
      break;
    }
    break;
  case kStateMsg_Deactivate:
    if (mInitiallyInactive) {
      AddMaterial(kMT_Character, kMT_Solid, mgr);
      if (mSolidCollision) {
        mPendingDeath = true;
      }
    }
    break;
  }
}

void CWarWasp::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mTeamMatesMelee = true;
    mCanApplyDamage = false;
    if (mAiMgr != kInvalidUniqueId) {
      bool added = CTeamAiMgr::AddAttacker(kAT_Melee, mgr, mAiMgr, GetUniqueId());
      int state = 3;
      if (added) {
        state = 0;
      }
      mStateProg = state;
    } else {
      mStateProg = 0;
    }
    break;
  case kStateMsg_Update:
    switch (mStateProg) {
    case 0:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_MeleeAttack) {
        mStateProg = 2;
      } else {
        CVector3f aimPos = GetProjectileAimPos(mgr, -1.25f);
        CVector3f delta = aimPos - GetTranslation();
        if (delta.CanBeNormalized()) {
          aimPos += 7.5f * delta.AsNormalized();
        }
        BodyCtrl()->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_One, aimPos));
      }
      break;
    case 2:
      if (BodyCtrl()->GetCurrentStateId() != pas::kAS_MeleeAttack) {
        mStateProg = 3;
      } else {
        BodyCtrl()->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                     GetTranslation());
      }
      break;
    default:
      break;
    }
    break;
  case kStateMsg_Deactivate:
    CTeamAiMgr::ResetTeamAiRole(kAT_Melee, mgr, mAiMgr, GetUniqueId(), false);
    mAttackRemTime = CalcTimeToNextAttack(mgr);
    mTeamMatesMelee = false;
    break;
  }
}

void CWarWasp::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mInProjectileAttack = true;
    if (mAiMgr != kInvalidUniqueId) {
      bool added = CTeamAiMgr::AddAttacker(kAT_Projectile, mgr, mAiMgr, GetUniqueId());
      int state = 3;
      if (added) {
        state = 0;
      }
      mStateProg = state;
    } else {
      mStateProg = 0;
    }
    break;
  case kStateMsg_Update:
    switch (mStateProg) {
    case 0:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_ProjectileAttack) {
        mStateProg = 2;
      } else {
        SetDestPos(GetProjectileAimPos(mgr, -0.07f));
        BodyCtrl()->CommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::kS_One, mDestPos, false));
      }
      break;
    case 2:
      if (BodyCtrl()->GetCurrentStateId() != pas::kAS_ProjectileAttack) {
        BodyCtrl()->CommandMgr().DeliverTargetVector(mDestPos - GetTranslation());
        mStateProg = 3;
      }
      break;
    default:
      break;
    }
    break;
  case kStateMsg_Deactivate:
    CTeamAiMgr::ResetTeamAiRole(kAT_Projectile, mgr, mAiMgr, GetUniqueId(), false);
    mAttackRemTime = CalcTimeToNextAttack(mgr);
    mInProjectileAttack = false;
    break;
  }
}

void CWarWasp::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    SetUpPathFindBehavior(mgr);
    break;
  case kStateMsg_Update: {
    if (mgr.GetPlayer()->GetOrbitState() != CPlayer::kOS_NoOrbit &&
        mgr.GetPlayer()->GetOrbitTargetId() == GetUniqueId()) {
      BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    } else {
      BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
    }
    if (GetSearchPath() && !PathShagged(mgr, 0.f) && !GetSearchPath()->IsOver()) {
      CPatterned::PathFind(mgr, msg, dt);
    } else {
      ApplyNormalSteering(mgr);
    }
    ApplySeparationBehavior(mgr, 9.f);
    CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    float range = 2.f * mMaxAttackRange;
    if (delta.MagSquared() > range * range) {
      BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
      BodyCtrl()->CommandMgr().SetSteeringSpeedRange(1.f, 1.f);
    } else {
      BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
    }
    break;
  }
  case kStateMsg_Deactivate:
    BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
    break;
  }
}

void CWarWasp::Shuffle(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
    mStateProg = 2;
    break;
  case kStateMsg_Update: {
    int numRoles = 0;
    if (const CTeamAiMgr* teamMgr = TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(mAiMgr))) {
      numRoles = teamMgr->GetNumAssignedAiRoles();
    }
    if (numRoles != 0u && mAttackRemTime > 0.f) {
      CVector3f shuffleDest = CalcShuffleDest(mgr);
      float zDelta = shuffleDest.GetZ() - GetTranslation().GetZ();
      const CVector3f& pos = GetTranslation();
      if (zDelta * zDelta > 1.f) {
        CVector3f dest(GetTranslation()[kDX], GetTranslation()[kDY], shuffleDest[kDZ]);
        CVector3f move = mSteeringBehaviors.Arrival(*this, dest, 1.f);
        if (move.MagSquared() > 0.01f) {
          BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
        }
      } else {
        CVector3f move = shuffleDest - pos;
        if (move.MagSquared() > 64.f) {
          pas::EStepDirection stepDir = FindBestStepDirection(move);
          if (stepDir != pas::kSD_Forward) {
            BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(stepDir, pas::kStep_Normal));
          } else {
            BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(
                mSteeringBehaviors.Seek(*this, shuffleDest), CVector3f::Zero(), 1.f));
            ApplySeparationBehavior(mgr, 15.f);
          }
        } else {
          mStateProg = 3;
        }
      }
      BodyCtrl()->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                   GetTranslation());
    } else {
      mStateProg = 3;
    }
    break;
  }
  case kStateMsg_Deactivate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    break;
  }
}

void CWarWasp::JumpBack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mStateProg = 0;
    mJumpBackRepeat = true;
    SwarmRemove(mgr);
    break;
  case kStateMsg_Update:
    switch (mStateProg) {
    case 0:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_Step) {
        mStateProg = 2;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Backward, pas::kStep_Normal));
      }
      break;
    case 2:
      if (BodyCtrl()->GetCurrentStateId() != pas::kAS_Step) {
        int state = 3;
        if (mJumpBackRepeat) {
          state = 0;
        }
        mStateProg = state;
        mJumpBackRepeat = false;
      } else {
        const CVector3f& playerPos = mgr.GetPlayer()->GetTranslation();
        BodyCtrl()->CommandMgr().DeliverTargetVector(playerPos - GetTranslation());
      }
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

void CWarWasp::Retreat(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    SwarmRemove(mgr);
    BodyCtrl()->SetLocomotionType(pas::kLT_Internal5);
    break;
  case kStateMsg_Update:
    BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(
        CVector3f(mSteeringBehaviors.Flee2D(*this, mgr.GetPlayer()->GetTranslation().DropZ()),
                  0.f),
        CVector3f::Zero(), 1.f));
    break;
  case kStateMsg_Deactivate:
    mHitByPlayerProjectile = false;
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    break;
  }
}

void CWarWasp::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    float maxSpeed = BodyCtrl()->BodyStateInfo().GetMaxSpeed();
    if (maxSpeed > 0.f) {
      BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
      float speed = 0.9f * BodyCtrl()->BodyStateInfo().GetLocomotionSpeed(pas::kLA_Walk) / maxSpeed;
      BodyCtrl()->CommandMgr().SetSteeringSpeedRange(speed, speed);
    }
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    mHeardNoise = false;
    break;
  }
  case kStateMsg_Deactivate:
    BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_Normal);
    break;
  default:
    break;
  }
  CPatterned::Patrol(mgr, msg, dt);
}

void CWarWasp::Deactivate(CStateManager& mgr, EStateMsg msg, float dt) {
  CBodyController& bc = *BodyCtrl();
  switch (msg) {
  case kStateMsg_Activate:
    mStateProg = 1;
    mIsRetreating = true;
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    SwarmRemove(mgr);
    mAiMgr = kInvalidUniqueId;
    mTargetPos = mLatestLeashPosition;
    SetDestPos(mTargetPos);
    if (GetSearchPath()) {
      CPatterned::PathFind(mgr, msg, dt);
    }
    break;
  case kStateMsg_Update:
    switch (mStateProg) {
    case 1:
      if (SteerToDeactivatePos(mgr, msg, dt)) {
        RemoveMaterial(kMT_Solid, mgr);
        mStateProg = 0;
      }
      break;
    case 0:
      if (bc.GetCurrentStateId() == pas::kAS_Generate) {
        RemoveMaterial(kMT_Character, kMT_Target, kMT_Orbit, mgr);
        mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
        mStateProg = 2;
      } else {
        bc.CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_One, -1));
      }
      break;
    case 2:
      if (bc.GetCurrentStateId() != pas::kAS_Generate) {
        mStateProg = 3;
      }
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

void CWarWasp::Dodge(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (mDodgeDir != pas::kSD_Invalid) {
      BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(mDodgeDir, pas::kStep_Dodge));
      mStateProg = 2;
    }
    break;
  case kStateMsg_Update:
    if (BodyCtrl()->GetCurrentStateId() != pas::kAS_Step) {
      mStateProg = 3;
    } else {
      BodyCtrl()->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                   GetTranslation());
    }
    break;
  case kStateMsg_Deactivate:
    mDodgeDir = pas::kSD_Invalid;
    break;
  }
}

void CWarWasp::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    SwarmRemove(mgr);
    CPatterned::Patrol(mgr, msg, dt);
    CPatterned::UpdateDest(mgr);
    mTargetPos = mDestPos;
    if (GetSearchPath()) {
      CPatterned::PathFind(mgr, msg, dt);
    }
    break;
  case kStateMsg_Update:
    if (GetSearchPath() && !PathShagged(mgr, 0.f)) {
      CPatterned::PathFind(mgr, msg, dt);
    } else {
      CPatterned::Patrol(mgr, msg, dt);
    }
    ApplySeparationBehavior(mgr, 9.f);
    break;
  default:
    break;
  }
}

void CWarWasp::TelegraphAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    RemoveMaterial(kMT_Orbit, mgr);
    mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
    SwarmAdd(mgr);
    SetUpCircleTelegraphTeam(mgr);
    break;
  case kStateMsg_Update:
    if (mCircleBurstPos.IsNonZero()) {
      TryCircleTeamMerge(mgr);
      CVector3f delta = GetTranslation() - mCircleBurstPos;
      delta.SetZ(0.f);
      CVector3f move = GetTransform().GetForward();
      if (delta.CanBeNormalized()) {
        CVector3f dir = delta.AsNormalized();
        move = CVector3f::Cross(dir, CVector3f::Up());
        CVector3f radial = mMinAttackRange * dir;
        CVector3f seekOrigin = mCircleBurstPos + radial;
        if (mCircleAttackTeam > 0) {
          move = -1.f * move;
        }
        float seekHeight = mCircleTelegraphSeekHeight + GetTeamZStratum(mCircleAttackTeam);
        float seekMag = CalcSeekMagnitude(mgr);
        CVector3f seekDest = seekOrigin + 5.f * move;
        move = seekMag *
               mSteeringBehaviors.Seek(*this, seekDest + CVector3f(0.f, 0.f, seekHeight));
      }
      BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
      UpdateTelegraphMoveSpeed(mgr);
    }
    break;
  case kStateMsg_Deactivate:
    AddMaterial(kMT_Orbit, mgr);
    break;
  }
}

void CWarWasp::SpecialAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mTeamMatesMelee = true;
    mCanApplyDamage = true;
    mStateProg = 0;
    mSpeed = mInitialSpeed;
    break;
  case kStateMsg_Update:
    switch (mStateProg) {
    case 0:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_MeleeAttack) {
        mStateProg = 2;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_Eight));
      }
      break;
    case 2:
      if (BodyCtrl()->GetCurrentStateId() != pas::kAS_MeleeAttack) {
        mStateProg = 3;
      } else {
        CVector3f delta = mCircleBurstPos - GetTranslation();
        CVector3f forward = GetTransform().GetForward();
        if (CVector3f::Dot(forward, delta) > 0.f) {
          BodyCtrl()->CommandMgr().DeliverTargetVector(delta);
        }
      }
      break;
    default:
      break;
    }
    break;
  case kStateMsg_Deactivate:
    CTeamAiMgr::ResetTeamAiRole(kAT_Melee, mgr, mAiMgr, GetUniqueId(), false);
    mTeamMatesMelee = false;
    mCircleAttackTeam = -1;
    mCircleBurstOffTotemAngle = CalcOffTotemAngle(mgr);
    break;
  }
}

void CWarWasp::JoinCircleAttackTeam(int unit, CStateManager& mgr) {
  if (mCircleBurstPos.IsNonZero()) {
    if (mInitialCircleAttackTeam == -1) {
      mInitialCircleAttackTeamUnit = GetAttackTeamSize(mgr, unit);
      mInitialCircleAttackTeam = unit;
    }
    mCircleAttackTeam = unit;
    mAttackRemTime = CalcTimeToNextAttack(mgr);
    mCircleBurstOffTotemAngle = CalcOffTotemAngle(mgr);
  }
}

void CWarWasp::SwarmAdd(CStateManager& mgr) {
  if (mAiMgr != kInvalidUniqueId) {
    if (CTeamAiMgr* teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mAiMgr))) {
      CTeamAiRole::ETeamAiRole role =
          mFlavor == kFT_Two ? CTeamAiRole::kTAR_Projectile : CTeamAiRole::kTAR_Melee;
      if (!teamMgr->IsPartOfTeam(GetUniqueId())) {
        teamMgr->AssignTeamAiRole(*this, role, CTeamAiRole::kTAR_Invalid,
                                  CTeamAiRole::kTAR_Invalid);
      }
    }
  }
}

void CWarWasp::SwarmRemove(CStateManager& mgr) {
  if (mAiMgr != kInvalidUniqueId) {
    if (CTeamAiMgr* teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mAiMgr))) {
      if (teamMgr->IsPartOfTeam(GetUniqueId())) {
        teamMgr->RemoveTeamAiRole(GetUniqueId());
      }
    }
  }
}

void CWarWasp::ApplySeparationBehavior(CStateManager& mgr, float sep) {
  const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    if (const CPatterned* other = TCastToPtr< CPatterned >(const_cast< CEntity* >(list[i]))) {
      if (other != this && other->GetCurrentAreaId() == GetCurrentAreaId()) {
        float useSep = sep;
        if (const CTeamAiRole* role =
                CTeamAiMgr::GetTeamAiRole(mgr, mAiMgr, other->GetUniqueId())) {
          if (role->GetTeamAiRole() == CTeamAiRole::kTAR_Melee ||
              role->GetTeamAiRole() == CTeamAiRole::kTAR_Projectile) {
            useSep *= 2.f;
          }
        }
        CVector3f separation =
            mSteeringBehaviors.Separation(*this, other->GetTranslation(), useSep);
        if (separation.IsNonZero()) {
          BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(separation, CVector3f::Zero(), 1.f));
        }
      }
    }
  }
}

void CWarWasp::ApplyNormalSteering(CStateManager& mgr) {
  CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, mAiMgr, GetUniqueId());
  CVector3f teamPos = role ? role->GetTeamPosition() : GetProjectileAimPos(mgr, -1.25f);
  const CVector3f& teamDelta = teamPos - GetTranslation();
  CVector2f toTeamPos = teamDelta.DropZ();
  float zDelta = teamPos.GetZ() - GetTranslation().GetZ();
  if (toTeamPos.MagSquared() > 1.f || CMath::AbsF(zDelta) > 2.5f) {
    pas::EStepDirection stepDir = FindBestStepDirection(CVector3f(toTeamPos, 0.f));
    if (stepDir != pas::kSD_Forward) {
      BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(stepDir, pas::kStep_Normal));
    } else {
      BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(
          mSteeringBehaviors.Arrival(*this, teamPos, 3.f), CVector3f::Zero(), 1.f));
      CVector3f target(GetTranslation()[kDX], GetTranslation()[kDY], teamPos[kDZ]);
      CVector3f move = mSteeringBehaviors.Arrival(*this, target, 2.5f);
      if (move.MagSquared() > 0.01f) {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 3.f));
      }
    }
  } else {
    switch (mgr.Random()->Range(0, 2)) {
    case 0:
      BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Left, pas::kStep_Normal));
      break;
    case 1:
      BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Right, pas::kStep_Normal));
      break;
    case 2:
      if (ShouldTurn(mgr, 30.f) && delta.CanBeNormalized()) {
        BodyCtrl()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), delta.AsNormalized(), 1.f));
      }
      break;
    default:
      break;
    }
  }
  BodyCtrl()->CommandMgr().DeliverTargetVector(delta);
}

bool CWarWasp::SteerToDeactivatePos(CStateManager& mgr, EStateMsg msg, float dt) {
  CVector3f delta = mLatestLeashPosition - GetTranslation();
  float distanceSq = delta.MagSquared();
  float radius = mCSphere.GetSphere().GetRadius();
  if (distanceSq > 1.f + radius) {
    if (PathToHiveIsClear(mgr)) {
      CVector3f arrival = mSteeringBehaviors.Arrival(*this, mLatestLeashPosition, 15.f);
      float maxSpeed = BodyCtrl()->BodyStateInfo().GetMaxSpeed();
      float minMoveFactor;
      if (maxSpeed > 0.f) {
        minMoveFactor =
            (0.5f * BodyCtrl()->BodyStateInfo().GetLocomotionSpeed(pas::kLA_Walk)) / maxSpeed;
      } else {
        minMoveFactor = 1.f;
      }
      float moveFactor = CMath::Clamp(minMoveFactor, arrival.Magnitude(), 1.f);
      CVector3f move = arrival.CanBeNormalized() ? moveFactor * arrival.AsNormalized()
                                                 : moveFactor * GetTransform().GetForward();
      BodyCtrl()->CommandMgr().SetSteeringBlendMode(kSBM_FullSpeed);
      BodyCtrl()->CommandMgr().SetSteeringSpeedRange(moveFactor, moveFactor);
      if (distanceSq > 64.f + radius) {
        if (GetSearchPath() && !PathShagged(mgr, 0.f) && !GetSearchPath()->IsOver()) {
          CPatterned::PathFind(mgr, msg, dt);
        } else {
          BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
        }
      } else {
        RemoveMaterial(kMT_Solid, mgr);
        CVector3f target(GetTranslation()[kDX], GetTranslation()[kDY],
                         mLatestLeashPosition[kDZ]);
        CVector3f verticalMove = mSteeringBehaviors.Arrival(*this, target, 2.5f);
        if (verticalMove.MagSquared() > 0.01f) {
          BodyCtrl()->CommandMgr().DeliverCmd(
              CBCLocomotionCmd(verticalMove, CVector3f::Zero(), 3.f));
        }
        BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
      }
    }
    return false;
  } else if (distanceSq > 0.01f) {
    RemoveMaterial(kMT_Solid, mgr);
    CQuaternion targetRot = mInitialRot * CQuaternion::ZRotation(CRelAngle::FromRadians(M_PIF));
    CVector3f pos = CVector3f::Lerp(GetTranslation(), mLatestLeashPosition, 0.1f);
    CQuaternion rot =
        CQuaternion::SlerpLocal(CQuaternion::FromMatrix(GetTransform()), targetRot, 0.1f);
    SetTranslation(pos);
    SetRotation(rot.BuildNormalized());
    return false;
  }
  return true;
}

bool CWarWasp::PathToHiveIsClear(CStateManager& mgr) const {
  CVector3f pos = GetTranslation();
  CVector3f delta = mLatestLeashPosition - pos;
  CVector3f forward = GetTransform().GetForward();
  if (CVector3f::Dot(forward, delta) > 0.f) {
    CAABox bounds(pos - CVector3f(10.f, 10.f, 10.f), pos + CVector3f(10.f, 10.f, 10.f));
    TEntityList nearList;
    const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Character));
    mgr.BuildNearList(nearList, bounds, filter, nullptr);
    if (nearList.size() > 0) {
      float distanceSq = delta.MagSquared();
      for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
        if (const CWarWasp* other = PATTERNED_CAST_TO(CWarWasp, const_cast< CEntity* >(mgr.GetObjectById(*it)))) {
          if (other->GetUniqueId() != GetUniqueId() && other->mIsRetreating &&
              close_enough(other->mLatestLeashPosition, mLatestLeashPosition, 3.f)) {
            CVector3f otherDelta = other->GetTranslation() - GetTranslation();
            if (CVector3f::Dot(forward, otherDelta) > 0.f) {
              CVector3f toHive = other->GetTranslation() - mLatestLeashPosition;
              if (otherDelta.MagSquared() < 3.f && toHive.MagSquared() < distanceSq) {
                return false;
              }
            }
          }
        }
      }
    }
  }
  return true;
}

void CWarWasp::SetUpPathFindBehavior(CStateManager& mgr) {
  mPathObstructed = false;
  if (GetSearchPath()) {
    SwarmAdd(mgr);
    const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, mAiMgr, GetUniqueId());
    CVector3f pos =
        role ? role->GetTeamPosition() : GetCloseInPos(mgr, GetProjectileAimPos(mgr, -1.25f));
    SetDestPos(pos);
    const CVector3f& destPos = mDestPos;
    CVector3f delta = destPos - GetTranslation();
    if (delta.MagSquared() > 64.f || IsPatternObstructed(mgr, GetTranslation(), destPos)) {
      CVector3f aimPos = GetProjectileAimPos(mgr, -1.25f);
      CVector3f aimDelta = mDestPos - aimPos;
      if (aimDelta.CanBeNormalized()) {
        const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
            CMaterialList(kMT_Solid), CMaterialList(kMT_Player));
        float length = aimDelta.Magnitude();
        CVector3f dir = (1.f / length) * aimDelta;
        CRayCastResult res = mgr.RayStaticIntersection(aimPos, dir, length, filter);
        if (res.IsValid()) {
          SetDestPos(aimPos + 0.5f * (res.GetTime() * dir));
          mPathObstructed = true;
        }
      }
      CPatterned::PathFind(mgr, kStateMsg_Activate, 0.f);
    }
  }
}

void CWarWasp::UpdateTouchBounds() {
  CAABox bounds = GetAnimationData()->GetBoundingBox();
  mCSphere.SetSphereCenter(bounds.GetCenterPoint());
  CTransform4f xf(GetTransform().BuildMatrix3f(), CVector3f::Zero());
  SetBoundingBox(bounds.GetTransformedAABox(xf));
}

void CWarWasp::ApplyDamage(CStateManager& mgr) {
  if (mCanApplyDamage && BodyCtrl()->GetCurrentStateId() == pas::kAS_MeleeAttack) {
    CVector3f scale = GetModelScale();
    CVector3f locator = GetLocatorTransform(rstl::string(kStingLctrName)).GetTranslation();
    CVector3f pos = CVector3f::ByElementMultiply(scale, locator);
    pos = GetTransform() * pos;
    if (mgr.GetPlayer()->GetBoundingBox().PointInside(pos)) {
      mgr.ApplyDamage(
          GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), GetContactDamage(),
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
          CVector3f::Zero());
      mCanApplyDamage = false;
    }
  }
}

CVector3f CWarWasp::GetProjectileAimPos(const CStateManager& mgr, float zBias) const {
  CVector3f ret = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
  if (mgr.GetPlayer()->GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
    ret += CVector3f(0.f, 0.f, zBias);
  }
  return ret;
}

float CWarWasp::CalcTimeToNextAttack(CStateManager& mgr) const {
  float multiplier = 1.f;
  if (const CTeamAiMgr* teamMgr = TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(mAiMgr))) {
    uint maxCount = mFlavor == kFT_Two ? teamMgr->GetMaxProjectileAttackerCount()
                                           : teamMgr->GetMaxMeleeAttackerCount();
    uint count = mFlavor == kFT_Two
                     ? teamMgr->GetNumAssignedOfRole(CTeamAiRole::kTAR_Projectile)
                     : teamMgr->GetNumAssignedOfRole(CTeamAiRole::kTAR_Melee);
    if (count <= maxCount) {
      multiplier *= 0.5f;
    }
  }
  return multiplier * (mAttackTimeVariation * mgr.Random()->Float() + mAverageAttackTime);
}

CVector3f CWarWasp::CalcShuffleDest(const CStateManager& mgr) const {
  CVector3f aimPos = GetProjectileAimPos(mgr, -1.25f);
  CVector3f forward = mgr.GetPlayer()->GetTransform().GetForward();
  forward.SetZ(0.f);
  forward = forward.CanBeNormalized() ? forward.AsNormalized()
                                      : static_cast< const CVector3f& >(CVector3f::Forward());
  CVector3f dest = aimPos + (7.5f + mMaxAttackRange) * forward;
  dest.SetZ(GetCloseInZBasis(mgr));
  return dest;
}

float CWarWasp::GetCloseInZBasis(const CStateManager& mgr) const {
  return -0.5f + (mgr.GetPlayer()->GetTranslation()[kDZ] + mgr.GetPlayer()->GetEyeHeight());
}

CVector3f CWarWasp::GetCloseInPos(const CStateManager& mgr, const CVector3f& aimPos) const {
  float midRange = 0.5f * (mMinAttackRange + mMaxAttackRange);
  CVector3f ret = aimPos;
  if (mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
    ret += midRange * mgr.GetPlayer()->GetTransform().GetForward();
  } else {
    CVector3f delta = GetTranslation() - aimPos;
    ret += delta.CanBeNormalized() ? midRange * delta.AsNormalized()
                                   : midRange * GetTransform().GetForward();
  }
  ret.SetZ(0.5f + GetCloseInZBasis(mgr));
  return ret;
}

void CWarWasp::SetUpCircleBurstWaypoint(CStateManager& mgr) {
  const rstl::vector< SConnection >& conns = GetConnectionList();
  for (AUTO(it, conns.begin()); it != conns.end(); ++it) {
    if (it->mState == kSS_CloseIn && it->mMsg == kSM_Follow) {
      if (const CScriptWaypoint* waypoint = TCastToConstPtr< CScriptWaypoint >(
              mgr.GetObjectById(mgr.GetIdForScript(it->mObjId)))) {
        mCircleBurstPos = waypoint->GetTranslation();
        mCircleBurstDir = waypoint->GetTransform().GetForward();
        mCircleBurstRight = waypoint->GetTransform().GetRight();
        break;
      }
    }
  }
}

float CWarWasp::CalcSeekMagnitude(const CStateManager& mgr) const {
  float ret = 0.9f;
  ret *= ((mCircleAttackTeam >= 0 && mCircleAttackTeam < 3)
              ? skSeekMagTable[mCircleAttackTeam]
              : 1.f);
  if (const CTeamAiMgr* teamMgr = TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(mAiMgr))) {
    if (teamMgr->IsPartOfTeam(GetUniqueId()) && teamMgr->GetMaxMeleeAttackerCount() > 1) {
      if (GetUniqueId() != GetAttackTeamLeader(mgr, mCircleAttackTeam)) {
        float count = GetAttackTeamSize(mgr, mCircleAttackTeam) - 1;
        float spacing = CRelAngle::FromDegrees(40.f).AsRadians();
        float angle = (spacing * count) / count;
        float targetAngle = rstl::max_val(skMinAngle, angle);
        CVector3f fromCenter = GetTranslation() - mCircleBurstPos;
        CVector3f forward = GetTransform().GetForward();
        float minAngle = M_2PIF;
        const rstl::vector< CTeamAiRole >& roles = teamMgr->GetTeamAiRoles();
        for (AUTO(it, roles.begin()); it != roles.end(); ++it) {
          if (it->GetOwnerId() == GetUniqueId()) {
            continue;
          }
          if (const CWarWasp* other = PATTERNED_CAST_TO(CWarWasp, const_cast< CEntity* >(mgr.GetObjectById(it->GetOwnerId())))) {
            if (other->mCircleAttackTeam == mCircleAttackTeam) {
              CVector3f delta = other->GetTranslation() - GetTranslation();
              if (CVector3f::Dot(forward, delta) > 0.f) {
                CVector3f otherFromCenter = other->GetTranslation() - mCircleBurstPos;
                float angle = CVector3f::GetAngleDiff(fromCenter, otherFromCenter);
                if (angle < minAngle) {
                  minAngle = angle;
                }
              }
            }
          }
        }
        float tolerance = CRelAngle::FromDegrees(10.f).AsRadians();
        if (minAngle < targetAngle - tolerance) {
          ret = 0.8f;
        } else if (minAngle > tolerance + targetAngle) {
          ret = 1.f;
        }
      }
    }
  }
  return ret;
}

bool CWarWasp::CheckCircleAttackSpread(const CStateManager& mgr, int team) const {
  if (const CTeamAiMgr* teamMgr = TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(mAiMgr))) {
    int teamSize = GetAttackTeamSize(mgr, team);
    if (teamSize == 1) {
      return true;
    }
    TUniqueId leaderId = GetAttackTeamLeader(mgr, team);
    if (const CWarWasp* leader = PATTERNED_CAST_TO(CWarWasp, const_cast< CEntity* >(mgr.GetObjectById(leaderId)))) {
      CVector3f leaderPos = leader->GetTranslation();
      CVector3f leaderForward = leader->GetTransform().GetForward();
      CVector3f fromCenter = leaderPos - mCircleBurstPos;
      float maxAngle = 0.f;
      for (AUTO(it, teamMgr->GetTeamAiRoles().begin()); it != teamMgr->GetTeamAiRoles().end();
           it++) {
        if (it->GetOwnerId() == leaderId) {
          continue;
        }
        if (const CWarWasp* other = PATTERNED_CAST_TO(CWarWasp, const_cast< CEntity* >(mgr.GetObjectById(it->GetOwnerId())))) {
          if (team == other->mCircleAttackTeam) {
            CVector3f otherPos = other->GetTranslation();
            CVector3f delta = otherPos - leaderPos;
            if (CVector3f::Dot(leaderForward, delta) > 0.f) {
              return false;
            }
            CVector3f otherFromCenter = otherPos - mCircleBurstPos;
            float angle = CVector3f::GetAngleDiff(otherFromCenter, fromCenter);
            if (angle > maxAngle) {
              maxAngle = angle;
            }
          }
        }
      }
      float count = teamSize - 1;
      float spacing = CRelAngle::FromDegrees(40.f).AsRadians();
      return maxAngle < spacing * count + CRelAngle::FromDegrees(20.f).AsRadians();
    }
  }
  return false;
}

void CWarWasp::SetUpCircleTelegraphTeam(CStateManager& mgr) {
  if (mCircleAttackTeam == -1) {
    if (const CTeamAiMgr* teamMgr = TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(mAiMgr))) {
      if (teamMgr->IsPartOfTeam(GetUniqueId())) {
        const int maxCount = teamMgr->GetMaxMeleeAttackerCount();
        if (maxCount > 0) {
          int unit = 0;
          int count = 0;
          bool rejoinInitial = false;
          const rstl::vector< CTeamAiRole >& roles = teamMgr->GetTeamAiRoles();
          for (AUTO(it, roles.begin()); it != roles.end(); it++) {
            if (const CWarWasp* other = PATTERNED_CAST_TO(CWarWasp, const_cast< CEntity* >(mgr.GetObjectById(it->GetOwnerId())))) {
              if (mInitialCircleAttackTeam != -1 &&
                  mInitialCircleAttackTeam == other->mInitialCircleAttackTeam &&
                  other->mCircleAttackTeam >= 0) {
                unit = other->mCircleAttackTeam;
                rejoinInitial = true;
                break;
              }
              if (other->mCircleAttackTeam > unit) {
                unit = other->mCircleAttackTeam;
                count = 1;
              } else if (unit == other->mCircleAttackTeam) {
                ++count;
              }
            }
          }
          if (!rejoinInitial && (mInitialCircleAttackTeam != -1 || count >= maxCount)) {
            ++unit;
          }
          JoinCircleAttackTeam(unit, mgr);
          mCircleTelegraphSeekHeight = -0.5f * mgr.Random()->Float();
        }
      }
    }
  }
}

void CWarWasp::TryCircleTeamMerge(CStateManager& mgr) {
  int team = mCircleAttackTeam;
  if (team > 0) {
    if (const CTeamAiMgr* const teamMgr =
            TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(mAiMgr))) {
      if (teamMgr->IsPartOfTeam(GetUniqueId())) {
        if (GetAttackTeamLeader(mgr, team) == GetUniqueId() &&
            GetAttackTeamSize(mgr, team - 1) == 0) {
          const rstl::vector< CTeamAiRole >& roles = teamMgr->GetTeamAiRoles();
          for (AUTO(it, roles.begin()); it != roles.end(); it++) {
            if (CWarWasp* other = PATTERNED_CAST_TO(CWarWasp, mgr.ObjectById(it->GetOwnerId()))) {
              if (team == other->mCircleAttackTeam) {
                other->JoinCircleAttackTeam(team - 1, mgr);
              }
            }
          }
        }
      }
    }
  }
}

TUniqueId CWarWasp::GetAttackTeamLeader(const CStateManager& mgr, int team) const {
  if (const CTeamAiMgr* teamMgr = TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(mAiMgr))) {
    if (teamMgr->IsPartOfTeam(GetUniqueId())) {
      for (AUTO(it, teamMgr->GetTeamAiRoles().begin()); it != teamMgr->GetTeamAiRoles().end();
           it++) {
        if (const CWarWasp* other = PATTERNED_CAST_TO(CWarWasp, const_cast< CEntity* >(mgr.GetObjectById(it->GetOwnerId())))) {
          if (team == other->mCircleAttackTeam) {
            return it->GetOwnerId();
          }
        }
      }
    }
  }
  return kInvalidUniqueId;
}

int CWarWasp::GetAttackTeamSize(const CStateManager& mgr, int team) const {
  int count = 0;
  if (const CTeamAiMgr* teamMgr = TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(mAiMgr))) {
    if (teamMgr->IsPartOfTeam(GetUniqueId())) {
      for (AUTO(it, teamMgr->GetTeamAiRoles().begin()); it != teamMgr->GetTeamAiRoles().end();
           it++) {
        if (const CWarWasp* other = PATTERNED_CAST_TO(CWarWasp, const_cast< CEntity* >(mgr.GetObjectById(it->GetOwnerId())))) {
          if (team == other->mCircleAttackTeam) {
            ++count;
          }
        }
      }
    }
  }
  return count;
}

float CWarWasp::GetTeamZStratum(int team) const {
  float ret = 0.f;
  if (team > 0) {
    if ((team & 1) == 1) {
      ret = -3.f - 3.f * static_cast< float >(team >> 1);
    } else {
      ret = 3.f * static_cast< float >(team >> 1);
    }
  }
  return ret;
}

float CWarWasp::CalcOffTotemAngle(CStateManager& mgr) const {
  float angle = 1.3962636f * mgr.Random()->Float() + 0.17453292f;
  return angle;
}

void CWarWasp::UpdateTelegraphMoveSpeed(CStateManager& mgr) {
  TUniqueId leaderId = GetAttackTeamLeader(mgr, mCircleAttackTeam);
  if (const CWarWasp* leader = PATTERNED_CAST_TO(CWarWasp, const_cast< CEntity* >(mgr.GetObjectById(leaderId)))) {
    if (leaderId == GetUniqueId()) {
      float time = mStateMachineState.GetTime();
      float cycleTime = CMath::FastFmod(time, 2.8f);
      if (cycleTime < 2.f) {
        mSpeed = mInitialSpeed;
      } else {
        float t = (cycleTime - 2.f) / 0.8f;
        mSpeed = ((1.f - t) * 0.7f + 2.f * t) * mInitialSpeed;
      }
    } else {
      mSpeed = leader->mSpeed;
    }
  } else {
    mSpeed = mInitialSpeed;
  }
}

bool CWarWasp::IsListening() const { return true; }

CPathFindSearch* CWarWasp::GetSearchPath() { return &mPfSearch; }
