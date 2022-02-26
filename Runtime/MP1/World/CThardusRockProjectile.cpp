#include "Runtime/MP1/World/CThardusRockProjectile.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Camera/CCameraManager.hpp"
#include "Runtime/Camera/CFirstPersonCamera.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/Collision/CGameCollision.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CDestroyableRock.hpp"
#include "Runtime/World/CExplosion.hpp"
#include "Runtime/World/CLightParameters.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "Runtime/Factory/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"

#include "Runtime/MP1/World/CThardus.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path
namespace metaforce::MP1 {
namespace {
constexpr std::array<SSphereJointInfo, 1> skRockCollisions{{
    {"Rock_01_Collision_LCTR", 1.5f},
}};
} // namespace

CThardusRockProjectile::CThardusRockProjectile(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                               const zeus::CTransform& xf, CModelData&& modelData,
                                               const CActorParameters& aParms, const CPatternedInfo& patternedInfo,
                                               std::vector<CStaticRes>&& mDataVec, CAssetId stateMachine, float f1)
: CPatterned(ECharacter::ThardusRockProjectile, uid, name, EFlavorType::Zero, info, xf, std::move(modelData),
             patternedInfo, EMovementType::Flyer, EColliderType::One, EBodyType::Flyer, aParms,
             EKnockBackVariant::Medium)
, x57c_(std::move(mDataVec))
, x59c_stateMachine(stateMachine)
, x5c0_(f1) {
  CMaterialList exclude = GetMaterialFilter().GetExcludeList();
  exclude.Add(EMaterialTypes::Player);
  exclude.Add(EMaterialTypes::Character);
  exclude.Add(EMaterialTypes::NoPlatformCollision);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(GetMaterialFilter().GetIncludeList(), exclude));
  x50c_baseDamageMag = 1.f;
}

void CThardusRockProjectile::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (auto* thardus = static_cast<CThardus*>(mgr.ObjectById(x5d0_thardusId))) {
    if (!thardus->sub801db5b4(mgr)) {
      RemoveMaterial(EMaterialTypes::Orbit, mgr);
      RemoveMaterial(EMaterialTypes::Target, mgr);
      ModifyActorMaterial(mgr, true, EMaterialTypes::Orbit);
      ModifyActorMaterial(mgr, true, EMaterialTypes::Target);
    } else {
      AddMaterial(EMaterialTypes::Orbit, mgr);
      AddMaterial(EMaterialTypes::Target, mgr);
      ModifyActorMaterial(mgr, false, EMaterialTypes::Orbit);
      ModifyActorMaterial(mgr, false, EMaterialTypes::Target);
    }
  }
  CPatterned::Think(dt, mgr);
  x3b4_speed = x5de_ ? 0.7f : 0.65f;
  xe6_27_thermalVisorFlags = 1;
  x578_collisionManager->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
  UpdateDestroyableRockPositions(mgr);
  UpdateDestroyableRockCollisionActors(mgr);
  if (x58c_destroyableRocks.size() <= x5a0_) {
    ExplodeAndShake(mgr, GetTranslation());
    DeathDelete(mgr);
  }
}

void CThardusRockProjectile::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId other, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, other, mgr);

  switch (msg) {
  case EScriptObjectMessage::Touched: {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(other)) {
      if (TCastToPtr<CPlayer> player = mgr.ObjectById(colAct->GetLastTouchedObject())) {
        if (x420_curDamageRemTime <= 0.f) {
          mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
                          CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
          x420_curDamageRemTime = x424_damageWaitTime;
          if (mgr.GetPlayer().GetFrozenState()) {
            mgr.GetPlayer().UnFreeze(mgr);
          }
        }

        TUniqueId rockId = kInvalidUniqueId;
        for (size_t j = 0; j < x578_collisionManager->GetNumCollisionActors(); ++j) {
          if (x578_collisionManager->GetCollisionDescFromIndex(j).GetCollisionActorId() == other) {
            rockId = x58c_destroyableRocks[j];
          }
        }

        if (rockId != kInvalidUniqueId) {
          if (auto rock = static_cast<CDestroyableRock*>(mgr.ObjectById(rockId))) {
            if (rock->GetActive()) {
              rock->SetActive(false);
              colAct->SetActive(false);
              ++x5a0_;
            }
          }
        }
      }
    }
    break;
  }
  case EScriptObjectMessage::Deactivate: {
    SetChildrenActive(mgr, false);
    break;
  }
  case EScriptObjectMessage::Activate: {
    SetChildrenActive(mgr, true);
    break;
  }
  case EScriptObjectMessage::Deleted: {
    x578_collisionManager->Destroy(mgr);
    for (TUniqueId& uid : x58c_destroyableRocks) {
      mgr.FreeScriptObject(uid);
    }
    break;
  }
  case EScriptObjectMessage::Registered: {
    RemoveMaterial(EMaterialTypes::Solid, mgr);
    x58c_destroyableRocks.reserve(x57c_.size());

    for (size_t i = 0; i < x57c_.size(); ++i) {
      TUniqueId uid = mgr.AllocateUniqueId();
      CModelData mData{x57c_[i]};
      CHealthInfo hInfo = *GetHealthInfo(mgr);
      CActorParameters actParms(CLightParameters(false, 0.f, CLightParameters::EShadowTesselation::Invalid, 0.f, 0.f,
                                                 zeus::skWhite, true,
                                                 CLightParameters::EWorldLightingOptions::NoShadowCast,
                                                 CLightParameters::ELightRecalculationOptions::LargeFrameCount,
                                                 zeus::skZero3f, -1, -1, false, 0),
                                CScannableParameters(), {}, {}, {}, true, true, false, false, 0.f, 0.f, 1.f);
      auto rock = new CDestroyableRock(uid, true, skRockCollisions[i].name,
                                       CEntityInfo(GetAreaIdAlways(), CEntity::NullConnectionList), zeus::CTransform(),
                                       std::move(mData), 0.f, hInfo, CDamageVulnerability::NormalVulnerabilty(),
                                       GetMaterialList(), x59c_stateMachine, actParms, x57c_[i], 1);
      rock->Set_x340(false);
      rock->SetThermalMag(x50c_baseDamageMag);
      mgr.AddObject(rock);
      x58c_destroyableRocks.push_back(uid);
    }
    AddMaterial(EMaterialTypes::ScanPassthrough);
    InitializeCollisionManager(mgr);
    x450_bodyController->Activate(mgr);
    SetActive(false);
    SetChildrenActive(mgr, false);
    break;
  }
  case EScriptObjectMessage::Damage: {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(other)) {
      TUniqueId rockId = kInvalidUniqueId;
      for (size_t j = 0; j < x578_collisionManager->GetNumCollisionActors(); ++j) {
        if (other == x578_collisionManager->GetCollisionDescFromIndex(j).GetCollisionActorId()) {
          rockId = x58c_destroyableRocks[j];
        }
      }
      if (auto rock = static_cast<CDestroyableRock*>(mgr.ObjectById(rockId))) {
        rock->TakeDamage(zeus::skZero3f, 0.f);
        // NOTE(phil): Accessing Thardus as const here rather than non-const like the vanilla version
        // since we're not modifying him in any way
        if (const auto* thardus = static_cast<const CThardus*>(mgr.GetObjectById(x5d0_thardusId))) {
          if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Thermal ||
              thardus->Get_x7c4() != 3) {
            DoExplosion(mgr, x5c4_, GetTranslation(), GetModelData()->GetScale(), 0);
          }
          ProcessSoundEvent(x5d4_, 1.f, 0, 0.1f, 1000.f, 0.16f, 1.f, zeus::skZero3f, GetTranslation(),
                            mgr.GetNextAreaId(), mgr, true);
        }
      }
    }
    break;
  }
  default:
    break;
  }
}

void CThardusRockProjectile::Render(CStateManager& mgr) { CPatterned::Render(mgr); }

void CThardusRockProjectile::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg != EStateMsg::Update) {
    return;
  }

  GetBodyController()->GetCommandMgr().DeliverCmd(
      CBCLocomotionCmd(zeus::skZero3f, (mgr.GetPlayer().GetTranslation() - GetTranslation()).normalized(), 1.f));
}

void CThardusRockProjectile::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg != EStateMsg::Activate) {
    return;
  }

  mgr.FreeScriptObject(GetUniqueId());
  SendScriptMsgs(EScriptObjectState::MassiveDeath, mgr, EScriptObjectMessage::None);
  GenerateDeathExplosion(mgr);
}

void CThardusRockProjectile::LoopedAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x5a4_ = true;
  } else if (msg == EStateMsg::Update) {
    zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
    if (!x5bc_ || (aimPos - GetTranslation()).magSquared() <= x5c0_ * x5c0_) {
      x5bc_ = false;
    } else {
      x5b0_ = x45c_steeringBehaviors.Arrival(*this, aimPos, 0.f);
      x5bc_ = true;
    }

    zeus::CVector3f movePos = x5b0_;
    float radius = skRockCollisions[0].radius;
    auto result = mgr.RayStaticIntersection(GetTranslation(), zeus::skDown, 100.f,
                                            CMaterialFilter::MakeInclude({EMaterialTypes::Solid}));

    if (result.IsValid()) {
      movePos = (x45c_steeringBehaviors.Separation(*this, result.GetPoint(), 2.f * radius) + x5b0_).normalized();
    }
    GetBodyController()->GetCommandMgr().DeliverCmd(CBCLocomotionCmd{movePos, zeus::skZero3f, 1.f});
  }
}

void CThardusRockProjectile::GetUp(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x574_ = EAnimState::NotReady;
  } else if (msg == EStateMsg::Update) {
    auto result = mgr.RayStaticIntersection(GetTranslation(), zeus::skDown, 2.f,
                                            CMaterialFilter::MakeInclude({EMaterialTypes::Solid}));
    if (result.IsInvalid()) {
      CThardus* thardus = static_cast<CThardus*>(mgr.ObjectById(x5d0_thardusId));
      if (thardus != nullptr && !x5dc_) {
        x5dc_ = true;
        DoExplosion(mgr, x5cc_, result.GetPoint(), GetModelData()->GetScale(), 0);
        ProcessSoundEvent(SFXsfx07AE, 1.f, 0, 0.1f, 1.f, 0.16f, 1.f, zeus::skZero3f, GetTranslation(),
                          mgr.GetNextAreaId(), mgr, false);
      }
    } else if (mgr.GetCameraManager()->GetCurrentCameraId() ==
               mgr.GetCameraManager()->GetFirstPersonCamera()->GetUniqueId()) {
      const CCameraShakeData data = CCameraShakeData::BuildMissileCameraShake(0.25f, 0.5f, 50.f, GetTranslation());
      mgr.GetCameraManager()->AddCameraShaker(data, true);
    }

    if (x574_ == EAnimState::NotReady) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Getup) {
        x574_ = EAnimState::Repeat;
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCGetupCmd{pas::EGetupType::Zero});
      }
    } else if (x574_ == EAnimState::Repeat &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Getup) {
      x574_ = EAnimState::Over;
    }
  }
}

void CThardusRockProjectile::Lurk(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg != EStateMsg::Update) {
    return;
  }

  GetBodyController()->GetCommandMgr().DeliverCmd(
      CBCLocomotionCmd{zeus::skZero3f, (mgr.GetPlayer().GetTranslation() - GetTranslation()).normalized(), 1.f});
}

bool CThardusRockProjectile::Delay(CStateManager& mgr, float arg) { return x5a8_ < x330_stateMachineState.GetTime(); }

bool CThardusRockProjectile::AnimOver(CStateManager& mgr, float arg) { return x574_ == EAnimState::Over; }

bool CThardusRockProjectile::ShouldAttack(CStateManager& mgr, float arg) {
  if (x5ac_ >= x330_stateMachineState.GetTime() || x56c_ == 3) {
    return false;
  }
  x56c_ = 2;
  return true;
}

bool CThardusRockProjectile::HitSomething(CStateManager& mgr, float arg) { return x572_; }

bool CThardusRockProjectile::ShouldMove(CStateManager& mgr, float arg) { return x56c_ != 0; }

void CThardusRockProjectile::SetChildrenActive(CStateManager& mgr, bool active) {
  for (size_t i = 0; i < x58c_destroyableRocks.size(); ++i) {
    const TUniqueId uid = x58c_destroyableRocks[i];
    const auto& jInfo = x578_collisionManager->GetCollisionDescFromIndex(i);
    if (auto rock = static_cast<CDestroyableRock*>(mgr.ObjectById(uid))) {
      if (auto colAct = static_cast<CCollisionActor*>(mgr.ObjectById(jInfo.GetCollisionActorId()))) {
        rock->SetActive(active);
        colAct->SetActive(active);
      }
    }
  }
}

void CThardusRockProjectile::InitializeCollisionManager(CStateManager& mgr) {
  std::vector<CJointCollisionDescription> joints;
  AddSphereCollisionList(skRockCollisions.data(), 1, joints);
  x578_collisionManager = std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), joints, true);
  SetMaterialProperties(x578_collisionManager, mgr);

  for (size_t j = 0; j < x578_collisionManager->GetNumCollisionActors(); ++j) {
    const CJointCollisionDescription& colDesc = x578_collisionManager->GetCollisionDescFromIndex(j);
    auto rock = static_cast<const CDestroyableRock*>(mgr.GetObjectById(x58c_destroyableRocks[j]));
    if (rock != nullptr) {
      if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(colDesc.GetCollisionActorId())) {
        colAct->SetDamageVulnerability(*rock->GetDamageVulnerability());
        *colAct->HealthInfo(mgr) = *rock->GetHealthInfo(mgr);
      }
    }
  }
}
void CThardusRockProjectile::AddSphereCollisionList(const SSphereJointInfo* info, size_t count,
                                                    std::vector<CJointCollisionDescription>& vecOut) {
  const auto* animData = GetModelData()->GetAnimationData();
  for (size_t i = 0; i < count; ++i) {
    CSegId seg = animData->GetLocatorSegId(info[i].name);
    if (seg.IsInvalid()) {
      continue;
    }
    vecOut.push_back(CJointCollisionDescription::SphereCollision(seg, info[i].radius, info[i].name, 0.001f));
  }
}
void CThardusRockProjectile::SetMaterialProperties(const std::unique_ptr<CCollisionActorManager>& colMgr,
                                                   CStateManager& mgr) {
  for (size_t j = 0; j < colMgr->GetNumCollisionActors(); ++j) {
    const CJointCollisionDescription& desc = colMgr->GetCollisionDescFromIndex(j);
    if (auto colAct = static_cast<CCollisionActor*>(mgr.ObjectById(desc.GetCollisionActorId()))) {
      CMaterialList include = GetMaterialFilter().GetIncludeList();
      CMaterialList exclude = GetMaterialFilter().GetExcludeList();
      include.Add(colAct->GetMaterialFilter().GetIncludeList());
      exclude.Add(colAct->GetMaterialFilter().GetExcludeList());
      colAct->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
    }
  }
}
void CThardusRockProjectile::DoExplosion(CStateManager& mgr, CAssetId particleId, const zeus::CVector3f& pos,
                                         const zeus::CVector3f& scale, u32 w2) {
  TUniqueId uid = mgr.AllocateUniqueId();
  std::string name = fmt::format(FMT_STRING("ROCK_PROJECTILE_EFFECT-{}-{}"), particleId.Value(), uid.Value());
  TLockedToken<CGenDescription> descTok = g_SimplePool->GetObj({SBIG('PART'), particleId});
  mgr.AddObject(new CExplosion(descTok, uid, true, CEntityInfo(mgr.GetNextAreaId(), NullConnectionList), name,
                               zeus::CTransform(zeus::CMatrix3f(), pos), w2, scale, zeus::skWhite));
}
void CThardusRockProjectile::ExplodeAndShake(CStateManager& mgr, const zeus::CVector3f& pos) {
  if (x5d0_thardusId == kInvalidUniqueId) {
    return;
  }

  if (auto thardus = static_cast<CThardus*>(mgr.ObjectById(x5d0_thardusId))) {
    DoExplosion(mgr, x5c8_, pos, GetModelData()->GetScale(), 0);
    thardus->ApplyCameraShake(0.75f, 125.f, 1.f, mgr, pos);
  }
}
void CThardusRockProjectile::ModifyActorMaterial(CStateManager& mgr, bool remove, EMaterialTypes mat) {
  for (size_t i = 0; i < x58c_destroyableRocks.size(); ++i) {
    TUniqueId rockId = x58c_destroyableRocks[i];
    const auto& jInfo = x578_collisionManager->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CActor> colAct = mgr.ObjectById(jInfo.GetCollisionActorId())) {
      if (TCastToPtr<CActor> rock = mgr.ObjectById(rockId)) {
        if (remove) {
          colAct->RemoveMaterial(mat, mgr);
          rock->RemoveMaterial(mat, mgr);
        } else {
          colAct->AddMaterial(mat, mgr);
          rock->AddMaterial(mat, mgr);
        }
      }
    }
  }
}
void CThardusRockProjectile::UpdateDestroyableRockPositions(CStateManager& mgr) {
  const zeus::CVector3f scale = GetModelData()->GetScale();
  for (size_t i = 0; i < x58c_destroyableRocks.size(); ++i) {
    zeus::CTransform locatorXf =
        GetModelData()->GetAnimationData()->GetLocatorTransform(skRockCollisions[i].name, nullptr);
    if (TCastToPtr<CActor> rock = mgr.ObjectById(x58c_destroyableRocks[i])) {
      locatorXf = GetTransform() * (zeus::CTransform::Scale(scale) * locatorXf);
      rock->SetTransform(locatorXf);
    }
  }
}
void CThardusRockProjectile::UpdateDestroyableRockCollisionActors(CStateManager& mgr) {
  for (size_t j = 0; j < x578_collisionManager->GetNumCollisionActors(); ++j) {
    const auto jInfo = x578_collisionManager->GetCollisionDescFromIndex(j);
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(jInfo.GetCollisionActorId())) {
      if (!colAct->GetActive()) {
        continue;
      }
      if (auto rock = static_cast<CDestroyableRock*>(mgr.ObjectById(x58c_destroyableRocks[j]))) {
        colAct->SetDamageVulnerability(*rock->GetDamageVulnerability());
        const CHealthInfo* hInfo = colAct->GetHealthInfo(mgr);
        *rock->HealthInfo(mgr) = *hInfo;
        CMaterialFilter filter = (x5bc_ && x56c_ != 3) ? CMaterialFilter::MakeInclude({EMaterialTypes::Wall})
                                                       : CMaterialFilter::MakeInclude({EMaterialTypes::Solid});
        bool collided = CGameCollision::DetectStaticCollisionBoolean(mgr, *colAct->GetCollisionPrimitive(),
                                                                     colAct->GetTransform(), filter);
        if (hInfo->GetHP() <= 0.f || (collided && x5a4_)) {
          rock->SetActive(false);
          colAct->SetActive(false);
          ++x5a0_;
          if (hInfo->GetHP() <= 0.f) {
            x5dd_ = true;
          }
        }
      }
    }
  }
}
} // namespace metaforce::MP1
