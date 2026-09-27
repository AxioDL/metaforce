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
, mCollisionManager(nullptr)
, x570_(0.f)
, x574_(0.f)
, x578_(0.f)
, mTentacleTipAct(kInvalidUniqueId)
, mForceVector(CVector3f::Zero())
, mTriggerId(kInvalidUniqueId)
, x58e_24_(false) {
  ActorLights()->SetCastShadows(false);
  mKnockBackController.SetAutoResetImpulse(false);
  SetDrawShadow(false);
}

CFlaahgraTentacle::~CFlaahgraTentacle() {}

void CFlaahgraTentacle::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    CPatterned::Think(dt, mgr);
    mCollisionManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
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
    mBodyController->Activate(mgr);
    SetupCollisionManager(mgr);
    break;
  case kSM_Deleted:
    mCollisionManager->Destroy(mgr);
    if (CScriptTrigger* trigger = TCastToPtr< CScriptTrigger >(mgr.ObjectById(mTriggerId))) {
      trigger->SetForceField(mForceVector);
    }
    break;
  case kSM_Touched:
    if (const CCollisionActor* colActor =
            TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(uid))) {
      TUniqueId touched = colActor->GetLastTouchedObject();
      if (touched == mgr.GetPlayer()->GetUniqueId() && mCurDamageRemTime <= 0.f) {
        mgr.ApplyDamage(
            GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), GetContactDamage(),
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
        mCurDamageRemTime = mDamageWaitTime;
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
          TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(mTentacleTipAct))) {
    return colActor->GetTranslation();
  }
  return CPatterned::GetAimPosition(mgr, dt);
}

void CFlaahgraTentacle::Death(CStateManager& mgr, const CVector3f& direction,
                              EScriptObjectState state) {
  if (IsAlive()) {
    StateMachineState().SetState(mgr, *this, GetStateMachine(), rstl::string_l("Dead"));
    if (mBodyController->GetPercentageFrozen() > 0.f) {
      mBodyController->UnFreeze();
    }
    mAlive = false;
  }
}

bool CFlaahgraTentacle::Inside(CStateManager& mgr, float arg) {
  return mBodyController->GetLocomotionType() == pas::kLT_Crouch;
}

bool CFlaahgraTentacle::ShouldAttack(CStateManager& mgr, float arg) {
  if (x578_ > 0.f) {
    return true;
  }
  if (x574_ <= 0.f && !mgr.GetPlayer()->IsInsideFluid()) {
    if (const CCollisionActor* colActor =
            TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(mTentacleTipAct))) {
      CVector3f delta3 = mgr.GetPlayer()->GetTranslation() - colActor->GetTranslation();
      CVector2f delta = delta3.DropZ();
      float mag = delta.MagSquared();
      float minRangeSq = mMinAttackRange * mMinAttackRange;
      float maxRangeSq = mMaxAttackRange * mMaxAttackRange;
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
      if (CScriptTrigger* trigger = TCastToPtr< CScriptTrigger >(mgr.ObjectById(mTriggerId))) {
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
      if (CScriptTrigger* trigger = TCastToPtr< CScriptTrigger >(mgr.ObjectById(mTriggerId))) {
        if (!trigger->GetPlayerInside()) {
          mBodyController->SetLocomotionType(pas::kLT_Relaxed);
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
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() == pas::kAS_MeleeAttack) {
        x568_ = 2;
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCMeleeAttackCmd(x578_ > 0.f ? pas::kS_Zero : pas::kS_One));
      }
      break;
    case 2:
      if (mBodyController->GetBodyStateInfo().GetCurrentStateId() != pas::kAS_MeleeAttack) {
        x568_ = 3;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    x574_ = mAttackTimeVariation * mgr.Random()->Float() + mAverageAttackTime;
    x578_ = 0.f;
    break;
  }
}

void CFlaahgraTentacle::Dead(CStateManager& mgr, EStateMsg msg, float arg) {
  mBodyController->SetLocomotionType(pas::kLT_Crouch);
}

void CFlaahgraTentacle::SetupCollisionManager(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > jointList;
  jointList.reserve(3);
  AddSphereCollisionList(skJointList, 3, jointList);
  mCollisionManager =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), jointList, true);
  for (uint i = 0; i < mCollisionManager->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = mCollisionManager->GetCollisionDescFromIndex(i);
    TUniqueId uid = desc.GetCollisionActorId();
    if (CCollisionActor* const colActor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      colActor->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
          CMaterialList(kMT_Player),
          CMaterialList(kMT_Character, kMT_CollisionActor, kMT_NoStaticCollision,
                        kMT_NoPlatformCollision)));
      colActor->AddMaterial(kMT_ScanPassthrough, mgr);
      colActor->SetDamageVulnerability(
          *static_cast< const CFlaahgraTentacle* >(this)->GetDamageVulnerability());
      if (mTentacleTipAct == kInvalidUniqueId &&
          desc.GetName() == rstl::string_l(skpTentacleTip)) {
        mTentacleTipAct = uid;
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
    if (CScriptTrigger* trigger = TCastToPtr< CScriptTrigger >(mgr.ObjectById(mTriggerId))) {
      trigger->SetForceField(mForceVector);
    }
  }
}

void CFlaahgraTentacle::RetractTentacle(CStateManager& mgr) {
  mBodyController->SetLocomotionType(pas::kLT_Crouch);
  if (CScriptTrigger* trigger = TCastToPtr< CScriptTrigger >(mgr.ObjectById(mTriggerId))) {
    trigger->SetForceField(CVector3f::Zero());
  }
}

void CFlaahgraTentacle::SaveBombSlotInfo(CStateManager& mgr) {
  const rstl::vector< SConnection >& connections = GetConnectionList();
  for (AUTO(it, connections.begin()); it != connections.end(); ++it) {
    if (it->mState == kSS_Modify && it->mMsg == kSM_ToggleActive) {
      TUniqueId uid = mgr.GetIdForScript(it->mObjId);
      if (const CScriptTrigger* trigger =
              TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(uid))) {
        mTriggerId = uid;
        mForceVector = trigger->GetForceField();
        return;
      }
    }
  }
}
