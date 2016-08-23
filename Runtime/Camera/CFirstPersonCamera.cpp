#include "CFirstPersonCamera.hpp"
#include "Character/CCharLayoutInfo.hpp"

namespace urde
{

CFirstPersonCamera::CFirstPersonCamera(TUniqueId uid, const zeus::CTransform& xf, TUniqueId id2,
                                       float, float a, float  b, float c, float d)
: CGameCamera(uid, true, "First Person Camera",
              CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList),
              xf, a, b, c, d, id2, 0, 0)
{
}

void CFirstPersonCamera::ProcessInput(const CFinalInput&, CStateManager& mgr)
{
}

void CFirstPersonCamera::Reset(const zeus::CTransform&, CStateManager& mgr)
{
}

}
