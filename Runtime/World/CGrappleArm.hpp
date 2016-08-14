#ifndef __URDE_CGRAPPLEARM_HPP__
#define __URDE_CGRAPPLEARM_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "Character/CModelData.hpp"

namespace urde
{

class CGrappleArm
{
    CModelData x0_modelData;

public:
    CGrappleArm(const zeus::CVector3f& vec);
};

}

#endif // __URDE_CGRAPPLEARM_HPP__
