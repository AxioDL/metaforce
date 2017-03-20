#include "CAutoMapper.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"
#include "Camera/CGameCamera.hpp"
#include "AutoMapper/CMapUniverse.hpp"
#include "AutoMapper/CMapArea.hpp"
#include "zeus/CEulerAngles.hpp"
#include "World/CPlayer.hpp"
#include "Particle/CGenDescription.hpp"

namespace urde
{

CAutoMapper::CAutoMapper(CStateManager& stateMgr)
: x24_world(*stateMgr.WorldNC())
{
    x8_mapu = g_SimplePool->GetObj("MAPU_MapUniverse");
    x30_miniMapSamus = g_SimplePool->GetObj("CMDL_MiniMapSamus");
    x3c_hintBeacon = g_SimplePool->GetObj("TXTR_HintBeacon");

    xa0_curAreaId = xa4_otherAreaId = stateMgr.GetWorld()->IGetCurrentAreaId();
    zeus::CMatrix3f camRot = stateMgr.GetCameraManager()->GetCurrentCamera(stateMgr)->GetTransform().buildMatrix3f();
    xa8_[0] = xa8_[1] = xa8_[2] = BuildMiniMapWorldRenderState(stateMgr, camRot, xa0_curAreaId);

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
        SetCurWorldAssetId(x24_world.IGetWorldAssetId());
        x4_loadPhase = ELoadPhase::Done;
    case ELoadPhase::Done:
        return true;
    default: break;
    }
    return false;
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

CAutoMapper::SAutoMapperRenderState
CAutoMapper::BuildMiniMapWorldRenderState(const CStateManager& stateMgr,
                                          const zeus::CQuaternion& rot,
                                          TAreaId area) const
{
    zeus::CQuaternion camOrient = GetMiniMapCameraOrientation(stateMgr);
    zeus::CQuaternion useOrient = (camOrient.dot(rot) >= 0.f) ? camOrient : camOrient.buildEquivalent();
    return SAutoMapperRenderState(GetMiniMapViewportSize(), useOrient, g_tweakAutoMapper->GetX28(),
                                  g_tweakAutoMapper->GetX30(), GetAreaPointOfInterest(stateMgr, area),
                                  GetMapAreaMiniMapDrawDepth(), GetMapAreaMiniMapDrawDepth(),
                                  GetMapAreaMiniMapDrawAlphaSurfaceVisited(stateMgr),
                                  GetMapAreaMiniMapDrawAlphaOutlineVisited(stateMgr),
                                  GetMapAreaMiniMapDrawAlphaSurfaceUnvisited(stateMgr),
                                  GetMapAreaMiniMapDrawAlphaOutlineUnvisited(stateMgr));
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
    ret.rotateX(zeus::degToRad(g_tweakAutoMapper->GetX2C()));
    return ret;
}

zeus::CVector3f CAutoMapper::GetAreaPointOfInterest(const CStateManager&, TAreaId aid) const
{
    const CMapArea* mapa = x24_world.IGetMapWorld()->GetMapArea(aid);
    return mapa->GetAreaPostTransform(x24_world, aid) * mapa->GetAreaCenterPoint();
}

zeus::CVector2i CAutoMapper::GetMiniMapViewportSize()
{
    float scaleX = g_Viewport.x8_width / 640.f;
    float scaleY = g_Viewport.xc_height / 480.f;
    return {int(scaleX * g_tweakAutoMapper->GetMiniMapViewportWidth()),
            int(scaleY * g_tweakAutoMapper->GetMiniMapViewportHeight())};
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaSurfaceVisited(const CStateManager& stateMgr)
{
    float mapAlphaInterp = g_tweakGui->GetMapAlphaInterpolant();
    return g_tweakAutoMapper->GetAlphaOutlineUnvisited() * (1.f - mapAlphaInterp) *
           stateMgr.Player()->GetMapAlpha() + mapAlphaInterp;
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaOutlineVisited(const CStateManager& stateMgr)
{
    float mapAlphaInterp = g_tweakGui->GetMapAlphaInterpolant();
    return g_tweakAutoMapper->GetAlphaOutlineVisited() * (1.f - mapAlphaInterp) *
           stateMgr.Player()->GetMapAlpha() + mapAlphaInterp;
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaSurfaceUnvisited(const CStateManager& stateMgr)
{
    float mapAlphaInterp = g_tweakGui->GetMapAlphaInterpolant();
    return g_tweakAutoMapper->GetAlphaSurfaceUnvisited() * (1.f - mapAlphaInterp) *
           stateMgr.Player()->GetMapAlpha() + mapAlphaInterp;
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaOutlineUnvisited(const CStateManager& stateMgr)
{
    float mapAlphaInterp = g_tweakGui->GetMapAlphaInterpolant();
    return g_tweakAutoMapper->GetAlphaOutlineUnvisited() * (1.f - mapAlphaInterp) *
           stateMgr.Player()->GetMapAlpha() + mapAlphaInterp;
}

void CAutoMapper::UnmuteAllLoopedSounds()
{

}

void CAutoMapper::OnNewInGameGuiState(EInGameGuiState, const CStateManager&)
{

}

}
