#include "CPathCamera.hpp"
#include "CCameraManager.hpp"
#include "TCastTo.hpp"

namespace urde
{

CPathCamera::CPathCamera(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                         const zeus::CTransform& xf, bool active, float f1, float f2,
                         float f3, float f4, float f5, u32 flags, EInitialSplinePosition initPos)
: CGameCamera(uid, active, name, info, xf,
              CCameraManager::ThirdPersonFOV(),
              CCameraManager::NearPlane(),
              CCameraManager::FarPlane(),
              CCameraManager::Aspect(), kInvalidUniqueId, 0, 0)
, x188_spline(flags & 1), x1dc_(f1), x1e0_(f2), x1e4_(f3), x1e8_initPos(initPos)
, x1ec_flags(flags), x1f0_(f4), x1f4_(f5)
{
}

void CPathCamera::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CPathCamera::ProcessInput(const CFinalInput&, CStateManager& mgr)
{
}

void CPathCamera::Reset(const zeus::CTransform&, CStateManager& mgr)
{
}

}
