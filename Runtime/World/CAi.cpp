#include "Runtime/World/CAi.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Character/CModelData.hpp"
#include "Runtime/World/CScriptWater.hpp"
#include "Runtime/World/CStateMachine.hpp"

namespace urde {

static CMaterialList MakeAiMaterialList(const CMaterialList& in) {
  CMaterialList ret = in;
  ret.Add(EMaterialTypes::AIBlock);
  ret.Add(EMaterialTypes::CameraPassthrough);
  return ret;
}

CAi::CAi(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
         CModelData&& mData, const zeus::CAABox& box, float mass, const CHealthInfo& hInfo,
         const CDamageVulnerability& dmgVuln, const CMaterialList& list, CAssetId fsm,
         const CActorParameters& actorParams, float stepUp, float stepDown)
: CPhysicsActor(uid, active, name, info, xf, std::move(mData), MakeAiMaterialList(list), box, SMoverData(mass),
                actorParams, stepUp, stepDown)
, x258_healthInfo(hInfo)
, x260_damageVulnerability(dmgVuln)
, x2c8_stateMachine(g_SimplePool->GetObj({FOURCC('AFSM'), fsm})) {
  _CreateShadow();

  if (x94_simpleShadow) {
    CreateShadow(true);
    x94_simpleShadow->SetAlwaysCalculateRadius(false);
  }

  if (x90_actorLights)
    x90_actorLights->SetCastShadows(true);
}

void CAi::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::InitializedInArea) {
    CMaterialList exclude = GetMaterialFilter().GetExcludeList();
    CMaterialList include = GetMaterialFilter().GetIncludeList();
    include.Add(EMaterialTypes::AIBlock);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
  }

  CActor::AcceptScriptMsg(msg, uid, mgr);
}

EWeaponCollisionResponseTypes CAi::GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                            const urde::CWeaponMode&, urde::EProjectileAttrib) const {
  return EWeaponCollisionResponseTypes::EnemyNormal;
}

void CAi::FluidFXThink(EFluidState state, CScriptWater& water, urde::CStateManager& mgr) {
  if (state == EFluidState::EnteredFluid || state == EFluidState::LeftFluid) {
    float dt = mgr.GetFluidPlaneManager()->GetLastSplashDeltaTime(GetUniqueId());
    if (dt >= 0.02f) {
      float vel = (0.5f * GetMass()) * GetVelocity().magSquared();

      if (vel > 500.f) {
        zeus::CVector3f pos = x34_transform.origin;
        pos.z() = float(water.GetTriggerBoundsWR().max.z());
        mgr.GetFluidPlaneManager()->CreateSplash(GetUniqueId(), mgr, water, pos,
                                                 0.1f + ((0.4f * zeus::min(vel, 30000.f) - 500.f) / 29500.f), true);
      }
    }
  }

  if (mgr.GetFluidPlaneManager()->GetLastRippleDeltaTime(GetUniqueId()) <
      (GetHealthInfo(mgr)->GetHP() > 0.f ? 0.2f : 0.7f))
    return;

  zeus::CVector3f pos = x34_transform.origin;
  zeus::CVector3f center = pos;
  center.z() = float(water.GetTriggerBoundsWR().max.z());
  pos.normalize();
  water.GetFluidPlane().AddRipple(GetMass(), GetUniqueId(), center, GetVelocity(), water, mgr, pos);
}

CAiStateFunc CAi::GetStateFunc(const char* func) { return m_FuncMap->GetStateFunc(func); }

CAiTriggerFunc CAi::GetTrigerFunc(const char* func) { return m_FuncMap->GetTriggerFunc(func); }

const CStateMachine* CAi::GetStateMachine() const { return x2c8_stateMachine.GetObj(); }
void CAi::CreateFuncLookup(CAiFuncMap* funcMap) { m_FuncMap = funcMap; }
CAiFuncMap* CAi::m_FuncMap = nullptr;
} // namespace urde
