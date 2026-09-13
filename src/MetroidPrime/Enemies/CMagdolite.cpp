#include "MetroidPrime/Enemies/CMagdolite.hpp"

#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CFluidPlaneCPU.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CFlameThrower.hpp"

#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

#include "float.h"

static const char* skMouthLocator = "LCTR_MAGMOUTH";
static const char* skHeadLocator = "head";
static const char* skHeadBone = "head";
static const char* skTopLocator = "Top_LCTR";
static const char* skBottomLocator = "Bottom_LCTR";

static const SSphereJointInfo skSpineJoints[] = {
    {"spine1", 0.75f}, {"spine3", 0.75f}, {"spine5", 0.75f}, {"spine7", 0.75f}, {"spine9", 0.75f},
};

struct SOBBJointInfo {
  const char* from;
  const char* to;
  float bounds[3];
};

static SOBBJointInfo skHeadJoints[] = {
    {"head", skTopLocator, {1.f, 0.15f, 0.5f}},
    {"head", skBottomLocator, {0.75f, 0.15f, 0.25f}},
};

CMagdolite::CMagdolite(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
                       const CActorParameters& actParms, float headTrackAngle, float losAngle,
                       const CDamageInfo& flameThrowerDamage, const CDamageInfo& headContactDamage,
                       const CDamageVulnerability& headVuln, const CDamageVulnerability& bodyVuln,
                       CAssetId headlessModel, CAssetId headlessSkin, float minHp,
                       float initialDelay, float minDelay, float maxDelay,
                       const CFlameInfo& flameInfo, float f7, float f8, float f9)
: CPatterned(kC_Magdolite, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Flyer, kCT_One,
             kBT_BiPedal, actParms, kCS_Large)
, x568_initialDelay(initialDelay)
, x56c_minDelay(minDelay)
, x570_maxDelay(maxDelay)
, x574_minHp(minHp)
, x578_losMaxDistance(cosf(CRelAngle::FromDegrees(losAngle).AsRadians()))
, x57c_headTrackAngle(headTrackAngle)
, x580_collisionManager(nullptr)
, x584_boneTracker(*GetAnimationData(), rstl::string_l(skHeadBone),
                   CRelAngle::FromDegrees(headTrackAngle).AsRadians(), M_PIF / 2.f, kBTF_ParentIk)
, x5bc_instaKillVulnerability(headVuln)
, x624_normalVulnerability(bodyVuln)
, x690_headlessModel(TLockedToken< CSkinnedModel >(rs_new CSkinnedModel(
      gpSimplePool->GetObj(SObjectTag('CMDL', headlessModel)),
      gpSimplePool->GetObj(SObjectTag('CSKR', headlessSkin)),
      GetAnimationData()->GetModelData()->GetLayoutInfo(), CSkinnedModel::kDO_Owned)))
, x6a8_flameInfo(flameInfo)
, x6c8_flameThrowerId(kInvalidUniqueId)
, x6cc_flameThrowerDesc(gpSimplePool->GetObj("FlameThrower"))
, x6d4_flameThrowerDamage(flameThrowerDamage)
, x6f0_headContactDamage(headContactDamage)
, x710_attackOffset(CVector3f::Zero())
, x71c_attackTarget(xf.GetTranslation())
, x728_cachedTarget(CVector3f::Zero())
, x734_(0.f)
, x738_(0.f)
, x73c_(0.f)
, x740_(0.f)
, x744_(f7)
, x748_(f8)
, x74c_(f9)
, x750_aiStage(0)
, x754_24_retreat(false)
, x754_25_up(false)
, x754_26_lostMyHead(false)
, x754_27_flameThrowerActive(false)
, x754_28_alert(false)
, x754_29_useDetectionRange(true)
, x754_30_inProjectileAttack(false)
, x758_(0.f) {
  x460_knockBackController.SetAutoResetImpulse(false);
  x460_knockBackController.SetEnableBurn(false);
  x690_headlessModel->SetLayoutInfo(GetAnimationData()->GetModelData()->GetLayoutInfo());
}

ENTITY_ACCEPT_IMPL(CMagdolite)

void CMagdolite::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  if (!GetActive()) {
    return;
  }
  x758_ += dt;
  if (GetBodyCtrl()->GetPercentageFrozen() > 0.f && x754_27_flameThrowerActive) {
    SetFlameThrowerActive(mgr, false);
  }
  if (!IsAlive()) {
    if (CFlameThrower* flame = static_cast< CFlameThrower* >(mgr.ObjectById(x6c8_flameThrowerId))) {
      const CTransform4f xf = GetLctrTransform(rstl::string_l(skMouthLocator));
      flame->SetTransform(xf, mgr, dt);
    }
  }

  const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
  const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
  const CTransform4f mouthXf = GetLctrTransform(rstl::string_l(skMouthLocator));
  const CVector3f aimDir = (aimPos - mouthXf.GetTranslation()).AsNormalized();
  const float maxAngle = CRelAngle::FromDegrees(x57c_headTrackAngle).AsRadians();
  float angle = CMath::Min(CVector3f::GetAngleDiff(mouthXf.GetForward(), aimDir), maxAngle);
  if (CVector3f::Dot(mouthXf.GetUp(), aimDir) < 0.f) {
    angle = -angle;
  }
  const float blend = (angle + maxAngle) / (2.f * maxAngle);
  x710_attackOffset = CVector3f::Lerp(playerPos, aimPos, blend);

  if (GetActive()) {
    if (IsAlive()) {
      AnimationData()->PreRender();
      x584_boneTracker.Update(dt);
      x584_boneTracker.PreRender(mgr, *AnimationData(), GetTransform(), GetModelScale(),
                                 *GetBodyCtrl());
      if (CFlameThrower* flame =
              static_cast< CFlameThrower* >(mgr.ObjectById(x6c8_flameThrowerId))) {
        const CTransform4f xf = GetLctrTransform(rstl::string_l(skMouthLocator));
        flame->SetTransform(xf, mgr, dt);
      }
    }
    x580_collisionManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
    const CVector3f scale = GetModelScale();
    const CVector3f headPos = GetLocatorTransform(rstl::string_l(skHeadBone)).GetTranslation();
    CVector3f offset(scale.GetX() * headPos.GetX(), scale.GetY() * headPos.GetY(),
                     scale.GetZ() * headPos.GetZ());
    offset = GetTransform().Rotate(offset);
    MoveCollisionPrimitive(offset);
    SetTransformDirty(true);
  }

  switch (x750_aiStage) {
  case 1:
    if (x734_ < x738_ / 2.f) {
      x740_ += x73c_ * dt;
    } else {
      x740_ -= x73c_ * dt;
    }
    x734_ += x740_ * dt;
    {
      const CVector3f pos = GetTranslation();
      SetTranslation(pos + CVector3f(0.f, 0.f, x740_ * dt));
    }
    if (GetTranslation().GetZ() > x728_cachedTarget.GetZ()) {
      SetTranslation(x728_cachedTarget);
      x750_aiStage = 0;
    }
    break;
  case 2:
    if (x734_ < x738_ / 2.f) {
      x740_ += x73c_ * dt;
    } else {
      x740_ -= x73c_ * dt;
    }
    x734_ += x740_ * dt;
    {
      const CVector3f pos = GetTranslation();
      SetTranslation(pos - CVector3f(0.f, 0.f, x740_ * dt));
    }
    if (GetTranslation().GetZ() < x728_cachedTarget.GetZ()) {
      SetTranslation(x728_cachedTarget);
      x750_aiStage = 0;
    }
    break;
  }
}

void CMagdolite::KnockBack(const CVector3f& direction, CStateManager& mgr, const CDamageInfo& info,
                           float magnitude, bool direct, const bool inDeferred) {
  CPatterned::KnockBack(direction, mgr, info, magnitude, direct, inDeferred);
}

void CMagdolite::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }

void CMagdolite::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  bool handled = false;
  switch (msg) {
  case kSM_Damage:
  case kSM_InvulnDamage:
    if (const CGameProjectile* proj = TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(uid))) {
      if (proj->GetOwnerId() == mgr.GetPlayer()->GetUniqueId()) {
        if (GetBodyCtrl()->GetPercentageFrozen() > 0.f &&
            x5bc_instaKillVulnerability.GetVulnerability(
                proj->GetCurrentDamageInfo().GetWeaponMode(), CDamageVulnerability::kRD_No) !=
                kVN_Deflect) {
          if (IsAlive()) {
            x754_26_lostMyHead = true;
            x401_30_pendingDeath = true;
          }
        } else {
          if (x70c_curHealth - HealthInfo(mgr)->GetHP() > x574_minHp) {
            x70c_curHealth = HealthInfo(mgr)->GetHP();
            x754_24_retreat = true;
          } else if (x624_normalVulnerability.GetVulnerability(
                         proj->GetCurrentDamageInfo().GetWeaponMode(),
                         CDamageVulnerability::kRD_No) != kVN_Deflect) {
            x400_24_hitByPlayerProjectile = true;
          }
        }
      }
    }
    handled = true;
    break;
  case kSM_Registered: {
    BodyCtrl()->Activate(mgr);
    RemoveMaterial(kMT_Solid, mgr);
    AddMaterial(kMT_NonSolidDamageable, mgr);
    x584_boneTracker.SetActive(false);
    SetDrawShadow(false);
    const CAABox bounds = GetBoundingBox();
    const float x = bounds.GetWidth() * 0.5f;
    const float z = bounds.GetDepth() * 0.5f;
    const float y = bounds.GetHeight() * 0.5f;
    SetBoundingBox(CAABox(-x, -y, -z, x, y, z));
    CreateCollisionActors(mgr);
    x330_stateMachineState.SetDelay(0.f);
    CreateFlameThrower(mgr);
    x70c_curHealth = HealthInfo(mgr)->GetHP();
    break;
  }
  case kSM_Deleted:
    x580_collisionManager->Destroy(mgr);
    if (x6c8_flameThrowerId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(x6c8_flameThrowerId);
      x6c8_flameThrowerId = kInvalidUniqueId;
    }
    break;
  case kSM_Touched:
    DoContactDamage(uid, mgr);
    break;
  case kSM_SuspendedMove:
    if (x580_collisionManager.get()) {
      x580_collisionManager->SetMovable(mgr, false);
    }
    break;
  }
  if (!handled) {
    CPatterned::AcceptScriptMsg(msg, uid, mgr);
  }
}

void CMagdolite::CreateCollisionActors(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > joints;
  joints.reserve(8);
  const CAnimData* animData = GetAnimationData();
  for (uint i = 0; i < 5; ++i) {
    const SSphereJointInfo& info = skSpineJoints[i];
    CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
        animData->GetLocatorSegId(rstl::string_l(info.name)), info.radius,
        rstl::string_l(info.name), 200.f);
    joints.push_back(desc);
  }
  for (uint i = 0; i < 2; ++i) {
    const SOBBJointInfo& info = skHeadJoints[i];
    CJointCollisionDescription desc = CJointCollisionDescription::OBBAutoSizeCollision(
        animData->GetLocatorSegId(rstl::string_l(info.from)),
        animData->GetLocatorSegId(rstl::string_l(info.to)),
        CVector3f(info.bounds[0], info.bounds[1], info.bounds[2]),
        CJointCollisionDescription::kOT_One, rstl::string_l(info.to), 200.f);
    joints.push_back(desc);
  }
  x580_collisionManager =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints, GetActive());
  for (uint i = 0; i < x580_collisionManager->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = x580_collisionManager->GetCollisionDescFromIndex(i);
    const TUniqueId id = desc.GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      actor->AddMaterial(kMT_AIJoint, mgr);
    }
  }
  for (uint i = 0; i < x580_collisionManager->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = x580_collisionManager->GetCollisionDescFromIndex(i);
    if (desc.GetName() == rstl::string_l(skTopLocator) ||
        desc.GetName() == rstl::string_l(skBottomLocator)) {
      const TUniqueId id = desc.GetCollisionActorId();
      if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
        actor->AddMaterial(kMT_ProjectilePassthrough, mgr);
        x69c_headCollisionActors.push_back(actor->GetUniqueId());
      }
    }
  }
}

const CDamageVulnerability* CMagdolite::GetDamageVulnerability() const {
  if (IsAlive()) {
    return CAi::GetDamageVulnerability();
  }
  return &CDamageVulnerability::ImmuneVulnerability();
}

const CDamageVulnerability* CMagdolite::GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                               const CDamageInfo&) const {
  return GetDamageVulnerability();
}

void CMagdolite::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                                 float dt) {
  bool handled = false;
  switch (type) {
  case kUE_DamageOn:
    SetFlameThrowerActive(mgr, true);
    break;
  case kUE_DamageOff:
    SetFlameThrowerActive(mgr, false);
    break;
  case kUE_BreakLockOn:
    RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
    handled = true;
    mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CMagdolite::SnapToFacePlayer(CStateManager& mgr) {
  CVector3f direction = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  direction.SetZ(0.f);
  direction = direction.AsNormalized();
  CRelAngle angle(CVector3f::GetAngleDiff(GetTransform().GetForward(), direction));
  if (CVector3f::Dot(GetTransform().GetRight(), direction) > 0.f) {
    angle *= -1.f;
  }
  SetRotation(GetRotation() * CQuaternion::ZRotation(angle));
}

void CMagdolite::InActive(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    SetEnableRender(false);
    BodyCtrl()->SetLocomotionType(pas::kLT_Internal7);
    RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
    break;
  case kStateMsg_Deactivate:
    AddMaterial(kMT_Orbit, kMT_Target, mgr);
    x754_25_up = false;
    SnapToFacePlayer(mgr);
    SetEnableRender(true);
    break;
  }
}

void CMagdolite::Lurk(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    x584_boneTracker.SetActive(false);
    x330_stateMachineState.SetDelay(0.f);
    AddMaterial(kMT_Target, kMT_Orbit, mgr);
    break;
  case kStateMsg_Deactivate:
    x754_25_up = true;
    break;
  }
}

void CMagdolite::Active(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
    x584_boneTracker.SetActive(true);
    x584_boneTracker.SetTarget(mgr.GetPlayer()->GetUniqueId());
    x400_24_hitByPlayerProjectile = false;
    x754_29_useDetectionRange = false;
    x330_stateMachineState.SetDelay(x568_initialDelay);
    break;
  case kStateMsg_Update: {
    CVector3f direction = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    direction.SetZ(0.f);
    if (direction.CanBeNormalized()) {
      const CVector3f& normal = direction.AsNormalized();
      if (CVector3f::Dot(GetTransform().GetForward(), normal) < x578_losMaxDistance) {
        BodyCtrl()->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), direction.AsNormalized(), 1.f));
      }
    }
    break;
  }
  case kStateMsg_Deactivate:
    x330_stateMachineState.SetDelay((x570_maxDelay - x56c_minDelay) * mgr.Random()->Float() +
                                    x56c_minDelay);
    break;
  }
}

void CMagdolite::Taunt(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    x584_boneTracker.SetActive(true);
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Taunt, &CPatterned::TryTaunt, 1);
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    x584_boneTracker.SetActive(false);
    break;
  }
}

void CMagdolite::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    x32c_animState = kAS_Ready;
    x584_boneTracker.SetActive(false);
    const float zDiff = (GetLctrTransform(rstl::string_l(skHeadLocator)).GetTranslation() -
                         mgr.GetPlayer()->GetTranslation())
                            .GetZ();
    float height = zDiff - x74c_;
    if (height < 0.f) {
      height = 0.f;
    } else if (height > x748_) {
      height = x748_;
    }
    x728_cachedTarget = x71c_attackTarget - CVector3f(0.f, 0.f, height);
    x740_ = 0.f;
    x734_ = 0.f;
    x738_ = height;
    x73c_ = (2.f * x738_) / (x744_ * x744_);
    if (x728_cachedTarget.GetZ() < GetTranslation().GetZ()) {
      x750_aiStage = 2;
    } else {
      x750_aiStage = 1;
    }
    x754_30_inProjectileAttack = true;
    break;
  }
  case kStateMsg_Update: {
    TryCommand(mgr, pas::kAS_MeleeAttack, &CPatterned::TryMeleeAttack, 1);
    CVector3f direction = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    direction.SetZ(0.f);
    direction = direction.AsNormalized();
    CRelAngle angle(CVector3f::GetAngleDiff(GetTransform().GetForward(), direction));
    if (CVector3f::Dot(GetTransform().GetRight(), direction) > 0.f) {
      angle *= -1.f;
    }
    if (angle.AsDegrees() > x3b8_turnSpeed) {
      angle = CRelAngle::FromDegrees(x3b8_turnSpeed);
    } else if (angle.AsDegrees() < -x3b8_turnSpeed) {
      angle = CRelAngle::FromDegrees(-x3b8_turnSpeed);
    }
    angle *= arg;
    RotateInOneFrameOR(CQuaternion::ZRotation(angle), arg);
    break;
  }
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    SetFlameThrowerActive(mgr, false);
    x750_aiStage = 1;
    x728_cachedTarget = x71c_attackTarget;
    x740_ = 0.f;
    x734_ = 0.f;
    x738_ = x728_cachedTarget.GetZ() - GetTranslation().GetZ();
    x73c_ = (2.f * x738_) / (x744_ * x744_);
    x754_30_inProjectileAttack = false;
    break;
  }
}

void CMagdolite::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    x584_boneTracker.SetActive(true);
    x584_boneTracker.SetNoHorizontalAim(true);
    x584_boneTracker.SetTarget(kInvalidUniqueId);
    x754_30_inProjectileAttack = true;
    break;
  case kStateMsg_Update:
    if (TooClose(mgr, 0.f)) {
      TryCommand(mgr, pas::kAS_ProjectileAttack, &CPatterned::TryProjectileAttack, 0);
    } else {
      TryCommand(mgr, pas::kAS_ProjectileAttack, &CPatterned::TryProjectileAttack, 1);
    }
    x584_boneTracker.SetTargetPosition(x710_attackOffset);
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    SetFlameThrowerActive(mgr, false);
    x584_boneTracker.SetActive(false);
    x584_boneTracker.SetNoHorizontalAim(false);
    x754_30_inProjectileAttack = false;
    x584_boneTracker.SetTarget(mgr.GetPlayer()->GetUniqueId());
    break;
  }
}

void CMagdolite::Flinch(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    x400_24_hitByPlayerProjectile = false;
    x584_boneTracker.SetActive(false);
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_KnockBack, &CPatterned::TryKnockBack_Front, 0);
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    break;
  }
}

void CMagdolite::Retreat(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    x754_24_retreat = false;
    x584_boneTracker.SetActive(false);
    BodyCtrl()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
    x754_28_alert = true;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Generate, &CPatterned::TryGenerateDeactivate, 1);
    if (x32c_animState == kAS_Repeat) {
      BodyCtrl()->SetLocomotionType(pas::kLT_Internal7);
    }
    break;
  case kStateMsg_Deactivate:
    x754_28_alert = false;
    x32c_animState = kAS_NotReady;
    break;
  }
}

void CMagdolite::Generate(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    x754_24_retreat = false;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Generate, &CPatterned::TryGenerate, 0);
    if (x32c_animState == kAS_Repeat) {
      BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    }
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    break;
  }
}

void CMagdolite::GetUp(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    x754_24_retreat = false;
    x754_28_alert = true;
    AddMaterial(kMT_Orbit, kMT_Target, mgr);
    break;
  case kStateMsg_Update:
    if (x754_25_up) {
      TryCommand(mgr, pas::kAS_Step, &CPatterned::TryStep, pas::kSD_Up);
    } else {
      TryCommand(mgr, pas::kAS_Step, &CPatterned::TryStep, pas::kSD_Forward);
    }
    if (x32c_animState == kAS_Repeat) {
      BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
    }
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    x754_28_alert = false;
    break;
  }
}

void CMagdolite::Deactivate(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    x754_24_retreat = false;
    x584_boneTracker.SetActive(false);
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Step, &CPatterned::TryStep, pas::kSD_Down);
    if (x32c_animState == kAS_Repeat) {
      BodyCtrl()->SetLocomotionType(pas::kLT_Internal7);
    }
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    break;
  }
}

void CMagdolite::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  if (IsAlive()) {
    const CTransform4f oldXf = GetTransform();
    SetTransform(GetLctrTransform(rstl::string_l(skHeadLocator)));
    SendScriptMsgs(kSS_MassiveDeath, mgr, kSM_None);
    GenerateDeathExplosion(mgr);
    SetTransform(oldXf);
    AnimationData()->SubstituteModelData(x690_headlessModel);
    x460_knockBackController.SetEnableFreeze(false);
    BodyCtrl()->UnFreeze();
    if (!x754_26_lostMyHead) {
      x460_knockBackController.SetSeverity(pas::kS_Two);
    } else {
      AnimationData()->SubstituteModelData(x690_headlessModel);
    }
    CPatterned::Death(mgr, direction, state);
  }
}

void CMagdolite::Touch(CActor& actor, CStateManager& mgr) {}

bool CMagdolite::ShouldRetreat(CStateManager& mgr, float arg) { return x754_24_retreat; }

bool CMagdolite::Leash(CStateManager& mgr, float arg) {
  return (mgr.GetPlayer()->GetTranslation() - GetTranslation()).MagSquared() <
         x3c8_leashRadius * x3c8_leashRadius;
}

bool CMagdolite::LineOfSight(CStateManager& mgr, float arg) {
  const CVector3f mouth = GetLctrTransform(rstl::string_l(skMouthLocator)).GetTranslation();
  const CVector3f target = x710_attackOffset;
  CVector3f direction = target - mouth;
  direction.SetZ(0.f);
  const CVector3f forward = GetTransform().GetForward();
  if (direction.CanBeNormalized()) {
    direction = direction.AsNormalized();
    if (CVector3f::Dot(direction, forward) < x578_losMaxDistance) {
      return false;
    }
  }
  const CMaterialFilter filter =
      CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid, kMT_Character),
                                          CMaterialList(kMT_Player, kMT_ProjectilePassthrough));
  return mgr.RayCollideWorld(mouth, target, filter, this);
}

bool CMagdolite::InAttackPosition(CStateManager& mgr, float arg) {
  const CVector3f pos = GetTranslation();
  const CVector3f headPos = GetLctrTransform(rstl::string_l(skHeadLocator)).GetTranslation();
  const float headHeight = headPos.GetZ();
  const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
  CVector3f direction = aimPos - pos;
  const CVector3f forward = GetTransform().GetForward();
  if (aimPos.GetZ() > pos.GetZ() && aimPos.GetZ() < headHeight) {
    direction.SetZ(0.f);
  }
  const float distance = direction.Magnitude();
  if (close_enough(distance, 0.f)) {
    return false;
  }
  direction *= 1.f / distance;
  return !(CVector3f::Dot(direction, forward) < x578_losMaxDistance);
}

TUniqueId CMagdolite::GetBestConnectedObject(CStateManager& mgr, EScriptObjectState state,
                                             EScriptObjectMessage msg) {
  float bestDistance = FLT_MAX;
  TUniqueId bestId = kInvalidUniqueId;
  int count = 0;
  const float maxDistance = x754_29_useDetectionRange ? x3bc_detectionRange * x3bc_detectionRange
                                                      : x300_maxAttackRange * x300_maxAttackRange;
  const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
  for (AUTO(it, GetConnectionList().begin()); it != GetConnectionList().end(); ++it) {
    if (it->x0_state == state && it->x4_msg == msg) {
      TUniqueId id = mgr.GetIdForScript(it->x8_objId);
      const CEntity* entity = mgr.GetObjectById(id);
      if (entity && entity->GetActive()) {
        if (const CScriptWaypoint* waypoint = TCastToConstPtr< CScriptWaypoint >(entity)) {
          ++count;
          const float distance = (waypoint->GetTranslation() - playerPos).MagSquared();
          if (distance < maxDistance && distance < bestDistance) {
            bestId = id;
            bestDistance = distance;
          }
        }
      }
    }
  }
  if (!x754_29_useDetectionRange) {
    int skip = mgr.Random()->Next() % count;
    for (AUTO(it, GetConnectionList().begin()); it != GetConnectionList().end(); ++it) {
      if (it->x0_state == state && it->x4_msg == msg) {
        TUniqueId id = mgr.GetIdForScript(it->x8_objId);
        const CEntity* entity = mgr.GetObjectById(id);
        if (entity && entity->GetActive()) {
          if (const CScriptWaypoint* waypoint = TCastToConstPtr< CScriptWaypoint >(entity)) {
            if (skip == 0) {
              bestId = id;
              break;
            }
            --skip;
          }
        }
      }
    }
  }
  return bestId;
}

bool CMagdolite::HasAttackPattern(CStateManager& mgr, float arg) {
  return GetBestConnectedObject(mgr, kSS_Attack, kSM_Follow) != kInvalidUniqueId;
}

void CMagdolite::SelectTarget(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Update: {
    const TUniqueId target = GetBestConnectedObject(mgr, kSS_Attack, kSM_Follow);
    if (target != kInvalidUniqueId) {
      if (const CScriptWaypoint* waypoint =
              TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(target))) {
        SetTransform(waypoint->GetTransform());
        x71c_attackTarget = GetTranslation();
      }
    }
    break;
  }
  case kStateMsg_Deactivate:
    SnapToFacePlayer(mgr);
    break;
  }
}

void CMagdolite::CreateFlameThrower(CStateManager& mgr) {
  if (x6c8_flameThrowerId == kInvalidUniqueId) {
    x6c8_flameThrowerId = mgr.AllocateUniqueId();
    CFlameThrower* flame = rs_new CFlameThrower(
        x6cc_flameThrowerDesc, rstl::string_l("Magdolite_Flame"), kWT_Plasma, x6a8_flameInfo,
        CTransform4f::Identity(), kMT_CollisionActor, x6d4_flameThrowerDamage, x6c8_flameThrowerId,
        GetCurrentAreaId(), GetUniqueId(), CWeapon::kPA_None, kInvalidAssetId,
        CSfxManager::kInternalInvalidSfxId, kInvalidAssetId);
    mgr.AddObject(*flame);
  }
}

void CMagdolite::SetFlameThrowerActive(CStateManager& mgr, bool fire) {
  if ((!fire || x754_30_inProjectileAttack) && IsAlive()) {
    if (CFlameThrower* flame = static_cast< CFlameThrower* >(mgr.ObjectById(x6c8_flameThrowerId))) {
      if (fire) {
        flame->Fire(GetTransform(), mgr, false);
      } else {
        flame->Reset(mgr, false);
      }
    }
    x754_27_flameThrowerActive = fire;
  }
}

void CMagdolite::FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) {
  if (x754_28_alert && state == kFS_InFluid) {
    if (mgr.GetFluidPlaneManager()->GetLastRippleDeltaTime(GetUniqueId()) >= 2.3f) {
      const CVector3f center(GetTranslation().GetX(), GetTranslation().GetY(),
                             water.GetTriggerBoundsWR().GetMaxPoint().GetZ());
      water.FluidPlane().AddRipple(3.f, GetUniqueId(), center, water, mgr);
    }
  }
}

void CMagdolite::DoContactDamage(TUniqueId uid, CStateManager& mgr) {
  if (const CCollisionActor* actor = TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(uid))) {
    if (IsAlive()) {
      CDamageInfo damage = GetContactDamage();
      for (AUTO(it, x69c_headCollisionActors.begin()); it != x69c_headCollisionActors.end(); ++it) {
        if (actor->GetUniqueId() == *it) {
          damage = x6f0_headContactDamage;
          break;
        }
      }
      const TUniqueId touched = actor->GetLastTouchedObject();
      if (touched == mgr.GetPlayer()->GetUniqueId() && 0.f >= x420_curDamageRemTime) {
        mgr.ApplyDamage(
            GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), damage,
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
        x420_curDamageRemTime = x424_damageWaitTime;
      }
    }
  }
}

CMagdolite::~CMagdolite() {}
