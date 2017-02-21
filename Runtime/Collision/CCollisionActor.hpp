#ifndef __URDE_CCOLLISIONACTOR_HPP__
#define __URDE_CCOLLISIONACTOR_HPP__

#include "World/CPhysicsActor.hpp"

namespace urde
{
class CCollisionActor : public CPhysicsActor
{
public:
    CCollisionActor(TUniqueId, TAreaId, TUniqueId, const zeus::CVector3f&, const zeus::CVector3f&, bool, float);
    CCollisionActor(TUniqueId, TAreaId, TUniqueId, const zeus::CVector3f&, bool, float);
    CCollisionActor(TUniqueId, TAreaId, TUniqueId, bool, float, float);
};
}

#endif // __URDE_CCOLLISIONACTOR_HPP__
