#include "Runtime/MP1/World/CMetroidPrimeEssence.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/Collision/CGameCollision.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CCubeRenderer.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/World/CGameArea.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"
#include "Runtime/World/CWorld.hpp"

#include "Audio/SFX/MetroidPrime.h"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce::MP1 {
namespace {
std::array<SSphereJointInfo, 1> skJointInfo{{
    {"lockon_target_LCTR", 1.5f},
}};

std::array<u32, 4> skUnkInts1{{0, 1, 0, 2}};
std::array<u32, 3> skUnkInts2{{1, 2, 3}};

} // namespace
CMetroidPrimeEssence::CMetroidPrimeEssence(metaforce::TUniqueId uid, std::string_view name,
                                           const metaforce::CEntityInfo& info, const zeus::CTransform& xf,
                                           metaforce::CModelData&& mData, const metaforce::CPatternedInfo& pInfo,
                                           const metaforce::CActorParameters& actParms, metaforce::CAssetId particle1,
                                           const metaforce::CDamageInfo& dInfo, float f1, metaforce::CAssetId electric,
                                           u32 w1, metaforce::CAssetId particle2)
: CPatterned(ECharacter::MetroidPrimeEssence, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::Flyer, actParms, EKnockBackVariant::Medium)
, x568_(g_SimplePool->GetObj({FOURCC('PART'), particle2}))
, x574_searchPath(nullptr, 3, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x660_(particle1)
, x664_(electric)
, x698_(dInfo)
, x6b4_(xf.origin)
, x70c_(CSfxManager::TranslateSFXID(w1)) {
  CreateShadow(false);
  MakeThermalColdAndHot();
}

void CMetroidPrimeEssence::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  CPatterned::Think(dt, mgr);
  if (IsAlive()) {
    UpdatePhase(dt, mgr);
  }

  x450_bodyController->FaceDirection((mgr.GetPlayer().GetTranslation() - GetTranslation()).normalized(), dt);
  x658_collisionManager->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
  UpdateHealth(mgr);
  CountListeningAi(mgr);
  if (x70e_30_) {
    x6d4_ = 2.f * dt + x6d4_;
    if (x6d4_ >= 1.f) {
      x6d4_ = 0.f;
    }

    sub8027ce5c(-4.f * x6d4_ * (x6d4_ - 1.f));
  }
}

void CMetroidPrimeEssence::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId other, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, other, mgr);

  switch (msg) {
  case EScriptObjectMessage::Activate:
    x658_collisionManager->SetActive(mgr, true);
    break;
  case EScriptObjectMessage::Deactivate:
    x658_collisionManager->SetActive(mgr, false);
    break;
  case EScriptObjectMessage::Start:
    x70e_25_ = true;
    break;
  case EScriptObjectMessage::Stop:
    x70e_25_ = false;
    break;
  case EScriptObjectMessage::Touched: {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(other)) {
      if (colAct->GetLastTouchedObject() == mgr.GetPlayer().GetUniqueId() && x420_curDamageRemTime <= 0.f) {
        mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), GetContactDamage(),
                        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
        x420_curDamageRemTime = x424_damageWaitTime;
      }
    }
    break;
  }
  case EScriptObjectMessage::Registered: {
    SetupCollisionActorManager(mgr);
    x658_collisionManager->SetActive(mgr, true);
    x6cc_ = GetModelData()->GetScale().x();
    x6d0_ = 0.9f * x6cc_ + x6cc_;
    x55c_moveScale.splat(1.f / (0.625f * x6cc_));
    const float hp = GetHealthInfo(mgr)->GetHP();
    x6c0_ = 0.3f * hp;
    if (hp > 0.f) {
      x6c4_ = 1.f / hp;
    }
    x450_bodyController->Activate(mgr);
    break;
  }
  case EScriptObjectMessage::Deleted: {
    x658_collisionManager->Destroy(mgr);
    mgr.SetBossParams(kInvalidUniqueId, 0.f, 0);
    break;
  }
  case EScriptObjectMessage::InitializedInArea: {
    x574_searchPath.SetArea(mgr.GetWorld()->GetArea(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    x704_bossUtilityWaypointId = GetWaypointForState(mgr, EScriptObjectState::Play, EScriptObjectMessage::Activate);
    break;
  }
  case EScriptObjectMessage::Damage: {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(other)) {
      if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(colAct->GetLastTouchedObject())) {
        if (proj->GetOwnerId() == mgr.GetPlayer().GetUniqueId()) {
          if (colAct->GetDamageVulnerability()->WeaponHits(proj->GetDamageInfo().GetWeaponMode(), false) &&
              proj->GetDamageInfo().GetWeaponMode().GetType() == EWeaponType::Phazon) {
            sub8027cee0(mgr);
            TakeDamage(zeus::skForward, 1.f);
            if (!x70e_24_ && !x70e_26_) {
              GetBodyController()->GetCommandMgr().DeliverCmd(
                  CBCKnockBackCmd{GetTransform().frontVector(), pas::ESeverity::One});
              sub8027cce0(mgr);
            }
          }
        }
      }
    } else if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(other)) {
      mgr.ApplyDamage(other, x706_lockOnTargetCollider, proj->GetOwnerId(), proj->GetDamageInfo(),
                      CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
    }
    break;
  }
  default:
    break;
  }
}

void CMetroidPrimeEssence::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  CPatterned::PreRender(mgr, frustum);
}

void CMetroidPrimeEssence::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {

  if (GetActive() && x65c_) {
    g_Renderer->AddParticleGen(*x65c_);
  }
  CPatterned::AddToRenderer(frustum, mgr);
}

void CMetroidPrimeEssence::Render(CStateManager& mgr) {
  if (x70e_27_) {
    mgr.DrawSpaceWarp(x6b4_, 1.f);
  }
  CPatterned::Render(mgr);
}

zeus::CVector3f CMetroidPrimeEssence::GetAimPosition(const CStateManager& mgr, float dt) const {
  if (TCastToConstPtr<CCollisionActor> colAct = mgr.GetObjectById(x706_lockOnTargetCollider)) {
    return colAct->GetTranslation();
  }
  return CPatterned::GetAimPosition(mgr, dt);
}

void CMetroidPrimeEssence::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                                           float dt) {

  switch (type) {
  case EUserEventType::EggLay: {
    if (x70e_29_ && x6d8_ != 0 && x6e4_spawnedAiCount < x6f8_maxSpawnedCount) {
      const float ang1 = zeus::degToRad(22.5f) * mgr.GetActiveRandom()->Range(-1, 1);
      const float ang2 = zeus::degToRad(45.0f) * mgr.GetActiveRandom()->Range(-1, 1);
      zeus::CVector3f pos =
          x668_ * zeus::CVector3f{2.f * -std::sin(ang1), (2.f * (2.f * std::cos(ang1)) * std::sin(ang2)),
                                  2.f * ((2.f * std::cos(ang1)) * std::cos(ang2))};
      if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(x704_bossUtilityWaypointId)) {
        wp->SetTransform(zeus::lookAt(pos, mgr.GetPlayer().GetAimPosition(mgr, 0.f)));
        if (sub8027e870(wp->GetTransform(), mgr)) {
          SendScriptMsgs(EScriptObjectState::Zero, mgr, EScriptObjectMessage::None);
          x6b4_ = wp->GetTranslation();
        }
      }
    }
    return;
  }
  case EUserEventType::EventStart: {
    if (!x70e_31_) {
      SendScriptMsgs(EScriptObjectState::CameraTarget, mgr, EScriptObjectMessage::None);
      x70e_31_ = true;
    }
    return;
  }
  case EUserEventType::BeginAction: {
    CShockWaveInfo data(x660_, x698_, 2.f, x664_, x70c_);
    data.SetSpeedIncrease(180.f);
    CreateShockWave(mgr, data);
    ShakeCamera(mgr, 1.f);
    return;
  }
  case EUserEventType::Activate: {
    sub8027d824(mgr);
    return;
  }
  case EUserEventType::Deactivate:
    x70e_27_ = false;
    [[fallthrough]];
  default:
    break;
  }
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void CMetroidPrimeEssence::Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) {
  if (!IsAlive()) {
    return;
  }

  KillAiInArea(mgr);
  SetParticleEffectState(mgr, false);
  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x706_lockOnTargetCollider)) {
    colAct->AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
  }
  CPatterned::Death(mgr, direction, state);
}

void CMetroidPrimeEssence::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg != EStateMsg::Update || GetModelData()->GetAnimationData()->IsAnimTimeRemaining(dt, "Whole Body"sv)) {
    return;
  }

  DeathDelete(mgr);
}

void CMetroidPrimeEssence::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::PathFind(mgr, msg, dt);
  if (msg == EStateMsg::Update) {
    sub8027cb40(mgr.GetPlayer().GetTranslation());
  }
}

void CMetroidPrimeEssence::Halt(CStateManager& mgr, EStateMsg msg, float dt) {
  // Intentionally empty
}

void CMetroidPrimeEssence::Generate(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    zeus::CVector3f lookPos = mgr.GetPlayer().GetTranslation();
    lookPos.z() = GetTranslation().z();
    zeus::CTransform xf = zeus::lookAt(GetTranslation(), lookPos);
    xf.origin = GetTranslation();
    SetTransform(xf);
  } else if (msg == EStateMsg::Deactivate) {
    mgr.SetBossParams(GetUniqueId(), GetHealthInfo(mgr)->GetHP(), 91);
    SetParticleEffectState(mgr, true);
  }
}

void CMetroidPrimeEssence::JumpBack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    x700_ = sub8027cfd4(mgr, true);
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Step, &CPatterned::TryStep, x700_);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}

void CMetroidPrimeEssence::Skid(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Step, &CPatterned::TryStep, 5);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}

void CMetroidPrimeEssence::FadeIn(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x6f8_maxSpawnedCount = GetMaxSpawnCount(mgr);
    x32c_animState = EAnimState::Ready;
    x70e_24_ = true;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::ProjectileAttack, &CPatterned::TryProjectileAttack, 0);
  } else if (msg == EStateMsg::Deactivate) {
    x70e_24_ = false;
    x70e_27_ = false;
    x70e_29_ = false;
    x70e_30_ = false;
    x32c_animState = EAnimState::NotReady;
  }
}

void CMetroidPrimeEssence::FadeOut(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg != EStateMsg::Activate) {
    return;
  }

  DoPhaseTransition(mgr);
}

void CMetroidPrimeEssence::Taunt(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Taunt, &CPatterned::TryTaunt, 2);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}

void CMetroidPrimeEssence::TelegraphAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    x70e_30_ = true;
  } else if (msg == EStateMsg::Update) {
    if (!x70e_31_) {
      TryCommand(mgr, pas::EAnimationState::MeleeAttack, &CPatterned::TryMeleeAttack, 2);
    } else {
      TryCommand(mgr, pas::EAnimationState::ProjectileAttack, &CPatterned::TryProjectileAttack, 5);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    x70e_30_ = false;
    sub8027ce5c(dt);
  }
}

void CMetroidPrimeEssence::Dodge(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    x700_ = sub8027cfd4(mgr, false);
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Step, &CPatterned::TryStep, x700_);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}

void CMetroidPrimeEssence::PathFindEx(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::PathFind(mgr, msg, dt);
  if (msg == EStateMsg::Activate) {
    x70e_24_ = true;
  } else if (msg == EStateMsg::Update) {
    sub8027cb40(x2e0_destPos);
  } else if (msg == EStateMsg::Deactivate) {
    x70e_24_ = false;
  }
}

bool CMetroidPrimeEssence::HasPatrolPath(CStateManager& mgr, float dt) {
  return !x70e_31_ && CPatterned::HasPatrolPath(mgr, dt);
}

bool CMetroidPrimeEssence::ShouldAttack(CStateManager& mgr, float dt) {
  if (x70e_31_) {
    return x70e_25_;
  }
  return true;
}

bool CMetroidPrimeEssence::InPosition(CStateManager& mgr, float dt) {
  return (GetTranslation().z() - mgr.GetPlayer().GetTranslation().z()) > 0.25f;
}

bool CMetroidPrimeEssence::CoverFind(CStateManager& mgr, float dt) {
  return (x2e0_destPos - GetTranslation()).magSquared() < 90.f;
}

bool CMetroidPrimeEssence::ShouldTaunt(CStateManager& mgr, float dt) {
  const CHealthInfo* info = GetHealthInfo(mgr);
  if (!info || info->GetHP() <= x6c0_) {
    return false;
  }

  return mgr.GetActiveRandom()->Next() % 100 < 50;
}

bool CMetroidPrimeEssence::ShouldCrouch(CStateManager& mgr, float dt) {
  if (x6f0_ < x6f4_) {
    ++x6f0_;
    return false;
  }

  x6f4_ = std::min(x6e8_ + static_cast<u32>(3.f * (1.f - x6c4_ * GetHealthInfo(mgr)->GetHP())), x6ec_);
  x6f0_ = 0;
  return true;
}

bool CMetroidPrimeEssence::ShouldMove(CStateManager& mgr, float dt) { return x70e_31_; }

CPathFindSearch* CMetroidPrimeEssence::GetSearchPath() { return &x574_searchPath; }

void CMetroidPrimeEssence::sub8027cb40(const zeus::CVector3f& vec) {
  pas::EStepDirection stepDir = GetStepDirection(GetBodyController()->GetCommandMgr().GetMoveVector());
  GetBodyController()->GetCommandMgr().ClearLocomotionCmds();
  if (stepDir == pas::EStepDirection::Forward &&
      GetTransform().frontVector().normalized().dot((x2e0_destPos - GetTranslation()).normalized()) <
          zeus::degToRad(-15.f)) {
    stepDir = pas::EStepDirection::Backward;
  }

  GetBodyController()->GetCommandMgr().DeliverCmd(CBCStepCmd(stepDir, pas::EStepType::Normal));
  GetBodyController()->GetCommandMgr().DeliverTargetVector(vec - GetTranslation());
}

void CMetroidPrimeEssence::sub8027cce0(CStateManager& mgr) {
  if (CSfxManager::IsPlaying(x708_)) {
    return;
  }
  CAudioSys::C3DEmitterParmData emitterData{zeus::skZero3f, zeus::skZero3f, 1000.f, 0.1f, 1, SFXsfx0B67, 1.f,
                                            0.16f,          false,          127};
  emitterData.x0_pos = GetTargetTransform(mgr).origin;
  x708_ = CSfxManager::AddEmitter(emitterData, true, 127, false, GetAreaIdAlways());
}

zeus::CTransform CMetroidPrimeEssence::GetTargetTransform(CStateManager& mgr) {
  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x706_lockOnTargetCollider)) {
    return colAct->GetTransform();
  }

  return GetTransform();
}

void CMetroidPrimeEssence::sub8027ce5c(float dt) {
  const auto matCount = static_cast<float>(GetModelData()->GetNumMaterialSets() - 2);
  u32 iVar1 = matCount - (matCount * dt);
  if (x6fc_ != iVar1) {
    x6fc_ = iVar1;
  }
}

void CMetroidPrimeEssence::sub8027cee0(CStateManager& mgr) {
  const float hp = x6c4_ * GetHealthInfo(mgr)->GetHP();
  if (hp <= 0.f) {
    return;
  }

  bool sendMsg = false;
  if (x6d8_ == 0 && hp < 0.75f) {
    x6d8_ = 1;
  } else if (x6d8_ == 1 && hp < 0.5f) {
    sendMsg = true;
    x6d8_ = 2;
  } else if (x6d8_ == 2 && hp < 0.25f) {
    sendMsg = true;
    x6d8_ = 3;
  }

  if (sendMsg) {
    SendScriptMsgs(EScriptObjectState::DeactivateState, mgr, EScriptObjectMessage::None);
  }
}

u32 CMetroidPrimeEssence::sub8027cfd4(CStateManager& mgr, bool w1) {
  auto startIndex = static_cast<size_t>(!w1);
  zeus::CTransform xf = GetTargetTransform(mgr);
  std::array<zeus::CVector3f, 3> directions;
  directions[0] = -xf.frontVector();
  directions[2] = xf.rightVector();
  directions[1] = -directions[2];
  u32 uVar5 = 1 << size_t(startIndex);
  for (auto i = size_t(startIndex); i < 3; ++i) {
    CRayCastResult res = mgr.RayStaticIntersection(xf.origin, directions[i], 20.f, CMaterialFilter::skPassEverything);
    if (res.IsInvalid()) {
      uVar5 |= 1 << i;
    }
  }

  u32 uVar3 = 0;
  if (uVar5 < 8) {
    u32 numBits = zeus::PopCount(uVar5);
    if (numBits == 2) {
      u32 uVar1_ = mgr.GetActiveRandom()->Next();
      if ((uVar1_ & 1) == false) {
        uVar3 = (uVar5 & 1) ^ 1;
      } else {
        uVar3 = ((uVar5 >> 2) & 1) + 1;
      }
    } else if (numBits < 2) {
      uVar3 = uVar5 >> 1;
    } else if (numBits == 3) {
      uVar3 = mgr.GetActiveRandom()->Range(startIndex, 2);
    }
  }

  return skUnkInts2[uVar3];
}

void CMetroidPrimeEssence::DoPhaseTransition(CStateManager& mgr) {
  x330_stateMachineState.SetDelay(2.f);
  x70e_26_ = true;
  x70e_27_ = true;
  x6c8_ = 1.f;
  x70e_29_ = false;

  bool uVar3 = x6dc_ == skUnkInts1[size_t(mgr.GetPlayerState()->GetCurrentVisor())];
  if (skUnkInts1[size_t(mgr.GetPlayerState()->GetCurrentVisor())] == x6dc_) {
    x65c_ = std::make_unique<CElementGen>(x568_, CElementGen::EModelOrientationType::Normal,
                                          CElementGen::EOptionalSystemFlags::One);

    if (x65c_) {
      zeus::CTransform xf = GetTargetTransform(mgr);
      x65c_->SetGlobalScale(GetModelData()->GetScale());
      x65c_->SetGlobalOrientation(xf.getRotation());
      x65c_->SetGlobalTranslation(xf.origin);
    }
  }

  CSfxManager::AddEmitter(SFXsfx0B7D + uVar3, GetTranslation(), zeus::skZero3f, true, false, 127, kInvalidAreaId);
  x6e0_ = x6dc_;
  ++x6dc_;
  if (x6dc_ > 2) {
    x6dc_ = 0;
  }
}

void CMetroidPrimeEssence::ShakeCamera(CStateManager& mgr, float f1) {
  float mag = 0.5f - (0.01f * (GetTranslation() - mgr.GetPlayer().GetTranslation()).magnitude());
  if (mag < 0.f || mgr.GetPlayer().GetSurfaceRestraint() == CPlayer::ESurfaceRestraints::Air) {
    return;
  }

  mgr.GetCameraManager()->AddCameraShaker(CCameraShakeData(0.5f, mag), true);
}

void CMetroidPrimeEssence::CreateShockWave(CStateManager& mgr, const CShockWaveInfo& shockWaveData) {
  CRayCastResult res = RayStaticIntersection(mgr);
  if (res.IsInvalid()) {
    return;
  }

  mgr.AddObject(new CShockWave(mgr.AllocateUniqueId(), "Shockwave", CEntityInfo(GetAreaIdAlways(), NullConnectionList),
                               zeus::CTransform::Translate(res.GetPoint()), GetUniqueId(), shockWaveData, 1.5f, 0.5f));
}

CRayCastResult CMetroidPrimeEssence::RayStaticIntersection(CStateManager& mgr) {
  return mgr.RayStaticIntersection(GetTranslation(), -zeus::skUp, 30.f, CMaterialFilter::skPassEverything);
}

void CMetroidPrimeEssence::SetParticleEffectState(CStateManager& mgr, bool active) {
  GetModelData()->GetAnimationData()->SetParticleEffectState("Eyes"sv, active, mgr);
  GetModelData()->GetAnimationData()->SetParticleEffectState("Head"sv, active, mgr);
}

void CMetroidPrimeEssence::sub8027d824(CStateManager& mgr) {
  CRayCastResult res = RayStaticIntersection(mgr);
  if (res.IsInvalid()) {
    return;
  }

  x668_ = zeus::CTransform::Translate(res.GetPoint());
  if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(x704_bossUtilityWaypointId)) {
    wp->SetTransform(x668_);
    SendScriptMsgs(EScriptObjectState::AboutToMassivelyDie, mgr, EScriptObjectMessage::None);
    x70e_29_ = true;
  }
}

bool CMetroidPrimeEssence::sub8027e870(const zeus::CTransform& xf, CStateManager& mgr) {
  EntityList nearList;
  mgr.BuildNearList(nearList, {xf.origin - 2.f, xf.origin + 2.f}, CMaterialFilter::MakeInclude(EMaterialTypes::AIBlock),
                    this);

  CCollidableSphere sphere({zeus::skZero3f, 2.f}, CMaterialList(EMaterialTypes::Solid, EMaterialTypes::AIBlock));
  CCollisionInfoList infoList;
  TUniqueId tmpId = kInvalidUniqueId;
  CGameCollision::DetectCollision(
      mgr, sphere, xf,
      CMaterialFilter::MakeIncludeExclude(
          {EMaterialTypes ::Solid, EMaterialTypes ::Player, EMaterialTypes ::Character, EMaterialTypes ::AIBlock},
          {EMaterialTypes ::ProjectilePassthrough}),
      nearList, tmpId, infoList);

  if (infoList.GetCount() >= 1) {
    return false;
  }

  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x706_lockOnTargetCollider)) {
    zeus::CVector3f direction = (xf.origin - colAct->GetTranslation()).normalized();
    CRayCastResult res =
        mgr.RayStaticIntersection(colAct->GetTranslation(), direction, direction.magnitude(),
                                  CMaterialFilter::MakeExclude({EMaterialTypes::ProjectilePassthrough}));
    if (res.IsInvalid()) {
      return true;
    }
  }
  return false;
}

void CMetroidPrimeEssence::KillAiInArea(CStateManager& mgr) {
  for (auto* ent : mgr.GetListeningAiObjectList()) {
    if (TCastToPtr<CPatterned> ai = ent) {
      if (ai != this && ai->GetActive() && ai->GetAreaIdAlways() == GetAreaIdAlways()) {
        ai->MassiveDeath(mgr);
      }
    }
  }
}

void CMetroidPrimeEssence::CountListeningAi(CStateManager& mgr) {
  x6e0_ = 0;
  for (auto* ent : mgr.GetListeningAiObjectList()) {
    if (TCastToPtr<CPatterned> ai = ent) {
      if (ai != this && ai->GetActive() && ai->GetAreaIdAlways() == GetAreaIdAlways()) {
        ++x6e4_spawnedAiCount;
      }
    }
  }
}

void CMetroidPrimeEssence::UpdatePhase(float dt, CStateManager& mgr) {
  if (skUnkInts1[size_t(mgr.GetPlayerState()->GetCurrentVisor())] == x6dc_) {
    x42c_color.a() = 1.f - x6c8_;
    GetModelData()->SetScale(zeus::CVector3f((x6cc_ - x6d0_) + x6d0_));
    if (!x70e_28_) {
      AddMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
      SetParticleEffectState(mgr, true);
      x70e_28_ = true;
    }
  } else {
    x42c_color.a() = skUnkInts1[size_t(mgr.GetPlayerState()->GetCurrentVisor())] == x6e0_ ? x6c8_ : 0.f;
    GetModelData()->SetScale(zeus::CVector3f((x6cc_ - x6d0_) + x6d0_));
    if (x70e_28_) {
      RemoveMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
      SetParticleEffectState(mgr, false);
      x70e_28_ = false;
    }
  }

  zeus::CTransform xf = GetTargetTransform(mgr);
  if (x70e_26_) {
    x6c8_ -= 0.5f * dt;
    x6b4_ = xf.origin;
    if (x6c8_ < 0.f) {
      x6c8_ = 0.f;
      x70e_26_ = false;
      x70e_27_ = false;
    }
  }

  if (!x65c_) {
    return;
  }

  if (!x65c_->IsSystemDeletable()) {
    x65c_->SetGlobalOrientation(xf.getRotation());
    x65c_->SetGlobalTranslation(xf.origin);
    x65c_->Update(dt);
  } else {
    x65c_.reset();
  }
}

void CMetroidPrimeEssence::UpdateHealth(CStateManager& mgr) {
  if (!IsAlive()) {
    return;
  }

  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x706_lockOnTargetCollider)) {
    colAct->SetDamageVulnerability(*GetDamageVulnerability());
    HealthInfo(mgr)->SetHP(colAct->GetHealthInfo(mgr)->GetHP());
  }

  if (GetHealthInfo(mgr)->GetHP() <= 0.f) {
    Death(mgr, zeus::skZero3f, EScriptObjectState::DeathRattle);
    RemoveMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
  }
}

void CMetroidPrimeEssence::SetLockOnTargetHealthAndDamageVulns(CStateManager& mgr) {
  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x706_lockOnTargetCollider)) {
    *colAct->HealthInfo(mgr) = *HealthInfo(mgr);
    colAct->SetDamageVulnerability(*GetDamageVulnerability());
  }
}

void CMetroidPrimeEssence::AddSphereCollisions(SSphereJointInfo* info, size_t count,
                                               std::vector<CJointCollisionDescription>& vecOut) {
  const CAnimData* animData = GetModelData()->GetAnimationData();
  for (size_t i = 0; i < count; ++i) {
    CSegId segId = animData->GetLocatorSegId(info[i].name);
    if (segId.IsInvalid()) {
      continue;
    }
    vecOut.push_back(CJointCollisionDescription::SphereCollision(segId, info[i].radius, info[i].name, 1000.f));
  }
}

void CMetroidPrimeEssence::SetupCollisionActorManager(CStateManager& mgr) {
  std::vector<CJointCollisionDescription> joints;
  AddSphereCollisions(skJointInfo.data(), skJointInfo.size(), joints);
  x658_collisionManager =
      std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), joints, false);

  for (size_t i = 0; i < x658_collisionManager->GetNumCollisionActors(); ++i) {
    const auto& info = x658_collisionManager->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(info.GetCollisionActorId())) {
      if (info.GetName() == "lockon_target_LCTR"sv) {
        x706_lockOnTargetCollider = info.GetCollisionActorId();
      }
    }
  }

  SetLockOnTargetHealthAndDamageVulns(mgr);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      {EMaterialTypes::Solid}, {EMaterialTypes::CollisionActor, EMaterialTypes::Player, EMaterialTypes::Character}));
  AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
}

} // namespace metaforce::MP1
