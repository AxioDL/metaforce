#ifndef __URDE_CSNAKEWEEDSWARM_HPP__
#define __URDE_CSNAKEWEEDSWARM_HPP__

#include "World/CActor.hpp"
#include "Collision/CCollisionSurface.hpp"

namespace urde
{
class CSnakeWeedSwarm : public CActor
{
public:
    void ApplyRadiusDamage(const zeus::CVector3f& pos, const CDamageInfo& info,
                           CStateManager& stateMgr) {}
};
}

#endif // __URDE_CSNAKEWEEDSWARM_HPP__
