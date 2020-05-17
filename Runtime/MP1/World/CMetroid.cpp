#include "Runtime/MP1/World/CMetroid.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Camera/CFirstPersonCamera.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"
#include "Runtime/Collision/CGameCollision.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CTeamAiMgr.hpp"
#include "Runtime/World/CWorld.hpp"
#include "Runtime/World/ScriptLoader.hpp"

namespace urde::MP1 {
namespace {
constexpr CDamageVulnerability skGammaRedDamageVulnerability{
    EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Immune,
    EVulnerability::Deflect, EVulnerability::Normal,  EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Normal,  EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EDeflectType::None,
};

constexpr CDamageVulnerability skGammaWhiteDamageVulnerability{
    EVulnerability::Deflect, EVulnerability::Immune,  EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Deflect, EVulnerability::Normal,  EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Normal,  EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EDeflectType::None,
};

constexpr CDamageVulnerability skGammaPurpleDamageVulnerability{
    EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Immune,  EVulnerability::Deflect,
    EVulnerability::Deflect, EVulnerability::Normal,  EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Normal,  EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EDeflectType::None,
};

constexpr CDamageVulnerability skGammaOrangeDamageVulnerability{
    EVulnerability::Immune,  EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Deflect, EVulnerability::Normal,  EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Normal,  EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EDeflectType::None,
};

constexpr CDamageVulnerability skNormalDamageVulnerability{
    EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Deflect, EVulnerability::Normal,  EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect,
    EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EDeflectType::None,
};

constexpr auto skPirateSuckJoint = "Head_1"sv;

constexpr std::array skJointNameList = {
    "Head_1"sv,        "L_ankle"sv,    "L_elbow"sv,       "L_hip"sv,   "L_knee"sv,  "L_shoulder"sv,
    "L_varias2_SDK"sv, "L_wrist"sv,    "Pelvis"sv,        "R_ankle"sv, "R_elbow"sv, "R_hip"sv,
    "R_knee"sv,        "R_shoulder"sv, "R_varias2_SDK"sv, "Spine_1"sv, "Spine_2"sv,
};
} // namespace

CMetroidData::CMetroidData(CInputStream& in)
: x0_frozenVulnerability(in)
, x68_energyDrainVulnerability(in)
, xd0_energyDrainPerSec(in.readFloatBig())
, xd4_maxEnergyDrainAllowed(in.readFloatBig())
, xd8_telegraphAttackTime(in.readFloatBig())
, xdc_stage2GrowthScale(in.readFloatBig())
, xe0_stage2GrowthEnergy(in.readFloatBig())
, xe4_explosionGrowthEnergy(in.readFloatBig()) {
  xe8_animParms1 = ScriptLoader::LoadAnimationParameters(in);
  xf8_animParms2 = ScriptLoader::LoadAnimationParameters(in);
  x108_animParms3 = ScriptLoader::LoadAnimationParameters(in);
  x118_animParms4 = ScriptLoader::LoadAnimationParameters(in);
  x128_24_startsInWall = in.readBool();
}

CMetroid::CMetroid(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
                   const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                   const CActorParameters& aParms, const CMetroidData& metroidData, TUniqueId other)
: CPatterned(ECharacter::Metroid, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer,
             EColliderType::One, EBodyType::Flyer, aParms, EKnockBackVariant::Medium)
, x56c_data(metroidData)
, x6a0_collisionPrimitive(zeus::CSphere{zeus::skZero3f, 0.9f * GetModelData()->GetScale().y()}, GetMaterialList())
, x6c0_pathFindSearch(nullptr, 3, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x7cc_gammaType(flavor == EFlavorType::Two ? EGammaType::Red : EGammaType::Normal)
, x7d0_scale1(GetModelData()->GetScale())
, x7dc_scale2(GetModelData()->GetScale())
, x7e8_scale3(GetModelData()->GetScale())
, x81c_patternedInfo(pInfo)
, x954_actParams(aParms)
, x9bc_(other) {
  x808_loopAttackDistance =
      GetAnimationDistance(CPASAnimParmData{9, CPASAnimParm::FromEnum(2), CPASAnimParm::FromEnum(3)});
  UpdateTouchBounds();
  SetCoefficientOfRestitutionModifier(0.9f);
  x460_knockBackController.SetX82_24(false);
  x460_knockBackController.SetEnableBurn(false);
  x460_knockBackController.SetEnableBurnDeath(false);
  x460_knockBackController.SetEnableShock(false);
  if (flavor == CPatterned::EFlavorType::Two) {
    x460_knockBackController.SetEnableFreeze(false);
  }
  x81c_patternedInfo.SetActive(true);
}

void CMetroid::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case EScriptObjectMessage::Registered:
    x450_bodyController->Activate(mgr);
    UpdateVolume();
    break;
  case EScriptObjectMessage::Alert:
    x9bf_24_alert = true;
    break;
  case EScriptObjectMessage::Deactivate:
    SwarmRemove(mgr);
    break;
  case EScriptObjectMessage::Damage:
    if (TCastToConstPtr<CGameProjectile> projectile = mgr.GetObjectById(uid)) {
      const CDamageInfo& damageInfo = projectile->GetDamageInfo();
      if (GetDamageVulnerability()->WeaponHits(damageInfo.GetWeaponMode(), false)) {
        ApplyGrowth(damageInfo.GetDamage());
      }
    }
    x9bf_24_alert = true;
    break;
  case EScriptObjectMessage::InitializedInArea:
    if (x698_teamAiMgrId == kInvalidUniqueId) {
      x698_teamAiMgrId = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
    }
    x6c0_pathFindSearch.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    break;
  default:
    break;
  }
}

void CMetroid::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  if (CTeamAiMgr::GetTeamAiRole(mgr, x698_teamAiMgrId, GetUniqueId()) == nullptr) {
    SwarmAdd(mgr);
  }
  UpdateAttackChance(mgr, dt);
  SuckEnergyFromTarget(mgr, dt);
  PreventWorldCollisions(mgr, dt);
  UpdateTouchBounds();
  RestoreSolidCollision(mgr);
  CPatterned::Think(dt, mgr);
}

void CMetroid::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType eType, float dt) {
  if (eType == EUserEventType::GenerateEnd) {
    AddMaterial(EMaterialTypes::Solid, mgr);
  } else {
    CPatterned::DoUserAnimEvent(mgr, node, eType, dt);
  }
}

EWeaponCollisionResponseTypes CMetroid::GetCollisionResponseType(const zeus::CVector3f& vec1,
                                                                 const zeus::CVector3f& vec2, const CWeaponMode& mode,
                                                                 EProjectileAttrib attribute) const {
  EWeaponCollisionResponseTypes types = EWeaponCollisionResponseTypes::Unknown33;
  if (!GetDamageVulnerability()->WeaponHurts(mode, false) && x450_bodyController->GetPercentageFrozen() <= 0.f) {
    types = EWeaponCollisionResponseTypes::Unknown58;
  }
  return types;
}

const CDamageVulnerability* CMetroid::GetDamageVulnerability() const {
  if (IsSuckingEnergy()) {
    if (x9c0_24_isEnergyDrainVulnerable) {
      return &x56c_data.GetEnergyDrainVulnerability();
    }
    return &skNormalDamageVulnerability;
  }
  if (x9bf_25_ && !x450_bodyController->IsFrozen()) {
    return &x56c_data.GetEnergyDrainVulnerability();
  }
  if (x450_bodyController->GetPercentageFrozen() > 0.f) {
    return &x56c_data.GetFrozenVulnerability();
  }
  if (x3fc_flavor == CPatterned::EFlavorType::Two) {
    if (x7cc_gammaType == EGammaType::Red) {
      return &skGammaRedDamageVulnerability;
    }
    if (x7cc_gammaType == EGammaType::White) {
      return &skGammaWhiteDamageVulnerability;
    }
    if (x7cc_gammaType == EGammaType::Purple) {
      return &skGammaPurpleDamageVulnerability;
    }
    if (x7cc_gammaType == EGammaType::Orange) {
      return &skGammaOrangeDamageVulnerability;
    }
  }
  return CAi::GetDamageVulnerability();
}

zeus::CVector3f CMetroid::GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                                    const zeus::CVector3f& aimPos) const {
  CPlayer& player = mgr.GetPlayer();
  float range = 0.5f * (x2fc_minAttackRange + x300_maxAttackRange);
  const zeus::CVector3f& pos = GetTranslation();
  TUniqueId target = x7b0_attackTarget;
  if (target == player.GetUniqueId()) {
    const zeus::CVector3f& playerPos = player.GetTranslation();
    const zeus::CVector3f& playerFace = player.GetTransform().frontVector();
    if (player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed) {
      zeus::CVector3f face = pos - playerPos;
      face.z() = 0.f;
      if (face.canBeNormalized()) {
        face.normalize();
      } else {
        face = playerFace;
      }
      return playerPos + zeus::CVector3f{range * face.x(), range * face.y(), 0.5f};
    }
    return aimPos + zeus::CVector3f{range * playerFace.x(), range * playerFace.y(), 0.5f};
  }
  if (TCastToConstPtr<CActor> actor = mgr.GetObjectById(target)) {
    const zeus::CVector3f& actorPos = actor->GetTranslation();
    zeus::CVector3f face = pos - actorPos;
    face.z() = 0.f;
    if (face.canBeNormalized()) {
      face.normalize();
    } else {
      face = actor->GetTransform().frontVector();
    }
    return actorPos + zeus::CVector3f{range * face.x(), range * face.y(), 0.5f};
  }
  return pos;
}

void CMetroid::SelectTarget(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x568_state = x9bf_27_ ? EState::Over : EState::One;
    if (x7b0_attackTarget == kInvalidUniqueId) {
      CPlayer& player = mgr.GetPlayer();
      const zeus::CVector3f& pos = GetTranslation();
      float playerDistSq = (player.GetTranslation() - pos).magSquared();
      x7b0_attackTarget = player.GetUniqueId();
      if (!x450_bodyController->HasBeenFrozen()) {
        float range = std::max(x3bc_detectionRange, std::sqrt(playerDistSq));
        rstl::reserved_vector<TUniqueId, 1024> nearList;
        mgr.BuildNearList(nearList, zeus::CAABox{pos - range, pos + range},
                          CMaterialFilter::MakeInclude({EMaterialTypes::Character}), nullptr);
        CSpacePirate* closestPirate = nullptr;
        for (const auto id : nearList) {
          if (auto* pirate = CPatterned::CastTo<CSpacePirate>(mgr.ObjectById(id))) {
            if (IsPirateValidTarget(pirate, mgr)) {
              float distSq = (pirate->GetTranslation() - pos).magSquared();
              if (distSq < playerDistSq) {
                closestPirate = pirate;
                playerDistSq = distSq;
              }
            }
          }
        }
        if (closestPirate != nullptr) {
          x7b0_attackTarget = closestPirate->GetUniqueId();
          closestPirate->SetAttackTarget(GetUniqueId());
        }
      }
    }
    if (auto* pirate = CPatterned::CastTo<CSpacePirate>(mgr.ObjectById(x7b0_attackTarget))) {
      mgr.SendScriptMsg(pirate, GetUniqueId(), EScriptObjectMessage::Alert);
    }
  } else if (msg == EStateMsg::Update) {
    if (x568_state == EState::One) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Generate) {
        x568_state = EState::Two;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Three, zeus::skZero3f));
      }
    } else if (x568_state == EState::Two) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Generate) {
        if (x7b0_attackTarget != kInvalidUniqueId) {
          if (TCastToConstPtr<CActor> actor = mgr.GetObjectById(x7b0_attackTarget)) {
            x450_bodyController->GetCommandMgr().DeliverTargetVector(actor->GetTranslation() - GetTranslation());
          }
        }
      } else {
        x568_state = EState::Over;
      }
    }
  }
}

void CMetroid::Touch(CActor& act, CStateManager& mgr) {
  if (IsAlive()) {
    if (TCastToPtr<CGameProjectile> proj = act) {
      if (proj->GetOwnerId() == mgr.GetPlayer().GetUniqueId() && x3fc_flavor != CPatterned::EFlavorType::Two &&
          proj->HasAttrib(EProjectileAttrib::Ice)) {
        if (GetDamageVulnerability()->WeaponHits(CWeaponMode{EWeaponType::Ice, false}, false)) {
          float timeScale = proj->HasAttrib(EProjectileAttrib::Charged) ? 2.f : 1.f;
          const zeus::CVector3f& projPos = proj->GetTranslation();
          Freeze(mgr, projPos - GetTranslation(), x34_transform.transposeRotate(projPos - proj->GetPreviousPos()),
                 timeScale * x4fc_freezeDur);
        }
      }
      x9bf_26_shotAt = true;
    }
    CPatterned::Touch(act, mgr);
  }
}

bool CMetroid::IsPirateValidTarget(const CSpacePirate* target, CStateManager& mgr) {
  if (target->GetAttachedActor() == kInvalidUniqueId && target->IsTrooper()) {
    const CHealthInfo* healthInfo = target->GetHealthInfo(mgr);
    if (healthInfo != nullptr && healthInfo->GetHP() > 0.f) {
      return true;
    }
  }
  return false;
}

void CMetroid::UpdateAttackChance(CStateManager& mgr, float dt) {
  if (IsAttackInProgress(mgr)) {
    x7b4_attackChance = x308_attackTimeVariation * mgr.GetActiveRandom()->Float() + x304_averageAttackTime;
  } else if (x7b4_attackChance > 0.f) {
    float delta = dt;
    if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed) {
      delta = 2.f * dt;
    }
    x7b4_attackChance -= delta;
  }
}

bool CMetroid::IsAttackInProgress(CStateManager& mgr) {
  if (TCastToConstPtr<CActor> actor = mgr.GetObjectById(x7b0_attackTarget)) {
    if (x7b0_attackTarget == mgr.GetPlayer().GetUniqueId()) {
      const TUniqueId attachedActor = mgr.GetPlayer().GetAttachedActor();
      if (attachedActor != kInvalidUniqueId && attachedActor != GetUniqueId()) {
        return true;
      }
    } else if (const auto* pirate = CPatterned::CastTo<CSpacePirate>(actor.GetPtr())) {
      const TUniqueId attachedActor = pirate->GetAttachedActor();
      if (attachedActor != kInvalidUniqueId && attachedActor != GetUniqueId()) {
        return true;
      }
    }
  }
  return false;
}

void CMetroid::SuckEnergyFromTarget(CStateManager& mgr, float dt) {
  x9c0_24_isEnergyDrainVulnerable = false;
  if (x7b0_attackTarget == kInvalidUniqueId) {
    return;
  }
  if (x7c8_ == EUnknown::One) {
    InterpolateToPosRot(mgr, 0.4f);
    CPlayer& player = mgr.GetPlayer();
    if (x7b0_attackTarget == player.GetUniqueId()) {
      x402_28_isMakingBigStrike = true;
      x504_damageDur = 0.2f;
      mgr.SendScriptMsg(&player, GetUniqueId(), EScriptObjectMessage::Damage);
    }
    x7c0_ = 0.f;
  } else if (x7c8_ == EUnknown::Two) {
    CPlayer& player = mgr.GetPlayer();
    if (TCastToPtr<CActor> actor = mgr.ObjectById(x7b0_attackTarget)) {
      CHealthInfo* healthInfo = actor->HealthInfo(mgr);
      if (healthInfo != nullptr) {
        const float damage = dt * x56c_data.GetEnergyDrainPerSec() * GetDamageMultiplier();
        x7bc_ += damage;
        if (x7b0_attackTarget == player.GetUniqueId()) {
          player.SetNoDamageLoopSfx(true);
          constexpr auto filter = CMaterialFilter::MakeInclude({EMaterialTypes::Solid});
          constexpr CWeaponMode mode{EWeaponType::PoisonWater};
          CDamageInfo info{mode, damage, 0.f, 0.f};
          info.SetNoImmunity(true);
          mgr.ApplyDamage(GetUniqueId(), x7b0_attackTarget, GetUniqueId(), info, filter, zeus::skZero3f);
          player.SetNoDamageLoopSfx(false);
          x9c0_24_isEnergyDrainVulnerable = player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed;
        } else {
          x9c0_24_isEnergyDrainVulnerable = true;
          constexpr auto filter = CMaterialFilter::MakeInclude({EMaterialTypes::Solid});
          constexpr CWeaponMode mode{EWeaponType::Power};
          CDamageInfo info{mode, damage, 0.f, 0.f};
          info.SetNoImmunity(true);
          mgr.ApplyDamage(GetUniqueId(), x7b0_attackTarget, GetUniqueId(), info, filter, zeus::skZero3f);
        }
        if (GetGrowthStage() >= 2.f) {
          TakeDamage(zeus::skZero3f, 0.f);
        } else {
          ApplyGrowth(damage);
        }
      }
    }
    float arg = 0.95f;
    if (x7b0_attackTarget == player.GetUniqueId()) {
      auto morphBallState = player.GetMorphballTransitionState();
      if (morphBallState != CPlayer::EPlayerMorphBallState::Unmorphed &&
          morphBallState != CPlayer::EPlayerMorphBallState::Morphed) {
        arg = 0.4f;
      }
      if (morphBallState == CPlayer::EPlayerMorphBallState::Unmorphed) {
        const float magnitude = std::clamp(std::abs(std::sin(zeus::degToRad(90.f) * x7c0_)), 0.f, 1.f);
        mgr.GetPlayerState()->GetStaticInterference().AddSource(GetUniqueId(), magnitude, 0.2f);
        if (player.GetStaticTimer() < 0.2f) {
          player.SetHudDisable(0.2f, 0.5f, 2.5f);
        }
      }
      x402_28_isMakingBigStrike = true;
      x504_damageDur = 0.2f;
    }
    InterpolateToPosRot(mgr, arg);
    x7c0_ += dt;
  } else if (x7c8_ == EUnknown::Three) {
    const zeus::CQuaternion zRot = zeus::CQuaternion::fromAxisAngle({0.0f, 0.0f, 1.0f}, GetYaw());
    const zeus::CQuaternion rot = zeus::CQuaternion::slerpShort(GetTransform().basis, zRot, 0.95f);
    SetRotation(rot.normalized());
  }
}

void CMetroid::RestoreSolidCollision(CStateManager& mgr) {
  constexpr auto filter = CMaterialFilter::MakeInclude({EMaterialTypes::Solid, EMaterialTypes::AIBlock});
  const zeus::CVector3f& pos = GetTranslation();
  if (x9bf_30_ && !CGameCollision::DetectStaticCollisionBoolean(mgr, x6a0_collisionPrimitive, GetTransform(), filter)) {
    bool add = true;
    if (!x80c_.isZero()) {
      const zeus::CVector3f dir = pos - x80c_;
      float mag = dir.magnitude();
      if (mag > 0.f) {
        // TODO double check bool
        add = mgr.RayStaticIntersection(x80c_, (1.f / mag) * dir, mag, filter).IsInvalid();
      }
    }
    if (add) {
      AddMaterial(EMaterialTypes::Solid, mgr);
      x9bf_30_ = false;
    }
  }
  if (x9bf_31_) {
    constexpr auto nearFilter =
        CMaterialFilter::MakeInclude({EMaterialTypes::Solid, EMaterialTypes::Player, EMaterialTypes::Character});
    float radius = x808_loopAttackDistance * GetModelData()->GetScale().y();
    const zeus::CAABox box{pos - radius, pos + radius};
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    mgr.BuildNearList(nearList, box, nearFilter, this);
    if (!CGameCollision::DetectDynamicCollisionBoolean(x6a0_collisionPrimitive, GetTransform(), nearList, mgr)) {
      x9bf_31_ = false;
      CMaterialFilter matFilter = GetMaterialFilter();
      matFilter.ExcludeList().Remove({EMaterialTypes::Character, EMaterialTypes::Player});
      SetMaterialFilter(matFilter);
    }
  }
}

void CMetroid::PreventWorldCollisions(CStateManager& mgr, float dt) {
  float size = 2.f * x6a0_collisionPrimitive.GetSphere().radius;
  if (IsSuckingEnergy()) {
    if (x7b0_attackTarget == mgr.GetPlayer().GetUniqueId()) {
      if (TCastToPtr<CPhysicsActor> actor = mgr.ObjectById(x7b0_attackTarget)) {
        // why not use mgr.GetPlayer()? :thonking:
        float mass = 300.f;
        if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed) {
          float scale = std::min(1.f, 1.33f * x7c0_);
          mass = 300.f * (1.f - scale) + 7500.f * scale;
        }
        CGameCollision::AvoidStaticCollisionWithinRadius(mgr, *actor, 8, dt, 0.25f, size, mass, 0.5f);
      }
    }
    x7c4_ = 0.f;
  } else if (!x9bf_30_ && !x9bf_31_) {
    x7c4_ = 0.f;
  } else {
    x7c4_ += dt;
    if (x7c4_ <= 6.f) {
      if (x9bf_30_ && 0.25f < x7c4_) {
        RemoveMaterial(EMaterialTypes::Solid, mgr);
      }
    } else {
      MassiveDeath(mgr);
    }
    CGameCollision::AvoidStaticCollisionWithinRadius(mgr, *this, 8, dt, 0.25f, size, 15000.f, 0.5f);
  }
}

void CMetroid::SwarmRemove(CStateManager& mgr) {
  if (x698_teamAiMgrId == kInvalidUniqueId) {
    return;
  }
  if (TCastToPtr<CTeamAiMgr> aiMgr = mgr.ObjectById(x698_teamAiMgrId)) {
    if (aiMgr->IsPartOfTeam(GetUniqueId())) {
      aiMgr->RemoveTeamAiRole(GetUniqueId());
    }
  }
}

void CMetroid::SwarmAdd(CStateManager& mgr) {
  if (x698_teamAiMgrId == kInvalidUniqueId) {
    return;
  }
  if (TCastToPtr<CTeamAiMgr> aiMgr = mgr.ObjectById(x698_teamAiMgrId)) {
    if (!aiMgr->IsPartOfTeam(GetUniqueId())) {
      aiMgr->AssignTeamAiRole(*this, CTeamAiRole::ETeamAiRole::Melee, CTeamAiRole::ETeamAiRole::Invalid,
                              CTeamAiRole::ETeamAiRole::Invalid);
    }
  }
}

void CMetroid::ApplyGrowth(float arg) {
  x7f8_ += arg;
  const float energy = std::clamp(x7f8_ / x56c_data.GetStage2GrowthEnergy(), 0.f, 1.f);
  const float scale = x56c_data.GetStage2GrowthScale() - x7e8_scale3.y();
  x7d0_scale1 = zeus::CVector3f{energy * scale + x7e8_scale3.y()};
  TakeDamage(zeus::skZero3f, 0.f);
}

bool CMetroid::IsSuckingEnergy() const { return x7c8_ == EUnknown::Two && !x450_bodyController->IsFrozen(); }

void CMetroid::UpdateVolume() {
  // TODO
}

void CMetroid::UpdateTouchBounds() {
  const zeus::CTransform& locXf = GetLocatorTransform("lockon_target_LCTR"sv);
  x6a0_collisionPrimitive.SetSphereCenter(locXf * GetModelData()->GetScale());
}

void CMetroid::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    if (AttachToTarget(mgr)) {
      x568_state = EState::One;
      x7bc_ = 0.f;
      x7c8_ = EUnknown::One;
      x9bf_29_isAttacking = true;
      RemoveMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
      mgr.GetPlayer().SetOrbitRequestForTarget(GetUniqueId(), CPlayer::EPlayerOrbitRequest::ActivateOrbitSource, mgr);
      DisableSolidCollision(this);
      AddMaterial(EMaterialTypes::Trigger, mgr);
    } else {
      x568_state = EState::Over;
    }
  } else if (msg == EStateMsg::Update) {
    if (x568_state == EState::One) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::LoopAttack) {
        x568_state = EState::Two;
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCLoopAttackCmd(pas::ELoopAttackType::Three));
      }
    } else if (x568_state == EState::Two) {
      if (GetModelData()->GetAnimationData()->GetIsLoop()) {
        x7c8_ = EUnknown::Two;
      }
      const CPlayer& player = mgr.GetPlayer();
      if (x7b0_attackTarget == player.GetUniqueId() && player.GetAttachedActor() == GetUniqueId() &&
          player.GetAreaIdAlways() != GetAreaIdAlways()) {
        DetachFromTarget(mgr);
        x401_30_pendingDeath = true;
      } else if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::LoopAttack) {
        if (ShouldReleaseFromTarget(mgr)) {
          GetBodyController()->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::ExitState));
          DetachFromTarget(mgr);
          x7c8_ = EUnknown::Three;
        }
      } else {
        x568_state = EState::Over;
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    CTeamAiMgr::ResetTeamAiRole(CTeamAiMgr::EAttackType::Melee, mgr, x698_teamAiMgrId, GetUniqueId(), false);
    x7b4_attackChance = x308_attackTimeVariation * mgr.GetActiveRandom()->Float() + x304_averageAttackTime;
    x7c8_ = EUnknown::Zero;
    DetachFromTarget(mgr);
    x9bf_29_isAttacking = false;
    SetTransform({zeus::CQuaternion::fromAxisAngle({0.f, 0.f, 1.f}, GetYaw()), GetTranslation()});
    AddMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
    RemoveMaterial(EMaterialTypes::Trigger, mgr);
  }
}

bool CMetroid::AttachToTarget(CStateManager& mgr) {
  CPlayer& player = mgr.GetPlayer();
  if (x7b0_attackTarget == player.GetUniqueId()) {
    if (player.AttachActorToPlayer(GetUniqueId(), false)) {
      player.GetEnergyDrain().AddEnergyDrainSource(GetUniqueId(), 1.f);
      return true;
    }
    return false;
  }
  return PreDamageSpacePirate(mgr);
}

void CMetroid::DetachFromTarget(CStateManager& mgr) {
  CPlayer& player = mgr.GetPlayer();
  CActor* target = nullptr;
  zeus::CVector3f vec;
  zeus::CTransform xf;
  if (x7b0_attackTarget == player.GetUniqueId()) {
    if (player.GetAttachedActor() == GetUniqueId()) {
      player.DetachActorFromPlayer();
      if (player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed) {
        const auto q1 = zeus::CQuaternion::fromAxisAngle({0.f, 0.f, 1.f}, M_PIF);
        const auto q2 = zeus::CQuaternion::fromAxisAngle({0.f, 0.f, 1.f}, GetYaw());
        const auto mat = zeus::CMatrix3f{q2 * q1};
        vec = mat * zeus::skForward;
        xf = zeus::CTransform{mat, player.GetTranslation()};
      } else {
        vec = player.GetTransform().frontVector();
        xf = player.GetTransform();
      }
      mgr.GetPlayer().GetEnergyDrain().RemoveEnergyDrainSource(GetUniqueId());
      x80c_ = player.GetAimPosition(mgr, 0.f);
      target = &player;
    }
  } else if (x7b0_attackTarget != kInvalidUniqueId) {
    if (auto* pirate = CPatterned::CastTo<CSpacePirate>(mgr.ObjectById(x7b0_attackTarget))) {
      if (pirate->GetAttachedActor() == GetUniqueId()) {
        pirate->DetachActorFromPirate();
        vec = pirate->GetTransform().frontVector();
        xf = pirate->GetTransform();
        x80c_ = GetTranslation();
        target = pirate;
      }
    }
  }
  SetupExitFaceHugDirection(target, mgr, vec, xf);
  x9bf_31_ = true;
  x9bf_30_ = true;
}

bool CMetroid::ShouldReleaseFromTarget(CStateManager& mgr) {
  if (x450_bodyController->IsFrozen()) {
    return true;
  }
  CPlayer& player = mgr.GetPlayer();
  if (x7b0_attackTarget == player.GetUniqueId()) {
    if (x7bc_ >= x56c_data.GetMaxEnergyDrainAllowed() * GetDamageMultiplier() || IsPlayerUnderwater(mgr)) {
      return true;
    }
    if (player.GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Unmorphed) {
      return IsHunterAttacking(mgr);
    }
  } else if (x7b0_attackTarget != kInvalidUniqueId) {
    if (const auto* pirate = CPatterned::CastTo<CSpacePirate>(mgr.GetObjectById(x7b0_attackTarget))) {
      if (!pirate->AllEnergyDrained() && !pirate->GetBodyController()->GetBodyStateInfo().GetCurrentState()->IsDead()) {
        return false;
      }
    }
    return true;
  }
  return false;
}

void CMetroid::DisableSolidCollision(CMetroid* target) {
  CMaterialFilter filter = target->GetMaterialFilter();
  filter.ExcludeList().Add({EMaterialTypes::Character, EMaterialTypes::Player});
  target->SetMaterialFilter(filter);
}

void CMetroid::SetupExitFaceHugDirection(CActor* actor, CStateManager& mgr, const zeus::CVector3f& vec,
                                         const zeus::CTransform& xf) {
  // TODO
}

bool CMetroid::PreDamageSpacePirate(CStateManager& mgr) {
  // TODO
  return false;
}

bool CMetroid::IsPlayerUnderwater(CStateManager& mgr) {
  // TODO
  return false;
}

bool CMetroid::IsHunterAttacking(CStateManager& mgr) {
  // TODO
  return false;
}

float CMetroid::GetGrowthStage() {
  const float energy = x7f8_;
  const float stage2GrowthEnergy = x56c_data.GetStage2GrowthEnergy();
  if (energy < stage2GrowthEnergy) {
    return 1.f + energy / stage2GrowthEnergy;
  }
  const float explosionGrowthEnergy = x56c_data.GetExplosionGrowthEnergy();
  if (energy < explosionGrowthEnergy) {
    return 2.f + (energy - stage2GrowthEnergy) / (explosionGrowthEnergy - stage2GrowthEnergy);
  }
  return 3.f;
}

bool CMetroid::ShouldAttack(CStateManager& mgr, float arg) {
  if (!CanAttack(mgr)) {
    return false;
  }
  if (TCastToPtr<CTeamAiMgr> aiMgr = mgr.ObjectById(x698_teamAiMgrId)) {
    return aiMgr->AddMeleeAttacker(GetUniqueId());
  }
  return true;
}

bool CMetroid::CanAttack(CStateManager& mgr) {
  if (x7b4_attackChance <= 0.f) {
    CPlayer& player = mgr.GetPlayer();
    if (x7b0_attackTarget == player.GetUniqueId()) {
      if (IsPlayerUnderwater(mgr) ||
          (player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed &&
           player.GetMorphBall()->GetSpiderBallState() == CMorphBall::ESpiderBallState::Active)) {
        return false;
      }
      if (player.GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed && IsHunterAttacking(mgr)) {
        return false;
      }
    }
    const CEntity* target = mgr.GetObjectById(x7b0_attackTarget);
    if (target != nullptr && target->GetAreaIdAlways() == GetAreaIdAlways()) {
      return !IsAttackInProgress(mgr);
    }
  }
  return false;
}

void CMetroid::PathFind(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    GetBodyController()->SetLocomotionType(pas::ELocomotionType::Lurk);
    GetBodyController()->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::FullSpeed);
    GetBodyController()->GetCommandMgr().SetSteeringSpeedRange(1.f, 1.f);
    SetUpPathFindBehavior(mgr);
  } else if (msg == EStateMsg::Update) {
    const auto* searchPath = GetSearchPath();
    if (searchPath == nullptr || PathShagged(mgr, 0.f) || PathOver(mgr, 0.f)) {
      const auto* aiRole = CTeamAiMgr::GetTeamAiRole(mgr, x698_teamAiMgrId, GetUniqueId());
      if (aiRole == nullptr) {
        x7a4_ = GetOrigin(mgr, CTeamAiRole{GetUniqueId()}, GetAttackTargetPos(mgr));
      } else {
        x7a4_ = aiRole->GetTeamPosition();
      }
      ApplyForwardSteering(mgr, x7a4_);
    } else {
      CPatterned::PathFind(mgr, msg, arg);
    }
    ApplySeparationBehavior(mgr, 9.f);
  } else if (msg == EStateMsg::Deactivate) {
    GetBodyController()->GetCommandMgr().SetSteeringBlendMode(ESteeringBlendMode::Normal);
  }
}

void CMetroid::ApplyForwardSteering(CStateManager& mgr, const zeus::CVector3f& vec) {
  if ((vec - GetTranslation()).magSquared() <= 4.f) {
    if (ShouldTurn(mgr, 0.f) && x7b0_attackTarget != kInvalidUniqueId) {
      if (TCastToConstPtr<CActor> actor = mgr.GetObjectById(x7b0_attackTarget)) {
        const zeus::CVector3f dir = actor->GetTranslation() - GetTranslation();
        if (dir.canBeNormalized()) {
          GetBodyController()->GetCommandMgr().DeliverCmd(CBCLocomotionCmd{zeus::skZero3f, dir.normalized(), 1.f});
        }
      }
    }
  } else {
    const zeus::CVector3f arrival = x45c_steeringBehaviors.Arrival(*this, GetTranslation().toVec2f(), 0.5f);
    if (arrival.magSquared() > 0.01f) {
      GetBodyController()->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(arrival, zeus::skZero3f, 3.f));
    }
    if (TCastToConstPtr<CPhysicsActor> actor = mgr.GetObjectById(x7b0_attackTarget)) {
      const auto move = x45c_steeringBehaviors.Pursuit(*this, vec, actor->GetVelocity());
      GetBodyController()->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(move, zeus::skZero3f, 1.f));
    } else {
      const auto move = x45c_steeringBehaviors.Seek(*this, vec);
      GetBodyController()->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(move, zeus::skZero3f, 1.f));
    }
  }
}

void CMetroid::ApplySeparationBehavior(CStateManager& mgr, float arg) {
  // TODO
}

void CMetroid::SetUpPathFindBehavior(CStateManager& mgr) {
  x9bf_28_ = false;
  if (GetSearchPath() == nullptr) {
    return;
  }
  if (const auto* role = CTeamAiMgr::GetTeamAiRole(mgr, x698_teamAiMgrId, GetUniqueId())) {
    SetDestPos(role->GetTeamPosition());
  } else {
    SetDestPos(GetOrigin(mgr, CTeamAiRole{GetUniqueId()}, mgr.GetPlayer().GetTranslation()));
  }
  const zeus::CVector3f targetPos = GetAttackTargetPos(mgr);
  const zeus::CVector3f dir = GetDestPos() - targetPos;
  if (dir.canBeNormalized()) {
    constexpr auto filter = CMaterialFilter::MakeInclude({EMaterialTypes::Solid, EMaterialTypes::AIBlock});
    float mag = dir.magnitude();
    const zeus::CVector3f dirScaled = (1.f / mag) * dir;
    const auto result = mgr.RayStaticIntersection(targetPos, dirScaled, mag, filter);
    if (result.IsValid()) {
      SetDestPos(targetPos + 0.5f * result.GetT() * dirScaled);
      x9bf_28_ = true;
    }
  }
  x7a4_ = GetDestPos();
  CPatterned::PathFind(mgr, EStateMsg::Activate, 0.f);
}

zeus::CVector3f CMetroid::GetAttackTargetPos(CStateManager& mgr) {
  const TUniqueId targetId = x7b0_attackTarget;
  if (targetId != kInvalidUniqueId) {
    CPlayer& player = mgr.GetPlayer();
    if (targetId == player.GetUniqueId()) {
      if (player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed) {
        return player.GetMorphBall()->GetBallToWorld().origin;
      }
      return player.GetTranslation() + zeus::CVector3f{0.f, 0.f, -0.6f + player.GetEyeHeight()};
    }
    if (TCastToConstPtr<CActor> actor = mgr.GetObjectById(targetId)) {
      const auto locXf = actor->GetLocatorTransform(skPirateSuckJoint);
      return actor->GetTranslation() +
             zeus::CVector3f{0.f, 0.f, locXf.origin.z() * actor->GetModelData()->GetScale().z() + 0.4f};
    }
  }
  return mgr.GetPlayer().GetAimPosition(mgr, 0.f);
}

bool CMetroid::AggressionCheck(CStateManager& mgr, float arg) {
  if (x7b0_attackTarget != kInvalidUniqueId) {
    CEntity* target = mgr.ObjectById(x7b0_attackTarget);
    if (auto* pirate = CPatterned::CastTo<CSpacePirate>(target)) {
      if (!IsPirateValidTarget(pirate, mgr)) {
        x7b0_attackTarget = kInvalidUniqueId;
        return false;
      }
    }
    if (TCastToPtr<CActor> actor = target) {
      const zeus::CVector3f dir = actor->GetTranslation() - GetTranslation();
      if (x3bc_detectionRange * x3bc_detectionRange > dir.magSquared()) {
        if (x3c0_detectionHeightRange > 0.f) {
          return dir.z() * dir.z() < x3c0_detectionHeightRange * x3c0_detectionHeightRange;
        }
        return true;
      }
    } else {
      x7b0_attackTarget = kInvalidUniqueId;
    }
  }
  return false;
}

void CMetroid::Dodge(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    if (x818_dodgeDirection != pas::EStepDirection::Invalid) {
      GetBodyController()->GetCommandMgr().DeliverCmd(CBCStepCmd(x818_dodgeDirection, pas::EStepType::Dodge));
    }
  } else if (msg == EStateMsg::Update) {
    if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::Step) {
      if (x7b0_attackTarget != kInvalidUniqueId) {
        if (TCastToConstPtr<CActor> actor = mgr.GetObjectById(x7b0_attackTarget)) {
          GetBodyController()->GetCommandMgr().DeliverTargetVector(actor->GetTranslation() - GetTranslation());
        }
      }
    } else {
      x568_state = EState::Over;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x818_dodgeDirection = pas::EStepDirection::Invalid;
  }
}

void CMetroid::Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) {
  CPatterned::Death(mgr, direction, state);
  x328_25_verticalMovement = false;
  SetMuted(true);
  SwarmRemove(mgr);
}

void CMetroid::Generate(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    if (ShouldSpawnGammaMetroid()) {
      SpawnGammaMetroid(mgr);
    } else if (x7f8_ >= x56c_data.GetExplosionGrowthEnergy()) {
      MassiveDeath(mgr);
    }
    x568_state = EState::One;
    x7dc_scale2 = GetModelData()->GetScale();
    x9bf_25_ = true;
  } else if (msg == EStateMsg::Update) {
    CBodyController* bodyController = GetBodyController();
    if (x568_state == EState::One) {
      if (bodyController->GetCurrentStateId() == pas::EAnimationState::Generate) {
        x7f4_ = bodyController->GetAnimTimeRemaining();
        x568_state = x7f4_ > 0.f ? EState::Two : EState::Over;
      } else if (Attacked(mgr, 0.f)) {
        bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Two));
      } else {
        bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Seven));
      }
    } else if (x568_state == EState::Two) {
      if (bodyController->GetCurrentStateId() == pas::EAnimationState::Generate) {
        bool inside = false;
        if (!bodyController->IsFrozen() && ((inside = Inside(mgr, 3.f)) || Attacked(mgr, 0.f))) {
          float timeRem = bodyController->GetAnimTimeRemaining();
          float clamp = std::clamp(1.f - (timeRem / x7f4_), 0.f, 1.f);
          zeus::CVector3f scale;
          if (0.25f <= clamp) {
            float dVar13 = 0.75f * x7f4_;
            const zeus::CVector3f v = 0.5f * x7dc_scale2;
            scale = v + (dVar13 - timeRem) * (1.f / dVar13) * (x7d0_scale1 - v);
          } else {
            scale = std::clamp(1.f - 0.5f * (clamp / 0.25f), 0.f, 1.f) * x7dc_scale2;
          }
          if (inside) {
            ApplySplitGammas(mgr, arg);
          }
          GetModelData()->SetScale(scale);
          UpdateVolume();
        }
      } else {
        x568_state = EState::Over;
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    x7f4_ = 0.f;
    x9bf_25_ = false;
    if (Attacked(mgr, 0.f)) {
      x7fc_ = x7f8_;
      GetModelData()->SetScale(x7d0_scale1);
    }
    UpdateVolume();
  }
}

bool CMetroid::ShouldSpawnGammaMetroid() {
  // TODO
  return false;
}

void CMetroid::SpawnGammaMetroid(CStateManager& mgr) {
  // TODO
}

void CMetroid::ApplySplitGammas(CStateManager& mgr, float arg) {
  auto* metroid = CPatterned::CastTo<CMetroid>(mgr.ObjectById(x9bc_));
  if (metroid == nullptr) {
    return;
  }
  // TODO
}

void CMetroid::KnockBack(const zeus::CVector3f& dir, CStateManager& mgr, const CDamageInfo& info, EKnockBackType type,
                         bool inDeferred, float magnitude) {
  if (!IsAlive()) {
    return;
  }
  const CDamageVulnerability* vulnerability = GetDamageVulnerability();
  float percentFrozen = GetBodyController()->GetPercentageFrozen();
  const CWeaponMode& mode = info.GetWeaponMode();
  if (x7c8_ == EUnknown::Two) {
    if (vulnerability->WeaponHits(mode, false)) {
      x7bc_ = x56c_data.GetMaxEnergyDrainAllowed() * GetDamageMultiplier();
    }
  } else if (vulnerability->WeaponHurts(mode, false)) {
    x7b4_attackChance = x308_attackTimeVariation * mgr.GetActiveRandom()->Float() + x304_averageAttackTime;
    if (percentFrozen > 0.f) {
      GetBodyController()->UnFreeze();
    }
    CPatterned::KnockBack(dir, mgr, info, type, inDeferred, magnitude);
  } else if (percentFrozen <= 0.f && vulnerability->WeaponHits(mode, false) &&
             (mode.IsCharged() || mode.IsComboed() || mode.GetType() == EWeaponType::Missile) &&
             !ShouldSpawnGammaMetroid()) {
    CPatterned::KnockBack(dir, mgr, info, type, inDeferred, magnitude);
    x800_ = x804_;
  }
}

bool CMetroid::Attacked(CStateManager& mgr, float arg) {
  if (x7f8_ - x7fc_ > 0.f) {
    if (x7fc_ < x56c_data.GetStage2GrowthEnergy()) {
      return x56c_data.GetStage2GrowthEnergy() <= x7f8_;
    }
    if (x56c_data.GetExplosionGrowthEnergy() <= x7f8_) {
      return true;
    }
  }
  return false;
}

bool CMetroid::AttackOver(CStateManager& mgr, float arg) {
  if (x568_state != EState::Two || GetBodyController()->IsFrozen()) {
    return false;
  }
  float posZ = GetTranslation().z();
  const zeus::CVector3f targetPos = GetAttackTargetPos(mgr);
  float scale = 0.8f * GetModelData()->GetScale().y();
  if (zeus::close_enough(targetPos.z(), posZ, scale)) {
    if (TCastToConstPtr<CPhysicsActor> actor = mgr.GetObjectById(x7b0_attackTarget)) {
      const zeus::CAABox collisionBox = x6a0_collisionPrimitive.CalculateAABox(x34_transform);
      const zeus::CAABox scaledBox = zeus::CAABox{collisionBox.min - scale, collisionBox.max + scale};
      return scaledBox.intersects(actor->GetBoundingBox());
    }
  }
  return false;
}

bool CMetroid::InAttackPosition(CStateManager& mgr, float arg) {
  if (x7b0_attackTarget == kInvalidUniqueId) {
    return false;
  }
  const auto* actor = static_cast<const CActor*>(mgr.GetObjectById(x7b0_attackTarget));
  if (actor == nullptr || actor->GetAreaIdAlways() != GetAreaIdAlways()) {
    return false;
  }
  CPlayer& player = mgr.GetPlayer();
  const zeus::CVector3f& actorPos = actor->GetTranslation();
  const zeus::CVector3f& pos = GetTranslation();
  const zeus::CVector3f dir = pos - actorPos;
  const zeus::CVector2f actorFrontXY = actor->GetTransform().frontVector().toVec2f();
  float maxAngle = M_PIF;
  if (x7b0_attackTarget == player.GetUniqueId()) {
    if (IsPlayerUnderwater(mgr)) {
      return false;
    }
    if (player.GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed && !x9bf_29_isAttacking) {
      maxAngle = zeus::degToRad(45.f);
    }
  }
  if (zeus::CVector2f::getAngleDiff(dir.toVec2f(), actorFrontXY) < maxAngle &&
      dir.dot(GetTransform().frontVector()) < 0.f) {
    const zeus::CVector3f dir2 = x7a4_ - pos;
    if (dir2.magSquared() < x300_maxAttackRange * x300_maxAttackRange && actorPos.z() < pos.z() &&
        pos.z() < 0.5f + x7a4_.z()) {
      zeus::CVector3f attackDir = GetAttackTargetPos(mgr) - pos;
      if (attackDir.canBeNormalized()) {
        constexpr auto filter = CMaterialFilter::MakeInclude({EMaterialTypes::Solid, EMaterialTypes::AIBlock});
        float mag = attackDir.magnitude();
        return mgr.RayStaticIntersection(pos, (1.f / mag) * attackDir, mag, filter).IsInvalid();
      }
    }
  }
  return false;
}

bool CMetroid::InDetectionRange(CStateManager& mgr, float arg) {
  if (x7b0_attackTarget == kInvalidUniqueId) {
    if ((x9bf_24_alert || CPatterned::InDetectionRange(mgr, arg)) && !IsPlayerUnderwater(mgr) &&
        mgr.GetPlayer().GetAreaIdAlways() != GetAreaIdAlways()) {
      return true;
    }
    // TODO attack pirates
  } else if (x7b0_attackTarget != mgr.GetPlayer().GetUniqueId() ||
             !(IsPlayerUnderwater(mgr) || mgr.GetPlayer().GetAreaIdAlways() != GetAreaIdAlways())) {
    if (TCastToConstPtr<CActor> actor = mgr.GetObjectById(x7b0_attackTarget)) {
      const zeus::CVector3f dir = actor->GetTranslation() - GetTranslation();
      if (dir.magSquared() < x3bc_detectionRange * x3bc_detectionRange && x3c0_detectionHeightRange > 0.f) {
        return x3c0_detectionHeightRange * x3c0_detectionHeightRange > dir.z() * dir.z();
      }
    }
  }
  return false;
}

void CMetroid::TelegraphAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x568_state = EState::Zero;
    x7b8_telegraphAttackTime = x56c_data.GetTelegraphAttackTime();
    x800_ = 0.f;
    GetBodyController()->GetCommandMgr().ClearLocomotionCmds();
    GetBodyController()->SetLocomotionType(pas::ELocomotionType::Combat);
  } else if (msg == EStateMsg::Update) {
    if (x568_state == EState::Zero) {
      x7b8_telegraphAttackTime -= dt;
      if (x7b8_telegraphAttackTime >= 0.f) {
        if (TCastToConstPtr<CActor> actor = mgr.GetObjectById(x7b0_attackTarget)) {
          const zeus::CVector3f face = actor->GetTranslation() - GetTranslation();
          if (face.canBeNormalized()) {
            GetBodyController()->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(zeus::skZero3f, face.normalized(), 1.f));
          }
        }
      } else {
        x568_state = EState::Two;
        const float distance = 1.25f * (GetAttackTargetPos(mgr) - GetTranslation()).magnitude();
        const float speed = x3b4_speed > 0.f ? 1.15f / x3b4_speed : 0.f;
        x804_ = speed + (distance / GetBodyController()->GetBodyStateInfo().GetMaxSpeed());
        GetBodyController()->SetTurnSpeed(x3b4_speed > 0.f ? 20.f / x3b4_speed : 20.f);
      }
    } else if (x568_state == EState::Two) {
      x800_ += dt;
      const zeus::CVector3f move = x45c_steeringBehaviors.Seek(*this, GetAttackTargetPos(mgr));
      GetBodyController()->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(move, zeus::skZero3f, 1.f));
    }
  } else if (msg == EStateMsg::Deactivate) {
    GetBodyController()->SetTurnSpeed(x3b8_turnSpeed);
    if (Attacked(mgr, 0.f) || PatternShagged(mgr, 0.f)) {
      CTeamAiMgr::ResetTeamAiRole(CTeamAiMgr::EAttackType::Melee, mgr, x698_teamAiMgrId, GetUniqueId(), false);
    }
  }
}

void CMetroid::InterpolateToPosRot(CStateManager& mgr, float dt) {
  zeus::CVector3f pos;
  zeus::CQuaternion rot;
  ComputeSuckTargetPosRot(mgr, pos, rot);
  const float oneMinusDt = 1.f - dt;
  const auto posInterp = GetTranslation() * oneMinusDt + pos * dt;
  const auto quatInterp = zeus::CQuaternion::slerpShort(GetTransform().basis, rot, dt);
  SetTransform(quatInterp.toTransform(posInterp));
}

void CMetroid::ComputeSuckTargetPosRot(CStateManager& mgr, zeus::CVector3f& outPos, zeus::CQuaternion& outRot) {
  const auto& xf = GetTransform();
  outPos = xf.origin;
  outRot = xf.basis;
  if (x7b0_attackTarget == mgr.GetPlayer().GetUniqueId()) {
    ComputeSuckPlayerPosRot(mgr, outPos, outRot);
  } else {
    ComputeSuckPiratePosRot(mgr, outPos, outRot);
  }
}

void CMetroid::ComputeSuckPlayerPosRot(CStateManager& mgr, zeus::CVector3f& outPos, zeus::CQuaternion& outRot) {
  CPlayer& player = mgr.GetPlayer();
  const auto& playerXf = player.GetTransform();
  outPos = playerXf.origin;
  const auto& scale = GetModelData()->GetScale();
  const auto morphBallState = player.GetMorphballTransitionState();
  if (morphBallState == CPlayer::EPlayerMorphBallState::Morphing) {
    outPos += zeus::CVector3f{0.f, 0.f, 0.4f + ComputeMorphingPlayerSuckZPos(player)};
    outPos += 0.5f * playerXf.frontVector() - player.GetMorphBall()->GetBallRadius() * GetTransform().upVector();
    const auto xRot = zeus::CQuaternion::fromAxisAngle(zeus::skRight, zeus::degToRad(-90.f));
    const auto yRot = zeus::CQuaternion::fromAxisAngle(zeus::skForward, 0.f);
    const auto zRot = zeus::CQuaternion::fromAxisAngle(zeus::skUp, M_PIF);
    outRot = zeus::CQuaternion{playerXf.basis} * (zRot * xRot * yRot);
  } else if (morphBallState == CPlayer::EPlayerMorphBallState::Unmorphed) {
    const zeus::CQuaternion camRot = mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform().basis;
    outRot = camRot * zeus::CQuaternion::fromAxisAngle(zeus::skUp, M_PIF);
    const zeus::CMatrix3f camMtx = camRot.toTransform().basis;
    const zeus::CVector3f forward = camMtx * zeus::skForward;
    const zeus::CVector3f up = (-0.6f * scale.y()) * (camMtx * zeus::skUp);
    outPos += zeus::CVector3f{0.f, 0.f, player.GetEyeHeight()} + up + forward;
  } else if (morphBallState == CPlayer::EPlayerMorphBallState::Morphed) {
    const float ballRadius = player.GetMorphBall()->GetBallRadius();
    outPos += (2.f * ballRadius + 0.25f) * zeus::skUp;
    outPos -= ballRadius * (scale.y() * GetTransform().upVector());
    const auto xRot = zeus::CQuaternion::fromAxisAngle(zeus::skRight, zeus::degToRad(-90.f));
    const auto yRot = zeus::CQuaternion::fromAxisAngle(zeus::skForward, 0.f);
    const auto zRot = zeus::CQuaternion::fromAxisAngle(zeus::skUp, GetYaw());
    outRot = zRot * xRot * yRot;
  } else if (morphBallState == CPlayer::EPlayerMorphBallState::Unmorphing) {
    outPos += zeus::CVector3f{0.f, 0.f, 0.4f + ComputeMorphingPlayerSuckZPos(player)};
    outPos += 0.5f * playerXf.frontVector() - player.GetMorphBall()->GetBallRadius() * GetTransform().upVector();
    const auto xRot = zeus::CQuaternion::fromAxisAngle(zeus::skRight, zeus::degToRad(-90.f));
    const auto yRot = zeus::CQuaternion::fromAxisAngle(zeus::skForward, 0.f);
    const auto zRot = zeus::CQuaternion::fromAxisAngle(zeus::skUp, M_PIF);
    outRot = zeus::CQuaternion{playerXf.basis} * (zRot * xRot * yRot);

    float morphT = 0.f;
    if (player.GetMorphDuration() != 0.f) {
      morphT = std::clamp(player.GetMorphTime() / player.GetMorphDuration(), 0.f, 1.f);
    }
    if (morphT > 0.75f) {
      const zeus::CQuaternion camRot = mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform().basis;
      const zeus::CQuaternion rot = camRot * zeus::CQuaternion::fromAxisAngle(zeus::skUp, M_PIF);
      const zeus::CMatrix3f camMtx = camRot.toTransform().basis;
      const zeus::CVector3f forward = camMtx * zeus::skForward;
      const zeus::CVector3f up = (-0.6f * scale.y()) * (camMtx * zeus::skUp);
      const zeus::CVector3f pos = playerXf.origin + zeus::CVector3f{0.f, 0.f, player.GetEyeHeight()} + up + forward;
      const float t = (morphT - 0.75f) / 0.25f;
      outRot = zeus::CQuaternion::slerpShort(outRot, rot, t);
      outPos = zeus::CVector3f::lerp(outPos, pos, t); // outPos * (1.f - t) + (pos * t);
    }
  }
}

void CMetroid::ComputeSuckPiratePosRot(CStateManager& mgr, zeus::CVector3f& outPos, zeus::CQuaternion& outRot) {
  // TODO
}

float CMetroid::ComputeMorphingPlayerSuckZPos(const CPlayer& player) const {
  float ret = 0.f;
  const CModelData* modelData = player.GetModelData();
  const float scaleZ = modelData->GetScale().z();
  if (modelData != nullptr && modelData->GetAnimationData() != nullptr) { // && modelData->GetNormalModel() ?
    for (const auto& joint : skJointNameList) {
      const zeus::CTransform xf = player.GetLocatorTransform(joint);
      const float z = xf.origin.z() * scaleZ;
      if (z > ret) {
        ret = z;
      }
    }
  }
  return ret;
}

bool CMetroid::InPosition(CStateManager& mgr, float arg) {
  CPathFindSearch* searchPath = GetSearchPath();
  if (searchPath != nullptr) {
    return searchPath->GetCurrentWaypoint() < searchPath->GetWaypoints().size() - 1;
  }
  return (x7a4_ - GetTranslation()).magSquared() < 4.f;
}

bool CMetroid::InRange(CStateManager& mgr, float arg) {
  if (x7b0_attackTarget == kInvalidUniqueId) {
    return false;
  }
  if (TCastToConstPtr<CActor> actor = mgr.GetObjectById(x7b0_attackTarget)) {
    if (const auto* pirate = CPatterned::CastTo<CSpacePirate>(actor.GetPtr())) {
      if (!IsPirateValidTarget(pirate, mgr)) {
        return false;
      }
    }
    return (actor->GetTranslation() - GetTranslation()).magSquared() < x300_maxAttackRange * x300_maxAttackRange;
  }
  return false;
}

bool CMetroid::Inside(CStateManager& mgr, float arg) {
  if (x9bc_ == kInvalidUniqueId) {
    return false;
  }
  if (const auto* other = CPatterned::CastTo<CMetroid>(mgr.GetObjectById(x9bc_))) {
    float radius = x6a0_collisionPrimitive.GetSphere().radius;
    if (arg > 0.f) {
      radius *= arg;
    }
    return (other->GetTranslation() - GetTranslation()).magSquared() < radius * radius;
  }
  return false;
}

bool CMetroid::Leash(CStateManager& mgr, float arg) {
  if (x7b0_attackTarget == mgr.GetPlayer().GetUniqueId() && IsPlayerUnderwater(mgr)) {
    return true;
  }
  if ((x3a0_latestLeashPosition - GetTranslation()).magSquared() <= x3c8_leashRadius * x3c8_leashRadius) {
    return false;
  }
  if (x7b0_attackTarget != kInvalidUniqueId) {
    if (TCastToConstPtr<CActor> actor = mgr.GetObjectById(x7b0_attackTarget)) {
      if ((actor->GetTranslation() - GetTranslation()).magSquared() <=
          x3cc_playerLeashRadius * x3cc_playerLeashRadius) {
        return false;
      }
      if (x3d4_curPlayerLeashTime <= x3d0_playerLeashTime) {
        return false;
      }
    }
  }
  return true;
}

bool CMetroid::LostInterest(CStateManager& mgr, float arg) {
  if (x7b0_attackTarget == kInvalidUniqueId) {
    return true;
  }
  if (TCastToConstPtr<CActor> actor = mgr.GetObjectById(x7b0_attackTarget)) {
    if (const auto* pirate = CPatterned::CastTo<CSpacePirate>(actor.GetPtr())) {
      return pirate->GetAttachedActor() != kInvalidUniqueId || GetBodyController()->HasBeenFrozen();
    }
    return x7b0_attackTarget == mgr.GetPlayer().GetUniqueId() &&
           (IsPlayerUnderwater(mgr) || mgr.GetPlayer().GetAreaIdAlways() != GetAreaIdAlways());
  }
  return true;
}

bool CMetroid::PatternShagged(CStateManager& mgr, float arg) {
  if (x7b0_attackTarget == kInvalidUniqueId) {
    return true;
  }
  if (const auto* pirate = CPatterned::CastTo<CSpacePirate>(mgr.GetObjectById(x7b0_attackTarget))) {
    if (!pirate->IsAlive()) {
      return true;
    }
  }
  if (!CanAttack(mgr)) {
    return true;
  }
  if (x568_state == EState::Two) {
    return x804_ <= x800_;
  }
  return false;
}

bool CMetroid::ShouldDodge(CStateManager& mgr, float arg) {
  // TODO
  return CAi::ShouldDodge(mgr, arg);
}

bool CMetroid::ShouldTurn(CStateManager& mgr, float arg) {
  if (x7b0_attackTarget == kInvalidUniqueId) {
    return false;
  }
  if (TCastToConstPtr<CActor> actor = mgr.GetObjectById(x7b0_attackTarget)) {
    const zeus::CTransform& xf = GetTransform();
    const zeus::CVector2f dir = (actor->GetTranslation() - xf.origin).toVec2f();
    const float diff = zeus::CVector2f::getAngleDiff(xf.frontVector().toVec2f(), dir);
    return diff > zeus::degToRad(15.f);
  }
  return false;
}

bool CMetroid::SpotPlayer(CStateManager& mgr, float arg) {
  CPlayer& player = mgr.GetPlayer();
  if (IsPlayerUnderwater(mgr) || player.GetAreaIdAlways() != GetAreaIdAlways()) {
    return false;
  }
  const TUniqueId playerUid = player.GetUniqueId();
  if (x7b0_attackTarget != kInvalidUniqueId) {
    return x7b0_attackTarget == playerUid;
  }
  if (TCastToPtr<CTeamAiMgr> aiMgr = mgr.ObjectById(x698_teamAiMgrId)) {
    const float range = x3bc_detectionRange * x3bc_detectionRange;
    for (const auto& role : aiMgr->GetRoles()) {
      const TUniqueId uid = role.GetOwnerId();
      if (uid != GetUniqueId()) {
        if (const auto* other = CPatterned::CastTo<CMetroid>(mgr.GetObjectById(uid))) {
          if (other->x7b0_attackTarget == playerUid &&
              (other->GetTranslation() - GetTranslation()).magSquared() < range) {
            return true;
          }
        }
      }
    }
  }
  return false;
}

void CMetroid::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    GetBodyController()->SetLocomotionType(pas::ELocomotionType::Lurk);
    x7b0_attackTarget = kInvalidUniqueId;
    x9bf_26_shotAt = false;
  }
  CPatterned::Patrol(mgr, msg, arg);
}

void CMetroid::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  // TODO
  CPatterned::TargetPatrol(mgr, msg, dt);
}

void CMetroid::TurnAround(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg != EStateMsg::Update || x7b0_attackTarget == kInvalidUniqueId) {
    return;
  }
  if (TCastToConstPtr<CActor> actor = mgr.GetObjectById(x7b0_attackTarget)) {
    const zeus::CVector3f face = actor->GetTranslation() - GetTranslation();
    if (ShouldTurn(mgr, 0.f) && face.canBeNormalized()) {
      GetBodyController()->GetCommandMgr().DeliverCmd(CBCLocomotionCmd{zeus::skZero3f, face.normalized(), 1.f});
    }
  }
}

void CMetroid::WallHang(CStateManager& mgr, EStateMsg msg, float dt) {
  // TODO
  CAi::WallHang(mgr, msg, dt);
}

} // namespace urde::MP1
