#ifndef __URDE_CPLAYERVISOR_HPP__
#define __URDE_CPLAYERVISOR_HPP__

#include "RetroTypes.hpp"
#include "Camera/CCameraFilter.hpp"
#include "zeus/CVector2f.hpp"
#include "Audio/CSfxManager.hpp"
#include "CPlayerState.hpp"

namespace urde
{
class CStateManager;
class CTargetingManager;
class CModel;

namespace MP1
{

class CPlayerVisor
{
    struct SScanTarget
    {
        TUniqueId x0_objId = kInvalidUniqueId;
        float x4_ = 0.f;
        float x8_ = 0.f;
        bool xc_ = false;
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
    u32 x30_ = 0;
    u32 x34_ = 0;
    float x38_ = 0.f;
    float x3c_ = 0.f;
    zeus::CVector2f x40_;
    zeus::CVector2f x48_;
    zeus::CVector2f x50_;
    float x58_scanMagInterp = 1.f;
    CSfxHandle x5c_visorLoopSfx;
    CSfxHandle x60_;
    CCameraFilterPass x64_scanDim;
    CCameraBlurPass x90_xrayBlur;
    float xc4_vpScaleX = 1.f;
    float xc8_vpScaleY = 1.f;
    TLockedToken<CModel> xcc_scanFrameCorner;
    TLockedToken<CModel> xd8_scanFrameCenterSide;
    TLockedToken<CModel> xe4_scanFrameCenterTop;
    TLockedToken<CModel> xf0_scanFrameStretchSide;
    TLockedToken<CModel> xfc_scanFrameStretchTop;
    TLockedToken<CModel> x108_newScanPane;
    TLockedToken<CModel> x114_scanShield;
    TLockedToken<CModel> x124_scanIconNoncritical;
    TLockedToken<CModel> x130_scanIconCritical;
    rstl::reserved_vector<SScanTarget, 64> x13c_scanTargets;
    TLockedToken<CTexture> x540_xrayPalette;
    float x54c_ = 0.f;
    float x550_ = 0.f;

    int FindEmptyInactiveScanTarget() const;
    int FindCachedInactiveScanTarget(TUniqueId uid) const;
    void DrawScanObjectIndicators(const CStateManager& mgr) const;
    void UpdateScanObjectIndicators(const CStateManager& mgr);
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
