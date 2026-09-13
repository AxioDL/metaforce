#include "MetroidPrime/Enemies/CThardusRockProjectile.hpp"

#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CCameraShakeData.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Enemies/CThardus.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"

const CPatternedCollisionUtils::SSphereJointInfo CThardusRockProjectile::skJointInfoList[] = {
    {"Rock_01_Collision_LCTR", 1.5f},
};

CThardusRockProjectile::CThardusRockProjectile(const TUniqueId uid, const rstl::string& name,
                                               const CEntityInfo& info, const CTransform4f& xf,
                                               const CModelData& modelData,
                                               const CActorParameters& aParms,
                                               const CPatternedInfo& patternedInfo,
                                               const rstl::vector< CModelData >& mDataVec,
                                               const CAssetId stateMachine, const float f1)
: CPatterned(kC_ThardusRockProjectile, uid, name, kFT_Zero, info, xf, modelData, patternedInfo,
             kMT_Flyer, kCT_One, kBT_Flyer, aParms, kCS_Medium)
, x568_(1.f)
, x570_(kInvalidUniqueId)
, x572_(false)
, mAnimState(kAS_Invalid)
, x57c_(mDataVec)
, mStateMachineId(stateMachine)
, x5a0_(0)
, x5a4_(true)
, mDelayTime(0.f)
, mAttackTime(0.f)
, x5b0_(0.f, 1.f, 0.f)
, x5bc_(true)
, x5c0_(f1)
, x5c4_(0)
, x5c8_(0)
, x5cc_(0)
, mThardusId(kInvalidUniqueId)
, x5d4_(0)
, x5d8_(0)
, x5dc_(false)
, x5dd_(false) {
  CMaterialList exclude = GetMaterialFilter().GetExcludeList();
  exclude.Add(CMaterialList(kMT_Player, kMT_Character, kMT_NoPlatformCollision));
  SetMaterialFilter(
      CMaterialFilter::MakeIncludeExclude(GetMaterialFilter().GetIncludeList(), exclude));
  x50c_baseDamageMag = 1.f;
}

ENTITY_ACCEPT_IMPL(CThardusRockProjectile)

void CThardusRockProjectile::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                             CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_Registered: {
    RemoveMaterial(kMT_Solid, mgr);
    const uint count = x57c_.size();
    mDestroyableRocks.reserve(count);
    for (uint i = 0; i < count; ++i) {
      const TUniqueId rockId = mgr.AllocateUniqueId();
      CDestroyableRock* const rock = rs_new CDestroyableRock(
          rockId, true, rstl::string(skJointInfoList[i].name),
          CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList), CTransform4f::Identity(),
          x57c_[i], 0.f, *GetHealthInfo(mgr), CDamageVulnerability::NormalVulnerability(),
          GetMaterialList(), mStateMachineId,
          CActorParameters(
              CLightParameters(false, 0.f, CLightParameters::kST_Invalid, 0.f, 0.f,
                               CColor(1.f, 1.f, 1.f, 1.f), true, CLightParameters::kLO_NoShadowCast,
                               CLightParameters::kLR_Never, CVector3f(0.f, 0.f, 0.f), -1, -1, false,
                               0),
              CScannableParameters(kInvalidAssetId), rstl::pair< CAssetId, CAssetId >(0, 0),
              rstl::pair< CAssetId, CAssetId >(0, 0), CVisorParameters::None(), true, true, false,
              false, 0.f, 0.f, 1.f),
          x57c_[i], 1);
      rock->SetX340(false);
      rock->SetThermalMag(x50c_baseDamageMag);
      mgr.AddObject(rock);
      mDestroyableRocks.push_back(rockId);
    }
    AddMaterial(kMT_ScanPassthrough, mgr);
    InitializeCollisionManager(mgr);
    BodyCtrl()->Activate(mgr);
    SetActive(false);
    SetChildrenActive(mgr, false);
    break;
  }
  case kSM_Deleted: {
    mCollisionManager->Destroy(mgr);
    const uint count = mDestroyableRocks.size();
    for (uint i = 0; i < count; ++i) {
      mgr.DeleteObjectRequest(mDestroyableRocks[i]);
    }
    break;
  }
  case kSM_Damage:
    if (CCollisionActor* col = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      const uint count = mCollisionManager->GetNumCollisionActors();
      TUniqueId rockId = kInvalidUniqueId;
      for (uint i = 0; i < count; ++i) {
        if (uid == mCollisionManager->GetCollisionDescFromIndex(i).GetCollisionActorId()) {
          rockId = mDestroyableRocks[i];
          break;
        }
      }
      CDestroyableRock* rock = static_cast< CDestroyableRock* >(mgr.ObjectById(rockId));
      if (rock != nullptr) {
        rock->TakeDamage(CVector3f(0.f, 0.f, 0.f), 0.f);
        const TUniqueId touchedId = col->GetLastTouchedObject();
        const CGameProjectile* projectile =
            TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(touchedId));
        if (projectile != nullptr && mThardusId != kInvalidUniqueId) {
          CThardus* thardus = static_cast< CThardus* >(mgr.ObjectById(mThardusId));
          if (thardus != nullptr) {
            const CPlayerState::EPlayerVisor visor = mgr.GetPlayerState()->GetCurrentVisor();
            if (visor != CPlayerState::kPV_Thermal ||
                visor == CPlayerState::kPV_Thermal && thardus->GetThermalFlashState() != 3) {
              AddParticleEffect(mgr, x5c4_, projectile->GetTranslation(),
                                CVector3f(GetModelData()->GetScale()), 0);
            }
            ProcessSoundEvent(x5d4_, 1.f, 0, 0.1f, 1000.f, 20, 127, CVector3f::Zero(),
                              projectile->GetTranslation(), mgr.GetNextAreaId().Value(), mgr, true);
          }
        }
      }
    }
    break;
  case kSM_Touched: {
    CEntity* const entity = mgr.ObjectById(uid);
    if (CCollisionActor* col = TCastToPtr< CCollisionActor >(entity)) {
      const TUniqueId touchedId = col->GetLastTouchedObject();
      CEntity* const touched = mgr.ObjectById(touchedId);
      if (CPlayer* player = TCastToPtr< CPlayer >(touched)) {
        if (x420_curDamageRemTime <= 0.f) {
          const CDamageInfo damage = GetContactDamage();
          mgr.ApplyDamage(
              GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
              CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
              CVector3f::Zero());
          x420_curDamageRemTime = x424_damageWaitTime;
          if (mgr.GetPlayer()->GetFrozenState()) {
            mgr.Player()->BreakFrozenState(mgr);
          }
        }

        const uint count = mCollisionManager->GetNumCollisionActors();
        TUniqueId rockId = kInvalidUniqueId;
        for (uint i = 0; i < count; ++i) {
          if (mCollisionManager->GetCollisionDescFromIndex(i).GetCollisionActorId() == uid) {
            rockId = mDestroyableRocks[i];
            break;
          }
        }
        if (rockId != kInvalidUniqueId) {
          CDestroyableRock* rock = static_cast< CDestroyableRock* >(mgr.ObjectById(rockId));
          if (rock != nullptr && rock->GetActive()) {
            rock->SetActive(false);
            col->SetActive(false);
            ++x5a0_;
          }
        }
      }
    }
    break;
  }
  case kSM_Activate:
    SetChildrenActive(mgr, true);
    break;
  case kSM_Deactivate:
    SetChildrenActive(mgr, false);
    break;
  }
}

void CThardusRockProjectile::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (CThardus* thardus = static_cast< CThardus* >(mgr.ObjectById(mThardusId))) {
    if (!thardus->CanLockOnToRockProjectiles(mgr)) {
      RemoveMaterial(kMT_Orbit, mgr);
      RemoveMaterial(kMT_Target, mgr);
      ModifyChildrenMaterial(mgr, kMM_Remove, kMT_Orbit);
      ModifyChildrenMaterial(mgr, kMM_Remove, kMT_Target);
    } else {
      AddMaterial(kMT_Orbit, mgr);
      AddMaterial(kMT_Target, mgr);
      ModifyChildrenMaterial(mgr, kMM_Add, kMT_Orbit);
      ModifyChildrenMaterial(mgr, kMM_Add, kMT_Target);
    }
  }

  CPatterned::Think(dt, mgr);
  x3b4_speed = x5de_ ? 0.7f : 0.65f;
  SetThermalFlags(kTF_Hot);
  mCollisionManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  UpdateDestroyableRockPositions(mgr);
  UpdateDestroyableRockCollisionActors(mgr);
  if (x5a0_ >= mDestroyableRocks.size()) {
    AddDeathEffects(mgr, GetTranslation());
    DeathDelete(mgr);
  }
}

void CThardusRockProjectile::GetUp(CStateManager& mgr, const EStateMsg msg, const float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimState = kAS_NotReady;
    break;
  case kStateMsg_Update: {
    const CRayCastResult result =
        mgr.RayStaticIntersection(GetTranslation(), CVector3f::Down(), 2.f,
                                  CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid)));
    if (result.IsInvalid()) {
      if (mgr.GetCameraManager()->GetCurrentCameraId() ==
          mgr.GetCameraManager()->GetFirstPersonCamera()->GetUniqueId()) {
        const CCameraShakeData& data =
            CCameraShakeData::HardVertShakeDistance(0.25f, 0.5f, 50.f, GetTranslation());
        mgr.CameraManager()->AddCameraShaker(data, true);
      }
    } else {
      CThardus* thardus = static_cast< CThardus* >(mgr.ObjectById(mThardusId));
      if (thardus != nullptr && !x5dc_) {
        x5dc_ = true;
        const CVector3f position = result.GetPoint();
        AddParticleEffect(mgr, x5cc_, position, CVector3f(GetModelData()->GetScale()), 0);
        ProcessSoundEvent(0x7ae, 1.f, 0, 0.1f, 1000.f, 20, 127, CVector3f::Zero(), GetTranslation(),
                          mgr.GetNextAreaId().Value(), mgr, false);
      }
    }

    switch (mAnimState) {
    case kAS_NotReady:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_Getup) {
        mAnimState = kAS_Repeat;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCGetupCmd(pas::kGetup_Zero));
      }
      break;
    case kAS_Repeat:
      if (BodyCtrl()->GetCurrentStateId() != pas::kAS_Getup) {
        mAnimState = kAS_Over;
      }
      break;
    }
    break;
  }
  case kStateMsg_Deactivate:
    break;
  }
}

void CThardusRockProjectile::Patrol(CStateManager& mgr, const EStateMsg msg, const float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    break;
  case kStateMsg_Update: {
    const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    BodyCtrl()->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(CVector3f::Zero(), delta.AsNormalized(), 1.f));
    break;
  }
  case kStateMsg_Deactivate:
    break;
  }
}

void CThardusRockProjectile::LoopedAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x5a4_ = true;
    break;
  case kStateMsg_Update: {
    const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    const CVector3f delta = aimPos - GetTranslation();
    if (x5bc_ && delta.MagSquared() > x5c0_ * x5c0_) {
      x5b0_ = x45c_steeringBehaviors.Arrival(*this, aimPos, 0.f);
      x5bc_ = true;
    } else {
      x5bc_ = false;
    }

    CVector3f move = x5b0_;
    const float radius = skJointInfoList[0].radius;
    const CRayCastResult result =
        mgr.RayStaticIntersection(GetTranslation(), CVector3f::Down(), 100.f,
                                  CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid)));
    if (result.IsValid()) {
      const CVector3f& separation =
          x45c_steeringBehaviors.Separation(*this, result.GetPoint(), 2.f * radius);
      move = CVector3f(separation + x5b0_).AsNormalized();
    }
    BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(move, CVector3f::Zero(), 1.f));
    break;
  }
  case kStateMsg_Deactivate:
    break;
  }
}

void CThardusRockProjectile::Dead(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mgr.DeleteObjectRequest(GetUniqueId());
    SendScriptMsgs(kSS_MassiveDeath, mgr, kSM_None);
    GenerateDeathExplosion(mgr);
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CThardusRockProjectile::Lurk(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    break;
  case kStateMsg_Update: {
    const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    BodyCtrl()->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(CVector3f::Zero(), delta.AsNormalized(), 1.f));
    break;
  }
  case kStateMsg_Deactivate:
    break;
  }
}

bool CThardusRockProjectile::ShouldMove(CStateManager& mgr, float arg) { return mAiState >= 1; }

bool CThardusRockProjectile::ShouldAttack(CStateManager& mgr, float arg) {
  if (GetStateMachineTime() > mAttackTime && mAiState != 3) {
    mAiState = 2;
    return true;
  }

  return false;
}

bool CThardusRockProjectile::HitSomething(CStateManager& mgr, float arg) { return x572_; }

bool CThardusRockProjectile::AnimOver(CStateManager& mgr, float arg) { return mAnimState == 3; }

bool CThardusRockProjectile::Delay(CStateManager& mgr, float arg) {
  return GetStateMachineTime() > mDelayTime;
}

void CThardusRockProjectile::InitializeCollisionManager(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > joints;
  joints.reserve(1);
  AddSphereCollisionList(skJointInfoList, 1, joints);
  mCollisionManager =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints, true);
  SetMaterialProperties(mCollisionManager, mgr);

  const uint count = mCollisionManager->GetNumCollisionActors();
  for (uint i = 0; i < count; ++i) {
    const CJointCollisionDescription& desc = mCollisionManager->GetCollisionDescFromIndex(i);
    const TUniqueId colId = desc.GetCollisionActorId();
    const TUniqueId rockId = mDestroyableRocks[i];
    const CDestroyableRock* rock =
        static_cast< const CDestroyableRock* >(mgr.GetObjectById(rockId));
    if (rock != nullptr) {
      if (CCollisionActor* col = TCastToPtr< CCollisionActor >(mgr.ObjectById(colId))) {
        col->SetDamageVulnerability(*rock->GetDamageVulnerability());
        *col->HealthInfo(mgr) = *rock->GetHealthInfo(mgr);
      }
    }
  }
}

void CThardusRockProjectile::SetMaterialProperties(
    rstl::single_ptr< CCollisionActorManager >& colMgr, CStateManager& mgr) {
  for (uint i = 0; i < colMgr->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = colMgr->GetCollisionDescFromIndex(i);
    const TUniqueId id = desc.GetCollisionActorId();
    CActor* actor = static_cast< CActor* >(mgr.ObjectById(id));
    if (actor != nullptr) {
      actor->AddMaterial(kMT_ScanPassthrough, mgr);
      CMaterialList include = GetMaterialFilter().GetIncludeList();
      CMaterialList exclude = GetMaterialFilter().GetExcludeList();
      const CMaterialList actorInclude = actor->GetMaterialFilter().GetIncludeList();
      const CMaterialList actorExclude = actor->GetMaterialFilter().GetExcludeList();
      actor->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include.Union(actorInclude),
                                                                   exclude.Union(actorExclude)));
    }
  }
}

void CThardusRockProjectile::AddSphereCollisionList(
    const CPatternedCollisionUtils::SSphereJointInfo* info, int count,
    rstl::vector< CJointCollisionDescription >& list) {
  const CAnimData* animData = GetModelData()->GetAnimationData();
  for (int i = 0; i < count; ++i) {
    const CSegId seg = animData->GetLocatorSegId(rstl::string_l(info[i].name));
    if (seg != CSegId::Invalid()) {
      const CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
          seg, info[i].radius, rstl::string_l(info[i].name), 0.001f);
      list.push_back(desc);
    }
  }
}

void CThardusRockProjectile::UpdateDestroyableRockPositions(CStateManager& mgr) {
  const uint count = mDestroyableRocks.size();
  const CTransform4f& transform = GetTransform();
  const CVector3f& scale = CVector3f(GetModelData()->GetScale());
  for (uint i = 0; i < count; ++i) {
    const CTransform4f localXf = GetModelData()->GetAnimationData()->GetLocatorTransform(
        rstl::string(skJointInfoList[i].name), nullptr);
    const TUniqueId id = mDestroyableRocks[i];
    if (CActor* rock = TCastToPtr< CActor >(mgr.ObjectById(id))) {
      rock->SetTransform(transform * (CTransform4f::Scale(scale) * localXf));
    }
  }
}
void CThardusRockProjectile::UpdateDestroyableRockCollisionActors(CStateManager& mgr) {
  const uint count = mCollisionManager->GetNumCollisionActors();
  for (uint i = 0; i < count; ++i) {
    const CJointCollisionDescription& desc = mCollisionManager->GetCollisionDescFromIndex(i);
    const TUniqueId colId = desc.GetCollisionActorId();
    CCollisionActor* col = TCastToPtr< CCollisionActor >(mgr.ObjectById(colId));
    if (col != nullptr && col->GetActive()) {
      const TUniqueId rockId = mDestroyableRocks[i];
      CDestroyableRock* rock = static_cast< CDestroyableRock* >(mgr.ObjectById(rockId));
      if (rock != nullptr) {
        col->SetDamageVulnerability(
            *static_cast< const CDestroyableRock* >(rock)->GetDamageVulnerability());
        const CHealthInfo* health = col->HealthInfo(mgr);
        *rock->HealthInfo(mgr) = *health;
        const bool collided = CGameCollision::DetectStaticCollisionBoolean(
            mgr, *col->GetCollisionPrimitive(), col->GetTransform(),
            x5bc_ && mAiState != 3 ? CMaterialFilter::MakeInclude(CMaterialList(kMT_Wall))
                                   : CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid)));
        if (health->GetHP() <= 0.f || (collided && x5a4_)) {
          rock->SetActive(false);
          col->SetActive(false);
          ++x5a0_;
          if (health->GetHP() <= 0.f) {
            x5dd_ = true;
          }
        }
      }
    }
  }
}

void CThardusRockProjectile::SetChildrenActive(CStateManager& mgr, bool active) {
  const int rockCount = mDestroyableRocks.size();
  for (uint i = 0; i < rockCount; i++) {
    const TUniqueId rockId = mDestroyableRocks[i];
    const TUniqueId colId = mCollisionManager->GetCollisionDescFromIndex(i).GetCollisionActorId();
    CEntity* rockAct = mgr.ObjectById(rockId);
    if (!rockAct) {
      continue;
    }
    CEntity* colAct = mgr.ObjectById(colId);
    if (!colAct) {
      continue;
    }

    rockAct->SetActive(active);
    colAct->SetActive(active);
  }
}

void CThardusRockProjectile::ModifyChildrenMaterial(CStateManager& mgr, EModifyMaterial modify,
                                                    EMaterialTypes material) {
  const int rockCount = mDestroyableRocks.size();
  for (uint i = 0; i < rockCount; i++) {
    TUniqueId rockId = mDestroyableRocks[i];
    TUniqueId colId = mCollisionManager->GetCollisionDescFromIndex(i).GetCollisionActorId();

    CActor* rockAct = TCastToPtr< CActor >(mgr.ObjectById(rockId));

    if (!rockAct) {
      continue;
    }
    CActor* colAct = TCastToPtr< CActor >(mgr.ObjectById(colId));
    if (!colAct) {
      continue;
    }
    switch (modify) {
    case kMM_Add:
      rockAct->AddMaterial(material, mgr);
      colAct->AddMaterial(material, mgr);
      break;
    case kMM_Remove:
      rockAct->RemoveMaterial(material, mgr);
      colAct->RemoveMaterial(material, mgr);
      break;
    }
  }
}

void CThardusRockProjectile::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }

void CThardusRockProjectile::AddGravity(CStateManager& mgr) {
  SetVerticalMovement(false);
  Stop();
  SetMomentumWR(CVector3f(0.f, 0.f, 2.f * -GetWeight()));
  mAiState = 3;
}

void CThardusRockProjectile::AddDeathEffects(CStateManager& mgr, const CVector3f& position) {
  if (mThardusId == kInvalidUniqueId) {
    return;
  }

  CThardus* thardus = static_cast< CThardus* >(mgr.ObjectById(mThardusId));
  if (!thardus) {
    return;
  }

  AddParticleEffect(mgr, x5c8_, position, CVector3f(GetModelData()->GetScale()), 0);
  thardus->ShakeScreen(mgr, position, 0.75f, 125.f, 1.f);
  const CPlayer* player = mgr.GetPlayer();
  const CVector3f diff = player->GetTranslation() - GetTranslation();
  const float widthTimes10 = GetModelData()->GetScale().GetX() * 10.f;

  if (diff.MagSquared() < widthTimes10 * widthTimes10) {
    mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetUniqueId(),
                    CDamageInfo(CWeaponMode(kWT_AI), GetContactDamage().GetDamage() * 0.5f,
                                widthTimes10, GetContactDamage().GetKnockBackPower() * 0.5f),
                    CMaterialFilter::GetPassEverything(), diff.AsNormalized());
  }

  ProcessSoundEvent(x5d8_, 1.f, 0, 0.1f, 1000.f, 20, 127, CVector3f::Zero(), GetTranslation(),
                    mgr.GetNextAreaId().Value(), mgr, true);

  if (x5dd_) {
    thardus->StartTimedThermalFlash(mgr, 2.f, *this);
    SendScriptMsgs(kSS_Zero, mgr, kSM_None);
  }
}

void CThardusRockProjectile::AddParticleEffect(CStateManager& mgr, CAssetId particle,
                                               const CVector3f& position, const CVector3f& scale,
                                               int unk) {
  TUniqueId uid = mgr.AllocateUniqueId();
  char buf[1024];
  sprintf(buf, "ROCK_PROJECTILE_EFFECT-%u-%i", particle, uid.Value());

  CEntity* ent = rs_new CExplosion(
      TLockedToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', particle))), uid,
      true, CEntityInfo(mgr.GetNextAreaId(), rstl::vector< SConnection >()), rstl::string(buf),
      CTransform4f(CMatrix3f::Identity(), position), unk, scale, CColor::White());

  mgr.AddObject(*ent);
}
