#include "CPlayerVisor.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"
#include "Graphics/CModel.hpp"
#include "CStateManager.hpp"
#include "World/CPlayer.hpp"

namespace urde
{
namespace MP1
{

CPlayerVisor::CPlayerVisor(CStateManager&)
{
    x25_24_visorTransitioning = false;
    x25_25_ = false;
    xcc_scanFrameCorner = g_SimplePool->GetObj("CMDL_ScanFrameCorner");
    xd8_scanFrameCenterSide = g_SimplePool->GetObj("CMDL_ScanFrameCenterSide");
    xe4_scanFrameCenterTop = g_SimplePool->GetObj("CMDL_ScanFrameCenterTop");
    xf0_scanFrameStretchSide = g_SimplePool->GetObj("CMDL_ScanFrameStretchSide");
    xfc_scanFrameStretchTop = g_SimplePool->GetObj("CMDL_ScanFrameStretchTop");
    x108_newScanPane = g_SimplePool->GetObj("CMDL_NewScanPane");
    x114_scanShield = g_SimplePool->GetObj("CMDL_ScanShield");
    x124_scanIconNoncritical = g_SimplePool->GetObj("CMDL_ScanIconNoncritical");
    x130_scanIconCritical = g_SimplePool->GetObj("CMDL_ScanIconCritical");
    x13c_scanTargets.resize(64);
    x540_xrayPalette = g_SimplePool->GetObj("TXTR_XRayPalette");
    x0_scanWindowSizes.push_back({});
    x0_scanWindowSizes.push_back({g_tweakGui->GetScanWindowIdleWidth(), g_tweakGui->GetScanWindowIdleHeight()});
    x0_scanWindowSizes.push_back({g_tweakGui->GetScanWindowActiveWidth(), g_tweakGui->GetScanWindowActiveHeight()});
}

CPlayerVisor::~CPlayerVisor()
{
    CSfxManager::SfxStop(x5c_visorLoopSfx);
    CSfxManager::SfxStop(x60_);
}

int CPlayerVisor::FindEmptyInactiveScanTarget() const
{
    for (int i=0 ; i<x13c_scanTargets.size() ; ++i)
    {
        const SScanTarget& tgt = x13c_scanTargets[i];
        if (tgt.x4_ == 0.f)
            return i;
    }
    return -1;
}

int CPlayerVisor::FindCachedInactiveScanTarget(TUniqueId uid) const
{
    for (int i=0 ; i<x13c_scanTargets.size() ; ++i)
    {
        const SScanTarget& tgt = x13c_scanTargets[i];
        if (tgt.x0_objId == uid && tgt.x4_ > 0.f)
            return i;
    }
    return -1;
}

void CPlayerVisor::DrawScanObjectIndicators(const CStateManager& mgr) const
{

}

void CPlayerVisor::UpdateScanObjectIndicators(const CStateManager& mgr)
{

}

void CPlayerVisor::UpdateScanWindow(float dt, const CStateManager& mgr)
{

}

CPlayerVisor::EScanWindowState
CPlayerVisor::GetDesiredScanWindowState(const CStateManager& mgr) const
{
    if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Scan)
    {
        switch (mgr.GetPlayer().GetScanningState())
        {
        case CPlayer::EPlayerScanState::Scanning:
        case CPlayer::EPlayerScanState::ScanComplete:
            return EScanWindowState::Scan;
        default:
            return EScanWindowState::Idle;
        }
    }
    return EScanWindowState::NotInScanVisor;
}

void CPlayerVisor::LockUnlockAssets()
{

}

void CPlayerVisor::DrawScanEffect(const CStateManager& mgr, const CTargetingManager* tgtMgr) const
{

}

void CPlayerVisor::DrawXRayEffect(const CStateManager&) const
{
    const_cast<CCameraBlurPass&>(x90_xrayBlur).Draw();
}

void CPlayerVisor::DrawThermalEffect(const CStateManager&) const
{
    // Empty
}

void CPlayerVisor::UpdateCurrentVisor(float transFactor)
{
    switch (x1c_curVisor)
    {
    case CPlayerState::EPlayerVisor::XRay:
        x90_xrayBlur.SetBlur(CCameraBlurPass::EBlurType::Xray, 36.f * transFactor, 0.f);
        break;
    case CPlayerState::EPlayerVisor::Scan:
    {
        zeus::CColor dimColor = zeus::CColor::lerp(g_tweakGuiColors->GetScanVisorHudLightMultiply(),
                                                   zeus::CColor::skWhite, 1.f - transFactor);
        x64_scanDim.SetFilter(CCameraFilterPass::EFilterType::Multiply,
                              CCameraFilterPass::EFilterShape::Fullscreen,
                              0.f, dimColor, -1);
        break;
    }
    default: break;
    }
}

void CPlayerVisor::FinishTransitionIn()
{
    switch (x1c_curVisor)
    {
    case CPlayerState::EPlayerVisor::Combat:
        x90_xrayBlur.DisableBlur(0.f);
        break;
    case CPlayerState::EPlayerVisor::XRay:
        x90_xrayBlur.SetBlur(CCameraBlurPass::EBlurType::Xray, 36.f, 0.f);
        if (!x5c_visorLoopSfx)
            x5c_visorLoopSfx = CSfxManager::SfxStart(1384, x24_visorSfxVol, 0.f, false, 0x7f, true, kInvalidAreaId);
        break;
    case CPlayerState::EPlayerVisor::Scan:
    {
        zeus::CColor dimColor = zeus::CColor::lerp(g_tweakGuiColors->GetScanVisorScreenDimColor(),
                                                   g_tweakGuiColors->GetScanVisorHudLightMultiply(),
                                                   x2c_scanDimInterp);
        x64_scanDim.SetFilter(CCameraFilterPass::EFilterType::Multiply,
                              CCameraFilterPass::EFilterShape::Fullscreen,
                              0.f, dimColor, -1);
        if (!x5c_visorLoopSfx)
            x5c_visorLoopSfx = CSfxManager::SfxStart(1404, x24_visorSfxVol, 0.f, false, 0x7f, true, kInvalidAreaId);
        break;
    }
    case CPlayerState::EPlayerVisor::Thermal:
        if (!x5c_visorLoopSfx)
            x5c_visorLoopSfx = CSfxManager::SfxStart(1388, x24_visorSfxVol, 0.f, false, 0x7f, true, kInvalidAreaId);
        break;
    default: break;
    }
}

void CPlayerVisor::BeginTransitionIn(const CStateManager&)
{
    switch (x1c_curVisor)
    {
    case CPlayerState::EPlayerVisor::XRay:
        x90_xrayBlur.SetBlur(CCameraBlurPass::EBlurType::Xray, 0.f, 0.f);
        xc4_vpScaleX = 0.9f;
        xc8_vpScaleY = 0.9f;
        CSfxManager::SfxStart(1383, x24_visorSfxVol, 0.f, false, 0x7f, false, kInvalidAreaId);
        break;
    case CPlayerState::EPlayerVisor::Scan:
        CSfxManager::SfxStart(1383, x24_visorSfxVol, 0.f, false, 0x7f, false, kInvalidAreaId);
        x64_scanDim.SetFilter(CCameraFilterPass::EFilterType::Multiply,
                              CCameraFilterPass::EFilterShape::Fullscreen,
                              0.f, zeus::CColor::skWhite, -1);
        break;
    case CPlayerState::EPlayerVisor::Thermal:
        CSfxManager::SfxStart(1383, x24_visorSfxVol, 0.f, false, 0x7f, false, kInvalidAreaId);
        break;
    default: break;
    }
}

void CPlayerVisor::FinishTransitionOut(const CStateManager&)
{
    switch (x1c_curVisor)
    {
    case CPlayerState::EPlayerVisor::XRay:
        x90_xrayBlur.DisableBlur(0.f);
        xc4_vpScaleX = 1.f;
        xc8_vpScaleY = 1.f;
        break;
    case CPlayerState::EPlayerVisor::Scan:
        x64_scanDim.DisableFilter(0.f);
        x34_ = 0;
        x30_ = 0;
        break;
    case CPlayerState::EPlayerVisor::Thermal:
        x90_xrayBlur.DisableBlur(0.f);
        break;
    default: break;
    }
}

void CPlayerVisor::BeginTransitionOut()
{
    if (x5c_visorLoopSfx)
    {
        CSfxManager::SfxStop(x5c_visorLoopSfx);
        x5c_visorLoopSfx.reset();
    }

    switch (x1c_curVisor)
    {
    case CPlayerState::EPlayerVisor::XRay:
        CSfxManager::SfxStart(1382, x24_visorSfxVol, 0.f, false, 0x7f, false, kInvalidAreaId);
        break;
    case CPlayerState::EPlayerVisor::Scan:
        if (x60_)
        {
            CSfxManager::SfxStop(x60_);
            x60_.reset();
        }
        CSfxManager::SfxStart(1382, x24_visorSfxVol, 0.f, false, 0x7f, false, kInvalidAreaId);
        break;
    case CPlayerState::EPlayerVisor::Thermal:
        CSfxManager::SfxStart(1382, x24_visorSfxVol, 0.f, false, 0x7f, false, kInvalidAreaId);
        break;
    default: break;
    }
}

void CPlayerVisor::Update(float dt, const CStateManager& mgr)
{
    x90_xrayBlur.Update(dt);

    CPlayerState& playerState = *mgr.GetPlayerState();
    CPlayerState::EPlayerVisor activeVisor = playerState.GetActiveVisor(mgr);
    CPlayerState::EPlayerVisor curVisor = playerState.GetCurrentVisor();
    CPlayerState::EPlayerVisor transVisor = playerState.GetTransitioningVisor();
    bool visorTransitioning = playerState.GetIsVisorTransitioning();

    UpdateScanWindow(dt, mgr);

    if (x20_nextVisor != transVisor)
        x20_nextVisor = transVisor;

    LockUnlockAssets();

    if (mgr.GetPlayer().GetScanningState() == CPlayer::EPlayerScanState::ScanComplete)
        x2c_scanDimInterp = std::max(0.f, x2c_scanDimInterp - 2.f * dt);
    else
        x2c_scanDimInterp = std::min(x2c_scanDimInterp + 2.f * dt, 1.f);

    if (visorTransitioning)
    {
        if (!x25_24_visorTransitioning)
            BeginTransitionOut();
        if (x1c_curVisor != curVisor)
        {
            FinishTransitionOut(mgr);
            x1c_curVisor = curVisor;
            BeginTransitionIn(mgr);
        }
        UpdateCurrentVisor(playerState.GetVisorTransitionFactor());
    }
    else
    {
        if (x25_24_visorTransitioning)
        {
            FinishTransitionIn();
        }
        else if (curVisor == CPlayerState::EPlayerVisor::Scan)
        {
            zeus::CColor dimColor = zeus::CColor::lerp(g_tweakGuiColors->GetScanVisorScreenDimColor(),
                                                       g_tweakGuiColors->GetScanVisorHudLightMultiply(),
                                                       x2c_scanDimInterp);
            x64_scanDim.SetFilter(CCameraFilterPass::EFilterType::Multiply,
                                  CCameraFilterPass::EFilterShape::Fullscreen,
                                  0.f, dimColor, -1);
        }
    }

    x25_24_visorTransitioning = visorTransitioning;

    if (x1c_curVisor != activeVisor)
    {
        if (x24_visorSfxVol != 0.f)
        {
            x24_visorSfxVol = 0.f;
            CSfxManager::SfxVolume(x5c_visorLoopSfx, x24_visorSfxVol);
            CSfxManager::SfxVolume(x60_, x24_visorSfxVol);
        }
    }
    else
    {
        if (x24_visorSfxVol != 1.f)
        {
            x24_visorSfxVol = 1.f;
            CSfxManager::SfxVolume(x5c_visorLoopSfx, x24_visorSfxVol);
            CSfxManager::SfxVolume(x60_, x24_visorSfxVol);
        }
    }

    float scanMag = g_tweakGui->GetScanWindowMagnification();
    if (x58_scanMagInterp < scanMag)
        x58_scanMagInterp = std::min(x58_scanMagInterp + 2.f * dt, scanMag);
    else
        x58_scanMagInterp = std::max(x58_scanMagInterp - 2.f * dt, scanMag);
}

void CPlayerVisor::Draw(const CStateManager& mgr, const CTargetingManager* tgtManager) const
{

}

void CPlayerVisor::Touch()
{
    if (x124_scanIconNoncritical.IsLoaded())
        x124_scanIconNoncritical->Touch(0);
    if (x130_scanIconCritical.IsLoaded())
        x130_scanIconCritical->Touch(0);
}

float CPlayerVisor::GetDesiredViewportScaleX(const CStateManager& mgr) const
{
    return mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::Combat ? 1.f : xc4_vpScaleX;
}

float CPlayerVisor::GetDesiredViewportScaleY(const CStateManager& mgr) const
{
    return mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::Combat ? 1.f : xc8_vpScaleY;
}

}
}
