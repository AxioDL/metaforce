#ifndef __URDE_CACTORLIGHTS_HPP__
#define __URDE_CACTORLIGHTS_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{

class CActorLights
{
public:
    CActorLights(u32, const zeus::CVector3f& vec, int, int, float);
    void ActivateLights() const {}
};

}

#endif // __URDE_CACTORLIGHTS_HPP__
