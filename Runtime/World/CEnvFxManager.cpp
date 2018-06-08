#include "CEnvFxManager.hpp"
#include "Graphics/CTexture.hpp"
#include "CActor.hpp"

namespace urde
{

CEnvFxManager::CEnvFxManager()
{

}

void CEnvFxManager::Update(float, const CStateManager&)
{

}

void CEnvFxManager::Render(const CStateManager& mgr)
{

}

void CEnvFxManager::AsyncLoadResources(CStateManager& mgr)
{
}

void CEnvFxManager::SetFxDensity(s32 val, float density)
{
    x34_fxDensity = density;
    x38_ = val;
}

void CEnvFxManager::AreaLoaded()
{
    for (CEnvFxManagerGrid& grid : x50_grids)
        grid.x0_24_ = true;
}

void CEnvFxManager::Cleanup()
{
    xb68_envRainSplashId = kInvalidUniqueId;
    xb6a_ = false;
    xb6c_ = 0;
    xb70_ = 0;
}

void CEnvFxManager::sub801e4f10(CActor*, float, const CStateManager& , u32, u32)
{
}

}
