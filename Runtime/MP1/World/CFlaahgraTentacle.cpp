#include "Runtime/MP1/World/CFlaahgraTentacle.hpp"

#include <array>

#include "Runtime/CStateManager.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptTrigger.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
namespace {
constexpr std::string_view skpTentacleTip = "Arm_12"sv;
constexpr std::array<SSphereJointInfo, 3> skJointList{{
    {"Arm_8", 2.f},
    {"Arm_10", 1.2f},
    {"Arm_12", 1.2f},
}};
} // Anonymous namespace

CFlaahgraTentacle::CFlaahgraTentacle(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                     const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                                     const CActorParameters& actParms)
: CPatterned(ECharacter::FlaahgraTentacle, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::Restricted, actParms, EKnockBackVariant::Large) {
  GetActorLights()->SetCastShadows(false);
  x460_knockBackController.SetAutoResetImpulse(false);
  CreateShadow(false);
}
void CFlaahgraTentacle::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {

  switch (msg) {
  case EScriptObjectMessage::Registered: {
    x450_bodyController->Activate(mgr);
    SetupCollisionManager(mgr);
    break;
  }
  case EScriptObjectMessage::Deleted: {
    x56c_collisionManager->Destroy(mgr);
    if (const TCastToPtr<CScriptTrigger> trigger = mgr.ObjectById(x58c_triggerId)) {
      trigger->SetForceVector(x580_forceVector);
    }
    break;
  }
  case EScriptObjectMessage::Touched: {
    if (const TCastToConstPtr<CCollisionActor> colAct = mgr.GetObjectById(uid)) {
      if (colAct->GetLastTouchedObject() == mgr.GetPlayer().GetUniqueId() && x420_curDamageRemTime <= 0.f) {
        mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), GetContactDamage(),
                        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
        x420_curDamageRemTime = x424_damageWaitTime;
      }
    }
    break;
  }
  case EScriptObjectMessage::Play: {
    x578_ = 0.04f;
    break;
  }
  case EScriptObjectMessage::Deactivate: {
    DeathDelete(mgr);
    break;
  }
  case EScriptObjectMessage::Open: {
    ExtractTentacle(mgr);
    break;
  }
  case EScriptObjectMessage::Close: {
    RetractTentacle(mgr);
    break;
  }
  case EScriptObjectMessage::InitializedInArea: {
    SaveBombSlotInfo(mgr);
    break;
  }
  default:
    break;
  }

  CPatterned::AcceptScriptMsg(msg, uid, mgr);
}

void CFlaahgraTentacle::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  CPatterned::Think(dt, mgr);
  x56c_collisionManager->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);

  if (x574_ > 0.f) {
    x574_ -= dt;
  }

  if (x578_ > 0.f) {
    x578_ -= dt;
  }
}

void CFlaahgraTentacle::AddSphereCollisionList(const SSphereJointInfo* sphereJoints, size_t jointCount,
                                               std::vector<CJointCollisionDescription>& outJoints) {
  const CAnimData* animData = GetModelData()->GetAnimationData();

  for (size_t i = 0; i < jointCount; ++i) {
    const SSphereJointInfo& sphereJoint = sphereJoints[i];
    const CSegId segId = animData->GetLocatorSegId(sphereJoint.name);

    if (segId.IsInvalid()) {
      continue;
    }

    outJoints.push_back(CJointCollisionDescription::SphereCollision(segId, sphereJoint.radius, sphereJoint.name, 10.f));
  }
}

void CFlaahgraTentacle::SetupCollisionManager(CStateManager& mgr) {
  std::vector<CJointCollisionDescription> jointList;
  AddSphereCollisionList(skJointList.data(), skJointList.size(), jointList);
  x56c_collisionManager =
      std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), jointList, true);

  for (u32 i = 0; i < x56c_collisionManager->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = x56c_collisionManager->GetCollisionDescFromIndex(i);
    if (const TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(desc.GetCollisionActorId())) {
      colAct->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Player},
          {EMaterialTypes::Character,
           EMaterialTypes::CollisionActor,
           EMaterialTypes::NoStaticCollision,
           EMaterialTypes::NoPlatformCollision}));
      colAct->AddMaterial(EMaterialTypes::ScanPassthrough);
      colAct->SetDamageVulnerability(*GetDamageVulnerability());

      if (x57c_tentacleTipAct == kInvalidUniqueId && desc.GetName() == skpTentacleTip) {
        x57c_tentacleTipAct = desc.GetCollisionActorId();
      }
    }
  }

  RemoveMaterial(EMaterialTypes::Solid, EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
  AddMaterial(EMaterialTypes::Scannable, mgr);

}
zeus::CVector3f CFlaahgraTentacle::GetAimPosition(const CStateManager& mgr, float dt) const {
  if (const TCastToConstPtr<CCollisionActor> colAct = mgr.GetObjectById(x57c_tentacleTipAct)) {
    return colAct->GetTranslation();
  }

  return CPatterned::GetAimPosition(mgr, dt);
}
void CFlaahgraTentacle::ExtractTentacle(CStateManager& mgr) {
  if (!Inside(mgr, 0.f)) {
    return;
  }

  x58e_24_ = true;

  if (const TCastToPtr<CScriptTrigger> trigger = mgr.ObjectById(x58c_triggerId)) {
    trigger->SetForceVector(x580_forceVector);
  }
}

void CFlaahgraTentacle::RetractTentacle(CStateManager& mgr) {
  x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
  if (const TCastToPtr<CScriptTrigger> trigger = mgr.ObjectById(x58c_triggerId)) {
    trigger->SetForceVector({});
  }
}
void CFlaahgraTentacle::SaveBombSlotInfo(CStateManager& mgr) {
  for (const SConnection& conn : GetConnectionList()) {
    if (conn.x0_state != EScriptObjectState::Modify || conn.x4_msg != EScriptObjectMessage::ToggleActive) {
      continue;
    }

    const TUniqueId uid = mgr.GetIdForScript(conn.x8_objId);
    if (const TCastToConstPtr<CScriptTrigger> trigger = mgr.GetObjectById(uid)) {
      x58c_triggerId = uid;
      x580_forceVector = trigger->GetForceVector();
      return;
    }
  }
}

bool CFlaahgraTentacle::ShouldAttack(CStateManager& mgr, float) {
  if (x578_ > 0.f) {
    return true;
  }

  if (x574_ > 0.f || mgr.GetPlayer().IsInWaterMovement()) {
    return false;
  }

  if (const TCastToConstPtr<CCollisionActor> colAct = mgr.GetObjectById(x57c_tentacleTipAct)) {
    const float mag = (colAct->GetTranslation().toVec2f() - mgr.GetPlayer().GetTranslation().toVec2f()).magSquared();
    return mag >= (x2fc_minAttackRange * x2fc_minAttackRange) && mag <= (x300_maxAttackRange * x300_maxAttackRange);
  }

  return false;
}

void CFlaahgraTentacle::Attack(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_ = 0;
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::MeleeAttack) {
        x568_ = 2;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(
            CBCMeleeAttackCmd((x578_ > 0.f ? pas::ESeverity::Zero : pas::ESeverity::One), {}));
      }

    } else if (x568_ == 2 && x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::MeleeAttack) {
      x568_ = 3;
    }

  } else if (msg == EStateMsg::Deactivate) {
    x574_ = (x308_attackTimeVariation * mgr.GetActiveRandom()->Float()) + x304_averageAttackTime;
    x578_ = 0.f;
  }
}
void CFlaahgraTentacle::Retreat(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Update) {
    if (!x58e_24_) {
      return;
    }

    if (x330_stateMachineState.GetTime() <= 1.f) {
      return;
    }

    if (const TCastToConstPtr<CScriptTrigger> trigger = mgr.ObjectById(x58c_triggerId)) {
      if (trigger->IsPlayerTriggerProc()) {
        x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    x58e_24_ = false;
  }
}
void CFlaahgraTentacle::InActive(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x570_ = 0.f;
  } else if (msg == EStateMsg::Update) {
    if (Inside(mgr, 0.f)) {
      return;
    }

    if (const TCastToConstPtr<CScriptTrigger> trigger = mgr.ObjectById(x58c_triggerId)) {
      if (trigger->IsPlayerTriggerProc()) {
        if (x570_ > 1.f) {
          RetractTentacle(mgr);
          ExtractTentacle(mgr);
        } else {
          x570_ += arg;
        }
      }
    }
  }
}

}