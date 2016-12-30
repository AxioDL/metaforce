#include "CCinematicCamera.hpp"

namespace urde
{

CCinematicCamera::CCinematicCamera(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                   const zeus::CTransform& xf, bool active, float, float a, float b, float c, float d,
                                   u32 e)
: CGameCamera(uid, active, name, info, xf, a, b, c, d, kInvalidUniqueId, e & 0x20, 0)
{
}

void CCinematicCamera::ProcessInput(const CFinalInput&, CStateManager& mgr) {}

void CCinematicCamera::Reset(const zeus::CTransform&, CStateManager& mgr) {}
}
