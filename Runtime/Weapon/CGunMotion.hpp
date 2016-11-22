#ifndef __URDE_CGUNMOTION_HPP__
#define __URDE_CGUNMOTION_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "Character/CModelData.hpp"

namespace urde
{

class CGunMotion
{
    CModelData x0_modelData;

public:
    CGunMotion(u32, const zeus::CVector3f& vec);
};

}

#endif // __URDE_CGUNMOTION_HPP__
