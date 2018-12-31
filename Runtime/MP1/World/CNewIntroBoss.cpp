#include "MP1/World/CNewIntroBoss.hpp"
#include "Character/CCharLayoutInfo.hpp"
#include "Collision/CCollisionActor.hpp"
#include "Collision/CCollisionActorManager.hpp"
#include "Collision/CGameCollision.hpp"
#include "Collision/CJointCollisionDescription.hpp"
#include "Weapon/CPlasmaProjectile.hpp"
#include "World/CPlayer.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"
#include "CNewIntroBoss.hpp"

namespace urde::MP1 {

CNewIntroBoss::CNewIntroBoss(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                             CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
                             float f1, CAssetId projectile, const CDamageInfo& dInfo, CAssetId part1, CAssetId part2,
                             CAssetId tex1, CAssetId tex2)
: CPatterned(ECharacter::NewIntroBoss, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::Restricted, actParms, EKnockBackVariant::Medium)
, x570_(f1)
, x574_boneTracking(*GetModelData()->GetAnimationData(), "Head_1"sv, zeus::degToRad(80.f), zeus::degToRad(180.f), false)
, x5ac_projectileInfo(projectile, dInfo)
, x5f0_(part1)
, x5f4_(part2)
, x5f8_(tex1)
, x5fc_(tex2) {
  const_cast<TToken<CWeaponDescription>*>(&x5ac_projectileInfo.Token())->Lock();
  x574_boneTracking.SetActive(true);
}

void CNewIntroBoss::Accept(IVisitor& visitor) { visitor.Visit(this); }

static const SSphereJointInfo skSphereJoints[] = {{"Head_1", 1.5f}, {"Tail_1", 1.5f}};

static const SOBBJointInfo skOBBJoints[] = {
    {"Pelvis", "Spine_3", {4.f, 1.f, 4.f}},
    {"Spine_3", "Tail_1", {2.f, 1.f, 2.f}},
    {"Tail_1", "Tail_2", {1.f, 1.f, 1.f}},
    {"Tail_2", "Tail_3", {1.f, 1.f, 1.f}},
    {"Tail_3", "Tail_4", {1.f, 1.f, 1.f}},
    {"R_shoulder_front", "R_elbow_front", {.5f, .5f, .5f}},
    {"R_elbow_front", "R_wrist_front", {.5f, .5f, .5f}},
    {"L_shoulder_front", "L_elbow_front", {.5f, .5f, .5f}},
    {"L_elbow_front", "L_wrist_front", {.5f, .5f, .5f}},
    {"R_shoulder_back", "R_elbow_back", {.5f, .5f, .5f}},
    {"R_elbow_back", "R_wrist_back", {.5f, .5f, .5f}},
    {"L_shoulder_back", "L_elbow_back", {.5f, .5f, .5f}},
    {"L_elbow_back", "L_wrist_back", {.5f, .5f, .5f}},
};

void CNewIntroBoss::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::Registered) {
    RemoveMaterial(EMaterialTypes::Solid, mgr);
    RemoveMaterial(EMaterialTypes::Target, mgr);
    RemoveMaterial(EMaterialTypes::Orbit, mgr);
    RemoveMaterial(EMaterialTypes::Occluder, mgr);
    x450_bodyController->Activate(mgr);

    if (x5d4_stage1Projectile == kInvalidUniqueId) {
      CBeamInfo stage1BeamInfo = CBeamInfo(3, x5f0_, x5f4_, x5f8_, x5fc_, 50, 1.f, 1.f, 1.5f, 20.f, 1.f, 4.f, 8.f,
                                           zeus::CColor::skYellow, zeus::CColor(0.1098f, 0.5764f, 0.1592f), 150.f);
      CBeamInfo stage2BeamInfo(3, x5f0_, x5f4_, x5f8_, x5fc_, 50, 1.f, 1.f, 2.f, 20.f, 1.f, 4.f, 8.f,
                               zeus::CColor::skYellow, zeus::CColor(0.1098f, 0.5764f, 0.1592f), 150.f);

      x5d4_stage1Projectile = mgr.AllocateUniqueId();
      x5d6_stage2Projectile = mgr.AllocateUniqueId();
      x5d8_stage3Projectile = mgr.AllocateUniqueId();
      CPlasmaProjectile* stage1Projectile =
          new CPlasmaProjectile(x5ac_projectileInfo.Token(), "IntroBoss_Beam"sv, EWeaponType::AI, stage1BeamInfo, {},
                                EMaterialTypes::Character, x5ac_projectileInfo.GetDamage(), x5d4_stage1Projectile,
                                GetAreaIdAlways(), GetUniqueId(), 8, true, EProjectileAttrib::KeepInCinematic);
      CPlasmaProjectile* stage2Projectile =
          new CPlasmaProjectile(x5ac_projectileInfo.Token(), "IntroBoss_Beam_Stage2"sv, EWeaponType::AI, stage2BeamInfo,
                                {}, EMaterialTypes::Character, x5ac_projectileInfo.GetDamage(), x5d6_stage2Projectile,
                                GetAreaIdAlways(), GetUniqueId(), 8, true, EProjectileAttrib::KeepInCinematic);
      CPlasmaProjectile* stage3Projectile =
          new CPlasmaProjectile(x5ac_projectileInfo.Token(), "IntroBoss_Beam_Stage2"sv, EWeaponType::AI, stage2BeamInfo,
                                {}, EMaterialTypes::Character, x5ac_projectileInfo.GetDamage(), x5d8_stage3Projectile,
                                GetAreaIdAlways(), GetUniqueId(), 8, true, EProjectileAttrib::KeepInCinematic);
      mgr.AddObject(stage1Projectile);
      mgr.AddObject(stage2Projectile);
      mgr.AddObject(stage3Projectile);
      x676_curProjectile = x5d4_stage1Projectile;
    }

    std::vector<CJointCollisionDescription> jointCollisions;
    jointCollisions.reserve(15);

    const CAnimData* animData = GetModelData()->GetAnimationData();
    for (const SSphereJointInfo& joint : skSphereJoints) {
      CSegId seg = animData->GetLocatorSegId(joint.name);
      jointCollisions.push_back(CJointCollisionDescription::SphereCollision(seg, joint.radius, joint.name, 0.001f));
    }

    for (const SOBBJointInfo& joint : skOBBJoints) {
      CSegId from = animData->GetLocatorSegId(joint.from);
      CSegId to = animData->GetLocatorSegId(joint.to);
      jointCollisions.push_back(CJointCollisionDescription::OBBAutoSizeCollision(
          from, to, joint.bounds, CJointCollisionDescription::EOrientationType::One, joint.from, 0.001f));
    }

    x5ec_collisionManager.reset(
        new CCollisionActorManager(mgr, GetUniqueId(), GetAreaIdAlways(), jointCollisions, GetActive()));
    x640_initialHp = GetHealthInfo(mgr)->GetHP();

    for (u32 i = 0; i < x5ec_collisionManager->GetNumCollisionActors(); ++i) {
      const CJointCollisionDescription& desc = x5ec_collisionManager->GetCollisionDescFromIndex(i);
      TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(desc.GetCollisionActorId());
      if (desc.GetName() == skSphereJoints[0].name) {
        x600_headActor = desc.GetCollisionActorId();
        if (colAct) {
          CHealthInfo* thisHealthInfo = HealthInfo(mgr);
          CHealthInfo* colHealthInfo = colAct->HealthInfo(mgr);
          *colHealthInfo = *thisHealthInfo;
          colAct->SetDamageVulnerability(*GetDamageVulnerability());
          colAct->RemoveMaterial(EMaterialTypes::Orbit, mgr);
        }
      } else if (desc.GetName() == skOBBJoints[0].from) {
        x602_pelvisActor = desc.GetCollisionActorId();
        if (colAct) {
          CHealthInfo* thisHealthInfo = HealthInfo(mgr);
          CHealthInfo* colHealthInfo = colAct->HealthInfo(mgr);
          *colHealthInfo = *thisHealthInfo;
          colAct->SetDamageVulnerability(CDamageVulnerability::NormalVulnerabilty());
          colAct->AddMaterial(EMaterialTypes::Orbit, mgr);
          MoveScannableObjectInfoToActor(colAct, mgr);
        }
      } else
        colAct->RemoveMaterial(EMaterialTypes::Orbit, mgr);
    }
  } else if (msg == EScriptObjectMessage::Deleted) {
    DeleteBeam(mgr);
    x5ec_collisionManager->Destroy(mgr);
  } else if (msg == EScriptObjectMessage::Damage) {
    if (uid == x600_headActor || uid == x602_pelvisActor)
      TakeDamage({}, 0.f);
  }

  bool active = GetActive();
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  if (active == GetActive())
    return;

  if (x5ec_collisionManager)
    x5ec_collisionManager->SetActive(mgr, GetActive());
  x63c_attackTime = 8.f;
}

pas::ELocomotionType CNewIntroBoss::GetLocoForHealth(const CStateManager& mgr) const {
  float hp = GetHealthInfo(mgr)->GetHP();

  if (hp > .66f * x640_initialHp)
    return pas::ELocomotionType::Relaxed;
  else if (hp > .33f * x640_initialHp)
    return pas::ELocomotionType::Lurk;

  return pas::ELocomotionType::Combat;
}

void CNewIntroBoss::OnScanStateChanged(EScanState state, CStateManager& mgr) {
  CPatterned::OnScanStateChanged(state, mgr);
  if (state != EScanState::Done)
    return;

  TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x600_headActor);
  if (colAct)
    colAct->AddMaterial(EMaterialTypes::Orbit, mgr);
  colAct = mgr.ObjectById(x602_pelvisActor);
  if (colAct)
    colAct->RemoveMaterial(EMaterialTypes::Orbit, mgr);
}

void CNewIntroBoss::DeleteBeam(CStateManager& mgr) {
  if (x5d4_stage1Projectile != kInvalidUniqueId) {
    mgr.FreeScriptObject(x5d4_stage1Projectile);
    x5d4_stage1Projectile = kInvalidUniqueId;
  }
  if (x5d6_stage2Projectile != kInvalidUniqueId) {
    mgr.FreeScriptObject(x5d6_stage2Projectile);
    x5d6_stage2Projectile = kInvalidUniqueId;
  }
  if (x5d8_stage3Projectile != kInvalidUniqueId) {
    mgr.FreeScriptObject(x5d8_stage3Projectile);
    x5d8_stage3Projectile = kInvalidUniqueId;
  }

  StopRumble(mgr);
}

void CNewIntroBoss::StopRumble(CStateManager& mgr) {
  if (x674_rumbleVoice == -1)
    return;

  mgr.GetRumbleManager().StopRumble(x674_rumbleVoice);
  x674_rumbleVoice = -1;
}

void CNewIntroBoss::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  if (x638_ < 0.2f)
    x638_ += dt;

  if (x400_25_alive) {
    x574_boneTracking.SetTargetPosition(x62c_targetPos + zeus::CVector3f{0.f, 0.f, 10.f});
    x574_boneTracking.Update(dt);
  }

  if (x63c_attackTime > 0.f)
    x63c_attackTime -= dt;

  ModelData()->AnimationData()->PreRender();

  if (x400_25_alive)
    x574_boneTracking.PreRender(mgr, *ModelData()->AnimationData(), x34_transform, ModelData()->GetScale(),
                                *x450_bodyController);

  x5ec_collisionManager->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);

  CPlasmaProjectile* curProjectile = static_cast<CPlasmaProjectile*>(mgr.ObjectById(x676_curProjectile));
  if (curProjectile && curProjectile->GetActive()) {
    x628_ += dt;
    zeus::CTransform xf = GetLctrTransform(x5dc_damageLocator);

    if (x400_25_alive) {
      zeus::CQuaternion clampedQuat = zeus::CQuaternion::clampedRotateTo(
          (x610_lookPos + (zeus::min(x628_ / 1.5f, 1.f) * (x61c_ - x610_lookPos))) - xf.origin, xf.frontVector(),
          zeus::CRelAngle::FromDegrees(30.f));
      curProjectile->UpdateFX(clampedQuat.toTransform() * xf.getRotation(), dt, mgr);
    } else
      curProjectile->UpdateFX(xf, dt, mgr);
  }

  TCastToPtr<CCollisionActor> headAct = mgr.ObjectById(x600_headActor);
  TCastToPtr<CCollisionActor> pelvisAct = mgr.ObjectById(x602_pelvisActor);

  if (headAct && pelvisAct) {
    CHealthInfo* pelvisHealth = pelvisAct->HealthInfo(mgr);
    CHealthInfo* headHealth = headAct->HealthInfo(mgr);
    if (headHealth->GetHP() < pelvisHealth->GetHP()) {
      *HealthInfo(mgr) = *headHealth;
      *pelvisAct->HealthInfo(mgr) = *headHealth;
    } else {
      *HealthInfo(mgr) = *pelvisHealth;
      *headAct->HealthInfo(mgr) = *pelvisHealth;
    }
  }

  if (HealthInfo(mgr)->GetHP() <= 0.f && x400_25_alive) {
    if (curProjectile)
      curProjectile->ResetBeam(mgr, true);

    x450_bodyController->SetPlaybackRate(1.f);
    SetTransform(x644_);
    StopRumble(mgr);
    Death(mgr, GetTransform().frontVector(), EScriptObjectState::DeathRattle);
  }
}

void CNewIntroBoss::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType event, float dt) {
  if (event == EUserEventType::DamageOn) {
    x5dc_damageLocator = node.GetLocatorName();
    zeus::CTransform xf = GetLctrTransform(x5dc_damageLocator);
    zeus::CVector3f playerPos = PlayerPos(mgr);
    x604_predictedPlayerPos = x610_lookPos = x62c_targetPos = playerPos;
    x628_ = 0.f;
    if (GetLocoForHealth(mgr) == pas::ELocomotionType::Combat)
      x676_curProjectile = x5d8_stage3Projectile;
    else if (GetLocoForHealth(mgr) == pas::ELocomotionType::Lurk)
      x676_curProjectile = x5d6_stage2Projectile;
    else
      x676_curProjectile = x5d4_stage1Projectile;

    if (CPlasmaProjectile* projectile = static_cast<CPlasmaProjectile*>(mgr.ObjectById(x676_curProjectile))) {
      if (!projectile->GetActive()) {
        projectile->Fire(zeus::lookAt(xf.origin, x610_lookPos), mgr, false);

        if (x674_rumbleVoice == -1)
          x674_rumbleVoice =
              mgr.GetRumbleManager().Rumble(mgr, ERumbleFxId::IntroBossProjectile, 1.f, ERumblePriority::Two);
      }
    }

  } else if (event == EUserEventType::DamageOff) {
    if (CPlasmaProjectile* projectile = static_cast<CPlasmaProjectile*>(mgr.ObjectById(x676_curProjectile)))
      projectile->ResetBeam(mgr, false);

    StopRumble(mgr);
    x63c_attackTime = GetNextAttackTime(mgr);
    SendScriptMsgs(EScriptObjectState::Attack, mgr, EScriptObjectMessage::None);
  } else
    CActor::DoUserAnimEvent(mgr, node, event, dt);
}

void CNewIntroBoss::AddToRenderer(const zeus::CFrustum&, const CStateManager& mgr) const { EnsureRendered(mgr); }

float CNewIntroBoss::GetNextAttackTime(CStateManager& mgr) const {
  float attackTime = 2.f * mgr.GetActiveRandom()->Float() + 6.f;
  float hp = GetHealthInfo(mgr)->GetHP();

  if (hp > .66 * x640_initialHp)
    return attackTime;
  else if (hp > .33 * x640_initialHp)
    return attackTime - (0.4125f * attackTime);
  return attackTime - (0.825f * attackTime);
}

zeus::CVector3f CNewIntroBoss::PlayerPos(const CStateManager& mgr) const {
  CRayCastResult result = CGameCollision::RayStaticIntersection(
      mgr, mgr.GetPlayer().GetTranslation() + zeus::CVector3f{0.f, 0.f, mgr.GetPlayer().GetEyeHeight() * 0.5f},
      zeus::CVector3f::skDown, 30.f, CMaterialFilter::MakeInclude({EMaterialTypes::Solid}));
  if (result.IsValid())
    return result.GetPoint() + zeus::CVector3f{0.f, 0.f, (mgr.GetPlayer().GetEyeHeight() * 0.5f) + 0.2f};

  return mgr.GetPlayer().GetTranslation() + zeus::CVector3f{0.f, 0.f, (mgr.GetPlayer().GetEyeHeight() * 0.5f) + 0.2f};
}

bool CNewIntroBoss::ShouldTurn(CStateManager& mgr, float dt) {
  zeus::CVector3f playerVel = 1.f * mgr.GetPlayer().GetVelocity();
  zeus::CVector3f playerPos = PlayerPos(mgr);
  x604_predictedPlayerPos = playerPos + playerVel;

  zeus::CVector2f diffPos = (x604_predictedPlayerPos - GetTranslation()).toVec2f();

  return zeus::CVector2f::getAngleDiff(GetTransform().frontVector().toVec2f(), diffPos) > zeus::degToRad(1.f);
}

bool CNewIntroBoss::ShouldAttack(CStateManager& mgr, float dt) {
  if (x63c_attackTime > 0.f)
    return false;

  if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Turn)
    return false;

  return !ShouldTurn(mgr, dt);
}

bool CNewIntroBoss::AIStage(CStateManager& mgr, float) { return x568_locomotion != GetLocoForHealth(mgr); }

bool CNewIntroBoss::AnimOver(urde::CStateManager&, float) { return x56c_ == 3; }

void CNewIntroBoss::Generate(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x56c_ = 0;
    x568_locomotion = GetLocoForHealth(mgr);
    SendScriptMsgs(EScriptObjectState::Entered, mgr, EScriptObjectMessage::None);
  } else if (msg == EStateMsg::Update) {
    if (x56c_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Generate) {
        x56c_ = 2;
        return;
      }

      x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(GetGenerateForHealth(mgr)));
    } else if (x56c_ == 2) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Generate) {
        x56c_ = 3;
        SendScriptMsgs(EScriptObjectState::Exited, mgr, EScriptObjectMessage::None);
      }
    }
  }
}

pas::EGenerateType CNewIntroBoss::GetGenerateForHealth(const CStateManager& mgr) const {
  return GetHealthInfo(mgr)->GetHP() > 0.33f * x640_initialHp ? pas::EGenerateType::Three : pas::EGenerateType::Four;
}

bool CNewIntroBoss::InAttackPosition(CStateManager& mgr, float dt) {
  return x330_stateMachineState.GetTime() > 0.25f && x678_ &&
         x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Turn &&
         !ShouldTurn(mgr, dt);
}

void CNewIntroBoss::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate)
    x56c_ = 0;
  else if (msg == EStateMsg::Update) {
    if (x56c_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::ProjectileAttack)
        x56c_ = 2;
      else {
        x450_bodyController->GetCommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::ESeverity::One, mgr.GetPlayer().GetTranslation(), false));
      }
    } else if (x56c_ == 2) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::ProjectileAttack) {
        x56c_ = 3;
        x638_ = 0.f;
      }

      if (const CPlasmaProjectile* proj =
              static_cast<const CPlasmaProjectile*>(mgr.GetObjectById(x676_curProjectile))) {
        if (!proj->GetActive())
          x62c_targetPos = mgr.GetPlayer().GetTranslation();
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    if (GetLocoForHealth(mgr) == pas::ELocomotionType::Lurk || GetLocoForHealth(mgr) == pas::ELocomotionType::Combat)
      x678_ ^= 1;
    else
      x678_ = false;
  }
}

void CNewIntroBoss::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate || msg == EStateMsg::Update) {
    x450_bodyController->SetLocomotionType(x568_locomotion);
    if (x638_ > 0.2f)
      x62c_targetPos = PlayerPos(mgr);
    else
      x62c_targetPos = x610_lookPos + ((x638_ / 0.2f) * (PlayerPos(mgr) - x610_lookPos));

    if (ShouldTurn(mgr, 0.f)) {
      x450_bodyController->GetCommandMgr().DeliverCmd(
          CBCLocomotionCmd({}, (x604_predictedPlayerPos - GetTranslation()).normalized(), 1.f));
    }
  }
}

} // namespace urde::MP1
