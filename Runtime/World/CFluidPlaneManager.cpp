#include "CFluidPlaneManager.hpp"

namespace urde
{

CFluidPlaneManager::CFluidProfile CFluidPlaneManager::sProfile = {};

CFluidPlaneManager::CFluidPlaneManager()
: x0_rippleManager(20, 0.5f)
{

}

void CFluidPlaneManager::CFluidProfile::Clear()
{

}

void CFluidPlaneManager::StartFrame(bool b)
{
    x121_ = b;
    sProfile.Clear();
}

void CFluidPlaneManager::Update(float dt)
{

}

float CFluidPlaneManager::GetLastRippleDeltaTime(TUniqueId rippler) const
{
    return x0_rippleManager.GetLastRippleDeltaTime(rippler);
}

float CFluidPlaneManager::GetLastSplashDeltaTime(TUniqueId splasher) const
{
    return 0.f;
}

void CFluidPlaneManager::CreateSplash(TUniqueId splasher, CStateManager& mgr, const CScriptWater& water,
                                      const zeus::CVector3f& pos, float factor, bool)
{

}

}
