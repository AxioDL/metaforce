#include "CCameraManager.hpp"
#include "CCameraShakeData.hpp"
#include "CFirstPersonCamera.hpp"
#include "Audio/CSfxManager.hpp"
#include "CGameCamera.hpp"
#include "CStateManager.hpp"
#include "Input/CRumbleManager.hpp"
#include "World/CScriptWater.hpp"
#include "World/CPlayer.hpp"
#include "World/CScriptWater.hpp"
#include "CPlayerState.hpp"
#include "GameGlobalObjects.hpp"
#include "Graphics/CGraphics.hpp"
#include "CObjectList.hpp"
#include "TCastTo.hpp"

namespace urde
{
float CCameraManager::sAspect = 1.42f;
float CCameraManager::sFarPlane = 750.f;
float CCameraManager::sNearPlane = 0.2f;
float CCameraManager::sFirstPersonFOV = 55.f;
float CCameraManager::sThirdPersonFOV = 60.f;

CCameraManager::CCameraManager(TUniqueId curCameraId) : x0_curCameraId(curCameraId)
{
    CSfxManager::AddListener(CSfxManager::ESfxChannels::Game, zeus::CVector3f::skZero, zeus::CVector3f::skZero,
                             {1.f, 0.f, 0.f}, {0.f, 0.f, 1.f}, 50.f, 50.f, 1000.f, 1, 0x7f);
    sAspect = float(gViewport.x8_width / gViewport.xc_height);
    sFirstPersonFOV = g_tweakGame->GetFirstPersonFOV();
}

zeus::CVector3f CCameraManager::GetGlobalCameraTranslation(const CStateManager& stateMgr) const
{
    const CGameCamera* camera = GetCurrentCamera(stateMgr);
    return camera->GetTransform() * x30_shakeOffset;
}

zeus::CTransform CCameraManager::GetCurrentCameraTransform(const CStateManager& stateMgr) const
{
    const CGameCamera* camera = GetCurrentCamera(stateMgr);
    return camera->GetTransform() * zeus::CTransform::Translate(x30_shakeOffset);
}

void CCameraManager::RemoveCameraShaker(int id)
{
    for (auto it = x18_shakers.begin(); it != x18_shakers.end(); ++it)
        if (it->x20_shakerId == id)
        {
            x18_shakers.erase(it);
            break;
        }
}

int CCameraManager::AddCameraShaker(const CCameraShakeData& data)
{
    x18_shakers.emplace_back(data);
    x18_shakers.back().x20_shakerId = ++x2c_lastShakeId;
    return x2c_lastShakeId;
}

void CCameraManager::AddCinemaCamera(TUniqueId id, CStateManager& stateMgr) { x4_cineCameras.push_back(id); }

void CCameraManager::SetInsideFluid(bool val, TUniqueId fluidId)
{
    if (val)
    {
        ++x74_fluidCounter;
        x78_fluidId = fluidId;
    }
    else
        --x74_fluidCounter;
}

void CCameraManager::Update(float dt, CStateManager& stateMgr)
{
    sub800097AC(dt, stateMgr);
    ThinkCameras(dt, stateMgr);
    UpdateListener(stateMgr);
    UpdateRumble(dt, stateMgr);
    UpdateFog(dt, stateMgr);

#if 0
    const CGameCamera* camera = GetCurrentCamera(stateMgr);
    zeus::CVector3f heading = camera->GetTransform().basis * zeus::CVector3f{0.f, 1.f, 0.f};
    CSfxManager::UpdateListener(camera->GetTransform().origin, zeus::CVector3f::skZero, heading, {0.f, 0.f, 1.f}, 0x7f);
    x30_shakeOffset = zeus::CVector3f::skZero;

    for (auto it = x18_shakers.begin(); it != x18_shakers.end();)
    {
        if (it->x1c_curTime >= it->x18_duration)
        {
            it = x18_shakers.erase(it);
            continue;
        }
        x30_shakeOffset += it->GeneratePoint(dt, *stateMgr.GetActiveRandom());
        ++it;
    }

    if (x18_shakers.size())
    {
        if (!xa0_25_rumbling || xa0_24_)
        {
            stateMgr.GetRumbleManager().Rumble(ERumbleFxId::Seven, stateMgr, ERumblePriority::Two);
            xa0_25_rumbling = true;
        }
    }
    else
    {
        xa0_25_rumbling = false;
        if (x84_rumbleId != -1)
        {
            stateMgr.GetRumbleManager().StopRumble(x84_rumbleId);
            x84_rumbleId = -1;
        }
    }

    if (x74_fluidCounter)
    {
        const CScriptWater* water = TCastToConstPtr<CScriptWater>(stateMgr.GetObjectById(x78_fluidId));
        if (water)
        {
            // TODO: Finish
            zeus::CColor tmpColor;     // Get from water
            zeus::CVector2f tmpVector; // Get from camera
            x3c_fog.SetFogExplicit(ERglFogMode::PerspExp, tmpColor, tmpVector);
            stateMgr.GetCameraFilterPass(4).SetFilter(CCameraFilterPass::EFilterType::Multiply,
                                                      CCameraFilterPass::EFilterShape::Fullscreen, 0.f, tmpColor, -1);
        }
        xa0_26_inWater = true;
    }
    else
    {
        xa0_26_inWater = false;
        x3c_fog.DisableFog();
        stateMgr.GetCameraFilterPass(4).DisableFilter(dt);
    }

    x3c_fog.Update(dt);
#endif
}

CGameCamera* CCameraManager::GetCurrentCamera(CStateManager& stateMgr) const
{
    CObjectList* camList = stateMgr.ObjectListById(EGameObjectList::GameCamera);
    return TCastToPtr<CGameCamera>(camList->GetObjectById(GetCurrentCameraId())).GetPtr();
}

const CGameCamera* CCameraManager::GetCurrentCamera(const CStateManager& stateMgr) const
{
    const CObjectList* camList = stateMgr.GetObjectListById(EGameObjectList::GameCamera);
    return static_cast<const CGameCamera*>(camList->GetObjectById(GetCurrentCameraId()));
}

float CCameraManager::sub80009148() const
{
    const zeus::CVector3f uVec = x7c_fpCamera->GetTransform().upVector();
    return 1.f - std::min(std::fabs(std::min(std::fabs(uVec.dot(zeus::kUpVec)), 1.f) / std::cos(zeus::degToRad(30.f))),
                          1.f);
}

void CCameraManager::sub800097AC(float, CStateManager& mgr)
{
}

void CCameraManager::ThinkCameras(float dt, CStateManager& mgr)
{
    CGameCameraList gcList = mgr.GetCameraObjectList();

    for (CEntity* ent : gcList)
    {
        TCastToPtr<CGameCamera> gc(ent);
        if (gc)
        {
            gc->Think(dt, mgr);
            gc->UpdatePerspective(dt);
        }
    }

    if (IsInCinematicCamera())
        return;

    TUniqueId camId = GetLastCameraId();
    const CGameCamera* cam = TCastToConstPtr<CGameCamera>(mgr.GetObjectById(camId));

    if (cam)
        x3bc_curFov = cam->GetFov();
}

void CCameraManager::UpdateFog(float, CStateManager&)
{

}

void CCameraManager::UpdateRumble(float, CStateManager&)
{
}

void CCameraManager::UpdateListener(CStateManager& mgr)
{
    const zeus::CTransform xf = GetCurrentCameraTransform(mgr);
    CSfxManager::UpdateListener(xf.origin, zeus::CVector3f::skZero, xf.frontVector(), xf.upVector(), 127);
}

float CCameraManager::CalculateFogDensity(CStateManager& mgr, const CScriptWater* water)
{
    float f31 = 1.f /* 1.f - water->x1b4_->x40_; */;
    float f1 = 0;
    if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::GravitySuit))
        f1 = (g_tweakPlayer->GetX54() * g_tweakPlayer->GetX50()) + f31;
    else
        f1 = (g_tweakPlayer->GetX5C() * g_tweakPlayer->GetX58()) + f31;

    return f1 * x94_;
}

void CCameraManager::ResetCameras(CStateManager& mgr)
{
    zeus::CTransform xf = mgr.GetPlayer().CreateTransformFromMovementDirection();
    xf.origin = mgr.GetPlayer().GetEyePosition();

    for (CEntity* ent : mgr.GetCameraObjectList())
    {
        TCastToPtr<CGameCamera> camObj(ent);
        camObj->Reset(xf, mgr);
    }
}

void CCameraManager::SetSpecialCameras(CFirstPersonCamera& fp, CBallCamera& ball)
{
    x7c_fpCamera = &fp;
    x80_ballCamera = &ball;
}
}
