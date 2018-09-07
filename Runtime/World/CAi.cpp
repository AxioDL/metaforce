#include "CAi.hpp"
#include "Character/CModelData.hpp"
#include "CStateManager.hpp"
#include "CStateMachine.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"

namespace urde
{

static CMaterialList MakeAiMaterialList(const CMaterialList& in)
{
    CMaterialList ret = in;
    ret.Add(EMaterialTypes::AIBlock);
    ret.Add(EMaterialTypes::CameraPassthrough);
    return ret;
}

CAi::CAi(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
         CModelData&& mData, const zeus::CAABox& box, float mass, const CHealthInfo& hInfo,
         const CDamageVulnerability& dmgVuln, const CMaterialList& list, CAssetId fsm, const CActorParameters& actorParams,
         float stepUp, float stepDown)
: CPhysicsActor(uid, active, name, info, xf, std::move(mData), MakeAiMaterialList(list), box, SMoverData(mass),
                actorParams, stepUp, stepDown)
, x258_healthInfo(hInfo)
, x260_damageVulnerability(dmgVuln)
, x2c8_stateMachine(g_SimplePool->GetObj({FOURCC('AFSM'), fsm}))
{
    _CreateShadow();

    if (x94_simpleShadow)
    {
        CreateShadow(true);
        x94_simpleShadow->SetAlwaysCalculateRadius(false);
    }

    if (x90_actorLights)
        x260_damageVulnerability.SetX38_25(true);
}

void CAi::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    if (msg == EScriptObjectMessage::InitializedInArea)
    {
        CMaterialList exclude = GetMaterialFilter().GetExcludeList();
        CMaterialList include = GetMaterialFilter().GetIncludeList();
        include.Add(EMaterialTypes::AIBlock);
        SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
    }

    CActor::AcceptScriptMsg(msg, uid, mgr);
}

CAiStateFunc CAi::GetStateFunc(const char* func) { return m_FuncMap->GetStateFunc(func); }

CAiTriggerFunc CAi::GetTrigerFunc(const char* func) { return m_FuncMap->GetTriggerFunc(func); }

const CStateMachine* CAi::GetStateMachine() const { return x2c8_stateMachine.GetObj(); }
void CAi::CreateFuncLookup(CAiFuncMap* funcMap) { m_FuncMap = funcMap; }
CAiFuncMap* CAi::m_FuncMap = nullptr;
}
