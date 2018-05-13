#ifndef __URDE_CSCRIPTPLAYERACTOR_HPP__
#define __URDE_CSCRIPTPLAYERACTOR_HPP__

#include "CScriptActor.hpp"
#include "CPlayerState.hpp"

namespace urde
{
class CScriptPlayerActor : public CScriptActor
{
    CAnimRes x2e8_suitRes;
    CPlayerState::EBeamId x304_beam;
    CPlayerState::EPlayerSuit x308_suit = CPlayerState::EPlayerSuit::Invalid;
    s32 x30c_ = -1;
    s32 x310_loadedCharIdx = -1;
    u32 x314_ = 0;
    u32 x318_ = 0;
    u32 x31c_ = 0;
    u32 x320_ = 0;
    TLockedToken<CModel> x324_suitModel;
    u8 x334_ = 0;
    u8 x344_ = 0;
    u32 x348_ = 0;
    float x34c_ = 0.f;
    u32 x350_flags;
    union
    {
        struct
        {
            bool x354_24_ : 1;
            bool x354_25_ : 1;
            bool x354_26_ : 1;
            bool x354_27_ : 1;
            bool x354_28_ : 1;
            bool x354_29_ : 1;
            bool x354_30_ : 1;
            bool x354_31_ : 1;
            bool x355_24_ : 1;
        };
        u32 _dummy = 0;
    };

    u32 GetSuitCharIdx(const CStateManager& mgr, CPlayerState::EPlayerSuit suit) const;
    void LoadSuit(u32 charIdx);
    void LoadBeam(CPlayerState::EBeamId beam);

public:
    CScriptPlayerActor(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                       const zeus::CTransform& xf, const CAnimRes& animRes, CModelData&& mData,
                       const zeus::CAABox& aabox, bool b1, const CMaterialList& list, float mass,
                       float zMomentum, const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                       const CActorParameters& aParams, bool loop, bool active, u32 flags, CPlayerState::EBeamId beam);

    void Think(float, CStateManager &);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager &);
    void SetActive(bool active);
    void PreRender(CStateManager &, const zeus::CFrustum &);
    void AddToRenderer(const zeus::CFrustum &, const CStateManager &) const;
    void Render(const CStateManager &mgr) const;
    void TouchModels();

};
}

#endif // __URDE_CSCRIPTPLAYERACTOR_HPP__
