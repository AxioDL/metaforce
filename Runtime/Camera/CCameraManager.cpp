#include "CCameraManager.hpp"
#include "CCameraShakeData.hpp"
#include "Audio/CSfxManager.hpp"
#include "CGameCamera.hpp"
#include "CStateManager.hpp"
#include "Input/CRumbleManager.hpp"
#include "World/CScriptWater.hpp"

namespace urde
{

CCameraManager::CCameraManager(TUniqueId curCameraId)
: x0_curCameraId(curCameraId)
{
    CSfxManager::AddListener(CSfxManager::ESfxChannels::One,
                             zeus::CVector3f::skZero, zeus::CVector3f::skZero,
                             {1.f, 0.f, 0.f}, {0.f, 0.f, 1.f}, 50.f, 50.f, 1000.f, 1, 0x7f);
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
    for (auto it=x18_shakers.begin() ; it != x18_shakers.end() ; ++it)
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

void CCameraManager::AddCinemaCamera(TUniqueId id, CStateManager& stateMgr)
{
    x4_cineCameras.push_back(id);
}

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
    const CGameCamera* camera = GetCurrentCamera(stateMgr);
    zeus::CVector3f heading = camera->GetTransform().basis * zeus::CVector3f{0.f, 1.f, 0.f};
    CSfxManager::UpdateListener(camera->GetTransform().origin, zeus::CVector3f::skZero,
                                heading, {0.f, 0.f, 1.f}, 0x7f);
    x30_shakeOffset = zeus::CVector3f::skZero;

    for (auto it=x18_shakers.begin() ; it != x18_shakers.end() ;)
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
        if (!x86_25_rumbling || x86_24_)
        {
            stateMgr.GetRumbleManager().Rumble(ERumbleFxId::Seven, stateMgr, ERumblePriority::Two);
            x86_25_rumbling = true;
        }
    }
    else
    {
        x86_25_rumbling = false;
        if (x84_rumbleId != -1)
        {
            stateMgr.GetRumbleManager().StopRumble(x84_rumbleId);
            x84_rumbleId = -1;
        }
    }

    if (x74_fluidCounter)
    {
        const CScriptWater* water = dynamic_cast<const CScriptWater*>(stateMgr.GetObjectById(x78_fluidId));
        if (water)
        {
            // TODO: Finish
            zeus::CColor tmpColor; // Get from water
            zeus::CVector2f tmpVector; // Get from camera
            x3c_fog.SetFogExplicit(ERglFogMode::Four, tmpColor, tmpVector);
            stateMgr.GetCameraFilterPass(4).SetFilter(CCameraFilterPass::EFilterType::One,
                                                      CCameraFilterPass::EFilterShape::Zero,
                                                      0.f, tmpColor, -1);
        }
        x86_26_inWater = true;
    }
    else
    {
        x86_26_inWater = false;
        x3c_fog.DisableFog();
        stateMgr.GetCameraFilterPass(4).DisableFilter(dt);
    }

    x3c_fog.Update(dt);
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

}
