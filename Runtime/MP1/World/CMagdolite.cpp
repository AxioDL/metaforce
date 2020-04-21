#include "Runtime/MP1/World/CMagdolite.hpp"

#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/Collision/CJointCollisionDescription.hpp"
#include "Runtime/Graphics/CSkinnedModel.hpp"
#include "Runtime/Weapon/CFlameThrower.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptWater.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "TCastTo.hpp" // Generated file, do not modify include path

#include <array>

namespace urde::MP1 {

namespace {
constexpr std::array<SSphereJointInfo, 5> skSpineJoints{{
    {"spine1", 0.75f},
    {"spine3", 0.75f},
    {"spine5", 0.75f},
    {"spine7", 0.75f},
    {"spine9", 0.75f},
}};

constexpr std::array<SOBBJointInfo, 2> skHeadJoints{{
    {"head", "Top_LCTR", {1.f, 0.15f, 0.5f}},
    {"head", "Bottom_LCTR", {0.75f, 0.15f, 0.25f}},
}};
} // namespace

CMagdolite::CMagdolite(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                       CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms, float f1,
                       float f2, const CDamageInfo& dInfo1, const CDamageInfo& dInfo2,
                       const CDamageVulnerability& dVuln1, const CDamageVulnerability& dVuln2, CAssetId modelId,
                       CAssetId skinId, float f3, float f4, float f5, float f6, const CFlameInfo& magData, float f7,
                       float f8, float f9)
: CPatterned(ECharacter::Magdolite, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::BiPedal, actParms, EKnockBackVariant::Large)
, x568_initialDelay(f4)
, x56c_minDelay(f5)
, x570_maxDelay(f6)
, x574_minHp(f3)
, x578_losMaxDistance(std::cos(zeus::degToRad(f2)))
, x57c_(f1)
, x584_boneTracker(*GetModelData()->GetAnimationData(), "head"sv, zeus::degToRad(f1), zeus::degToRad(90.f),
                   EBoneTrackingFlags::ParentIk)
, x5bc_instaKillVulnerability(dVuln1)
, x624_normalVulnerability(dVuln2)
, x690_headlessModel(
      CToken(TObjOwnerDerivedFromIObj<CSkinnedModel>::GetNewDerivedObject(std::make_unique<CSkinnedModel>(
          g_SimplePool->GetObj({SBIG('CMDL'), modelId}), g_SimplePool->GetObj({SBIG('CSKR'), skinId}),
          x64_modelData->GetAnimationData()->GetModelData()->GetLayoutInfo(), 1, 1))))
, x6a8_flameInfo(magData)
, x6cc_flameThrowerDesc(g_SimplePool->GetObj("FlameThrower"sv))
, x6d4_flameThrowerDamage(dInfo1)
, x6f0_headContactDamage(dInfo2)
, x71c_attackTarget(xf.origin)
, x744_(f7)
, x748_(f8)
, x74c_(f9) {
  x460_knockBackController.SetAutoResetImpulse(false);
  x460_knockBackController.SetEnableBurn(false);
  // redundant
  // x690_headlessModel->SetLayoutInfo(GetModelData()->GetAnimationData()->GetModelData()->GetLayoutInfo());
}

void CMagdolite::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::Touched:
    ApplyContactDamage(uid, mgr);
    break;
  case EScriptObjectMessage::Damage:
  case EScriptObjectMessage::InvulnDamage:
    if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(uid)) {
      if (proj->GetOwnerId() == mgr.GetPlayer().GetUniqueId()) {
        if (GetBodyController()->GetPercentageFrozen() <= 0.f ||
            x5bc_instaKillVulnerability.GetVulnerability(proj->GetDamageInfo().GetWeaponMode(), false) ==
                EVulnerability::Deflect) {
          float hp = HealthInfo(mgr)->GetHP();
          if (x70c_curHealth - hp <= x574_minHp) {
            if (x624_normalVulnerability.GetVulnerability(proj->GetDamageInfo().GetWeaponMode(), false) !=
                EVulnerability::Deflect) {
              x400_24_hitByPlayerProjectile = true;
            }
          } else {
            x70c_curHealth = hp;
            x754_24_retreat = true;
          }
        } else if (x400_24_hitByPlayerProjectile) {
          x754_26_lostMyHead = true;
          x401_30_pendingDeath = true;
        }
      }
    }
    return;
  case EScriptObjectMessage::SuspendedMove:
    if (x580_collisionManager) {
      x580_collisionManager->SetMovable(mgr, false);
    }
    break;
  case EScriptObjectMessage::Registered: {
    x450_bodyController->Activate(mgr);
    RemoveMaterial(EMaterialTypes::Solid, mgr);
    AddMaterial(EMaterialTypes::NonSolidDamageable, mgr);
    x584_boneTracker.SetActive(false);
    CreateShadow(false);
    const zeus::CVector3f boxExtents = GetBoundingBox().extents();
    SetBoundingBox({-boxExtents, boxExtents});
    SetupCollisionActors(mgr);
    x330_stateMachineState.SetDelay(0.f);
    CreateFlameThrower(mgr);
    x70c_curHealth = GetHealthInfo(mgr)->GetHP();
    break;
  }
  case EScriptObjectMessage::Deleted:
    x580_collisionManager->Destroy(mgr);
    if (x6c8_flameThrowerId != kInvalidUniqueId) {
      mgr.FreeScriptObject(x6c8_flameThrowerId);
      x6c8_flameThrowerId = kInvalidUniqueId;
    }
    break;
  default:
    break;
  }
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
}

void CMagdolite::ApplyContactDamage(TUniqueId uid, CStateManager& mgr) {
  if (TCastToConstPtr<CCollisionActor> colAct = mgr.GetObjectById(uid)) {
    if (!IsAlive()) {
      return;
    }
    CDamageInfo dInfo = GetContactDamage();

    for (TUniqueId testId : x69c_) {
      if (testId == colAct->GetUniqueId()) {
        dInfo = x6f0_headContactDamage;
        break;
      }
    }

    if (colAct->GetLastTouchedObject() == mgr.GetPlayer().GetUniqueId() && x420_curDamageRemTime <= 0.f) {
      mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), dInfo,
                      CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
      x420_curDamageRemTime = x424_damageWaitTime;
    }
  }
}

void CMagdolite::SetupCollisionActors(CStateManager& mgr) {
  std::vector<CJointCollisionDescription> joints;
  joints.reserve(8);
  const CAnimData* animData = GetModelData()->GetAnimationData();
  for (const auto& info : skSpineJoints) {
    joints.push_back(CJointCollisionDescription::SphereCollision(animData->GetLocatorSegId(info.name), info.radius,
                                                                 info.name, 200.f));
  }

  for (const auto& info : skHeadJoints) {
    joints.push_back(CJointCollisionDescription::OBBAutoSizeCollision(
        animData->GetLocatorSegId(info.from), animData->GetLocatorSegId(info.to), info.bounds,
        CJointCollisionDescription::EOrientationType::One, info.to, 200.f));
  }
  x580_collisionManager =
      std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), joints, GetActive());

  for (int i = 0; i < x580_collisionManager->GetNumCollisionActors(); ++i) {
    const auto& desc = x580_collisionManager->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(desc.GetCollisionActorId())) {
      colAct->AddMaterial(EMaterialTypes::AIJoint, mgr);
    }
  }

  for (int i = 0; i < x580_collisionManager->GetNumCollisionActors(); ++i) {
    const auto& desc = x580_collisionManager->GetCollisionDescFromIndex(i);
    if (desc.GetName() == "Top_LCTR"sv || desc.GetName() == "Bottom_LCTR"sv) {
      if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(desc.GetCollisionActorId())) {
        colAct->AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
        x69c_.push_back(colAct->GetUniqueId());
      }
    }
  }
}

void CMagdolite::CreateFlameThrower(CStateManager& mgr) {
  if (x6c8_flameThrowerId != kInvalidUniqueId) {
    return;
  }

  x6c8_flameThrowerId = mgr.AllocateUniqueId();
  mgr.AddObject(new CFlameThrower(x6cc_flameThrowerDesc, "Magdolite_Flame"sv, EWeaponType::Plasma, x6a8_flameInfo, {},
                                  EMaterialTypes::CollisionActor, x6d4_flameThrowerDamage, x6c8_flameThrowerId,
                                  GetAreaIdAlways(), GetUniqueId(), EProjectileAttrib::None, CAssetId(), -1,
                                  CAssetId()));
}

void CMagdolite::LaunchFlameThrower(CStateManager& mgr, bool fire) {
  if ((!fire || x754_30_inProjectileAttack) && IsAlive()) {
    if (auto* fl = static_cast<CFlameThrower*>(mgr.ObjectById(x6c8_flameThrowerId))) {
      if (!fire) {
        fl->Reset(mgr, false);
      } else {
        fl->Fire(GetTransform(), mgr, false);
      }
    }
    x754_27_flameThrowerActive = fire;
  }
}

void CMagdolite::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  if (!GetActive()) {
    return;
  }

  x758_ += dt;
  if (GetBodyController()->GetPercentageFrozen() > 0.f && x754_27_flameThrowerActive) {
    LaunchFlameThrower(mgr, false);
  }

  if (!IsAlive()) {
    if (auto* fl = static_cast<CFlameThrower*>(mgr.ObjectById(x6c8_flameThrowerId))) {
      fl->SetTransform(GetLctrTransform("LCTR_MAGMOUTH"sv), dt);
    }
  }

  zeus::CVector3f plPos = mgr.GetPlayer().GetTranslation();
  zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
  zeus::CTransform xf = GetLctrTransform("LCTR_MAGMOUTH"sv);
  zeus::CVector3f aimDir = (aimPos - xf.origin).normalized();
  float angleDiff = zeus::CVector3f::getAngleDiff(xf.frontVector(), aimDir);
  float dVar7 = std::min(angleDiff, zeus::degToRad(x57c_));
  if (xf.upVector().dot(aimDir) < 0.f) {
    dVar7 = -dVar7;
  }

  float dVar8 = dVar7 + zeus::degToRad(x57c_) / 2.f * zeus::degToRad(x57c_);
  float dVar2 = 1.f - dVar8;
  x710_attackOffset = plPos * dVar2 + (aimPos * dVar8);

  if (GetActive()) { // lol tautologies
    if (IsAlive()) {
      GetModelData()->GetAnimationData()->PreRender();
      x584_boneTracker.Update(dt);
      x584_boneTracker.PreRender(mgr, *GetModelData()->GetAnimationData(), GetTransform(), GetModelData()->GetScale(),
                                 *GetBodyController());

      if (auto* fl = static_cast<CFlameThrower*>(mgr.ObjectById(x6c8_flameThrowerId))) {
        fl->SetTransform(GetLctrTransform("LCTR_MAGMOUTH"sv), dt);
      }
    }
    x580_collisionManager->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
    zeus::CTransform headXf = GetLocatorTransform("head"sv);
    MoveCollisionPrimitive(GetTransform().rotate(GetModelData()->GetScale() * headXf.origin));
    xe4_27_notInSortedLists = true;
  }

  if (x750_aiStage == 2) {
    if (x738_ * 0.5f <= x734_) {
      x740_ -= x73c_ * dt;
    } else {
      x740_ += x73c_ * dt;
    }
    x734_ += x740_ * dt;
    SetTranslation(GetTranslation() - zeus::CVector3f{0.f, 0.f, x740_ * dt});
    if (GetTranslation().z() < x728_cachedTarget.z()) {
      SetTranslation(x728_cachedTarget);
      x750_aiStage = 0;
    }
  } else if (x750_aiStage == 1) {
    if (x738_ * 0.5f <= x734_) {
      x740_ -= x73c_ * dt;
    } else {
      x740_ += x73c_ * dt;
    }
    x734_ += x740_ * dt;

    SetTranslation(GetTranslation() + zeus::CVector3f{0.f, 0.f, x740_ * dt});
    if (GetTranslation().z() > x728_cachedTarget.z()) {
      SetTranslation(x728_cachedTarget);
      x750_aiStage = 0;
    }
  }
}

void CMagdolite::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  if (type == EUserEventType::DamageOff) {
    LaunchFlameThrower(mgr, false);
  } else if (type == EUserEventType::DamageOn) {
    LaunchFlameThrower(mgr, true);
  } else if (type == EUserEventType::BreakLockOn) {
    RemoveMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
    mgr.GetPlayer().SetOrbitRequestForTarget(GetUniqueId(), CPlayer::EPlayerOrbitRequest::ActivateOrbitSource, mgr);
    return;
  }
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void CMagdolite::Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) {
  if (!IsAlive()) {
    return;
  }
  zeus::CTransform tmpXf = GetTransform();
  SetTransform(GetLctrTransform("head"sv));
  SendScriptMsgs(EScriptObjectState::MassiveDeath, mgr, EScriptObjectMessage::None);
  GenerateDeathExplosion(mgr);
  SetTransform(tmpXf);
  GetModelData()->GetAnimationData()->SubstituteModelData(x690_headlessModel);
  x460_knockBackController.SetEnableFreeze(false);
  GetBodyController()->UnFreeze();
  if (!x754_26_lostMyHead) {
    x460_knockBackController.SetSeverity(pas::ESeverity::Two);
  } else {
    GetModelData()->GetAnimationData()->SubstituteModelData(x690_headlessModel);
  }
  CPatterned::Death(mgr, direction, state);
}

void CMagdolite::FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) {
  if (x754_28_alert && state == EFluidState::InFluid) {
    CFluidPlaneManager* flMgr = mgr.GetFluidPlaneManager();
    if (flMgr->GetLastRippleDeltaTime(GetUniqueId()) >= 2.3f) {
      zeus::CVector3f center = GetTranslation();
      center.z() = float(water.GetTriggerBoundsWR().max.z());
      water.GetFluidPlane().AddRipple(3.f, GetUniqueId(), center, water, mgr);
    }
  }
}

void CMagdolite::SelectTarget(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Update) {
    TUniqueId targetId = FindSuitableTarget(mgr, EScriptObjectState::Attack, EScriptObjectMessage::Follow);
    if (targetId != kInvalidUniqueId) {
      if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(targetId)) {
        SetTransform(wp->GetTransform());
        x71c_attackTarget = GetTranslation();
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    UpdateOrientation(mgr);
  }
}

void CMagdolite::Generate(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    x754_24_retreat = false;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Generate, &CPatterned::TryGenerateNoXf, 0);
    if (x32c_animState == EAnimState::Repeat) {
      GetBodyController()->SetLocomotionType(pas::ELocomotionType::Relaxed);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}

void CMagdolite::Deactivate(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    x754_24_retreat = false;
    x584_boneTracker.SetActive(false);
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Step, &CPatterned::TryStep, int(pas::EStepDirection::Down));
    if (x32c_animState == EAnimState::Repeat) {
      GetBodyController()->SetLocomotionType(pas::ELocomotionType::Internal7);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}

void CMagdolite::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    x584_boneTracker.SetActive(false);
    zeus::CVector3f plPos = mgr.GetPlayer().GetTranslation();
    zeus::CTransform headXf = GetLctrTransform("head"sv);
    float zDiff = headXf.origin.z() - plPos.z();

    float fVar1 = (zDiff - x74c_);
    float fVar2 = 0.f;
    if (fVar1 >= 0.f) {
      fVar2 = fVar1;
      if (x748_ < fVar1) {
        fVar2 = x748_;
      }
    }
    x728_cachedTarget = x71c_attackTarget - zeus::CVector3f(0.f, 0.f, fVar1 - fVar2);
    x740_ = 0.f;
    x734_ = 0.f;
    x738_ = fVar2;
    x73c_ = (2.f * x738_) / (x744_ * x744_);
    if (GetTranslation().z() <= x728_cachedTarget.z()) {
      x750_aiStage = 1;
    } else {
      x750_aiStage = 2;
    }
    x754_30_inProjectileAttack = true;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::MeleeAttack, &CPatterned::TryMeleeAttack, 1);
    zeus::CVector3f direction = (mgr.GetPlayer().GetTranslation().toVec2f() - GetTranslation().toVec2f()).normalized();
    float angle = zeus::CVector3f::getAngleDiff(GetTransform().frontVector(), direction);
    if (GetTransform().rightVector().dot(direction) > 0.f) {
      angle *= -1.f;
    }

    if ((57.29f * angle) <= x3b8_turnSpeed && (57.29f * angle) < -x3b8_turnSpeed) {
      angle = zeus::degToRad(-x3b8_turnSpeed);
    } else {
      angle = zeus::degToRad(x3b8_turnSpeed);
    }

    RotateInOneFrameOR(zeus::CQuaternion::fromAxisAngle({0.f, 0.f, 1.f}, angle), arg);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    LaunchFlameThrower(mgr, false);
    x750_aiStage = 1;
    x728_cachedTarget = x71c_attackTarget;
    x740_ = 0.f;
    x744_ = 0.f;
    x738_ = x728_cachedTarget.z() - GetTranslation().z();
    x73c_ = (2.f * x738_) / (x744_ * x744_);
    x754_30_inProjectileAttack = false;
  }
}

void CMagdolite::Active(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    GetBodyController()->SetLocomotionType(pas::ELocomotionType::Lurk);
    x584_boneTracker.SetActive(true);
    x584_boneTracker.SetTarget(mgr.GetPlayer().GetUniqueId());
    x400_24_hitByPlayerProjectile = false;
    x754_29_useDetectionRange = false;
    x330_stateMachineState.SetDelay(x568_initialDelay);
  } else if (msg == EStateMsg::Update) {
    zeus::CVector3f posDiff = (mgr.GetPlayer().GetTranslation().toVec2f() - GetTranslation().toVec2f());
    if (posDiff.canBeNormalized()) {
      posDiff.normalize();
      if (GetTransform().frontVector().dot(posDiff) < x578_losMaxDistance) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCLocomotionCmd({}, posDiff, 1.f));
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    x330_stateMachineState.SetDelay(((x570_maxDelay - x56c_minDelay) * mgr.GetActiveRandom()->Float()) + x56c_minDelay);
  }
}

void CMagdolite::InActive(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    xe7_27_enableRender = false;
    GetBodyController()->SetLocomotionType(pas::ELocomotionType::Internal7);
    RemoveMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
  } else if (msg == EStateMsg::Deactivate) {
    AddMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
    x754_25_up = false;
    UpdateOrientation(mgr);
    xe7_27_enableRender = true;
  }
}

void CMagdolite::GetUp(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    x754_24_retreat = false;
    x754_28_alert = true;
    AddMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
  } else if (msg == EStateMsg::Update) {
    if (!x754_25_up) {
      TryCommand(mgr, pas::EAnimationState::Step, &CPatterned::TryStep, int(pas::EStepDirection::Forward));
    } else {
      TryCommand(mgr, pas::EAnimationState::Step, &CPatterned::TryStep, int(pas::EStepDirection::Up));
    }
    if (x32c_animState == EAnimState::Repeat) {
      GetBodyController()->SetLocomotionType(pas::ELocomotionType::Lurk);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    x754_28_alert = false;
  }
}

void CMagdolite::Taunt(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    x584_boneTracker.SetActive(true);
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Taunt, &CPatterned::TryTaunt, 1);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    x584_boneTracker.SetActive(false);
  }
}

void CMagdolite::Lurk(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    GetBodyController()->SetLocomotionType(pas::ELocomotionType::Relaxed);
    x584_boneTracker.SetActive(false);
    x330_stateMachineState.SetDelay(0.f);
    AddMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
  } else if (msg == EStateMsg::Deactivate) {
    x754_25_up = true;
  }
}

void CMagdolite::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    LaunchFlameThrower(mgr, true);
    x584_boneTracker.SetActive(true);
    x584_boneTracker.SetNoHorizontalAim(true);
    x584_boneTracker.SetTarget(kInvalidUniqueId);
    x754_30_inProjectileAttack = true;
  } else if (msg == EStateMsg::Update) {
    if (TooClose(mgr, 0.f)) {
      TryCommand(mgr, pas::EAnimationState::ProjectileAttack, &CPatterned::TryProjectileAttack, 0);
    } else {
      TryCommand(mgr, pas::EAnimationState::ProjectileAttack, &CPatterned::TryProjectileAttack, 1);
    }
    x584_boneTracker.SetTargetPosition(x710_attackOffset);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    LaunchFlameThrower(mgr, false);
    x584_boneTracker.SetActive(false);
    x584_boneTracker.SetNoHorizontalAim(false);
    x754_30_inProjectileAttack = false;
    x584_boneTracker.SetTarget(mgr.GetPlayer().GetUniqueId());
  }
}

void CMagdolite::Flinch(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    x400_24_hitByPlayerProjectile = false;
    x584_boneTracker.SetActive(false);
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::KnockBack, &CPatterned::TryKnockBack, 0);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}

void CMagdolite::Retreat(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    x754_24_retreat = false;
    x584_boneTracker.SetActive(false);
    GetBodyController()->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::NextState));
    x754_28_alert = true;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Generate, &CPatterned::TryGenerate, 1);
    if (x32c_animState == EAnimState::Repeat) {
      GetBodyController()->SetLocomotionType(pas::ELocomotionType::Internal7);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x754_28_alert = false;
    x32c_animState = EAnimState::NotReady;
  }
}

bool CMagdolite::InAttackPosition(CStateManager& mgr, float arg) {
  zeus::CTransform xf = GetLctrTransform("head"sv);
  zeus::CVector3f plAimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
  zeus::CVector3f diff = (plAimPos - GetTranslation());
  if (GetTranslation().z() < plAimPos.z() && plAimPos.z() < xf.origin.z()) {
    diff.z() = 0.f;
  }

  if (std::fabs(diff.magnitude()) >= FLT_EPSILON) {
    return ((1.f / diff.magnitude()) * diff).dot(GetTransform().frontVector()) < x578_losMaxDistance;
  }

  return false;
}

bool CMagdolite::Leash(CStateManager& mgr, float arg) {
  float dist = (GetTranslation() - mgr.GetPlayer().GetTranslation()).magSquared();
  bool ret = dist < x3c8_leashRadius * x3c8_leashRadius;
  return ret;
}

bool CMagdolite::HasAttackPattern(CStateManager& mgr, float arg) {
  return FindSuitableTarget(mgr, EScriptObjectState::Attack, EScriptObjectMessage::Follow) != kInvalidUniqueId;
}

bool CMagdolite::LineOfSight(CStateManager& mgr, float arg) {
  zeus::CTransform mouthXf = GetLctrTransform("LCTR_MAGMOUTH"sv);
  zeus::CVector3f diff = x710_attackOffset - mouthXf.origin;
  if (diff.canBeNormalized()) {
    diff.normalize();
    if (diff.dot(GetTransform().frontVector()) < x578_losMaxDistance) {
      return false;
    }
  }

  bool ret = mgr.RayCollideWorld(
      mouthXf.origin, x710_attackOffset,
      CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid, EMaterialTypes::Character},
                                          {EMaterialTypes::Player, EMaterialTypes::ProjectilePassthrough}),
      this);
  return ret;
}

bool CMagdolite::ShouldRetreat(CStateManager& mgr, float arg) { return x754_24_retreat; }

void CMagdolite::UpdateOrientation(CStateManager& mgr) {
  zeus::CVector3f plDiff = (mgr.GetPlayer().GetTranslation().toVec2f() - GetTranslation().toVec2f());
  plDiff = plDiff.normalized();
  float angle = zeus::CVector3f::getAngleDiff(GetTransform().frontVector(), plDiff);
  if (GetTransform().rightVector().dot(plDiff) > 0.f) {
    angle *= -1.f;
  }
  zeus::CQuaternion q = GetTransform().basis;
  q.rotateZ(angle);
  SetTransform(q.toTransform(GetTranslation()));
}

TUniqueId CMagdolite::FindSuitableTarget(CStateManager& mgr, EScriptObjectState state, EScriptObjectMessage msg) {
  float lastDistance = FLT_MAX;
  int wpCount = 0;
  float maxDistanceSq =
      x754_29_useDetectionRange ? x3bc_detectionRange * x3bc_detectionRange : x300_maxAttackRange * x300_maxAttackRange;

  TUniqueId tmpId = kInvalidUniqueId;
  for (const auto& conn : x20_conns) {
    if (conn.x0_state != state || conn.x4_msg != msg) {
      continue;
    }
    TUniqueId uid = mgr.GetIdForScript(conn.x8_objId);
    if (const CEntity* ent = mgr.GetObjectById(uid)) {
      if (!ent->GetActive()) {
        continue;
      }

      if (TCastToConstPtr<CScriptWaypoint> wp = ent) {
        ++wpCount;
        const float dist = (wp->GetTranslation() - mgr.GetPlayer().GetTranslation()).magSquared();
        if (dist < maxDistanceSq) {
          if (dist < lastDistance) {
            lastDistance = dist;
            tmpId = uid;
          }
        }
      }
    }
  }

  if (!x754_29_useDetectionRange) {
    int skipCount = mgr.GetActiveRandom()->Next();
    skipCount = skipCount - (skipCount / wpCount) * wpCount;
    for (const auto& conn : x20_conns) {
      if (conn.x0_state != state || conn.x4_msg != msg) {
        continue;
      }
      TUniqueId uid = mgr.GetIdForScript(conn.x8_objId);
      if (const CEntity* ent = mgr.GetObjectById(uid)) {
        if (!ent->GetActive()) {
          continue;
        }

        if (TCastToConstPtr<CScriptWaypoint>(ent)) {
          tmpId = uid;
          if (skipCount == 0) {
            break;
          }
          --skipCount;
        }
      }
    }
  }

  return tmpId;
}

} // namespace urde::MP1
