#include "MetroidPrime/Enemies/CPhazonHealingNodule.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"

static EMaterialTypes skCharacterMaterial = kMT_Character;

CPhazonHealingNodule::CPhazonHealingNodule(TUniqueId uid, const rstl::string& name,
                                           const CEntityInfo& info, const CTransform4f& xf,
                                           const CModelData& mData,
                                           const CActorParameters& actParams,
                                           const CPatternedInfo& pInfo, CAssetId particleDescId,
                                           const rstl::string& actorLctr)
: CPatterned(kC_PhazonHealingNodule, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Flyer,
             kCT_One, kBT_Restricted, actParams, kCS_Medium)
, mActive(0)
, mEmitting(false)
, mConnId(kInvalidUniqueId)
, mElectricDesc(gpSimplePool->GetObj(SObjectTag('ELSC', particleDescId)))
, mParticleElectric(nullptr)
, mInitialHealthInfo(pInfo.GetHealthInfo())
, mActorLctr(actorLctr) {
  CMaterialList exclude = GetMaterialFilter().GetExcludeList();
  exclude.Add(skCharacterMaterial);
  SetMaterialFilter(
      CMaterialFilter::MakeIncludeExclude(GetMaterialFilter().GetIncludeList(), exclude));
}

void CPhazonHealingNodule::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                           CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered:
    if (!BodyCtrl()->GetIsActive()) {
      BodyCtrl()->Activate(mgr);
    }
    *GetDamageVulnerability() = CDamageVulnerability::ImmuneVulnerability();
    KnockBackCtrl().SetAutoResetImpulse(false);
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    RemoveMaterial(kMT_Target, mgr);
    RemoveMaterial(kMT_Orbit, mgr);
    AddMaterial(kMT_Immovable, mgr);
    break;
  case kSM_InitializedInArea: {
    CPatterned::AcceptScriptMsg(msg, uid, mgr);
    const uint count = GetConnectionList().size();
    for (uint i = 0; i < count; ++i) {
      const SConnection& conn = GetConnectionList()[i];
      const TUniqueId connId = mgr.GetIdForScript(conn.mObjId);
      if (conn.mState == kSS_Patrol && connId != kInvalidUniqueId &&
          conn.mMsg == kSM_Activate) {
        mConnId = connId;
      }
    }
    break;
  }
  case kSM_Increment:
    mActive = 1;
    break;
  case kSM_Decrement:
    mActive = 0;
    mParticleElectric.reset();
    mEmitting = false;
    break;
  case kSM_Reset:
    *HealthInfo(mgr) = mInitialHealthInfo;
    break;
  default:
    CPatterned::AcceptScriptMsg(msg, uid, mgr);
    break;
  }
}

void CPhazonHealingNodule::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                           EUserEventType type, float dt) {
  switch (type) {
  case kUE_BeginAction:
    mEmitting = true;
    mParticleElectric =
        rstl::rc_ptr< CParticleElectric >(rs_new CParticleElectric(mElectricDesc));
    mParticleElectric->SetParticleEmission(true);
    break;
  case kUE_EndAction:
    mEmitting = false;
    mParticleElectric.reset();
    break;
  default:
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
    break;
  }
}

void CPhazonHealingNodule::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  CPatterned::Think(dt, mgr);
  if (HealthInfo(mgr)->GetHP() <= 0.f) {
    mParticleElectric.reset();
    mEmitting = false;
    StateMachineState().SetState(mgr, *this, GetStateMachine(), rstl::string_l("Patrol"));
    mActive = 0;
  }
  if (mParticleElectric) {
    UpdateParticleElectric(mgr);
    mParticleElectric->Update(dt);
  }
}

void CPhazonHealingNodule::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    RemoveMaterial(kMT_Target, mgr);
    RemoveMaterial(kMT_Orbit, mgr);
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CPhazonHealingNodule::Lurk(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

bool CPhazonHealingNodule::InDetectionRange(CStateManager&, float) { return mActive == 1; }

bool CPhazonHealingNodule::InRange(CStateManager&, float) { return mActive == 0; }

void CPhazonHealingNodule::Render(const CStateManager& mgr) const {
  if (mParticleElectric) {
    mParticleElectric->Render();
  }
  CPatterned::Render(mgr);
}

void CPhazonHealingNodule::Growth(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = 0;
    BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
    break;
  case kStateMsg_Update:
    switch (mState) {
    case 0:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_Step) {
        mState = 2;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Forward, pas::kStep_Normal));
      }
      break;
    case 2:
      if (BodyCtrl()->GetCurrentStateId() != pas::kAS_Step) {
        mState = 3;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
  default:
    break;
  }
}

void CPhazonHealingNodule::Faint(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    mState = 0;
    break;
  case kStateMsg_Update:
    switch (mState) {
    case 0:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_Step) {
        mState = 2;
        BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Backward, pas::kStep_Normal));
      }
      break;
    case 2:
      if (BodyCtrl()->GetCurrentStateId() != pas::kAS_Step) {
        mState = 3;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
  default:
    break;
  }
}

bool CPhazonHealingNodule::AnimOver(CStateManager&, float) { return mState == 3; }

void CPhazonHealingNodule::UpdateParticleElectric(CStateManager& mgr) {
  if (mParticleElectric) {
    if (const CPatterned* actor =
            static_cast< const CPatterned* >(mgr.GetObjectById(mConnId))) {
      const CVector3f start = GetLctrTransform(rstl::string_l("Electricity_LCTR")).GetTranslation();
      const CVector3f end = actor->GetLctrTransform(mActorLctr).GetTranslation();
      mParticleElectric->SetOverrideIPos(start);
      mParticleElectric->SetOverrideFPos(end);
    }
  }
}

void CPhazonHealingNodule::Death(CStateManager& mgr, const CVector3f&, EScriptObjectState) {
  SendScriptMsgs(kSS_Dead, mgr, kSM_None);
  SendScriptMsgs(kSS_DeathRattle, mgr, kSM_None);
}

void CPhazonHealingNodule::MassiveDeath(CStateManager& mgr) {
  Death(mgr, CVector3f::Zero(), kSS_Dead);
}

void CPhazonHealingNodule::MassiveFrozenDeath(CStateManager& mgr) {
  Death(mgr, CVector3f::Zero(), kSS_Dead);
}

void CPhazonHealingNodule::PhazeOut(CStateManager& mgr) { Death(mgr, CVector3f::Zero(), kSS_Dead); }

void CPhazonHealingNodule::KnockBack(const CVector3f&, CStateManager&, const CDamageInfo&, float,
                                     bool, const bool) {}

CPhazonHealingNodule::~CPhazonHealingNodule() {}
