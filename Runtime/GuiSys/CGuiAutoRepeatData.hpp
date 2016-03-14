#ifndef __URDE_CGUIAUTOREPEATDATA_HPP__
#define __URDE_CGUIAUTOREPEATDATA_HPP__

#include "CGuiPhysicalMsg.hpp"

namespace urde
{

class CGuiAutoRepeatData
{
    EPhysicalControllerID x0_instEvent;
    EPhysicalControllerID x4_contEvent;
    float x8_nextDue[4] = {0.f, 0.f, 0.f, 0.f};
    float x18_remainder[4] = {0.2f, 0.2f, 0.2f, 0.2f};
    bool x28_pressed[4] = {};
    bool x2c_defer[4] = {};
public:
    CGuiAutoRepeatData(EPhysicalControllerID inst, EPhysicalControllerID cont)
    : x0_instEvent(inst), x4_contEvent(cont) {}
    void SetActive(int cIdx, float eTime)
    {
        x8_nextDue[cIdx] = eTime + 0.6f;
        x28_pressed[cIdx] = true;
        x2c_defer[cIdx] = true;
        x18_remainder[cIdx] = 0.2f;
    }
    void AddAutoEvent(int cIdx, CGuiPhysicalMsg::PhysicalMap& events, float eTime);
};

}

#endif // __URDE_CGUIAUTOREPEATDATA_HPP__
