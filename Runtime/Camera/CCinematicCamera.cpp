#include "CCinematicCamera.hpp"
#include "TCastTo.hpp"

namespace urde
{

CCinematicCamera::CCinematicCamera(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                   const zeus::CTransform& xf, bool active, float, float a, float b, float c, float d,
                                   u32 w1)
: CGameCamera(uid, active, name, info, xf, a, b, c, d, kInvalidUniqueId, w1 & 0x20, 0), x21c_w1(w1)
{
}

void CCinematicCamera::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CCinematicCamera::ProcessInput(const CFinalInput&, CStateManager& mgr) {}

void CCinematicCamera::Reset(const zeus::CTransform&, CStateManager& mgr) {}

void CCinematicCamera::WasDeactivated(CStateManager& mgr)
{

}
}
