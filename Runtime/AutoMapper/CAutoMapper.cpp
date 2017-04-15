#include "CAutoMapper.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"
#include "Camera/CGameCamera.hpp"
#include "AutoMapper/CMapUniverse.hpp"
#include "AutoMapper/CMapArea.hpp"
#include "zeus/CEulerAngles.hpp"
#include "World/CPlayer.hpp"
#include "Particle/CGenDescription.hpp"
#include "MP1/MP1.hpp"
#include "Input/ControlMapper.hpp"

namespace urde
{

void CAutoMapper::SAutoMapperRenderState::InterpolateWithClamp(const SAutoMapperRenderState& a,
                                                               SAutoMapperRenderState& out,
                                                               const SAutoMapperRenderState& b,
                                                               float t)
{
    t = zeus::clamp(0.f, t, 1.f);
    float easeIn = zeus::clamp(0.f, t * t * t, 1.f);
    float omt = 1.f - t;
    float easeOut = zeus::clamp(0.f, 1.f - omt * omt * omt, 1.f);

    float easeInOut;
    if (t >= 0.5f)
        easeInOut = zeus::clamp(0.f, 0.5f * std::sqrt(2.f * t - 1.f) + 0.5f, 1.f);
    else
        easeInOut = zeus::clamp(0.f, 1.f - (0.5f * std::sqrt(2.f * omt - 1.f) + 0.5f), 1.f);

    float eases[] = {};
    eases[1] = t;
    eases[2] = easeOut;
    eases[3] = easeIn;
    eases[4] = easeInOut;

    if (b.x44_viewportEase != Ease::None)
    {
        float easeB = eases[int(b.x44_viewportEase)];
        float easeA = 1.f - easeB;
        out.x0_viewportSize = zeus::CVector2i(b.x0_viewportSize.x * easeB + a.x0_viewportSize.x * easeA,
                                              b.x0_viewportSize.y * easeB + a.x0_viewportSize.y * easeA);
    }

    if (b.x48_camEase != Ease::None)
    {
        float easeB = eases[int(b.x48_camEase)];
        float easeA = 1.f - easeB;
        out.x8_camOrientation = zeus::CQuaternion::slerp(a.x8_camOrientation, b.x8_camOrientation, easeB);
        out.x18_camDist = b.x18_camDist * easeB + a.x18_camDist * easeA;
        out.x1c_camAngle = b.x1c_camAngle * easeB + a.x1c_camAngle * easeA;
    }

    if (b.x4c_pointEase != Ease::None)
    {
        float easeB = eases[int(b.x4c_pointEase)];
        float easeA = 1.f - easeB;
        out.x20_areaPoint = b.x20_areaPoint * easeB + a.x20_areaPoint * easeA;
    }

    if (b.x50_depth1Ease != Ease::None)
    {
        float easeB = eases[int(b.x50_depth1Ease)];
        float easeA = 1.f - easeB;
        out.x2c_drawDepth1 = b.x2c_drawDepth1 * easeB + a.x2c_drawDepth1 * easeA;
    }

    if (b.x54_depth2Ease != Ease::None)
    {
        float easeB = eases[int(b.x54_depth2Ease)];
        float easeA = 1.f - easeB;
        out.x30_drawDepth2 = b.x30_drawDepth2 * easeB + a.x30_drawDepth2 * easeA;
    }

    if (b.x58_alphaEase != Ease::None)
    {
        float easeB = eases[int(b.x58_alphaEase)];
        float easeA = 1.f - easeB;
        out.x34_alphaSurfaceVisited = b.x34_alphaSurfaceVisited * easeB + a.x34_alphaSurfaceVisited * easeA;
        out.x38_alphaOutlineVisited = b.x38_alphaOutlineVisited * easeB + a.x38_alphaOutlineVisited * easeA;
        out.x3c_alphaSurfaceUnvisited = b.x3c_alphaSurfaceUnvisited * easeB + a.x3c_alphaSurfaceUnvisited * easeA;
        out.x40_alphaOutlineUnvisited = b.x40_alphaOutlineUnvisited * easeB + a.x40_alphaOutlineUnvisited * easeA;
    }
}

CAutoMapper::CAutoMapper(CStateManager& stateMgr)
: x24_world(stateMgr.WorldNC())
{
    x8_mapu = g_SimplePool->GetObj("MAPU_MapUniverse");
    x30_miniMapSamus = g_SimplePool->GetObj("CMDL_MiniMapSamus");
    x3c_hintBeacon = g_SimplePool->GetObj("TXTR_HintBeacon");

    xa0_curAreaId = xa4_otherAreaId = stateMgr.GetWorld()->IGetCurrentAreaId();
    zeus::CMatrix3f camRot = stateMgr.GetCameraManager()->GetCurrentCamera(stateMgr)->GetTransform().buildMatrix3f();
    xa8_renderStates[0] = xa8_renderStates[1] = xa8_renderStates[2] = BuildMiniMapWorldRenderState(stateMgr, camRot, xa0_curAreaId);

    x48_mapIcons.push_back(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), g_tweakPlayerRes->x4_saveStationIcon}));
    x48_mapIcons.push_back(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), g_tweakPlayerRes->x8_missileStationIcon}));
    x48_mapIcons.push_back(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), g_tweakPlayerRes->xc_elevatorIcon}));
    x48_mapIcons.push_back(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), g_tweakPlayerRes->x10_minesBreakFirstTopIcon}));
    x48_mapIcons.push_back(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), g_tweakPlayerRes->x14_minesBreakFirstBottomIcon}));

    for (int i=0 ; i<9 ; ++i)
    {
        x210_lstick.push_back(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), g_tweakPlayerRes->x24_lStick[i]}));
        x25c_cstick.push_back(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), g_tweakPlayerRes->x4c_cStick[i]}));
    }

    for (int i=0 ; i<2 ; ++i)
    {
        x2a8_ltrigger.push_back(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), g_tweakPlayerRes->x74_lTrigger[i]}));
        x2bc_rtrigger.push_back(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), g_tweakPlayerRes->x80_rTrigger[i]}));
        x2d0_abutton.push_back(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), g_tweakPlayerRes->x98_aButton[i]}));
    }
}

bool CAutoMapper::CheckLoadComplete()
{
    switch (x4_loadPhase)
    {
    case ELoadPhase::LoadResources:
        for (TLockedToken<CTexture>& tex : x48_mapIcons)
            if (!tex.IsLoaded())
                return false;
        if (!x30_miniMapSamus.IsLoaded())
            return false;
        if (!x3c_hintBeacon.IsLoaded())
            return false;
        x4_loadPhase = ELoadPhase::LoadUniverse;
    case ELoadPhase::LoadUniverse:
        if (x8_mapu.IsLoaded())
            return false;
        x14_dummyWorlds.resize(x8_mapu->GetNumMapWorldDatas());
        SetCurWorldAssetId(x24_world->IGetWorldAssetId());
        x4_loadPhase = ELoadPhase::Done;
    case ELoadPhase::Done:
        return true;
    default: break;
    }
    return false;
}

bool CAutoMapper::NotHintNavigating() const
{
    return x1e0_hintSteps.empty();
}

bool CAutoMapper::CanLeaveMapScreenInternal(const CStateManager& mgr) const
{
    if (!NotHintNavigating())
        return false;
    if (IsRenderStateInterpolating())
        return false;
    if (IsInMapperState(EAutoMapperState::MapScreenUniverse))
        return true;
    if (x24_world != mgr.GetWorld())
        return false;
    if (IsInMapperState(EAutoMapperState::MapScreen))
        return true;
    return false;
}

bool CAutoMapper::HasCurrentMapUniverseWorld() const
{
    ResId mlvlId = x24_world->IGetWorldAssetId();
    for (const CMapUniverse::CMapWorldData& wld : *x8_mapu)
        if (wld.GetWorldAssetId() == mlvlId)
            return true;
    return false;
}

bool CAutoMapper::CheckDummyWorldLoad(const CStateManager& mgr)
{
    const CMapUniverse::CMapWorldData& mapuWld = x8_mapu->GetMapWorldData(x9c_worldIdx);
    auto& dummyWorld = x14_dummyWorlds[x9c_worldIdx];
    if (!dummyWorld)
    {
        x32c_loadingDummyWorld = false;
        return false;
    }

    if (!dummyWorld->ICheckWorldComplete())
        return true;

    CWorldState& worldState = g_GameState->StateForWorld(dummyWorld->IGetWorldAssetId());
    CMapWorldInfo& mwInfo = *worldState.MapWorldInfo();
    zeus::CVector3f localPoint = mapuWld.GetWorldTransform().inverse() * xa8_renderStates[0].x20_areaPoint;
    zeus::CMatrix3f camRot = xa8_renderStates[0].x8_camOrientation.toTransform().buildMatrix3f();
    TAreaId aid = FindClosestVisibleArea(localPoint, zeus::CUnitVector3f(camRot[1]), mgr, *dummyWorld, mwInfo);
    if (aid == -1)
    {
        x32c_loadingDummyWorld = false;
        return false;
    }
    xa0_curAreaId = aid;

    dummyWorld->IMapWorld()->RecalculateWorldSphere(mwInfo, *dummyWorld);
    BeginMapperStateTransition(EAutoMapperState::MapScreen, mgr);
    x32c_loadingDummyWorld = false;
    return true;
}

bool CAutoMapper::CanLeaveMapScreen(const CStateManager& mgr) const
{
    return x328_ == 3 && CanLeaveMapScreenInternal(mgr);
}

void CAutoMapper::SetCurWorldAssetId(ResId mlvlId)
{
    int numWorlds = x8_mapu->GetNumMapWorldDatas();
    for (int i=0 ; i<numWorlds ; ++i)
        if (x8_mapu->GetMapWorldData(i).GetWorldAssetId() == mlvlId)
        {
            x9c_worldIdx = i;
            break;
        }
}

void CAutoMapper::BeginMapperStateTransition(EAutoMapperState state, const CStateManager& mgr)
{
    if (state == x1c0_nextState)
        return;
    if ((state == EAutoMapperState::MiniMap && x1c0_nextState != EAutoMapperState::MiniMap) ||
        (state != EAutoMapperState::MiniMap && x1c0_nextState == EAutoMapperState::MiniMap))
        CSfxManager::KillAll(CSfxManager::ESfxChannels::PauseScreen);

    x1bc_state = x1c0_nextState;
    x1c0_nextState = state;
    xa8_renderStates[2] = xa8_renderStates[0];
    xa8_renderStates[1] = xa8_renderStates[0];

    if (x1bc_state == EAutoMapperState::MiniMap && state == EAutoMapperState::MapScreen)
    {
        xa8_renderStates[1] = BuildMapScreenWorldRenderState(mgr,
                              xa8_renderStates[0].x8_camOrientation, xa0_curAreaId, false);
        ResetInterpolationTimer(g_tweakAutoMapper->GetOpenMapScreenTime());
    }
    else if (x1bc_state == EAutoMapperState::MapScreen && state == EAutoMapperState::MiniMap)
    {
        xa0_curAreaId = x24_world->IGetCurrentAreaId();
        xa8_renderStates[1] = BuildMiniMapWorldRenderState(mgr,
                              xa8_renderStates[0].x8_camOrientation, xa0_curAreaId);
        ResetInterpolationTimer(g_tweakAutoMapper->GetCloseMapScreenTime());
        x1f8_hintLocations.clear();
    }
    else if (x1bc_state == EAutoMapperState::MapScreen && state == EAutoMapperState::MapScreenUniverse)
    {
        CSfxManager::SfxStart(1426, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        xa8_renderStates[1] = BuildMapScreenUniverseRenderState(mgr,
                              xa8_renderStates[0].x8_camOrientation, xa0_curAreaId);
        TransformRenderStatesWorldToUniverse();
        ResetInterpolationTimer(g_tweakAutoMapper->GetSwitchToFromUniverseTime());
    }
    else if (x1bc_state == EAutoMapperState::MapScreenUniverse && state == EAutoMapperState::MapScreen)
    {
        CSfxManager::SfxStart(1427, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        xa8_renderStates[1] = BuildMapScreenWorldRenderState(mgr,
                              xa8_renderStates[0].x8_camOrientation, xa0_curAreaId, x1e0_hintSteps.size());
        TransformRenderStateWorldToUniverse(xa8_renderStates[1]);
        ResetInterpolationTimer(g_tweakAutoMapper->GetSwitchToFromUniverseTime());
        for (auto& wld : x14_dummyWorlds)
        {
            if (wld.get() != x24_world || x24_world == mgr.GetWorld())
                wld.reset();
        }
    }
    else if (x1bc_state == EAutoMapperState::MapScreenUniverse && state == EAutoMapperState::MiniMap)
    {
        x24_world = mgr.GetWorld();
        xa0_curAreaId = x24_world->IGetCurrentAreaId();
        xa8_renderStates[1] = BuildMiniMapWorldRenderState(mgr,
                              xa8_renderStates[0].x8_camOrientation, xa0_curAreaId);
        SetCurWorldAssetId(x24_world->IGetWorldAssetId());
        TransformRenderStateWorldToUniverse(xa8_renderStates[1]);
        ResetInterpolationTimer(g_tweakAutoMapper->GetCloseMapScreenTime());
        x1f8_hintLocations.clear();
        for (auto& wld : x14_dummyWorlds)
        {
            if (wld.get() != x24_world || x24_world == mgr.GetWorld())
                wld.reset();
        }
    }
}

void CAutoMapper::ResetInterpolationTimer(float t)
{
    x1c4_ = t;
    x1c8_ = 0.f;
}

CAutoMapper::SAutoMapperRenderState
CAutoMapper::BuildMiniMapWorldRenderState(const CStateManager& stateMgr,
                                          const zeus::CQuaternion& rot,
                                          TAreaId area) const
{
    zeus::CQuaternion camOrient = GetMiniMapCameraOrientation(stateMgr);
    zeus::CQuaternion useOrient = (camOrient.dot(rot) >= 0.f) ? camOrient : camOrient.buildEquivalent();
    SAutoMapperRenderState ret(GetMiniMapViewportSize(), useOrient, g_tweakAutoMapper->GetMiniCamDist(),
                               g_tweakAutoMapper->GetMiniCamAngle(), GetAreaPointOfInterest(stateMgr, area),
                               GetMapAreaMiniMapDrawDepth(), GetMapAreaMiniMapDrawDepth(),
                               GetMapAreaMiniMapDrawAlphaSurfaceVisited(stateMgr),
                               GetMapAreaMiniMapDrawAlphaOutlineVisited(stateMgr),
                               GetMapAreaMiniMapDrawAlphaSurfaceUnvisited(stateMgr),
                               GetMapAreaMiniMapDrawAlphaOutlineUnvisited(stateMgr));
    ret.x44_viewportEase = SAutoMapperRenderState::Ease::Out;
    ret.x48_camEase = SAutoMapperRenderState::Ease::Out;
    ret.x4c_pointEase = SAutoMapperRenderState::Ease::Out;
    ret.x50_depth1Ease = SAutoMapperRenderState::Ease::Linear;
    ret.x54_depth2Ease = SAutoMapperRenderState::Ease::In;
    ret.x58_alphaEase = SAutoMapperRenderState::Ease::Linear;
    return ret;
}

CAutoMapper::SAutoMapperRenderState
CAutoMapper::BuildMapScreenWorldRenderState(const CStateManager& mgr,
                                            const zeus::CQuaternion& rot,
                                            TAreaId area, bool doingHint) const
{
    float camDist = doingHint ? g_tweakAutoMapper->GetMaxCamDist() : g_tweakAutoMapper->GetCamDist();
    SAutoMapperRenderState ret(GetMapScreenViewportSize(), rot, camDist,
                               g_tweakAutoMapper->GetCamAngle(), GetAreaPointOfInterest(mgr, area),
                               GetMapAreaMaxDrawDepth(mgr, area),
                               GetMapAreaMaxDrawDepth(mgr, area),
                               g_tweakAutoMapper->GetAlphaSurfaceVisited(),
                               g_tweakAutoMapper->GetAlphaOutlineVisited(),
                               g_tweakAutoMapper->GetAlphaSurfaceUnvisited(),
                               g_tweakAutoMapper->GetAlphaOutlineUnvisited());
    ret.x44_viewportEase = SAutoMapperRenderState::Ease::Out;
    ret.x48_camEase = SAutoMapperRenderState::Ease::Linear;
    ret.x4c_pointEase = SAutoMapperRenderState::Ease::Out;
    ret.x50_depth1Ease = SAutoMapperRenderState::Ease::Linear;
    ret.x54_depth2Ease = SAutoMapperRenderState::Ease::Out;
    ret.x58_alphaEase = SAutoMapperRenderState::Ease::Linear;
    return ret;
}

CAutoMapper::SAutoMapperRenderState
CAutoMapper::BuildMapScreenUniverseRenderState(const CStateManager& mgr,
                                               const zeus::CQuaternion& rot,
                                               TAreaId area) const
{
    SAutoMapperRenderState ret(GetMapScreenViewportSize(), rot, g_tweakAutoMapper->GetUniverseCamDist(),
                               g_tweakAutoMapper->GetCamAngle(), GetAreaPointOfInterest(mgr, area),
                               GetMapAreaMaxDrawDepth(mgr, area),
                               GetMapAreaMaxDrawDepth(mgr, area),
                               0.f, 0.f, 0.f, 0.f);
    ret.x44_viewportEase = SAutoMapperRenderState::Ease::Out;
    ret.x48_camEase = SAutoMapperRenderState::Ease::Linear;
    ret.x4c_pointEase = SAutoMapperRenderState::Ease::Out;
    ret.x50_depth1Ease = SAutoMapperRenderState::Ease::Linear;
    ret.x54_depth2Ease = SAutoMapperRenderState::Ease::Out;
    ret.x58_alphaEase = SAutoMapperRenderState::Ease::Linear;
    return ret;
}

void CAutoMapper::LeaveMapScreenState()
{
    SetShouldPanningSoundBePlaying(false);
    SetShouldZoomingSoundBePlaying(false);
    SetShouldRotatingSoundBePlaying(false);
}

float CAutoMapper::GetBaseMapScreenCameraMoveSpeed()
{
    return g_tweakAutoMapper->GetBaseMapScreenCameraMoveSpeed();
}

float CAutoMapper::GetFinalMapScreenCameraMoveSpeed() const
{
    float ret = GetBaseMapScreenCameraMoveSpeed();
    if (g_tweakAutoMapper->GetScaleMoveSpeedWithCamDist())
        ret = ret * xa8_renderStates[0].x18_camDist / g_tweakAutoMapper->GetCamDist();
    return ret;
}

void CAutoMapper::ProcessMapRotateInput(const CFinalInput& input, const CStateManager& mgr)
{
    float up = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapCircleUp, input);
    float down = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapCircleDown, input);
    float left = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapCircleLeft, input);
    float right = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapCircleRight, input);
    int flags = 0x0;
    if (up > 0.f)
        flags |= 0x2;
    if (down > 0.f)
        flags |= 0x1;
    if (left > 0.f)
        flags |= 0x4;
    if (right > 0.f)
        flags |= 0x8;

    switch (flags)
    {
    case 1: // Down
        x2e4_lStickPos = 1;
        break;
    case 2: // Up
        x2e4_lStickPos = 5;
        break;
    case 4: // Left
        x2e4_lStickPos = 3;
        break;
    case 5: // Down-Left
        x2e4_lStickPos = 2;
        break;
    case 6: // Up-Left
        x2e4_lStickPos = 4;
        break;
    case 8: // Right
        x2e4_lStickPos = 7;
        break;
    case 9: // Down-Right
        x2e4_lStickPos = 8;
        break;
    case 10: // Up-Right
        x2e4_lStickPos = 6;
        break;
    default: break;
    }

    float maxMag = up;
    int dirSlot = 0;
    if (down > up)
    {
        maxMag = down;
        dirSlot = 1;
    }
    if (left > maxMag)
    {
        maxMag = left;
        dirSlot = 2;
    }
    if (right > maxMag)
    {
        maxMag = right;
        dirSlot = 3;
    }

    float dirs[4] = {};
    dirs[dirSlot] = maxMag;

    if (dirs[0] > 0.f || dirs[1] > 0.f || dirs[2] > 0.f || dirs[3] > 0.f)
    {
        float deltaFrames = input.DeltaTime() * 60.f;
        SetShouldRotatingSoundBePlaying(true);
        zeus::CEulerAngles eulers(xa8_renderStates[0].x8_camOrientation);
        float angX = eulers.x - std::floor(eulers.x / (2.f * M_PIF)) * 2.f * M_PIF;
        if (angX < 0.f)
            angX += 2.f * M_PIF;
        float angZ = eulers.z - std::floor(eulers.z / (2.f * M_PIF)) * 2.f * M_PIF;
        if (angZ < 0.f)
            angZ += 2.f * M_PIF;

        float dt = deltaFrames * g_tweakAutoMapper->GetCamRotateDegreesPerFrame();

        angZ -= zeus::degToRad(dt * dirs[2]);
        if (angZ - std::floor(angZ / (2.f * M_PIF)) * 2.f * M_PIF < 0.f)
            angZ += 2.f * M_PIF;
        angZ += zeus::degToRad(dt * dirs[3]);
        if (angZ - std::floor(angZ / (2.f * M_PIF)) * 2.f * M_PIF < 0.f)
            angZ += 2.f * M_PIF;

        angX -= zeus::degToRad(dt * dirs[0]);
        if (angX - std::floor(angX / (2.f * M_PIF)) * 2.f * M_PIF < 0.f)
            angX += 2.f * M_PIF;
        angX += zeus::degToRad(dt * dirs[1]);
        if (angX - std::floor(angX / (2.f * M_PIF)) * 2.f * M_PIF < 0.f)
            angX += 2.f * M_PIF;

        angX = zeus::radToDeg(angX);
        if (angX > 180.f)
            angX -= 360.f;
        angX = zeus::degToRad(zeus::clamp(g_tweakAutoMapper->GetMinCamRotateX(), angX,
                                          g_tweakAutoMapper->GetMaxCamRotateX()));
        if (angX - std::floor(angX / (2.f * M_PIF)) * 2.f * M_PIF < 0.f)
            angX += 2.f * M_PIF;

        zeus::CQuaternion quat;
        quat.rotateZ(angZ);
        quat.rotateX(angX);
        quat.rotateY(0.f);
        xa8_renderStates[0].x8_camOrientation = quat;
    }
    else
    {
        SetShouldRotatingSoundBePlaying(false);
    }
}

void CAutoMapper::ProcessMapZoomInput(const CFinalInput& input, const CStateManager& mgr)
{
    bool in = ControlMapper::GetDigitalInput(ControlMapper::ECommands::MapZoomIn, input);
    bool out = ControlMapper::GetDigitalInput(ControlMapper::ECommands::MapZoomOut, input);

    EZoomState nextZoomState = EZoomState::None;
    switch (x324_zoomState)
    {
    case EZoomState::None:
        if (in)
            nextZoomState = EZoomState::In;
        else if (out)
            nextZoomState = EZoomState::Out;
        break;
    case EZoomState::In:
        if (in)
            nextZoomState = EZoomState::In;
        else if (out)
            nextZoomState = EZoomState::Out;
        break;
    case EZoomState::Out:
        if (in)
            nextZoomState = EZoomState::In;
        else if (out)
            nextZoomState = EZoomState::Out;
        break;
    default: break;
    }

    x324_zoomState = nextZoomState;
    float delta = input.DeltaTime() * 60.f * (x1bc_state == EAutoMapperState::MapScreen ? 1.f : 4.f) *
                  g_tweakAutoMapper->GetCamZoomUnitsPerFrame();
    float oldDist = xa8_renderStates[0].x18_camDist;
    if (x324_zoomState == EZoomState::In)
    {
        xa8_renderStates[0].x18_camDist =
            GetClampedMapScreenCameraDistance(xa8_renderStates[0].x18_camDist - delta);
        x2f0_ = 1;
        x324_zoomState = EZoomState::In;
    }
    else if (x324_zoomState == EZoomState::Out)
    {
        xa8_renderStates[0].x18_camDist =
            GetClampedMapScreenCameraDistance(xa8_renderStates[0].x18_camDist + delta);
        x2ec_ = 1;
        x324_zoomState = EZoomState::Out;
    }

    if (oldDist == xa8_renderStates[0].x18_camDist)
        SetShouldZoomingSoundBePlaying(false);
    else
        SetShouldZoomingSoundBePlaying(true);
}

void CAutoMapper::ProcessMapPanInput(const CFinalInput& input, const CStateManager& mgr)
{
    float forward = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapMoveForward, input);
    float back = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapMoveBack, input);
    float left = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapMoveLeft, input);
    float right = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapMoveRight, input);
    zeus::CTransform camRot = xa8_renderStates[0].x8_camOrientation.toTransform();
    if (forward > 0.f || back > 0.f || left > 0.f || right > 0.f)
    {
        float deltaFrames = 60.f * input.DeltaTime();
        float speed = GetFinalMapScreenCameraMoveSpeed();
        int flags = 0x0;
        if (forward > 0.f)
            flags |= 0x1;
        if (back > 0.f)
            flags |= 0x2;
        if (left > 0.f)
            flags |= 0x4;
        if (right > 0.f)
            flags |= 0x8;

        switch (flags)
        {
        case 1: // Forward
            x2e8_rStickPos = 1;
            break;
        case 2: // Back
            x2e8_rStickPos = 5;
            break;
        case 4: // Left
            x2e8_rStickPos = 3;
            break;
        case 5: // Forward-Left
            x2e8_rStickPos = 2;
            break;
        case 6: // Back-Left
            x2e8_rStickPos = 4;
            break;
        case 8: // Right
            x2e8_rStickPos = 7;
            break;
        case 9: // Forward-Right
            x2e8_rStickPos = 8;
            break;
        case 10: // Back-Right
            x2e8_rStickPos = 6;
            break;
        default: break;
        }

        zeus::CVector3f dirVec(right - left, 0.f, forward - back);
        zeus::CVector3f deltaVec = camRot * (dirVec * deltaFrames * speed);
        zeus::CVector3f newPoint = xa8_renderStates[0].x20_areaPoint + deltaVec;
        if (deltaVec.magnitude() > input.DeltaTime())
            SetShouldPanningSoundBePlaying(true);
        else
            SetShouldPanningSoundBePlaying(false);

        if (x1bc_state == EAutoMapperState::MapScreen)
        {
            xa8_renderStates[0].x20_areaPoint =
                x24_world->IGetMapWorld()->ConstrainToWorldVolume(newPoint, camRot.basis[1]);
        }
        else
        {
            zeus::CVector3f localPoint = newPoint - x8_mapu->GetMapUniverseCenterPoint();
            if (localPoint.magnitude() > x8_mapu->GetMapUniverseRadius())
                newPoint = x8_mapu->GetMapUniverseCenterPoint() + localPoint.normalized() * x8_mapu->GetMapUniverseRadius();
            xa8_renderStates[0].x20_areaPoint = newPoint;
        }
    }
    else
    {
        SetShouldPanningSoundBePlaying(false);
        float speed = g_tweakAutoMapper->GetCamPanUnitsPerFrame() * GetBaseMapScreenCameraMoveSpeed();
        if (x1bc_state == EAutoMapperState::MapScreen)
        {
            const CMapArea* area = x24_world->IGetMapWorld()->GetMapArea(xa0_curAreaId);
            zeus::CVector3f worldPoint = area->GetAreaPostTransform(*x24_world, xa0_curAreaId) * area->GetAreaCenterPoint();
            zeus::CVector3f viewPoint = worldPoint - xa8_renderStates[0].x20_areaPoint;
            if (viewPoint.magnitude() < speed)
                xa8_renderStates[0].x20_areaPoint = worldPoint;
            else
                xa8_renderStates[0].x20_areaPoint += viewPoint.normalized() * speed;
        }
        else
        {
            std::pair<TAreaId, int> areas = FindClosestVisibleWorld(xa8_renderStates[0].x20_areaPoint, camRot.basis[1], mgr);
            const zeus::CTransform& hex = x8_mapu->GetMapWorldData(areas.first).GetMapAreaData(areas.second);
            zeus::CVector3f areaToHex = hex.origin - xa8_renderStates[0].x20_areaPoint;
            if (areaToHex.magnitude() < speed)
                xa8_renderStates[0].x20_areaPoint = hex.origin;
            else
                xa8_renderStates[0].x20_areaPoint += areaToHex.normalized() * speed;
        }
    }
}

void CAutoMapper::SetShouldPanningSoundBePlaying(bool b)
{
    if (b)
    {
        if (!x1cc_panningSfx)
            x1cc_panningSfx = CSfxManager::SfxStart(1406, 1.f, 0.f, false, 0x7f, true, kInvalidAreaId);
    }
    else
    {
        CSfxManager::SfxStop(x1cc_panningSfx);
        x1cc_panningSfx.reset();
    }
}

void CAutoMapper::SetShouldZoomingSoundBePlaying(bool b)
{
    if (b)
    {
        if (!x1d4_zoomingSfx)
            x1d4_zoomingSfx = CSfxManager::SfxStart(1376, 1.f, 0.f, false, 0x7f, true, kInvalidAreaId);
    }
    else
    {
        CSfxManager::SfxStop(x1d4_zoomingSfx);
        x1d4_zoomingSfx.reset();
    }
}

void CAutoMapper::SetShouldRotatingSoundBePlaying(bool b)
{
    if (b)
    {
        if (!x1d0_rotatingSfx)
            x1d0_rotatingSfx = CSfxManager::SfxStart(1375, 1.f, 0.f, false, 0x7f, true, kInvalidAreaId);
    }
    else
    {
        CSfxManager::SfxStop(x1d0_rotatingSfx);
        x1d0_rotatingSfx.reset();
    }
}

void CAutoMapper::ProcessMapScreenInput(const CFinalInput& input, const CStateManager& mgr)
{
    zeus::CMatrix3f camRot = xa8_renderStates[0].x8_camOrientation.toTransform().buildMatrix3f();
    if (x1bc_state == EAutoMapperState::MapScreen)
    {
        if (input.PA() && !x328_ && HasCurrentMapUniverseWorld())
            BeginMapperStateTransition(EAutoMapperState::MapScreenUniverse, mgr);
    }
    else if (x1bc_state == EAutoMapperState::MapScreenUniverse && input.PA())
    {
        const CMapUniverse::CMapWorldData& mapuWld = x8_mapu->GetMapWorldData(x9c_worldIdx);
        zeus::CVector3f pointLocal = mapuWld.GetWorldTransform().inverse() * xa8_renderStates[0].x20_areaPoint;
        if (mapuWld.GetWorldAssetId() != g_GameState->CurrentWorldAssetId())
        {
            x32c_loadingDummyWorld = true;
            CheckDummyWorldLoad(mgr);
        }
        else
        {
            x24_world = mgr.GetWorld();
            CMapWorldInfo& mwInfo = *g_GameState->StateForWorld(x24_world->IGetWorldAssetId()).MapWorldInfo();
            xa0_curAreaId = FindClosestVisibleArea(pointLocal, zeus::CUnitVector3f(camRot[1]), mgr, *x24_world, mwInfo);
            BeginMapperStateTransition(EAutoMapperState::MapScreen, mgr);
        }
    }

    x2f4_ = 0;
    if (input.PA())
        x2f4_ = 1;

    if (IsInMapperState(EAutoMapperState::MapScreen) || IsInMapperState(EAutoMapperState::MapScreenUniverse))
    {
        x2e4_lStickPos = 0;
        x2e8_rStickPos = 0;
        x2ec_ = 0;
        x2f0_ = 0;
        ProcessMapRotateInput(input, mgr);
        ProcessMapZoomInput(input, mgr);
        ProcessMapPanInput(input, mgr);
    }
}

zeus::CQuaternion CAutoMapper::GetMiniMapCameraOrientation(const CStateManager& stateMgr) const
{
    const CGameCamera* cam = stateMgr.GetCameraManager()->GetCurrentCamera(stateMgr);
    zeus::CEulerAngles camAngles(zeus::CQuaternion(cam->GetTransform().buildMatrix3f()));
    float rotMod = -(std::floor(camAngles.z * 0.15915494f) * 2.f * M_PIF - camAngles.z);
    if (rotMod < 0.f)
        rotMod += 2.f * M_PIF;

    zeus::CQuaternion ret;
    ret.rotateZ(rotMod);
    ret.rotateX(zeus::degToRad(g_tweakAutoMapper->GetMiniCamXAngle()));
    return ret;
}

zeus::CVector3f CAutoMapper::GetAreaPointOfInterest(const CStateManager&, TAreaId aid) const
{
    const CMapArea* mapa = x24_world->IGetMapWorld()->GetMapArea(aid);
    return mapa->GetAreaPostTransform(*x24_world, aid) * mapa->GetAreaCenterPoint();
}

TAreaId CAutoMapper::FindClosestVisibleArea(const zeus::CVector3f&, const zeus::CUnitVector3f&, const CStateManager&,
                                            const IWorld&, const CMapWorldInfo&) const
{
    return 0;
}

std::pair<TAreaId, int>
CAutoMapper::FindClosestVisibleWorld(const zeus::CVector3f&, const zeus::CUnitVector3f&, const CStateManager&) const
{
    return {};
}

zeus::CVector2i CAutoMapper::GetMiniMapViewportSize()
{
    float scaleX = g_Viewport.x8_width / 640.f;
    float scaleY = g_Viewport.xc_height / 480.f;
    return {int(scaleX * g_tweakAutoMapper->GetMiniMapViewportWidth()),
            int(scaleY * g_tweakAutoMapper->GetMiniMapViewportHeight())};
}

zeus::CVector2i CAutoMapper::GetMapScreenViewportSize()
{
    return {g_Viewport.x8_width, g_Viewport.xc_height};
}

float CAutoMapper::GetMapAreaMaxDrawDepth(const CStateManager&, TAreaId aid) const
{
    return x24_world->IGetMapWorld()->GetCurrentMapAreaDepth(*x24_world, aid);
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaSurfaceVisited(const CStateManager& stateMgr)
{
    float mapAlphaInterp = g_tweakGui->GetMapAlphaInterpolant();
    return g_tweakAutoMapper->GetMiniAlphaSurfaceVisited() * (1.f - mapAlphaInterp) *
           stateMgr.Player()->GetMapAlpha() + mapAlphaInterp;
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaOutlineVisited(const CStateManager& stateMgr)
{
    float mapAlphaInterp = g_tweakGui->GetMapAlphaInterpolant();
    return g_tweakAutoMapper->GetMiniAlphaOutlineVisited() * (1.f - mapAlphaInterp) *
           stateMgr.Player()->GetMapAlpha() + mapAlphaInterp;
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaSurfaceUnvisited(const CStateManager& stateMgr)
{
    float mapAlphaInterp = g_tweakGui->GetMapAlphaInterpolant();
    return g_tweakAutoMapper->GetMiniAlphaSurfaceUnvisited() * (1.f - mapAlphaInterp) *
           stateMgr.Player()->GetMapAlpha() + mapAlphaInterp;
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaOutlineUnvisited(const CStateManager& stateMgr)
{
    float mapAlphaInterp = g_tweakGui->GetMapAlphaInterpolant();
    return g_tweakAutoMapper->GetMiniAlphaOutlineUnvisited() * (1.f - mapAlphaInterp) *
           stateMgr.Player()->GetMapAlpha() + mapAlphaInterp;
}

float CAutoMapper::GetClampedMapScreenCameraDistance(float v)
{
    if (x1bc_state == EAutoMapperState::MapScreenUniverse)
        return zeus::clamp(g_tweakAutoMapper->GetMinUniverseCamDist(), v,
                           g_tweakAutoMapper->GetMaxUniverseCamDist());
    return zeus::clamp(g_tweakAutoMapper->GetMinCamDist(), v,
                       g_tweakAutoMapper->GetMaxCamDist());
}

void CAutoMapper::UnmuteAllLoopedSounds()
{
    CSfxManager::SfxVolume(x1cc_panningSfx, 1.f);
    CSfxManager::SfxVolume(x1d0_rotatingSfx, 1.f);
    CSfxManager::SfxVolume(x1d4_zoomingSfx, 1.f);
}

void CAutoMapper::ProcessControllerInput(const CFinalInput& input, CStateManager& mgr)
{
    if (!IsRenderStateInterpolating())
    {
        if (IsInMapperState(EAutoMapperState::MapScreen) || IsInMapperState(EAutoMapperState::MapScreenUniverse))
        {
            if (x32c_loadingDummyWorld)
            {
                /* TODO: Finish */
            }
        }
    }
}

void CAutoMapper::TransformRenderStatesWorldToUniverse()
{
    const CMapUniverse::CMapWorldData& mapuWld = x8_mapu->GetMapWorldData(x9c_worldIdx);
    zeus::CQuaternion rot = zeus::CQuaternion(mapuWld.GetWorldTransform().buildMatrix3f());
    xa8_renderStates[2].x8_camOrientation *= rot;
    xa8_renderStates[2].x20_areaPoint = mapuWld.GetWorldTransform() * xa8_renderStates[2].x20_areaPoint;
    xa8_renderStates[0].x8_camOrientation *= rot;
    xa8_renderStates[0].x20_areaPoint = mapuWld.GetWorldTransform() * xa8_renderStates[0].x20_areaPoint;
    xa8_renderStates[1].x8_camOrientation *= rot;
    xa8_renderStates[1].x20_areaPoint = mapuWld.GetWorldTransform() * xa8_renderStates[1].x20_areaPoint;
}

void CAutoMapper::TransformRenderStatesUniverseToWorld()
{
    const CMapUniverse::CMapWorldData& mapuWld = x8_mapu->GetMapWorldData(x9c_worldIdx);
    zeus::CTransform inv = mapuWld.GetWorldTransform().inverse();
    zeus::CQuaternion invRot = zeus::CQuaternion(inv.buildMatrix3f());
    xa8_renderStates[2].x8_camOrientation *= invRot;
    xa8_renderStates[2].x20_areaPoint = inv * xa8_renderStates[2].x20_areaPoint;
    xa8_renderStates[0].x8_camOrientation *= invRot;
    xa8_renderStates[0].x20_areaPoint = inv * xa8_renderStates[0].x20_areaPoint;
    xa8_renderStates[1].x8_camOrientation *= invRot;
    xa8_renderStates[1].x20_areaPoint = inv * xa8_renderStates[1].x20_areaPoint;
}

void CAutoMapper::TransformRenderStateWorldToUniverse(SAutoMapperRenderState& state)
{
    state.x20_areaPoint = x8_mapu->GetMapWorldData(x9c_worldIdx).GetWorldTransform() *
                          xa8_renderStates[1].x20_areaPoint;
}

void CAutoMapper::SetupHintNavigation()
{
    if (!g_GameState->GameOptions().GetIsHintSystemEnabled())
        return;
    x1e0_hintSteps.clear();
    x1f8_hintLocations.clear();
    CHintOptions& hintOpts = g_GameState->HintOptions();
    const CHintOptions::SHintState* curHint = hintOpts.GetCurrentDisplayedHint();
    bool navigating = false;
    if (curHint && curHint->CanContinue())
    {
        navigating = true;
        x1e0_hintSteps.push_back({SAutoMapperHintStep::ShowBeacon{}, 0.75f});
        const CGameHintInfo::CGameHint& nextHint = g_MemoryCardSys->GetHints()[hintOpts.GetNextHintIdx()];
        ResId curMlvl = x24_world->IGetWorldAssetId();
        for (const CGameHintInfo::SHintLocation& loc : nextHint.GetLocations())
        {
            if (loc.x0_mlvlId != curMlvl)
            {
                x1e0_hintSteps.push_back({SAutoMapperHintStep::SwitchToUniverse{}});
                x1e0_hintSteps.push_back({SAutoMapperHintStep::PanToWorld{}, curMlvl});
                x1e0_hintSteps.push_back({SAutoMapperHintStep::SwitchToWorld{}, curMlvl});
            }
            else
            {
                x1e0_hintSteps.push_back({SAutoMapperHintStep::PulseCurrentArea{}});
            }
            x1e0_hintSteps.push_back({SAutoMapperHintStep::PanToArea{}, loc.x8_areaId});
            x1e0_hintSteps.push_back({SAutoMapperHintStep::PulseTargetArea{}});
            x1e0_hintSteps.push_back({SAutoMapperHintStep::ShowBeacon{}, 1.f});
            x1f8_hintLocations.push_back({0, 0.f, loc.x0_mlvlId, loc.x8_areaId});
        }
    }

    for (int i=0 ; i<hintOpts.GetHintStates().size() ; ++i)
    {
        const CHintOptions::SHintState& state = hintOpts.GetHintStates()[i];
        if (navigating && hintOpts.GetNextHintIdx() == i)
            continue;
        if (state.x0_state != CHintOptions::EHintState::Displaying)
            continue;
        const CGameHintInfo::CGameHint& hint = g_MemoryCardSys->GetHints()[i];
        for (const CGameHintInfo::SHintLocation& loc : hint.GetLocations())
            x1f8_hintLocations.push_back({1, 1.f, loc.x0_mlvlId, loc.x8_areaId});
    }
}

void CAutoMapper::OnNewInGameGuiState(EInGameGuiState state, const CStateManager& mgr)
{
    if (state == EInGameGuiState::MapScreen)
    {
        MP1::CMain::EnsureWorldPaksReady();
        const CWorld& wld = *mgr.GetWorld();
        const_cast<CMapWorld*>(wld.GetMapWorld())->SetWhichMapAreasLoaded(wld, 0, 9999);
        SetupHintNavigation();
        BeginMapperStateTransition(EAutoMapperState::MapScreen, mgr);
        x28_frmeMapScreen = g_SimplePool->GetObj("FRME_MapScreen");
        SetResLockState(x210_lstick, true);
        SetResLockState(x25c_cstick, true);
        SetResLockState(x2a8_ltrigger, true);
        SetResLockState(x2bc_rtrigger, true);
        SetResLockState(x2d0_abutton, true);
    }
    else
    {
        MP1::CMain::EnsureWorldPakReady(g_GameState->CurrentWorldAssetId());
        if (x1bc_state == EAutoMapperState::MapScreenUniverse || x24_world == mgr.GetWorld())
        {
            BeginMapperStateTransition(EAutoMapperState::MiniMap, mgr);
            x328_ = 0;
        }
        LeaveMapScreenState();
    }
}

}
