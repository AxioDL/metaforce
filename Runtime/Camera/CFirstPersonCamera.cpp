#include "CFirstPersonCamera.hpp"
#include "Character/CCharLayoutInfo.hpp"

namespace urde
{

CFirstPersonCamera::CFirstPersonCamera(TUniqueId uid, const zeus::CTransform& xf, TUniqueId watchedObj,
                                       float, float fov, float  nearz, float farz, float aspect)
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

}
