#ifndef __URDE_CGRAPPLEARM_HPP__
#define __URDE_CGRAPPLEARM_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "Character/CModelData.hpp"

namespace urde
{

class CGrappleArm
{
public:
    enum class EArmState
    {
        Zero,
        One,
        Two,
        Three,
        Four,
        Five,
        Six,
        Seven,
        Eight,
    };
private:
    CModelData x0_modelData;
    zeus::CTransform x220_;
public:
    CGrappleArm(const zeus::CVector3f& vec);
    void AsyncLoadSuit(CStateManager& mgr);
    void SetX220(const zeus::CTransform& xf) { x220_ = xf; }
    void SetAnimState(EArmState state);
};

}

#endif // __URDE_CGRAPPLEARM_HPP__
