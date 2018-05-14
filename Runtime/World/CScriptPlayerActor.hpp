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
    CPlayerState::EBeamId x30c_setBeamId = CPlayerState::EBeamId::Invalid;
    s32 x310_loadedCharIdx = -1;
    std::unique_ptr<CModelData> x314_beamModelData;
    std::unique_ptr<CModelData> x318_suitModelData;
    TLockedToken<CModel> x31c_beamModel; // Used to be single_ptr
    TLockedToken<CModel> x320_suitModel; // Used to be single_ptr
    TLockedToken<CSkinRules> x324_suitSkin; // Used to be single_ptr
    TLockedToken<CSkinnedModel> x328_backupModelData; // Used to be optional
    TLockedToken<CTexture> x338_phazonIndirectTexture; // Used to be optional
    u32 x348_deallocateBackupCountdown = 0;
    float x34c_phazonOffsetAngle = 0.f;
    u32 x350_flags; /* 0x1: suit transition, 0x2: previous suit, 0x4: force reset
                     * 0x8: track in area data, 0x10: keep in state manager */
    union
    {
        struct
        {
            bool x354_24_setBoundingBox : 1;
            bool x354_25_deferOnlineModelData : 1;
            bool x354_26_deferOfflineModelData : 1;
            bool x354_27_beamModelLoading : 1;
            bool x354_28_suitModelLoading : 1;
            bool x354_29_loading : 1;
            bool x354_30_enableLoading : 1;
            bool x354_31_deferOnlineLoad : 1;
            bool x355_24_areaTrackingLoad : 1;
        };
        u32 _dummy = 0;
    };
    TUniqueId x356_nextPlayerActor = kInvalidUniqueId;

    u32 GetSuitCharIdx(const CStateManager& mgr, CPlayerState::EPlayerSuit suit) const;
    u32 GetNextSuitCharIdx(const CStateManager& mgr) const;
    void LoadSuit(u32 charIdx);
    void LoadBeam(CPlayerState::EBeamId beam);
    void PumpBeamModel(CStateManager& mgr);
    void BuildBeamModelData();
    void PumpSuitModel(CStateManager& mgr);
    void SetupOfflineModelData();
    void SetupOnlineModelData();

    void SetupEnvFx(CStateManager& mgr, bool set);
    void SetIntoStateManager(CStateManager& mgr, bool set);

    void TouchModels_Internal(const CStateManager& mgr) const;

public:
    CScriptPlayerActor(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                       const zeus::CTransform& xf, const CAnimRes& animRes, CModelData&& mData,
                       const zeus::CAABox& aabox, bool setBoundingBox, const CMaterialList& list, float mass,
                       float zMomentum, const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                       const CActorParameters& aParams, bool loop, bool active, u32 flags, CPlayerState::EBeamId beam);

    void Think(float, CStateManager &);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager &);
    void SetActive(bool active);
    void PreRender(CStateManager &, const zeus::CFrustum &);
    void AddToRenderer(const zeus::CFrustum &, const CStateManager &) const;
    void Render(const CStateManager &mgr) const;
    void TouchModels(const CStateManager& mgr) const;
};
}

#endif // __URDE_CSCRIPTPLAYERACTOR_HPP__
