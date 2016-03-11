#ifndef __URDE_CGUIAUTOREPEATDATA_HPP__
#define __URDE_CGUIAUTOREPEATDATA_HPP__

#include "CGuiPhysicalMsg.hpp"

namespace urde
{

class CGuiAutoRepeatData
{
    EPhysicalControllerID x0_a;
    EPhysicalControllerID x4_b;
    float x8_[8] = {0.f, 0.f, 0.f, 0.f, 0.2f, 0.2f, 0.2f, 0.2f};
    bool x28_[8] = {};
public:
    CGuiAutoRepeatData(EPhysicalControllerID a, EPhysicalControllerID b)
    : x0_a(a), x4_b(b) {}
};

}

#endif // __URDE_CGUIAUTOREPEATDATA_HPP__
