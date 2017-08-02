#include "CBallCamera.hpp"
#include "TCastTo.hpp"

namespace urde
{

CBallCamera::CBallCamera(TUniqueId uid, TUniqueId id2, const zeus::CTransform& xf,
                         float a, float b, float c, float d)
: CGameCamera(uid, true, "Ball Camera",
              CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList),
              xf, a, b, c, d, id2, 0, 0)
{
}

void CBallCamera::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CBallCamera::ProcessInput(const CFinalInput&, CStateManager& mgr)
{
}

void CBallCamera::Reset(const zeus::CTransform&, CStateManager& mgr)
{
}

void CBallCamera::SetState(EBallCameraState state, CStateManager& mgr)
{

}

}
