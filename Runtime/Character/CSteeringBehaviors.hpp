#ifndef __URDE_CSTEERINGBEHAVIORS_HPP__
#define __URDE_CSTEERINGBEHAVIORS_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{

class CSteeringBehaviors
{
public:
    static zeus::CVector3f ProjectOrbitalPosition(const zeus::CVector3f& pos, const zeus::CVector3f& vel,
                                                  const zeus::CVector3f& orbitPoint, float dt, float preThinkDt);
};

}

#endif // __URDE_CSTEERINGBEHAVIORS_HPP__
