#include "World/CScriptPlayerActor.hpp"
#include "World/CActorParameters.hpp"
#include "World/CLightParameters.hpp"
#include "GameGlobalObjects.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "CStateManager.hpp"
#include "Character/CAssetFactory.hpp"
#include "Character/CCharacterFactory.hpp"

namespace urde
{
CScriptPlayerActor::CScriptPlayerActor(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                       const zeus::CTransform& xf, const CAnimRes& animRes, CModelData&& mData,
                                       const zeus::CAABox& aabox, bool b1, const CMaterialList& list, float mass,
                                       float zMomentum, const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                                       const CActorParameters& aParams, bool loop, bool active, u32 flags,
                                       CPlayerState::EBeamId beam)
: CScriptActor(uid, name, info, xf, std::move(mData), aabox, mass, zMomentum, list, hInfo, dVuln, aParams, loop, active,
               0, 1.f, false, false, false, false)
, x2e8_suitRes(animRes)
, x304_beam(beam)
, x350_flags(flags)
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

u32 CScriptPlayerActor::GetSuitCharIdx(const CStateManager& mgr, CPlayerState::EPlayerSuit suit) const
{
    if (mgr.GetPlayerState()->IsFusionEnabled())
    {
        switch (suit)
        {
        case CPlayerState::EPlayerSuit::Power:
            return 4;
        case CPlayerState::EPlayerSuit::Varia:
            return 7;
        case CPlayerState::EPlayerSuit::Gravity:
            return 6;
        case CPlayerState::EPlayerSuit::Phazon:
            return 8;
        default:
            break;
        }
    }
    return u32(suit);
}

void CScriptPlayerActor::LoadSuit(u32 charIdx)
{
    if (charIdx != x310_loadedCharIdx)
    {
        TToken<CCharacterFactory> fac = g_CharFactoryBuilder->GetFactory(x2e8_suitRes);
        const CCharacterInfo& chInfo = fac->GetCharInfo(charIdx);
        x324_suitModel = g_SimplePool->GetObj({FOURCC('CMDL'), chInfo.GetModelId()});
        x354_28_ = true;
        x310_loadedCharIdx = charIdx;
    }
}

void CScriptPlayerActor::LoadBeam(CPlayerState::EBeamId beam)
{

}

void CScriptPlayerActor::Think(float dt, CStateManager& mgr)
{
    auto& pState = *mgr.GetPlayerState();

    if (x354_31_)
    {
        x354_25_ = true;
        x354_31_ = false;
        x308_suit = pState.GetCurrentSuitRaw();
        LoadSuit(GetSuitCharIdx(mgr, x308_suit));
    }

    if (x354_30_)
    {
        if (!(x350_flags & 0x1))
        {
            u32 tmpIdx = GetSuitCharIdx(mgr, pState.GetCurrentSuitRaw());
            if (tmpIdx != x310_loadedCharIdx)
            {
                SetModelData(std::make_unique<CModelData>(CModelData::CModelDataNull()));
                LoadSuit(tmpIdx);
                x354_25_ = true;
            }
        }

        LoadBeam(x304_beam != CPlayerState::EBeamId::Invalid ? x304_beam : pState.GetCurrentBeam());
    }
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
    if (x2e8_suitRes.GetCharacterNodeId() == 3)
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
