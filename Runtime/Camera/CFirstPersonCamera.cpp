#include "CFirstPersonCamera.hpp"
#include "GameGlobalObjects.hpp"
#include "Character/CCharLayoutInfo.hpp"
#include "CStateManager.hpp"
#include "World/CPlayer.hpp"
#include <math.h>

namespace urde
{

CFirstPersonCamera::CFirstPersonCamera(TUniqueId uid, const zeus::CTransform& xf, TUniqueId watchedObj,
                                       float f1, float fov, float nearz, float farz, float aspect)
: CGameCamera(uid, true, "First Person Camera",
              CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList),
              xf, fov, nearz, farz, aspect, watchedObj, false, 0)
{
}

void CFirstPersonCamera::ProcessInput(const CFinalInput&, CStateManager& mgr)
{
}

void CFirstPersonCamera::Reset(const zeus::CTransform&, CStateManager& mgr)
{
}

void CFirstPersonCamera::sub800E318()
{
    x1c8_ = zeus::CVector3f::skZero;
    x1d4_ = 0.f;
}

zeus::CTransform CFirstPersonCamera::GetGunFollowTransform()
{
    return x190_gunFollowXf;
}

void CFirstPersonCamera::UpdateTransform(CStateManager& mgr)
{
    CPlayer* player = static_cast<CPlayer*>(mgr.ObjectById(GetWatchedObject()));
    if (!player)
    {
        x34_transform = zeus::CTransform::Identity();
        xe4_27_ = true;
        xe4_28_ = true;
        xe4_29_ = true;
        return;
    }

    zeus::CTransform playerXf = player->GetTransform();
    float rZ = std::sin(x1c0_);
    if (std::fabs(rZ) > 1.0f)
        rZ = (rZ > -0.f ? -1.0f : 1.0f);

    float rY = std::cos(x1c0_);
    if (std::fabs(rY) > 1.0f)
        rY = (rY > -0.f ? -1.0f : 1.0f);

    zeus::CVector3f rVec = playerXf.rotate({0.f, rY, rZ});
#if 1
    if (player->x3dc_)
    {
        float f32 = g_tweakPlayer->GetPlayerSomething4();

    }
#endif
}

void CFirstPersonCamera::UpdateElevation(CStateManager&)
{

}

}
