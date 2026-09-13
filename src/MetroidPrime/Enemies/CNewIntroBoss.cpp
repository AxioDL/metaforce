#include "MetroidPrime/Enemies/CNewIntroBoss.hpp"

#include "Kyoto/Animation/CPOINode.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Weapons/CBeamProjectile.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"

#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector2f.hpp"

#include "Collision/CRayCastResult.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CCollisionActor.hpp"

struct SOBBJointInfo {
  const char* from;
  const char* to;
  float bX;
  float bY;
  float bZ;
};

const SOBBJointInfo skOOBJoints[] = {
    {"Pelvis", "Spine_3", 4.f, 1.f, 4.f},
    {"Spine_3", "Tail_1", 2.f, 1.f, 2.f},
    {"Tail_1", "Tail_2", 1.f, 1.f, 1.f},
    {"Tail_2", "Tail_3", 1.f, 1.f, 1.f},
    {"Tail_3", "Tail_4", 1.f, 1.f, 1.f},
    {"R_shoulder_front", "R_elbow_front", .5f, .5f, .5f},
    {"R_elbow_front", "R_wrist_front", .5f, .5f, .5f},
    {"L_shoulder_front", "L_elbow_front", .5f, .5f, .5f},
    {"L_elbow_front", "L_wrist_front", .5f, .5f, .5f},
    {"R_shoulder_back", "R_elbow_back", .5f, .5f, .5f},
    {"R_elbow_back", "R_wrist_back", .5f, .5f, .5f},
    {"L_shoulder_back", "L_elbow_back", .5f, .5f, .5f},
    {"L_elbow_back", "L_wrist_back", .5f, .5f, .5f},
};

const SSphereJointInfo skSphereJoints[] = {
    {"Head_1", 1.5f},
    {"Tail_5", 1.5f},
};

pas::ELocomotionType CNewIntroBoss::GetLocoForHealth(const CStateManager& mgr) const {
  const CHealthInfo* hInfo = GetHealthInfo(mgr);

  if (hInfo->GetHP() > (.66f * x640_initialHp)) {
    return pas::kLT_Relaxed;
  }

  if (hInfo->GetHP() > (.33f * x640_initialHp)) {
    return pas::kLT_Lurk;
  }

  return pas::kLT_Combat;
}

pas::EGenerateType CNewIntroBoss::GetGenerateForHealth(const CStateManager& mgr) const {
  return GetHealthInfo(mgr)->GetHP() > 0.33f * x640_initialHp ? pas::kGType_Three
                                                              : pas::kGType_Four;
}

float CNewIntroBoss::GetNextAttackTime(CStateManager& mgr) const {
  float attackTime = 2.f * mgr.Random()->Float() + 6.f;
  const CHealthInfo* hInfo = GetHealthInfo(mgr);

  if (hInfo->GetHP() > .66f * x640_initialHp) {
    return attackTime;
  }
  if (hInfo->GetHP() > .33f * x640_initialHp) {
    return attackTime - (0.4125f * attackTime);
  }

  return attackTime - (0.825f * attackTime);
}

CNewIntroBoss::CNewIntroBoss(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const CTransform4f& xf, const CModelData& mData,
                             const CPatternedInfo& pInfo, const CActorParameters& actParms,
                             float minTurnAngle, CAssetId projectile, const CDamageInfo& dInfo,
                             CAssetId beamContactFxId, CAssetId beamPulseFxId,
                             CAssetId beamTextureId, CAssetId beamGlowTextureId)
: CPatterned(kC_NewIntroBoss, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Flyer, kCT_One,
             kBT_Restricted, actParms, kCS_Medium)
, x568_locomotion(pas::kLT_Relaxed)
, x56c_stateProg(0)
, x570_minTurnAngle(minTurnAngle)
, x574_boneTracking(*GetAnimationData(), rstl::string_l("Head_1"), CMath::Deg2Rad(80.f),
                    CMath::Deg2Rad(180.f), kBTF_None)
, x5ac_projectileInfo(projectile, dInfo)
, x5d4_stage1Projectile(kInvalidUniqueId)
, x5d6_stage2Projectile(kInvalidUniqueId)
, x5d8_stage3Projectile(kInvalidUniqueId)
, x5f0_beamContactFxId(beamContactFxId)
, x5f4_beamPulseFxId(beamPulseFxId)
, x5f8_beamTextureId(beamTextureId)
, x5fc_beamGlowTextureId(beamGlowTextureId)
, x600_headActor(kInvalidUniqueId)
, x602_pelvisActor(kInvalidUniqueId)
, x604_predictedPlayerPos(CVector3f::Zero())
, x610_lookPos(CVector3f::Zero())
, x61c_startPlayerPos(CVector3f::Zero())
, x628_firingTime(0.f)
, x62c_targetPos(CVector3f::Zero())
, x638_(0.2f)
, x63c_attackTime(8.f)
, x640_initialHp(0.f)
, x644_initialXf(xf)
, x674_rumbleVoice(-1)
, x676_curProjectile(kInvalidUniqueId)
, x678_(false) {
  x5ac_projectileInfo.Token().Lock();
  x574_boneTracking.SetActive(true);
}

CProjectileInfo* CNewIntroBoss::ProjectileInfo() { return &x5ac_projectileInfo; }

CVector3f CNewIntroBoss::PlayerPos(const CStateManager& mgr) const {
  float z0 = mgr.GetPlayer()->GetEyeHeight() / 2.f;
  CVector3f pos = mgr.GetPlayer()->GetTranslation() + CVector3f(0.f, 0.f, z0);
  CRayCastResult result = CGameCollision::RayStaticIntersection(
      mgr, pos, CVector3f::Down(), 30.f, CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid)));
  if (result.IsInvalid()) {
    float z = mgr.GetPlayer()->GetEyeHeight() / 2.f + 0.2f;
    return mgr.GetPlayer()->GetTranslation() + CVector3f(0.f, 0.f, z);
  } else {
    float z = mgr.GetPlayer()->GetEyeHeight() / 2.f + 0.2f;
    return result.GetPoint() + CVector3f(0.f, 0.f, z);
  }
}

void CNewIntroBoss::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                    EUserEventType event, float dt) {
  bool handled = false;
  switch (event) {
  case kUE_DamageOn: {
    x5dc_damageLocator = node.GetLocatorName();
    CTransform4f xf = GetLctrTransform(x5dc_damageLocator);
    CVector3f playerPos = PlayerPos(mgr);
    x62c_targetPos = x610_lookPos = x604_predictedPlayerPos = playerPos;
    x61c_startPlayerPos = playerPos;
    x628_firingTime = 0.f;
    if (GetLocoForHealth(mgr) == pas::kLT_Combat) {
      x676_curProjectile = x5d8_stage3Projectile;
    } else if (GetLocoForHealth(mgr) == pas::kLT_Lurk) {
      x676_curProjectile = x5d6_stage2Projectile;
    } else {
      x676_curProjectile = x5d4_stage1Projectile;
    }

    CTransform4f projXf = CTransform4f::LookAt(xf.GetTranslation(), x610_lookPos);
    if (CBeamProjectile* projectile =
            static_cast< CBeamProjectile* >(mgr.ObjectById(x676_curProjectile))) {
      if (!projectile->GetActive()) {
        projectile->Fire(projXf, mgr, false);

        if (x674_rumbleVoice == -1)
          x674_rumbleVoice =
              mgr.GetRumbleManager()->Rumble(mgr, kRFX_IntroBossProjectile, 1.f, kRP_Two);
      }
    }
    handled = true;
    break;
  }
  case kUE_DamageOff: {
    if (CBeamProjectile* projectile =
            static_cast< CBeamProjectile* >(mgr.ObjectById(x676_curProjectile))) {
      projectile->ResetBeam(mgr, false);
    }

    StopRumble(mgr);
    x63c_attackTime = GetNextAttackTime(mgr);
    SendScriptMsgs(kSS_Attack, mgr, kSM_None);
    handled = true;
    break;
  }
  case kUE_Projectile:
  default:
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, event, dt);
  }
}

rstl::optional_object< CAABox > CNewIntroBoss::GetTouchBounds() const {
  return rstl::optional_object_null();
}

void CNewIntroBoss::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  if (x638_ < 0.2f) {
    x638_ += dt;
  }
  if (IsAlive()) {
    x574_boneTracking.SetTargetPosition(x62c_targetPos + CVector3f(0.f, 0.f, 10.f));
    x574_boneTracking.Update(dt);
  }
  if (x63c_attackTime > 0.f) {
    x63c_attackTime -= dt;
  }

  AnimationData()->PreRender();
  if (IsAlive()) {
    x574_boneTracking.PreRender(mgr, *ModelData()->AnimationData(), GetTransform(),
                                ModelData()->ScaleCopy(), *BodyCtrl());
  }
  x5ec_collisionManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);

  CBeamProjectile* projectile = static_cast< CBeamProjectile* >(mgr.ObjectById(x676_curProjectile));
  if (projectile && projectile->GetActive()) {
    x628_firingTime += dt;
    const CTransform4f xf = GetLctrTransform(x5dc_damageLocator);
    if (IsAlive()) {
      const float& weight = rstl::min_val(1.f, x628_firingTime / 1.5f);
      const CVector3f target =
          x610_lookPos + weight * (x61c_startPlayerPos - x610_lookPos) - xf.GetTranslation();
      const CQuaternion rotation = CQuaternion::ClampedRotateTo(CUnitVector3f(xf.GetForward()),
                                                                CUnitVector3f(CVector3f(target)),
                                                                CRelAngle::FromDegrees(30.f));
      CTransform4f beamXf = rotation.BuildTransform4f() * xf.GetRotation();
      beamXf.SetTranslation(xf.GetTranslation());
      projectile->UpdateFx(beamXf, dt, mgr);
    } else {
      projectile->UpdateFx(xf, dt, mgr);
    }
  }

  CCollisionActor* head = TCastToPtr< CCollisionActor >(mgr.ObjectById(x600_headActor));
  CCollisionActor* pelvis = TCastToPtr< CCollisionActor >(mgr.ObjectById(x602_pelvisActor));
  if (head && pelvis) {
    if (head->HealthInfo(mgr)->GetHP() < pelvis->HealthInfo(mgr)->GetHP()) {
      *HealthInfo(mgr) = *head->HealthInfo(mgr);
      *pelvis->HealthInfo(mgr) = *head->HealthInfo(mgr);
    } else if (pelvis) {
      *HealthInfo(mgr) = *pelvis->HealthInfo(mgr);
      *head->HealthInfo(mgr) = *pelvis->HealthInfo(mgr);
    }
  }

  if (HealthInfo(mgr)->GetHP() <= 0.f && IsAlive()) {
    if (projectile) {
      projectile->ResetBeam(mgr, true);
    }
    BodyCtrl()->SetPlaybackRate(1.f);
    SetTransform(x644_initialXf);
    StopRumble(mgr);
    Death(mgr, GetTransform().GetForward(), kSS_DeathRattle);
  }
}

bool CNewIntroBoss::ShouldTurn(CStateManager& mgr, float arg) {
  const CVector3f velocity = 1.f * mgr.GetPlayer()->GetVelocityWR();
  const CVector3f playerPos = PlayerPos(mgr);
  x604_predictedPlayerPos = playerPos + velocity;
  const CVector2f delta = (x604_predictedPlayerPos - GetTranslation()).DropZ();
  const CVector2f forward = GetTransform().GetForward().DropZ();
  const float angle = CVector2f::GetAngleDiff(forward, delta);
  return angle > CRelAngle::FromDegrees(x570_minTurnAngle).AsRadians();
}

void CNewIntroBoss::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
  case kStateMsg_Update:
    BodyCtrl()->SetLocomotionType(x568_locomotion);
    if (x638_ > 0.2f) {
      x62c_targetPos = PlayerPos(mgr);
    } else {
      const float weight = x638_ / 0.2f;
      x62c_targetPos = x610_lookPos + weight * (PlayerPos(mgr) - x610_lookPos);
    }
    if (ShouldTurn(mgr, 0.f)) {
      x56c_stateProg = 0;
      const CVector3f delta = x604_predictedPlayerPos - GetTranslation();
      BodyCtrl()->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(CVector3f::Zero(), delta.AsNormalized(), 1.f));
    }
    break;
  }
}

void CNewIntroBoss::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x56c_stateProg = 0;
    break;
  case kStateMsg_Update:
    switch (x56c_stateProg) {
    case 0:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_ProjectileAttack) {
        x56c_stateProg = 2;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::kS_One, mgr.GetPlayer()->GetTranslation(), false));
      }
      break;
    case 2:
      if (BodyCtrl()->GetCurrentStateId() != pas::kAS_ProjectileAttack) {
        x56c_stateProg = 3;
        x638_ = 0.f;
      }
      if (const CBeamProjectile* projectile =
              static_cast< const CBeamProjectile* >(mgr.GetObjectById(x676_curProjectile))) {
        if (!projectile->GetActive()) {
          x62c_targetPos = mgr.GetPlayer()->GetTranslation();
        }
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    if (GetLocoForHealth(mgr) == pas::kLT_Lurk || GetLocoForHealth(mgr) == pas::kLT_Combat) {
      x678_ = !x678_;
    } else {
      x678_ = false;
    }
    break;
  }
}

bool CNewIntroBoss::ShouldAttack(CStateManager& mgr, float arg) {
  if (x63c_attackTime <= 0.f && BodyCtrl()->GetCurrentStateId() != pas::kAS_Turn &&
      !ShouldTurn(mgr, 0.f)) {
    return true;
  }
  return false;
}

bool CNewIntroBoss::InAttackPosition(CStateManager& mgr, float arg) {
  if (GetStateMachineTime() > 0.25f && x678_ && BodyCtrl()->GetCurrentStateId() != pas::kAS_Turn &&
      !ShouldTurn(mgr, 0.f)) {
    return true;
  }
  return false;
}

bool CNewIntroBoss::AnimOver(CStateManager&, float) { return x56c_stateProg == 3; }

bool CNewIntroBoss::AIStage(CStateManager& mgr, float) {
  return x568_locomotion != GetLocoForHealth(mgr);
}

void CNewIntroBoss::Generate(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x56c_stateProg = 0;
    x568_locomotion = GetLocoForHealth(mgr);
    SendScriptMsgs(kSS_Entered, mgr, kSM_None);
    break;
  case kStateMsg_Update:
    switch (x56c_stateProg) {
    case 0:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_Generate) {
        x56c_stateProg = 2;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCGenerateCmd(GetGenerateForHealth(mgr), -1));
      }
      break;
    case 2:
      if (BodyCtrl()->GetCurrentStateId() != pas::kAS_Generate) {
        x56c_stateProg = 3;
        SendScriptMsgs(kSS_Exited, mgr, kSM_None);
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  default:
    break;
  }
}

void CNewIntroBoss::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered: {
    RemoveMaterial(kMT_Solid, mgr);
    RemoveMaterial(kMT_Target, mgr);
    RemoveMaterial(kMT_Orbit, mgr);
    BodyCtrl()->Activate(mgr);

    if (x5d4_stage1Projectile == kInvalidUniqueId) {
      const CBeamInfo stage1Info(3, x5f0_beamContactFxId, x5f4_beamPulseFxId, x5f8_beamTextureId,
                                 x5fc_beamGlowTextureId, 50, 1.f, 1.f, 1.5f, 20.f, 1.f, 4.f, 8.f,
                                 CColor::Yellow(),
                                 CColor(28.f / 255.f, 147.f / 255.f, 39.f / 255.f, 1.f), 150.f);
      const CBeamInfo stage2Info(3, x5f0_beamContactFxId, x5f4_beamPulseFxId, x5f8_beamTextureId,
                                 x5fc_beamGlowTextureId, 50, 1.f, 1.f, 2.f, 20.f, 1.f, 4.f, 8.f,
                                 CColor::Yellow(),
                                 CColor(28.f / 255.f, 147.f / 255.f, 39.f / 255.f, 1.f), 150.f);
      const CBeamInfo stage3Info(3, x5f0_beamContactFxId, x5f4_beamPulseFxId, x5f8_beamTextureId,
                                 x5fc_beamGlowTextureId, 50, 1.f, 1.f, 2.f, 20.f, 1.f, 4.f, 8.f,
                                 CColor::Yellow(),
                                 CColor(28.f / 255.f, 147.f / 255.f, 39.f / 255.f, 1.f), 150.f);
      x5d4_stage1Projectile = mgr.AllocateUniqueId();
      x5d6_stage2Projectile = mgr.AllocateUniqueId();
      x5d8_stage3Projectile = mgr.AllocateUniqueId();
      CPlasmaProjectile* const stage1 = rs_new CPlasmaProjectile(
          x5ac_projectileInfo.Token(), rstl::string_l("IntroBoss_Beam"), kWT_AI, stage1Info,
          CTransform4f::Identity(), kMT_Character, x5ac_projectileInfo.GetDamage(),
          x5d4_stage1Projectile, GetCurrentAreaId(), GetUniqueId(), CWeaponAssetInfo(), true,
          CWeapon::kPA_KeepInCinematic);
      CPlasmaProjectile* const stage2 = rs_new CPlasmaProjectile(
          x5ac_projectileInfo.Token(), rstl::string_l("IntroBoss_Beam_Stage2"), kWT_AI, stage2Info,
          CTransform4f::Identity(), kMT_Character, x5ac_projectileInfo.GetDamage(),
          x5d6_stage2Projectile, GetCurrentAreaId(), GetUniqueId(), CWeaponAssetInfo(), true,
          CWeapon::kPA_KeepInCinematic);
      CDamageInfo stage3Damage = x5ac_projectileInfo.GetDamage();
      stage3Damage.SetDamage(1.25f * stage3Damage.GetDamage());
      CPlasmaProjectile* const stage3 = rs_new CPlasmaProjectile(
          x5ac_projectileInfo.Token(), rstl::string_l("IntroBoss_Beam_Stage2"), kWT_AI, stage3Info,
          CTransform4f::Identity(), kMT_Character, stage3Damage, x5d8_stage3Projectile,
          GetCurrentAreaId(), GetUniqueId(), CWeaponAssetInfo(), true,
          CWeapon::kPA_KeepInCinematic);
      mgr.AddObject(*stage1);
      mgr.AddObject(*stage2);
      mgr.AddObject(*stage3);
      x676_curProjectile = x5d4_stage1Projectile;
    }

    {
      rstl::vector< CJointCollisionDescription > joints;
      joints.reserve(15);
      const CAnimData* animData = GetAnimationData();
      for (uint i = 0; i < 2; ++i) {
        const CJointCollisionDescription joint = CJointCollisionDescription::SphereCollision(
            animData->GetLocatorSegId(rstl::string_l(skSphereJoints[i].name)),
            skSphereJoints[i].radius, rstl::string_l(skSphereJoints[i].name), 0.001f);
        joints.push_back(joint);
      }
      for (uint i = 0; i < 13; ++i) {
        const CJointCollisionDescription joint = CJointCollisionDescription::OBBAutoSizeCollision(
            animData->GetLocatorSegId(rstl::string_l(skOOBJoints[i].from)),
            animData->GetLocatorSegId(rstl::string_l(skOOBJoints[i].to)),
            CVector3f(CVector3f(skOOBJoints[i].bX, skOOBJoints[i].bY, skOOBJoints[i].bZ)),
            CJointCollisionDescription::kOT_One, rstl::string_l(skOOBJoints[i].from), 0.001f);
        joints.push_back(joint);
      }
      x5ec_collisionManager = rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(),
                                                            joints, GetActive());
    }

    x640_initialHp = HealthInfo(mgr)->GetHP();
    for (uint i = 0; i < x5ec_collisionManager->GetNumCollisionActors(); ++i) {
      const CJointCollisionDescription& desc = x5ec_collisionManager->GetCollisionDescFromIndex(i);
      CCollisionActor* actor =
          TCastToPtr< CCollisionActor >(mgr.ObjectById(desc.GetCollisionActorId()));
      if (desc.GetName() == rstl::string_l(skSphereJoints[0].name)) {
        x600_headActor = desc.GetCollisionActorId();
        if (actor) {
          *actor->HealthInfo(mgr) = *HealthInfo(mgr);
          actor->SetDamageVulnerability(
              *static_cast< const CNewIntroBoss* >(this)->GetDamageVulnerability());
          actor->RemoveMaterial(kMT_Orbit, mgr);
        }
      } else if (desc.GetName() == rstl::string_l(skOOBJoints[0].from)) {
        x602_pelvisActor = desc.GetCollisionActorId();
        if (actor) {
          *actor->HealthInfo(mgr) = *HealthInfo(mgr);
          actor->SetDamageVulnerability(CDamageVulnerability::NormalVulnerability());
          actor->AddMaterial(kMT_Orbit, mgr);
          MoveScannableObjectInfoToActor(actor, mgr);
        }
      } else if (actor) {
        actor->RemoveMaterial(kMT_Orbit, mgr);
      }
    }
  } break;

  case kSM_Deleted: {
    DeleteBeam(mgr);
    x5ec_collisionManager->Destroy(mgr);
    break;
  }
  case kSM_Damage:
    if (uid == x600_headActor || uid == x602_pelvisActor) {
      TakeDamage(CVector3f::Zero(), 0.f);
    }
    break;
  default:
    break;
  }
  const bool active = GetActive();
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  if (active == GetActive()) {
    return;
  }

  if (!x5ec_collisionManager.null()) {
    x5ec_collisionManager->SetActive(mgr, GetActive());
  }
  x63c_attackTime = 8.f;
}

void CNewIntroBoss::OnScanStateChange(const EScanState state, CStateManager& mgr) {
  CActor::OnScanStateChange(state, mgr);

  if (state != kSS_Done) {
    return;
  }

  CCollisionActor* headActor = TCastToPtr< CCollisionActor >(mgr.ObjectById(x600_headActor));
  CCollisionActor* pelvisActor = TCastToPtr< CCollisionActor >(mgr.ObjectById(x602_pelvisActor));

  if (headActor) {
    headActor->AddMaterial(kMT_Orbit, mgr);
  }
  if (pelvisActor) {
    pelvisActor->RemoveMaterial(kMT_Orbit, mgr);
  }
}

void CNewIntroBoss::AddToRenderer(const CFrustumPlanes&, const CStateManager& mgr) const {
  EnsureRendered(mgr);
}

CAABox CNewIntroBoss::GetSortingBounds(const CStateManager&) const {
  const CAABox bounds = GetModelData()->GetBounds();
  const float minZ = bounds.GetMinPoint().GetZ();
  const float maxZ = bounds.GetMaxPoint().GetZ();
  return CAABox(-0.5f, -0.5f, minZ, 0.5f, 0.5f, maxZ).GetTransformedAABox(GetTransform());
}

ENTITY_ACCEPT_IMPL(CNewIntroBoss)

void CNewIntroBoss::DeleteBeam(CStateManager& mgr) {
  if (x5d4_stage1Projectile != kInvalidUniqueId) {
    mgr.DeleteObjectRequest(x5d4_stage1Projectile);
    x5d4_stage1Projectile = kInvalidUniqueId;
  }
  if (x5d6_stage2Projectile != kInvalidUniqueId) {
    mgr.DeleteObjectRequest(x5d6_stage2Projectile);
    x5d6_stage2Projectile = kInvalidUniqueId;
  }
  if (x5d8_stage3Projectile != kInvalidUniqueId) {
    mgr.DeleteObjectRequest(x5d8_stage3Projectile);
    x5d8_stage3Projectile = kInvalidUniqueId;
  }
  StopRumble(mgr);
}

void CNewIntroBoss::StopRumble(CStateManager& mgr) {
  if (x674_rumbleVoice != -1) {
    mgr.GetRumbleManager()->StopRumble(x674_rumbleVoice);
    x674_rumbleVoice = -1;
  }
}
