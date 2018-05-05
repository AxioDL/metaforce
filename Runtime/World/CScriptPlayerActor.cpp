#include "World/CScriptPlayerActor.hpp"
#include "World/CActorParameters.hpp"
#include "World/CLightParameters.hpp"
#include "GameGlobalObjects.hpp"
#include "Graphics/CBooRenderer.hpp"

namespace urde
{
CScriptPlayerActor::CScriptPlayerActor(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                       const zeus::CTransform& xf, const CAnimRes& animRes, CModelData&& mData,
                                       const zeus::CAABox& aabox, bool b1, const CMaterialList& list, float mass,
                                       float zMomentum, const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                                       const CActorParameters& aParams, bool loop, bool active, u32 w1, u32 w2)
: CScriptActor(uid, name, info, xf, std::move(mData), aabox, mass, zMomentum, list, hInfo, dVuln, aParams, loop, active,
               0, 1.f, false, false, false, false)
, x2e8_(animRes)
, x304_(w2)
, x350_(w1)
{
    x354_24_ = b1;
    x354_29_ = true;
    x354_30_ = true;
    CMaterialList exclude = GetMaterialFilter().GetExcludeList();
    CMaterialList include = GetMaterialFilter().GetIncludeList();
    include.Add(EMaterialTypes::Player);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));

    SetActorLights(aParams.GetLightParameters().MakeActorLights());
    xe7_29_actorActive = true;
    x2e3_24_cameraMoveIntoAlpha = true;
}

void CScriptPlayerActor::Think(float, CStateManager&)
{

}

void CScriptPlayerActor::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
}

void CScriptPlayerActor::SetActive(bool active)
{
    CActor::SetActive(active);
    xe7_29_actorActive = true;
}

void CScriptPlayerActor::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum)
{
    if (x2e8_.GetCharacterNodeId() == 3)
        g_Renderer->AllocatePhazonSuitMaskTexture();
    CScriptActor::PreRender(mgr, frustum);
}

void CScriptPlayerActor::AddToRenderer(const zeus::CFrustum&, const CStateManager&) const
{

}

void CScriptPlayerActor::Render(const CStateManager& mgr) const
{
}

void CScriptPlayerActor::TouchModels()
{

}
}
