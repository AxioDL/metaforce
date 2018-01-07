#ifndef __URDE_CPLAYERVISOR_HPP__
#define __URDE_CPLAYERVISOR_HPP__

#include "RetroTypes.hpp"
#include "Camera/CCameraFilter.hpp"
#include "zeus/CVector2f.hpp"
#include "Audio/CSfxManager.hpp"
#include "CPlayerState.hpp"
#include "Graphics/CModel.hpp"
#include "Graphics/Shaders/CColoredQuadFilter.hpp"
#include "Graphics/Shaders/CTexturedQuadFilter.hpp"

namespace urde
{
class CStateManager;
class CTargetingManager;

namespace MP1
{

class CPlayerVisor
{
    struct SScanTarget
    {
        TUniqueId x0_objId = kInvalidUniqueId;
        float x4_timer = 0.f;
        float x8_inRangeTimer = 0.f;
        bool xc_inBox = false;
    };

    enum class EScanWindowState
    {
        NotInScanVisor,
        Idle,
        Scan
    };

    rstl::reserved_vector<zeus::CVector2f, 3> x0_scanWindowSizes;
    CPlayerState::EPlayerVisor x1c_curVisor = CPlayerState::EPlayerVisor::Combat;
    CPlayerState::EPlayerVisor x20_nextVisor = CPlayerState::EPlayerVisor::Combat;
    float x24_visorSfxVol = 1.f; // used to be u8
    bool x25_24_visorTransitioning : 1;
    bool x25_25_ : 1;
    float x28_ = 0.f;
    float x2c_scanDimInterp = 1.f;
    EScanWindowState x30_prevState = EScanWindowState::NotInScanVisor;
    EScanWindowState x34_nextState = EScanWindowState::NotInScanVisor;
    float x38_windowInterpDuration = 0.f;
    float x3c_windowInterpTimer = 0.f;
    zeus::CVector2f x40_prevWindowDims;
    zeus::CVector2f x48_interpWindowDims;
    zeus::CVector2f x50_nextWindowDims;
    float x58_scanMagInterp = 1.f;
    CSfxHandle x5c_visorLoopSfx;
    CSfxHandle x60_scanningLoopSfx;
    CCameraFilterPass<CColoredQuadFilter> x64_scanDim;
    CCameraBlurPass x90_xrayBlur;
    float xc4_vpScaleX = 1.f;
    float xc8_vpScaleY = 1.f;
    TLockedToken<CModel> xcc_scanFrameCorner;
    TLockedToken<CModel> xd8_scanFrameCenterSide;
    TLockedToken<CModel> xe4_scanFrameCenterTop;
    TLockedToken<CModel> xf0_scanFrameStretchSide;
    TLockedToken<CModel> xfc_scanFrameStretchTop;
    //TCachedToken<CModel> x108_newScanPane;
    CTexturedQuadFilter x108_newScanPane;
    TLockedToken<CModel> x114_scanShield;
    int x120_assetLockCountdown = 0;
    TLockedToken<CModel> x124_scanIconNoncritical;
    TLockedToken<CModel> x130_scanIconCritical;
    rstl::reserved_vector<SScanTarget, 64> x13c_scanTargets;
    TLockedToken<CTexture> x540_xrayPalette;
    float x54c_frameColorInterp = 0.f;
    float x550_frameColorImpulseInterp = 0.f;

    int FindEmptyInactiveScanTarget() const;
    int FindCachedInactiveScanTarget(TUniqueId uid) const;
    bool DrawScanObjectIndicators(const CStateManager& mgr) const;
    void UpdateScanObjectIndicators(const CStateManager& mgr, float dt);
    void UpdateScanWindow(float dt, const CStateManager& mgr);
    EScanWindowState GetDesiredScanWindowState(const CStateManager& mgr) const;
    void LockUnlockAssets();
    void DrawScanEffect(const CStateManager& mgr, const CTargetingManager* tgtMgr) const;
    void DrawXRayEffect(const CStateManager& mgr) const;
    void DrawThermalEffect(const CStateManager& mgr) const;
    void UpdateCurrentVisor(float transFactor);
    void FinishTransitionIn();
    void BeginTransitionIn(const CStateManager& mgr);
    void FinishTransitionOut(const CStateManager& mgr);
    void BeginTransitionOut();

public:
    CPlayerVisor(CStateManager& stateMgr);
    ~CPlayerVisor();
    void Update(float dt, const CStateManager& stateMgr);
    void Draw(const CStateManager& stateMgr, const CTargetingManager* tgtManager) const;
    void Touch();
    float GetDesiredViewportScaleX(const CStateManager& stateMgr) const;
    float GetDesiredViewportScaleY(const CStateManager& stateMgr) const;
};

}
}

#endif // __URDE_CPLAYERVISOR_HPP__
