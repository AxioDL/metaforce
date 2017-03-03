#include "CFluidPlaneManager.hpp"

namespace urde
{

CFluidPlaneManager::CFluidProfile CFluidPlaneManager::sProfile = {};

void CFluidPlaneManager::CFluidProfile::Clear()
{

}

void CFluidPlaneManager::StartFrame(bool b)
{
    x121_ = b;
    sProfile.Clear();
}

}
