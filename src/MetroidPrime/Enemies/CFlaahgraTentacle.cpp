#include "MetroidPrime/Enemies/CFlaahgraTentacle.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/TCastTo.hpp"

const CFlaahgraTentacle::SSphereJointInfo CFlaahgraTentacle::skJointList[] = {
    {"Arm_8", 2.f},
    {"Arm_10", 1.2f},
    {"Arm_12", 1.2f},
};
const char* const CFlaahgraTentacle::skpTentacleTip = "Arm_12";

CFlaahgraTentacle::CFlaahgraTentacle(TUniqueId uid, const rstl::string& name,
                                     const CEntityInfo& info, const CTransform4f& xf,
                                     const CModelData& mData, const CPatternedInfo& pInfo,
                                     const CActorParameters& actParms)
: CPatterned(kC_FlaahgraTentacle, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Flyer, kCT_One,
             kBT_Restricted, actParms, kCS_Large)
, x568_(-1)
, x56c_collisionManager(nullptr)
, x570_(0.f)
, x574_(0.f)
, x578_(0.f)
, x57c_tentacleTipAct(kInvalidUniqueId)
, x580_forceVector(CVector3f::Zero())
, x58c_triggerId(kInvalidUniqueId)
, x58e_24_(false) {
  ActorLights()->SetCastShadows(false);
  x460_knockBackController.SetAutoResetImpulse(false);
  SetDrawShadow(false);
}

CFlaahgraTentacle::~CFlaahgraTentacle() {}

void CFlaahgraTentacle::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    CPatterned::Think(dt, mgr);
    x56c_collisionManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
    if (x574_ > 0.f) {
      x574_ -= dt;
    }
    if (x578_ > 0.f) {
      x578_ -= dt;
    }
  }
}

ENTITY_ACCEPT_IMPL(CFlaahgraTentacle)

void CFlaahgraTentacle::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                        CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered:
    x450_bodyController->Activate(mgr);
    SetupCollisionManager(mgr);
    break;
  case kSM_Deleted:
    x56c_collisionManager->Destroy(mgr);
    if (CScriptTrigger* trigger = TCastToPtr< CScriptTrigger >(mgr.ObjectById(x58c_triggerId))) {
      trigger->SetForceField(x580_forceVector);
    }
    break;
  case kSM_Touched:
    if (const CCollisionActor* colActor =
            TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(uid))) {
      TUniqueId touched = colActor->GetLastTouchedObject();
      if (touched == mgr.GetPlayer()->GetUniqueId() && x420_curDamageRemTime <= 0.f) {
        mgr.ApplyDamage(
            GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), GetContactDamage(),
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
        x420_curDamageRemTime = x424_damageWaitTime;
      }
    }
    break;
  case kSM_Play:
    x578_ = 0.04f;
    break;
  case kSM_Deactivate:
    DeathDelete(mgr);
    break;
  case kSM_Open:
    ExtractTentacle(mgr);
    break;
  case kSM_Close:
    RetractTentacle(mgr);
    break;
  case kSM_InitializedInArea:
    SaveBombSlotInfo(mgr);
    break;
  }
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
}

CVector3f CFlaahgraTentacle::GetAimPosition(const CStateManager& mgr, float dt) const {
  if (const CCollisionActor* colActor =
          TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(x57c_tentacleTipAct))) {
    return colActor->GetTranslation();
  }
  return CPatterned::GetAimPosition(mgr, dt);
}

void CFlaahgraTentacle::Death(CStateManager& mgr, const CVector3f& direction,
                              EScriptObjectState state) {
  if (IsAlive()) {
    StateMachineState().SetState(mgr, *this, GetStateMachine(), rstl::string_l("Dead"));
    if (x450_bodyController->GetPercentageFrozen() > 0.f) {
      x450_bodyController->UnFreeze();
    }
    x400_25_alive = false;
  }
}

bool CFlaahgraTentacle::Inside(CStateManager& mgr, float arg) {
  return x450_bodyController->GetLocomotionType() == pas::kLT_Crouch;
}

bool CFlaahgraTentacle::ShouldAttack(CStateManager& mgr, float arg) {
  if (x578_ > 0.f) {
    return true;
  }
  if (x574_ <= 0.f && !mgr.GetPlayer()->IsInsideFluid()) {
    if (const CCollisionActor* colActor =
            TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(x57c_tentacleTipAct))) {
      CVector3f delta3 = mgr.GetPlayer()->GetTranslation() - colActor->GetTranslation();
      CVector2f delta = delta3.DropZ();
      float mag = delta.MagSquared();
      float minRangeSq = x2fc_minAttackRange * x2fc_minAttackRange;
      float maxRangeSq = x300_maxAttackRange * x300_maxAttackRange;
      return mag >= minRangeSq && mag <= maxRangeSq;
    }
  }
  return false;
}

bool CFlaahgraTentacle::AnimOver(CStateManager& mgr, float arg) { return x568_ == 3; }

void CFlaahgraTentacle::InActive(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x570_ = 0.f;
    break;
  case kStateMsg_Update:
    if (!Inside(mgr, 0.f)) {
      if (CScriptTrigger* trigger = TCastToPtr< CScriptTrigger >(mgr.ObjectById(x58c_triggerId))) {
        if (trigger->GetPlayerInside()) {
          if (x570_ > 1.f) {
            RetractTentacle(mgr);
            ExtractTentacle(mgr);
          } else {
            x570_ += arg;
          }
        }
      }
    }
    break;
  }
}

void CFlaahgraTentacle::Retreat(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Update:
    if (x58e_24_ && GetStateMachineTime() > 1.f) {
      if (CScriptTrigger* trigger = TCastToPtr< CScriptTrigger >(mgr.ObjectById(x58c_triggerId))) {
        if (!trigger->GetPlayerInside()) {
          x450_bodyController->SetLocomotionType(pas::kLT_Relaxed);
        }
      }
    }
    break;
  case kStateMsg_Deactivate:
    x58e_24_ = false;
    break;
  }
}

void CFlaahgraTentacle::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x568_ = 0;
    break;
  case kStateMsg_Update:
    switch (x568_) {
    case 0:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_MeleeAttack) {
        x568_ = 2;
      } else {
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCMeleeAttackCmd(x578_ > 0.f ? pas::kS_Zero : pas::kS_One));
      }
      break;
    case 2:
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_MeleeAttack) {
        x568_ = 3;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    x574_ = x308_attackTimeVariation * mgr.Random()->Float() + x304_averageAttackTime;
    x578_ = 0.f;
    break;
  }
}

void CFlaahgraTentacle::Dead(CStateManager& mgr, EStateMsg msg, float arg) {
  x450_bodyController->SetLocomotionType(pas::kLT_Crouch);
}

void CFlaahgraTentacle::SetupCollisionManager(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > jointList;
  jointList.reserve(3);
  AddSphereCollisionList(skJointList, 3, jointList);
  x56c_collisionManager =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), jointList, true);
  for (uint i = 0; i < x56c_collisionManager->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = x56c_collisionManager->GetCollisionDescFromIndex(i);
    TUniqueId uid = desc.GetCollisionActorId();
    if (CCollisionActor* const colActor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      colActor->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
          CMaterialList(kMT_Player),
          CMaterialList(kMT_Character, kMT_CollisionActor, kMT_NoStaticCollision,
                        kMT_NoPlatformCollision)));
      colActor->AddMaterial(kMT_ScanPassthrough, mgr);
      colActor->SetDamageVulnerability(
          *static_cast< const CFlaahgraTentacle* >(this)->GetDamageVulnerability());
      if (x57c_tentacleTipAct == kInvalidUniqueId &&
          desc.GetName() == rstl::string_l(skpTentacleTip)) {
        x57c_tentacleTipAct = uid;
      }
    }
  }
  RemoveMaterial(kMT_Solid, kMT_Target, kMT_Orbit, mgr);
  AddMaterial(kMT_Scannable, mgr);
}

void CFlaahgraTentacle::AddSphereCollisionList(
    const SSphereJointInfo* sphereJoints, int jointCount,
    rstl::vector< CJointCollisionDescription >& outJoints) {
  const CAnimData& animData = *GetModelData()->GetAnimationData();
  for (int i = 0; i < jointCount; ++i) {
    CSegId segId = animData.GetLocatorSegId(rstl::string_l(sphereJoints[i].name));
    if (segId != CSegId::Invalid()) {
      CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
          segId, sphereJoints[i].radius, rstl::string_l(sphereJoints[i].name), 10.f);
      outJoints.push_back(desc);
    }
  }
}

void CFlaahgraTentacle::ExtractTentacle(CStateManager& mgr) {
  if (Inside(mgr, 0.f)) {
    x58e_24_ = true;
    if (CScriptTrigger* trigger = TCastToPtr< CScriptTrigger >(mgr.ObjectById(x58c_triggerId))) {
      trigger->SetForceField(x580_forceVector);
    }
  }
}

void CFlaahgraTentacle::RetractTentacle(CStateManager& mgr) {
  x450_bodyController->SetLocomotionType(pas::kLT_Crouch);
  if (CScriptTrigger* trigger = TCastToPtr< CScriptTrigger >(mgr.ObjectById(x58c_triggerId))) {
    trigger->SetForceField(CVector3f::Zero());
  }
}

void CFlaahgraTentacle::SaveBombSlotInfo(CStateManager& mgr) {
  const rstl::vector< SConnection >& connections = GetConnectionList();
  for (AUTO(it, connections.begin()); it != connections.end(); ++it) {
    if (it->x0_state == kSS_Modify && it->x4_msg == kSM_ToggleActive) {
      TUniqueId uid = mgr.GetIdForScript(it->x8_objId);
      if (const CScriptTrigger* trigger =
              TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(uid))) {
        x58c_triggerId = uid;
        x580_forceVector = trigger->GetForceField();
        return;
      }
    }
  }
}
