#include "CAi.hpp"
#include "Character/CModelData.hpp"
#include "CStateManager.hpp"
#include "CStateMachine.hpp"

namespace urde
{

static CMaterialList MakeAiMaterialList(const CMaterialList& in)
{
    CMaterialList ret = in;
    ret.Add(EMaterialTypes::FourtyEight);
    ret.Add(EMaterialTypes::TwentyOne);
    return ret;
}

CAi::CAi(TUniqueId uid, bool active, const std::string& name, const CEntityInfo& info, const zeus::CTransform& xf,
         CModelData&& mData, const zeus::CAABox& box, float mass, const CHealthInfo& hInfo,
         const CDamageVulnerability& dmgVuln, const CMaterialList& list, ResId, const CActorParameters& actorParams,
         float stepUp, float stepDown)
    : CPhysicsActor(uid, active, name, info, xf, std::move(mData), MakeAiMaterialList(list), box, SMoverData(mass),
                    actorParams, stepUp, stepDown),
      x258_healthInfo(hInfo),
      x260_damageVulnerability(dmgVuln)
{
}

CAiStateFunc CAi::GetStateFunc(const char* func)
{
    return m_FuncMap->GetStateFunc(func);
}

CAiTriggerFunc CAi::GetTrigerFunc(const char* func)
{
    return m_FuncMap->GetTriggerFunc(func);
}

const CStateMachine*CAi::GetStateMachine() const { return x2c8_stateMachine.GetObj(); }
void CAi::CreateFuncLookup(CAiFuncMap* funcMap)
{
    m_FuncMap = funcMap;
}
CAiFuncMap* CAi::m_FuncMap = nullptr;

}
