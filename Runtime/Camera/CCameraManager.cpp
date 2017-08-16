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
#include "Particle/CGenDescription.hpp"
#include "CObjectList.hpp"
#include "TCastTo.hpp"
#include "CCinematicCamera.hpp"
#include "CBallCamera.hpp"
#include "CInterpolationCamera.hpp"

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
    sAspect = float(g_Viewport.x8_width / g_Viewport.xc_height);
    sFirstPersonFOV = g_tweakGame->GetFirstPersonFOV();
}

bool CCameraManager::IsInFirstPersonCamera() const
{
    return x7c_fpCamera->GetUniqueId() == x0_curCameraId;
}

zeus::CVector3f CCameraManager::GetGlobalCameraTranslation(const CStateManager& stateMgr) const
{
    TCastToConstPtr<CGameCamera> camera(GetCurrentCamera(stateMgr));
    return camera->GetTransform() * x30_shakeOffset;
}

zeus::CTransform CCameraManager::GetCurrentCameraTransform(const CStateManager& stateMgr) const
{
    TCastToConstPtr<CGameCamera> camera(GetCurrentCamera(stateMgr));
    return camera->GetTransform() * zeus::CTransform::Translate(x30_shakeOffset);
}

void CCameraManager::RemoveCameraShaker(int id)
{
    for (auto it = x14_shakers.begin(); it != x14_shakers.end(); ++it)
        if (it->xbc_shakerId == id)
        {
            x14_shakers.erase(it);
            break;
        }
}

int CCameraManager::AddCameraShaker(const CCameraShakeData& data, bool sfx)
{
    x14_shakers.emplace_back(data);
    x14_shakers.back().xbc_shakerId = ++x2c_lastShakeId;
    if (!xa0_24_)
    {
        xa0_24_ = true;
        x90_ = 0.5f;
    }
    if (sfx && data.x0_duration > 0.f)
    {
        float vol =zeus::clamp(100.f, std::max(data.GetSomething(), data.GetSomething2()) * 9.f + 100.f, 127.f);
        CSfxHandle sfxHandle;
        if (data.xc0_flags & 0x1)
            sfxHandle = CSfxManager::AddEmitter(1133, data.xc4_sfxPos, zeus::CVector3f::skZero,
                                                vol / 127.f, false, false, 0x7f, kInvalidAreaId);
        else
            sfxHandle = CSfxManager::SfxStart(1133, vol / 127.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        sfxHandle->SetTimeRemaining(data.x0_duration);
    }
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
    UpdateCameraHints(dt, stateMgr);
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
            stateMgr.GetCameraFilterPass(4).SetFilter(EFilterType::Multiply,
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
    return static_cast<CGameCamera*>(camList->GetObjectById(GetCurrentCameraId()));
}

const CGameCamera* CCameraManager::GetCurrentCamera(const CStateManager& stateMgr) const
{
    const CObjectList* camList = stateMgr.GetObjectListById(EGameObjectList::GameCamera);
    return static_cast<const CGameCamera*>(camList->GetObjectById(GetCurrentCameraId()));
}

void CCameraManager::CreateStandardCameras(CStateManager& stateMgr)
{
    TUniqueId fpId = stateMgr.AllocateUniqueId();
    x7c_fpCamera = new CFirstPersonCamera(fpId, zeus::CTransform::Identity(),
                                          stateMgr.Player()->GetUniqueId(), g_tweakPlayer->GetX184(),
                                          sFirstPersonFOV, sNearPlane, sFarPlane, sAspect);
    stateMgr.AddObject(x7c_fpCamera);
    stateMgr.Player()->SetCameraState(CPlayer::EPlayerCameraState::Zero, stateMgr);
    SetCurrentCameraId(fpId, stateMgr);

    x80_ballCamera = new CBallCamera(stateMgr.AllocateUniqueId(), stateMgr.Player()->GetUniqueId(),
                                     zeus::CTransform::Identity(), sThirdPersonFOV,
                                     sNearPlane, sFarPlane, sAspect);
    stateMgr.AddObject(x80_ballCamera);

    x88_interpCamera = new CInterpolationCamera(stateMgr.AllocateUniqueId(), zeus::CTransform::Identity());
    stateMgr.AddObject(x88_interpCamera);
}

void CCameraManager::SkipCinematic(CStateManager& stateMgr)
{
    TUniqueId camId = GetCurrentCameraId();
    CCinematicCamera* ent = static_cast<CCinematicCamera*>(stateMgr.ObjectById(camId));
    while (ent)
    {
        ent->SetActive(false);
        ent->WasDeactivated(stateMgr);
        ent = TCastToPtr<CCinematicCamera>(GetCurrentCamera(stateMgr)).GetPtr();
    }
    stateMgr.GetPlayer().UpdateCinematicState(stateMgr);
    x7c_fpCamera->SkipCinematic();
}

float CCameraManager::sub80009148() const
{
    const zeus::CVector3f uVec = x7c_fpCamera->GetTransform().upVector();
    return 1.f - std::min(std::fabs(std::min(std::fabs(uVec.dot(zeus::CVector3f::skUp)), 1.f) /
                          std::cos(zeus::degToRad(30.f))), 1.f);
}

void CCameraManager::UpdateCameraHints(float, CStateManager& mgr)
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

    if (cam != nullptr)
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
    float f31 = water->GetFluidPlane().GetAlpha();
    float f1 = 0;
    if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::GravitySuit))
        f1 = (g_tweakPlayer->GetPlayerTranslationFriction(4) * g_tweakPlayer->GetPlayerTranslationFriction(3)) + f31;
    else
        f1 = (g_tweakPlayer->GetPlayerTranslationFriction(6) * g_tweakPlayer->GetPlayerTranslationFriction(5)) + f31;

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

void CCameraManager::ProcessInput(const CFinalInput& input, CStateManager& stateMgr)
{
    for (CEntity* ent : stateMgr.GetCameraObjectList())
    {
        if (ent == nullptr)
            continue;
        auto& cam = static_cast<CGameCamera&>(*ent);
        if (input.ControllerIdx() != cam.x16c_controllerIdx)
            continue;
        cam.ProcessInput(input, stateMgr);
    }
}

void CCameraManager::RenderCameras(const CStateManager& mgr)
{
    for (CEntity* cam : mgr.GetCameraObjectList())
        static_cast<CGameCamera*>(cam)->Render(mgr);
}

void CCameraManager::SetupBallCamera(CStateManager& mgr)
{

}

void CCameraManager::LeaveBallCamera(CStateManager& mgr, TUniqueId newCamId)
{

}

float CCameraManager::GetCameraBobMagnitude() const
{
    return 1.f - zeus::clamp(-1.f, zeus::clamp(-1.f,
        x7c_fpCamera->GetTransform().basis[1].dot(zeus::CVector3f::skUp), 1.f) /
        std::cos(2.f * M_PIF / 12.f), 1.f);
}
}
