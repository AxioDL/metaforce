#include "CPathCamera.hpp"
#include "CCameraManager.hpp"

namespace urde
{

CPathCamera::CPathCamera(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                         const zeus::CTransform& xf, bool active, bool, bool, bool, bool,
                         float, float, float, float, float, float, float, u32,
                         EInitialSplinePosition)
: CGameCamera(uid, active, name, info, xf,
              CCameraManager::ThirdPersonFOV(),
              CCameraManager::NearPlane(),
              CCameraManager::FarPlane(),
              CCameraManager::Aspect(), kInvalidUniqueId, 0, 0)
{
}

void CPathCamera::ProcessInput(const CFinalInput&, CStateManager& mgr)
{
}

void CPathCamera::Reset(const zeus::CTransform&, CStateManager& mgr)
{
}

}
