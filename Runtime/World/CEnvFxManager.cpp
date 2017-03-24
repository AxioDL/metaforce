#include "CEnvFxManager.hpp"
#include "Graphics/CTexture.hpp"

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

}
