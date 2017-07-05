#ifndef __URDE_CSTEERINGBEHAVIORS_HPP__
#define __URDE_CSTEERINGBEHAVIORS_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{

class CSteeringBehaviors
{
public:
    static zeus::CVector3f ProjectOrbitalPosition(const zeus::CVector3f&, const zeus::CVector3f&,
                                                  const zeus::CVector3f&, float);
};

}

#endif // __URDE_CSTEERINGBEHAVIORS_HPP__
