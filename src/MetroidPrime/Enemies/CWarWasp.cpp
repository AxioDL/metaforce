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
, x568_stateProg(-1)
, x570_cSphere(CSphere(CVector3f(0.f, 0.f, 1.8f), 1.f), GetMaterialList())
, x590_pfSearch(nullptr, 3, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x674_aiMgr(kInvalidUniqueId)
, x678_targetPos(CVector3f::Zero())
, x684_contactDamage(dInfo)
, x6a0_initialRot(CQuaternion::NoRotation())
, x6b0_circleBurstPos(CVector3f::Zero())
, x6bc_circleBurstDir(CVector3f(0.f, 0.f, 0.f))
, x6c8_circleBurstRight(CVector3f(0.f, 0.f, 0.f))
, x6d4_projectileInfo(projectileWeapon, projectileDamage)
, x6fc_initialSpeed(x3b4_speed)
, x700_attackRemTime(0.f)
, x704_dodgeDir(pas::kSD_Invalid)
, x708_circleAttackTeam(-1)
, x70c_initialCircleAttackTeam(-1)
, x710_initialCircleAttackTeamUnit(-1)
, x714_circleTelegraphSeekHeight(0.f)
, x718_circleBurstOffTotemAngle(1.5707964f)
, x72c_projectileVisorSfx(CSfxManager::TranslateSFXID(projectileVisorSfx))
, x72e_24_jumpBackRepeat(true)
, x72e_25_canApplyDamage(false)
, x72e_26_initiallyInactive(!pInfo.GetActive())
, x72e_27_teamMatesMelee(false)
, x72e_28_inProjectileAttack(false)
, x72e_29_pathObstructed(false)
, x72e_30_isRetreating(false)
, x72e_31_heardNoise(false) {
  if (flavor == kFT_Two) {
    x6d4_projectileInfo.Token().Lock();
  }
  UpdateTouchBounds();
  SetCoefficientOfRestitutionModifier(0.1f);
  if (projectileVisorParticle != kInvalidAssetId) {
    x71c_projectileVisorParticle =
        gpSimplePool->GetObj(SObjectTag('PART', projectileVisorParticle));
  }
  x328_29_noPatternShagging = true;
  x460_knockBackController.SetAnimationStateRange(kAR_KnockBack, kAR_KnockBack);
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
    if (x674_aiMgr == kInvalidUniqueId) {
      x674_aiMgr = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
    }

    const TAreaId aid = GetCurrentAreaId();
    x590_pfSearch.SetArea(mgr.World()->GetAreaAlways(aid).GetPostConstructed()->x10bc_pathArea);
    if (!x6b0_circleBurstPos.IsNonZero()) {
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
  x328_25_verticalMovement = false;
  AddMaterial(kMT_GroundCollider, mgr);
  SwarmRemove(mgr);
}

void CWarWasp::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (x700_attackRemTime > 0.f) {
    const CPlayer* player = mgr.GetPlayer();
    float multiplier = 1.f;
    if (player->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
      const CVector3f distance = GetTranslation() - player->GetTranslation();
      const CVector3f playerForward = player->GetTransform().GetForward();
      multiplier = 1.f - (CVector2f::GetAngleDiff(playerForward.ToVec2f(), distance.ToVec2f()) /
                          M_PIF * 0.666f);
    }

    x700_attackRemTime -= dt * multiplier;
  }

  ApplyDamage(mgr);
  CPatterned::Think(dt, mgr);
}

CProjectileInfo* CWarWasp::ProjectileInfo() { return &x6d4_projectileInfo; }

rstl::optional_object< CAABox > CWarWasp::GetTouchBounds() const {
  return x570_cSphere.CalculateAABox(GetTransform());
}

const CCollisionPrimitive* CWarWasp::GetCollisionPrimitive() const { return &x570_cSphere; }

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
    LaunchProjectile(projectileXf, mgr, 4, CWeapon::kPA_None, false, x71c_projectileVisorParticle,
                     x72c_projectileVisorSfx, true, CVector3f(1.f, 1.f, 1.f));
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
    x72e_25_canApplyDamage = true;
    break;
  case kUE_DamageOff:
    x72e_25_canApplyDamage = false;
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
    if (diff.MagSquared() < x3bc_detectionRange * x3bc_detectionRange) {
      x72e_31_heardNoise = true;
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
  if (!x6b0_circleBurstPos.IsNonZero()) {
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
  if (x72e_31_heardNoise || x400_24_hitByPlayerProjectile) {
    return true;
  }

  if (const CTeamAiMgr* teamMgr = TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(x674_aiMgr))) {
    return teamMgr->GetNumRoles() != 0;
  }

  return false;
}

bool CWarWasp::InAttackPosition(CStateManager& mgr, float arg) {
  CVector3f pos = GetTranslation();
  CVector3f aimPos = GetProjectileAimPos(mgr, -1.25f);
  CVector3f delta = aimPos - pos;
  float distanceSq = delta.MagSquared();
  float minRange = x2fc_minAttackRange - 5.f;
  bool ret = distanceSq > minRange * minRange &&
             distanceSq < (5.f + x300_maxAttackRange) * (5.f + x300_maxAttackRange) &&
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
  if (x700_attackRemTime <= 0.f) {
    if (const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x674_aiMgr, GetUniqueId())) {
      if (role->GetTeamAiRole() == CTeamAiRole::kTAR_Melee && !mgr.GetPlayer()->IsInsideFluid()) {
        if (CTeamAiMgr* teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x674_aiMgr))) {
          if (!teamMgr->HasMeleeAttackers()) {
            return !IsPatternObstructed(mgr, GetTranslation(), GetProjectileAimPos(mgr, -1.25f));
          }
        }
      }
    } else if (x3fc_flavor != kFT_Two && !mgr.GetPlayer()->IsInsideFluid()) {
      return !IsPatternObstructed(mgr, GetTranslation(), GetProjectileAimPos(mgr, -1.25f));
    }
  }
  return false;
}

bool CWarWasp::ShouldFire(CStateManager& mgr, float arg) {
  if (x700_attackRemTime <= 0.f) {
    if (const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x674_aiMgr, GetUniqueId())) {
      if (role->GetTeamAiRole() == CTeamAiRole::kTAR_Projectile) {
        CVector3f aimPos = GetProjectileAimPos(mgr, -1.25f);
        CVector3f delta = aimPos - GetTranslation();
        CVector3f forward = GetTransform().GetForward();
        if (delta.CanBeNormalized() && CVector3f::Dot(forward, delta.AsNormalized()) >= 0.906f) {
          if (CTeamAiMgr* teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x674_aiMgr))) {
            return !teamMgr->HasProjectileAttackers();
          }
        }
      }
    } else if (x3fc_flavor == kFT_Two) {
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
  if (x708_circleAttackTeam == 0 && !mgr.GetPlayer()->IsInsideFluid()) {
    if (const CTeamAiMgr* teamMgr = TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(x674_aiMgr))) {
      if (CheckCircleAttackSpread(mgr, x708_circleAttackTeam)) {
        TUniqueId leaderId = GetAttackTeamLeader(mgr, x708_circleAttackTeam);
        if (leaderId == GetUniqueId()) {
          if (x700_attackRemTime <= 0.f) {
            CVector2f delta =
                mgr.GetPlayer()->GetTranslation().DropZ() - x6b0_circleBurstPos.DropZ();
            if (delta.MagSquared() < 90.25f) {
              CVector3f fromCenter = GetTranslation() - x6b0_circleBurstPos;
              CVector3f threshold = x6bc_circleBurstDir;
              if (x718_circleBurstOffTotemAngle <= M_PIF / 2.f) {
                threshold = CVector3f::Slerp(x6c8_circleBurstRight, x6bc_circleBurstDir,
                                             CRelAngle::FromRadians(x718_circleBurstOffTotemAngle));
              } else {
                threshold = CVector3f::Slerp(
                    x6bc_circleBurstDir, -x6c8_circleBurstRight,
                    CRelAngle::FromRadians(x718_circleBurstOffTotemAngle - M_PIF / 2.f));
              }
              if (CVector3f::GetAngleDiff(threshold, fromCenter) <
                  CRelAngle::FromDegrees(10.f).AsRadians()) {
                return CTeamAiMgr::AddAttacker(kAT_Melee, mgr, x674_aiMgr, GetUniqueId());
              }
            }
          }
        } else {
          if (const CWarWasp* leader = CPatterned::CastTo(TPatternedCast< CWarWasp >(
                  const_cast< CEntity* >(mgr.GetObjectById(leaderId))))) {
            if (leader->x72e_27_teamMatesMelee) {
              return CTeamAiMgr::AddAttacker(kAT_Melee, mgr, x674_aiMgr, GetUniqueId());
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
            x704_dodgeDir = CVector3f::Dot(right, delta) > 0.f ? pas::kSD_Right : pas::kSD_Left;
            return true;
          }
        }
      }
    }
  }
  return false;
}

bool CWarWasp::Leash(CStateManager& mgr, float arg) {
  const CVector3f leashDelta = x3a0_latestLeashPosition - GetTranslation();
  if (leashDelta.MagSquared() > x3c8_leashRadius * x3c8_leashRadius) {
    const CVector3f playerDelta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    return playerDelta.MagSquared() > x3cc_playerLeashRadius * x3cc_playerLeashRadius &&
           x3d4_curPlayerLeashTime > x3d0_playerLeashTime;
  }
  return false;
}

bool CWarWasp::InPosition(CStateManager& mgr, float arg) {
  if (GetSearchPath()) {
    return GetSearchPath()->IsOver();
  }
  CVector3f delta = x678_targetPos - GetTranslation();
  return delta.MagSquared() < 1.f;
}

bool CWarWasp::AnimOver(CStateManager& mgr, float arg) { return x568_stateProg == 3; }

void CWarWasp::Generate(CStateManager& mgr, EStateMsg msg, float dt) {
  CBodyController* const bc = BodyCtrl();
  switch (msg) {
  case kStateMsg_Activate:
    bc->Activate(mgr);
    if (x72e_26_initiallyInactive) {
      RemoveMaterial(kMT_Character, kMT_Solid, mgr);
      x6a0_initialRot = CQuaternion::FromMatrix(GetTransform());
      CQuaternion rot = x6a0_initialRot;
      rot *= CQuaternion::ZRotation(
          CRelAngle::FromRadians((M_PIF / 4.f) * mgr.Random()->Float() - M_PIF / 8.f));
      SetRotation(rot.BuildNormalized());
      x568_stateProg = 0;
    } else {
      x568_stateProg = 3;
    }
    break;
  case kStateMsg_Update:
    switch (x568_stateProg) {
    case 0:
      if (bc->GetCurrentStateId() == pas::kAS_Generate) {
        x568_stateProg = 2;
      } else {
        const int anim = x388_anim;
        CBCGenerateCmd cmd(pas::kGType_Zero, anim);
        bc->CommandMgr().DeliverCmd(cmd);
      }
      break;
    case 2:
      if (bc->GetCurrentStateId() != pas::kAS_Generate) {
        x568_stateProg = 3;
      } else {
        CVector3f move = CVector3f::Zero();
        const CObjectList& list = mgr.GetObjectListById(kOL_ListeningAi);
        for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
          if (const CPatterned* other = TCastToPtr< CPatterned >(const_cast< CEntity* >(list[i]))) {
            if (other != this && other->GetCurrentAreaId() == GetCurrentAreaId()) {
              move += (5.f * dt) *
                      x45c_steeringBehaviors.Separation(*this, other->GetTranslation(), 5.f);
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
    if (x72e_26_initiallyInactive) {
      AddMaterial(kMT_Character, kMT_Solid, mgr);
      if (x328_26_solidCollision) {
        x401_30_pendingDeath = true;
      }
    }
    break;
  }
}

void CWarWasp::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x72e_27_teamMatesMelee = true;
    x72e_25_canApplyDamage = false;
    if (x674_aiMgr != kInvalidUniqueId) {
      bool added = CTeamAiMgr::AddAttacker(kAT_Melee, mgr, x674_aiMgr, GetUniqueId());
      int state = 3;
      if (added) {
        state = 0;
      }
      x568_stateProg = state;
    } else {
      x568_stateProg = 0;
    }
    break;
  case kStateMsg_Update:
    switch (x568_stateProg) {
    case 0:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_MeleeAttack) {
        x568_stateProg = 2;
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
        x568_stateProg = 3;
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
    CTeamAiMgr::ResetTeamAiRole(kAT_Melee, mgr, x674_aiMgr, GetUniqueId(), false);
    x700_attackRemTime = CalcTimeToNextAttack(mgr);
    x72e_27_teamMatesMelee = false;
    break;
  }
}

void CWarWasp::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x72e_28_inProjectileAttack = true;
    if (x674_aiMgr != kInvalidUniqueId) {
      bool added = CTeamAiMgr::AddAttacker(kAT_Projectile, mgr, x674_aiMgr, GetUniqueId());
      int state = 3;
      if (added) {
        state = 0;
      }
      x568_stateProg = state;
    } else {
      x568_stateProg = 0;
    }
    break;
  case kStateMsg_Update:
    switch (x568_stateProg) {
    case 0:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_ProjectileAttack) {
        x568_stateProg = 2;
      } else {
        SetDestPos(GetProjectileAimPos(mgr, -0.07f));
        BodyCtrl()->CommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::kS_One, x2e0_destPos, false));
      }
      break;
    case 2:
      if (BodyCtrl()->GetCurrentStateId() != pas::kAS_ProjectileAttack) {
        BodyCtrl()->CommandMgr().DeliverTargetVector(x2e0_destPos - GetTranslation());
        x568_stateProg = 3;
      }
      break;
    default:
      break;
    }
    break;
  case kStateMsg_Deactivate:
    CTeamAiMgr::ResetTeamAiRole(kAT_Projectile, mgr, x674_aiMgr, GetUniqueId(), false);
    x700_attackRemTime = CalcTimeToNextAttack(mgr);
    x72e_28_inProjectileAttack = false;
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
    float range = 2.f * x300_maxAttackRange;
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
    x568_stateProg = 2;
    break;
  case kStateMsg_Update: {
    int numRoles = 0;
    if (const CTeamAiMgr* teamMgr = TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(x674_aiMgr))) {
      numRoles = teamMgr->GetNumAssignedAiRoles();
    }
    if (numRoles != 0u && x700_attackRemTime > 0.f) {
      CVector3f shuffleDest = CalcShuffleDest(mgr);
      float zDelta = shuffleDest.GetZ() - GetTranslation().GetZ();
      const CVector3f& pos = GetTranslation();
      if (zDelta * zDelta > 1.f) {
        CVector3f dest(GetTranslation()[kDX], GetTranslation()[kDY], shuffleDest[kDZ]);
        CVector3f move = x45c_steeringBehaviors.Arrival(*this, dest, 1.f);
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
                x45c_steeringBehaviors.Seek(*this, shuffleDest), CVector3f::Zero(), 1.f));
            ApplySeparationBehavior(mgr, 15.f);
          }
        } else {
          x568_stateProg = 3;
        }
      }
      BodyCtrl()->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                   GetTranslation());
    } else {
      x568_stateProg = 3;
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
    x568_stateProg = 0;
    x72e_24_jumpBackRepeat = true;
    SwarmRemove(mgr);
    break;
  case kStateMsg_Update:
    switch (x568_stateProg) {
    case 0:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_Step) {
        x568_stateProg = 2;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Backward, pas::kStep_Normal));
      }
      break;
    case 2:
      if (BodyCtrl()->GetCurrentStateId() != pas::kAS_Step) {
        int state = 3;
        if (x72e_24_jumpBackRepeat) {
          state = 0;
        }
        x568_stateProg = state;
        x72e_24_jumpBackRepeat = false;
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
        CVector3f(x45c_steeringBehaviors.Flee2D(*this, mgr.GetPlayer()->GetTranslation().DropZ()),
                  0.f),
        CVector3f::Zero(), 1.f));
    break;
  case kStateMsg_Deactivate:
    x400_24_hitByPlayerProjectile = false;
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
    x72e_31_heardNoise = false;
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
    x568_stateProg = 1;
    x72e_30_isRetreating = true;
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    SwarmRemove(mgr);
    x674_aiMgr = kInvalidUniqueId;
    x678_targetPos = x3a0_latestLeashPosition;
    SetDestPos(x678_targetPos);
    if (GetSearchPath()) {
      CPatterned::PathFind(mgr, msg, dt);
    }
    break;
  case kStateMsg_Update:
    switch (x568_stateProg) {
    case 1:
      if (SteerToDeactivatePos(mgr, msg, dt)) {
        RemoveMaterial(kMT_Solid, mgr);
        x568_stateProg = 0;
      }
      break;
    case 0:
      if (bc.GetCurrentStateId() == pas::kAS_Generate) {
        RemoveMaterial(kMT_Character, kMT_Target, kMT_Orbit, mgr);
        mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
        x568_stateProg = 2;
      } else {
        bc.CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_One, -1));
      }
      break;
    case 2:
      if (bc.GetCurrentStateId() != pas::kAS_Generate) {
        x568_stateProg = 3;
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
    if (x704_dodgeDir != pas::kSD_Invalid) {
      BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(x704_dodgeDir, pas::kStep_Dodge));
      x568_stateProg = 2;
    }
    break;
  case kStateMsg_Update:
    if (BodyCtrl()->GetCurrentStateId() != pas::kAS_Step) {
      x568_stateProg = 3;
    } else {
      BodyCtrl()->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                   GetTranslation());
    }
    break;
  case kStateMsg_Deactivate:
    x704_dodgeDir = pas::kSD_Invalid;
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
    x678_targetPos = x2e0_destPos;
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
    if (x6b0_circleBurstPos.IsNonZero()) {
      TryCircleTeamMerge(mgr);
      CVector3f delta = GetTranslation() - x6b0_circleBurstPos;
      delta.SetZ(0.f);
      CVector3f move = GetTransform().GetForward();
      if (delta.CanBeNormalized()) {
        CVector3f dir = delta.AsNormalized();
        move = CVector3f::Cross(dir, CVector3f::Up());
        CVector3f radial = x2fc_minAttackRange * dir;
        CVector3f seekOrigin = x6b0_circleBurstPos + radial;
        if (x708_circleAttackTeam > 0) {
          move = -1.f * move;
        }
        float seekHeight = x714_circleTelegraphSeekHeight + GetTeamZStratum(x708_circleAttackTeam);
        float seekMag = CalcSeekMagnitude(mgr);
        CVector3f seekDest = seekOrigin + 5.f * move;
        move = seekMag *
               x45c_steeringBehaviors.Seek(*this, seekDest + CVector3f(0.f, 0.f, seekHeight));
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
    x72e_27_teamMatesMelee = true;
    x72e_25_canApplyDamage = true;
    x568_stateProg = 0;
    x3b4_speed = x6fc_initialSpeed;
    break;
  case kStateMsg_Update:
    switch (x568_stateProg) {
    case 0:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_MeleeAttack) {
        x568_stateProg = 2;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_Eight));
      }
      break;
    case 2:
      if (BodyCtrl()->GetCurrentStateId() != pas::kAS_MeleeAttack) {
        x568_stateProg = 3;
      } else {
        CVector3f delta = x6b0_circleBurstPos - GetTranslation();
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
    CTeamAiMgr::ResetTeamAiRole(kAT_Melee, mgr, x674_aiMgr, GetUniqueId(), false);
    x72e_27_teamMatesMelee = false;
    x708_circleAttackTeam = -1;
    x718_circleBurstOffTotemAngle = CalcOffTotemAngle(mgr);
    break;
  }
}

void CWarWasp::JoinCircleAttackTeam(int unit, CStateManager& mgr) {
  if (x6b0_circleBurstPos.IsNonZero()) {
    if (x70c_initialCircleAttackTeam == -1) {
      x710_initialCircleAttackTeamUnit = GetAttackTeamSize(mgr, unit);
      x70c_initialCircleAttackTeam = unit;
    }
    x708_circleAttackTeam = unit;
    x700_attackRemTime = CalcTimeToNextAttack(mgr);
    x718_circleBurstOffTotemAngle = CalcOffTotemAngle(mgr);
  }
}

void CWarWasp::SwarmAdd(CStateManager& mgr) {
  if (x674_aiMgr != kInvalidUniqueId) {
    if (CTeamAiMgr* teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x674_aiMgr))) {
      CTeamAiRole::ETeamAiRole role =
          x3fc_flavor == kFT_Two ? CTeamAiRole::kTAR_Projectile : CTeamAiRole::kTAR_Melee;
      if (!teamMgr->IsPartOfTeam(GetUniqueId())) {
        teamMgr->AssignTeamAiRole(*this, role, CTeamAiRole::kTAR_Invalid,
                                  CTeamAiRole::kTAR_Invalid);
      }
    }
  }
}

void CWarWasp::SwarmRemove(CStateManager& mgr) {
  if (x674_aiMgr != kInvalidUniqueId) {
    if (CTeamAiMgr* teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x674_aiMgr))) {
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
                CTeamAiMgr::GetTeamAiRole(mgr, x674_aiMgr, other->GetUniqueId())) {
          if (role->GetTeamAiRole() == CTeamAiRole::kTAR_Melee ||
              role->GetTeamAiRole() == CTeamAiRole::kTAR_Projectile) {
            useSep *= 2.f;
          }
        }
        CVector3f separation =
            x45c_steeringBehaviors.Separation(*this, other->GetTranslation(), useSep);
        if (separation.IsNonZero()) {
          BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(separation, CVector3f::Zero(), 1.f));
        }
      }
    }
  }
}

void CWarWasp::ApplyNormalSteering(CStateManager& mgr) {
  CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x674_aiMgr, GetUniqueId());
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
          x45c_steeringBehaviors.Arrival(*this, teamPos, 3.f), CVector3f::Zero(), 1.f));
      CVector3f target(GetTranslation()[kDX], GetTranslation()[kDY], teamPos[kDZ]);
      CVector3f move = x45c_steeringBehaviors.Arrival(*this, target, 2.5f);
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
  CVector3f delta = x3a0_latestLeashPosition - GetTranslation();
  float distanceSq = delta.MagSquared();
  float radius = x570_cSphere.GetSphere().GetRadius();
  if (distanceSq > 1.f + radius) {
    if (PathToHiveIsClear(mgr)) {
      CVector3f arrival = x45c_steeringBehaviors.Arrival(*this, x3a0_latestLeashPosition, 15.f);
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
                         x3a0_latestLeashPosition[kDZ]);
        CVector3f verticalMove = x45c_steeringBehaviors.Arrival(*this, target, 2.5f);
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
    CQuaternion targetRot = x6a0_initialRot * CQuaternion::ZRotation(CRelAngle::FromRadians(M_PIF));
    CVector3f pos = CVector3f::Lerp(GetTranslation(), x3a0_latestLeashPosition, 0.1f);
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
  CVector3f delta = x3a0_latestLeashPosition - pos;
  CVector3f forward = GetTransform().GetForward();
  if (CVector3f::Dot(forward, delta) > 0.f) {
    CAABox bounds(pos - CVector3f(10.f, 10.f, 10.f), pos + CVector3f(10.f, 10.f, 10.f));
    TEntityList nearList;
    const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Character));
    mgr.BuildNearList(nearList, bounds, filter, nullptr);
    if (nearList.size() > 0) {
      float distanceSq = delta.MagSquared();
      for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
        if (const CWarWasp* other = CPatterned::CastTo(
                TPatternedCast< CWarWasp >(const_cast< CEntity* >(mgr.GetObjectById(*it))))) {
          if (other->GetUniqueId() != GetUniqueId() && other->x72e_30_isRetreating &&
              close_enough(other->x3a0_latestLeashPosition, x3a0_latestLeashPosition, 3.f)) {
            CVector3f otherDelta = other->GetTranslation() - GetTranslation();
            if (CVector3f::Dot(forward, otherDelta) > 0.f) {
              CVector3f toHive = other->GetTranslation() - x3a0_latestLeashPosition;
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
  x72e_29_pathObstructed = false;
  if (GetSearchPath()) {
    SwarmAdd(mgr);
    const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x674_aiMgr, GetUniqueId());
    CVector3f pos =
        role ? role->GetTeamPosition() : GetCloseInPos(mgr, GetProjectileAimPos(mgr, -1.25f));
    SetDestPos(pos);
    const CVector3f& destPos = x2e0_destPos;
    CVector3f delta = destPos - GetTranslation();
    if (delta.MagSquared() > 64.f || IsPatternObstructed(mgr, GetTranslation(), destPos)) {
      CVector3f aimPos = GetProjectileAimPos(mgr, -1.25f);
      CVector3f aimDelta = x2e0_destPos - aimPos;
      if (aimDelta.CanBeNormalized()) {
        const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
            CMaterialList(kMT_Solid), CMaterialList(kMT_Player));
        float length = aimDelta.Magnitude();
        CVector3f dir = (1.f / length) * aimDelta;
        CRayCastResult res = mgr.RayStaticIntersection(aimPos, dir, length, filter);
        if (res.IsValid()) {
          SetDestPos(aimPos + 0.5f * (res.GetTime() * dir));
          x72e_29_pathObstructed = true;
        }
      }
      CPatterned::PathFind(mgr, kStateMsg_Activate, 0.f);
    }
  }
}

void CWarWasp::UpdateTouchBounds() {
  CAABox bounds = GetAnimationData()->GetBoundingBox();
  x570_cSphere.SetSphereCenter(bounds.GetCenterPoint());
  CTransform4f xf(GetTransform().BuildMatrix3f(), CVector3f::Zero());
  SetBoundingBox(bounds.GetTransformedAABox(xf));
}

void CWarWasp::ApplyDamage(CStateManager& mgr) {
  if (x72e_25_canApplyDamage && BodyCtrl()->GetCurrentStateId() == pas::kAS_MeleeAttack) {
    CVector3f scale = GetModelScale();
    CVector3f locator = GetLocatorTransform(rstl::string(kStingLctrName)).GetTranslation();
    CVector3f pos = CVector3f::ByElementMultiply(scale, locator);
    pos = GetTransform() * pos;
    if (mgr.GetPlayer()->GetBoundingBox().PointInside(pos)) {
      mgr.ApplyDamage(
          GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), GetContactDamage(),
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
          CVector3f::Zero());
      x72e_25_canApplyDamage = false;
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
  if (const CTeamAiMgr* teamMgr = TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(x674_aiMgr))) {
    uint maxCount = x3fc_flavor == kFT_Two ? teamMgr->GetMaxProjectileAttackerCount()
                                           : teamMgr->GetMaxMeleeAttackerCount();
    uint count = x3fc_flavor == kFT_Two
                     ? teamMgr->GetNumAssignedOfRole(CTeamAiRole::kTAR_Projectile)
                     : teamMgr->GetNumAssignedOfRole(CTeamAiRole::kTAR_Melee);
    if (count <= maxCount) {
      multiplier *= 0.5f;
    }
  }
  return multiplier * (x308_attackTimeVariation * mgr.Random()->Float() + x304_averageAttackTime);
}

CVector3f CWarWasp::CalcShuffleDest(const CStateManager& mgr) const {
  CVector3f aimPos = GetProjectileAimPos(mgr, -1.25f);
  CVector3f forward = mgr.GetPlayer()->GetTransform().GetForward();
  forward.SetZ(0.f);
  forward = forward.CanBeNormalized() ? forward.AsNormalized()
                                      : static_cast< const CVector3f& >(CVector3f::Forward());
  CVector3f dest = aimPos + (7.5f + x300_maxAttackRange) * forward;
  dest.SetZ(GetCloseInZBasis(mgr));
  return dest;
}

float CWarWasp::GetCloseInZBasis(const CStateManager& mgr) const {
  return -0.5f + (mgr.GetPlayer()->GetTranslation()[kDZ] + mgr.GetPlayer()->GetEyeHeight());
}

CVector3f CWarWasp::GetCloseInPos(const CStateManager& mgr, const CVector3f& aimPos) const {
  float midRange = 0.5f * (x2fc_minAttackRange + x300_maxAttackRange);
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
    if (it->x0_state == kSS_CloseIn && it->x4_msg == kSM_Follow) {
      if (const CScriptWaypoint* waypoint = TCastToConstPtr< CScriptWaypoint >(
              mgr.GetObjectById(mgr.GetIdForScript(it->x8_objId)))) {
        x6b0_circleBurstPos = waypoint->GetTranslation();
        x6bc_circleBurstDir = waypoint->GetTransform().GetForward();
        x6c8_circleBurstRight = waypoint->GetTransform().GetRight();
        break;
      }
    }
  }
}

float CWarWasp::CalcSeekMagnitude(const CStateManager& mgr) const {
  float ret = 0.9f;
  ret *= ((x708_circleAttackTeam >= 0 && x708_circleAttackTeam < 3)
              ? skSeekMagTable[x708_circleAttackTeam]
              : 1.f);
  if (const CTeamAiMgr* teamMgr = TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(x674_aiMgr))) {
    if (teamMgr->IsPartOfTeam(GetUniqueId()) && teamMgr->GetMaxMeleeAttackerCount() > 1) {
      if (GetUniqueId() != GetAttackTeamLeader(mgr, x708_circleAttackTeam)) {
        float count = GetAttackTeamSize(mgr, x708_circleAttackTeam) - 1;
        float spacing = CRelAngle::FromDegrees(40.f).AsRadians();
        float angle = (spacing * count) / count;
        float targetAngle = rstl::max_val(skMinAngle, angle);
        CVector3f fromCenter = GetTranslation() - x6b0_circleBurstPos;
        CVector3f forward = GetTransform().GetForward();
        float minAngle = M_2PIF;
        const rstl::vector< CTeamAiRole >& roles = teamMgr->GetTeamAiRoles();
        for (AUTO(it, roles.begin()); it != roles.end(); ++it) {
          if (it->GetOwnerId() == GetUniqueId()) {
            continue;
          }
          if (const CWarWasp* other = CPatterned::CastTo(TPatternedCast< CWarWasp >(
                  const_cast< CEntity* >(mgr.GetObjectById(it->GetOwnerId()))))) {
            if (other->x708_circleAttackTeam == x708_circleAttackTeam) {
              CVector3f delta = other->GetTranslation() - GetTranslation();
              if (CVector3f::Dot(forward, delta) > 0.f) {
                CVector3f otherFromCenter = other->GetTranslation() - x6b0_circleBurstPos;
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
  if (const CTeamAiMgr* teamMgr = TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(x674_aiMgr))) {
    int teamSize = GetAttackTeamSize(mgr, team);
    if (teamSize == 1) {
      return true;
    }
    TUniqueId leaderId = GetAttackTeamLeader(mgr, team);
    if (const CWarWasp* leader = CPatterned::CastTo(
            TPatternedCast< CWarWasp >(const_cast< CEntity* >(mgr.GetObjectById(leaderId))))) {
      CVector3f leaderPos = leader->GetTranslation();
      CVector3f leaderForward = leader->GetTransform().GetForward();
      CVector3f fromCenter = leaderPos - x6b0_circleBurstPos;
      float maxAngle = 0.f;
      for (AUTO(it, teamMgr->GetTeamAiRoles().begin()); it != teamMgr->GetTeamAiRoles().end();
           it++) {
        if (it->GetOwnerId() == leaderId) {
          continue;
        }
        if (const CWarWasp* other = CPatterned::CastTo(TPatternedCast< CWarWasp >(
                const_cast< CEntity* >(mgr.GetObjectById(it->GetOwnerId()))))) {
          if (team == other->x708_circleAttackTeam) {
            CVector3f otherPos = other->GetTranslation();
            CVector3f delta = otherPos - leaderPos;
            if (CVector3f::Dot(leaderForward, delta) > 0.f) {
              return false;
            }
            CVector3f otherFromCenter = otherPos - x6b0_circleBurstPos;
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
  if (x708_circleAttackTeam == -1) {
    if (const CTeamAiMgr* teamMgr = TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(x674_aiMgr))) {
      if (teamMgr->IsPartOfTeam(GetUniqueId())) {
        const int maxCount = teamMgr->GetMaxMeleeAttackerCount();
        if (maxCount > 0) {
          int unit = 0;
          int count = 0;
          bool rejoinInitial = false;
          const rstl::vector< CTeamAiRole >& roles = teamMgr->GetTeamAiRoles();
          for (AUTO(it, roles.begin()); it != roles.end(); it++) {
            if (const CWarWasp* other = CPatterned::CastTo(TPatternedCast< CWarWasp >(
                    const_cast< CEntity* >(mgr.GetObjectById(it->GetOwnerId()))))) {
              if (x70c_initialCircleAttackTeam != -1 &&
                  x70c_initialCircleAttackTeam == other->x70c_initialCircleAttackTeam &&
                  other->x708_circleAttackTeam >= 0) {
                unit = other->x708_circleAttackTeam;
                rejoinInitial = true;
                break;
              }
              if (other->x708_circleAttackTeam > unit) {
                unit = other->x708_circleAttackTeam;
                count = 1;
              } else if (unit == other->x708_circleAttackTeam) {
                ++count;
              }
            }
          }
          if (!rejoinInitial && (x70c_initialCircleAttackTeam != -1 || count >= maxCount)) {
            ++unit;
          }
          JoinCircleAttackTeam(unit, mgr);
          x714_circleTelegraphSeekHeight = -0.5f * mgr.Random()->Float();
        }
      }
    }
  }
}

void CWarWasp::TryCircleTeamMerge(CStateManager& mgr) {
  int team = x708_circleAttackTeam;
  if (team > 0) {
    if (const CTeamAiMgr* const teamMgr =
            TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(x674_aiMgr))) {
      if (teamMgr->IsPartOfTeam(GetUniqueId())) {
        if (GetAttackTeamLeader(mgr, team) == GetUniqueId() &&
            GetAttackTeamSize(mgr, team - 1) == 0) {
          const rstl::vector< CTeamAiRole >& roles = teamMgr->GetTeamAiRoles();
          for (AUTO(it, roles.begin()); it != roles.end(); it++) {
            if (CWarWasp* other = CPatterned::CastTo(
                    TPatternedCast< CWarWasp >(mgr.ObjectById(it->GetOwnerId())))) {
              if (team == other->x708_circleAttackTeam) {
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
  if (const CTeamAiMgr* teamMgr = TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(x674_aiMgr))) {
    if (teamMgr->IsPartOfTeam(GetUniqueId())) {
      for (AUTO(it, teamMgr->GetTeamAiRoles().begin()); it != teamMgr->GetTeamAiRoles().end();
           it++) {
        if (const CWarWasp* other = CPatterned::CastTo(TPatternedCast< CWarWasp >(
                const_cast< CEntity* >(mgr.GetObjectById(it->GetOwnerId()))))) {
          if (team == other->x708_circleAttackTeam) {
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
  if (const CTeamAiMgr* teamMgr = TCastToConstPtr< CTeamAiMgr >(mgr.GetObjectById(x674_aiMgr))) {
    if (teamMgr->IsPartOfTeam(GetUniqueId())) {
      for (AUTO(it, teamMgr->GetTeamAiRoles().begin()); it != teamMgr->GetTeamAiRoles().end();
           it++) {
        if (const CWarWasp* other = CPatterned::CastTo(TPatternedCast< CWarWasp >(
                const_cast< CEntity* >(mgr.GetObjectById(it->GetOwnerId()))))) {
          if (team == other->x708_circleAttackTeam) {
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
  TUniqueId leaderId = GetAttackTeamLeader(mgr, x708_circleAttackTeam);
  if (const CWarWasp* leader = CPatterned::CastTo(
          TPatternedCast< CWarWasp >(const_cast< CEntity* >(mgr.GetObjectById(leaderId))))) {
    if (leaderId == GetUniqueId()) {
      float time = x330_stateMachineState.GetTime();
      float cycleTime = CMath::FastFmod(time, 2.8f);
      if (cycleTime < 2.f) {
        x3b4_speed = x6fc_initialSpeed;
      } else {
        float t = (cycleTime - 2.f) / 0.8f;
        x3b4_speed = ((1.f - t) * 0.7f + 2.f * t) * x6fc_initialSpeed;
      }
    } else {
      x3b4_speed = leader->x3b4_speed;
    }
  } else {
    x3b4_speed = x6fc_initialSpeed;
  }
}

bool CWarWasp::IsListening() const { return true; }

CPathFindSearch* CWarWasp::GetSearchPath() { return &x590_pfSearch; }
