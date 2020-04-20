#include "Runtime/MP1/World/CPhazonHealingNodule.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CPatternedInfo.hpp"

namespace urde::MP1 {
CPhazonHealingNodule::CPhazonHealingNodule(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                           const zeus::CTransform& xf, CModelData&& mData,
                                           const CActorParameters& actParams, const CPatternedInfo& pInfo,
                                           CAssetId particleDescId, std::string actorLctr)
: CPatterned(ECharacter::PhazonHealingNodule, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::Restricted, actParams, EKnockBackVariant::Medium)
, x570_electricDesc(g_SimplePool->GetObj(SObjectTag{SBIG('ELSC'), particleDescId}))
, x580_initialHealthInfo(pInfo.GetHealthInfo())
, x58c_actorLctr(std::move(actorLctr)) {
  const CMaterialFilter& filter = GetMaterialFilter();
  CMaterialList include = filter.GetIncludeList();
  CMaterialList exclude = filter.GetExcludeList();
  exclude.Add(EMaterialTypes::Character);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
}

void CPhazonHealingNodule::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::Decrement:
    x568_active = 0;
    x57c_particleElectric.reset();
    x56c_emitting = false;
    break;
  case EScriptObjectMessage::Increment:
    x568_active = 1;
    break;
  case EScriptObjectMessage::Reset:
    *HealthInfo(mgr) = x580_initialHealthInfo;
    break;
  case EScriptObjectMessage::Registered:
    if (!GetBodyController()->GetActive()) {
      GetBodyController()->Activate(mgr);
    }
    // TODO remove const_cast
    *const_cast<CDamageVulnerability*>(GetDamageVulnerability()) = CDamageVulnerability::ImmuneVulnerabilty();
    GetKnockBackController().SetAutoResetImpulse(false);
    GetBodyController()->SetLocomotionType(pas::ELocomotionType::Relaxed);
    RemoveMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
    AddMaterial(EMaterialTypes::Immovable, mgr);
    break;
  case EScriptObjectMessage::InitializedInArea:
    CPatterned::AcceptScriptMsg(msg, uid, mgr);
    for (const auto& conn : GetConnectionList()) {
      auto connId = mgr.GetIdForScript(conn.x8_objId);
      if (conn.x0_state == EScriptObjectState::Patrol && connId != kInvalidUniqueId &&
          conn.x4_msg == EScriptObjectMessage::Activate) {
        x56e_connId = connId;
      }
    }
    break;
  default:
    CPatterned::AcceptScriptMsg(msg, uid, mgr);
    break;
  }
}

void CPhazonHealingNodule::Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) {
  SendScriptMsgs(EScriptObjectState::Dead, mgr, EScriptObjectMessage::None);
  SendScriptMsgs(EScriptObjectState::DeathRattle, mgr, EScriptObjectMessage::None);
}

void CPhazonHealingNodule::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                                           float dt) {
  if (type == EUserEventType::EndAction) {
    x56c_emitting = false;
    x57c_particleElectric.reset();
  } else if (type == EUserEventType::BeginAction) {
    x56c_emitting = true;
    x57c_particleElectric = std::make_unique<CParticleElectric>(x570_electricDesc);
    x57c_particleElectric->SetParticleEmission(true);
  } else {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CPhazonHealingNodule::Faint(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Update) {
    if (x588_state == 0) {
      if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::Step) {
        x588_state = 2;
        GetBodyController()->SetLocomotionType(pas::ELocomotionType::Relaxed);
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCStepCmd(pas::EStepDirection::Backward, pas::EStepType::Normal));
      }
    } else if (x588_state == 2 && GetBodyController()->GetCurrentStateId() != pas::EAnimationState::Step) {
      x588_state = 3;
    }
  } else if (msg == EStateMsg::Activate) {
    x588_state = 0;
  }
}

void CPhazonHealingNodule::Growth(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Update) {
    if (x588_state == 0) {
      if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::Step) {
        x588_state = 2;
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCStepCmd(pas::EStepDirection::Forward, pas::EStepType::Normal));
      }
    } else if (x588_state == 2 && GetBodyController()->GetCurrentStateId() != pas::EAnimationState::Step) {
      x588_state = 3;
    }
  } else if (msg == EStateMsg::Activate) {
    x588_state = 0;
    GetBodyController()->SetLocomotionType(pas::ELocomotionType::Lurk);
  }
}

void CPhazonHealingNodule::KnockBack(const zeus::CVector3f& vec, CStateManager& mgr, const CDamageInfo& info,
                                     EKnockBackType type, bool inDeferred, float magnitude) {}

void CPhazonHealingNodule::Lurk(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    GetBodyController()->SetLocomotionType(pas::ELocomotionType::Lurk);
  }
}

void CPhazonHealingNodule::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    GetBodyController()->SetLocomotionType(pas::ELocomotionType::Relaxed);
    RemoveMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
  }
}

void CPhazonHealingNodule::Render(CStateManager& mgr) {
  if (x57c_particleElectric) {
    x57c_particleElectric->Render();
  }
  CPatterned::Render(mgr);
}

void CPhazonHealingNodule::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  CPatterned::Think(dt, mgr);
  if (HealthInfo(mgr)->GetHP() <= 0.f) {
    x57c_particleElectric.reset();
    x56c_emitting = false;
    x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), "Patrol"sv);
    x568_active = 0;
  }
  if (x57c_particleElectric) {
    UpdateParticleElectric(mgr);
    x57c_particleElectric->Update(dt);
  }
}

void CPhazonHealingNodule::UpdateParticleElectric(CStateManager& mgr) {
  if (!x57c_particleElectric) {
    return;
  }
  if (auto entity = static_cast<const CPatterned*>(mgr.GetObjectById(x56e_connId))) {
    auto electricityLctrXf = GetLctrTransform("Electricity_LCTR"sv);
    auto actorLctrXf = entity->GetLctrTransform(x58c_actorLctr);
    x57c_particleElectric->SetOverrideIPos(electricityLctrXf.origin);
    x57c_particleElectric->SetOverrideFPos(actorLctrXf.origin);
  }
}

bool CPhazonHealingNodule::AnimOver(CStateManager&, float arg) { return x588_state == 3; }

bool CPhazonHealingNodule::InRange(CStateManager&, float arg) { return x568_active == 0; }

bool CPhazonHealingNodule::InDetectionRange(CStateManager&, float arg) { return x568_active == 1; }

void CPhazonHealingNodule::MassiveDeath(CStateManager& mgr) { Death(mgr, zeus::skZero3f, EScriptObjectState::Dead); }

void CPhazonHealingNodule::MassiveFrozenDeath(CStateManager& mgr) {
  Death(mgr, zeus::skZero3f, EScriptObjectState::Dead);
}

void CPhazonHealingNodule::PhazeOut(CStateManager& mgr) { Death(mgr, zeus::skZero3f, EScriptObjectState::Dead); }
} // namespace urde::MP1
