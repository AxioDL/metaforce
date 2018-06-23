#ifndef __URDE_CSNAKEWEEDSWARM_HPP__
#define __URDE_CSNAKEWEEDSWARM_HPP__

#include "World/CActor.hpp"
#include "Collision/CCollisionSurface.hpp"

namespace urde
{
class CAnimationParameters;
class CSnakeWeedSwarm : public CActor
{
public:
    CSnakeWeedSwarm(TUniqueId, bool, std::string_view, const CEntityInfo&, const zeus::CVector3f&, const zeus::CVector3f&,
                    const CAnimationParameters&, const CActorParameters&, float, float, float, float, float, float, float,
                    float, float, float, float, float, float, float, const CDamageInfo&, float, u32, u32, u32, u32, u32,
                    u32, float);

    void Accept(IVisitor&);
    void ApplyRadiusDamage(const zeus::CVector3f& pos, const CDamageInfo& info,
                           CStateManager& stateMgr) {}
};
}

#endif // __URDE_CSNAKEWEEDSWARM_HPP__
