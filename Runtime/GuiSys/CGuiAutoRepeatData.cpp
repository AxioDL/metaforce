#include "CGuiAutoRepeatData.hpp"

namespace urde
{

void CGuiAutoRepeatData::AddAutoEvent(int cIdx, CGuiPhysicalMsg::PhysicalMap& events, float eTime)
{
    if (x28_pressed[cIdx])
    {
        if (x2c_defer[cIdx])
        {
            x2c_defer[cIdx] = false;
            return;
        }

        if (eTime >= x8_nextDue[cIdx])
        {
            CGuiPhysicalMsg::AddControllerID(events, x0_instEvent, true);
            x8_nextDue[cIdx] = x18_remainder[cIdx] + eTime;
            x18_remainder[cIdx] -= 0.05;
            if (x18_remainder[cIdx] < 0.05)
                x18_remainder[cIdx] = 0.05;
        }
        if (CGuiPhysicalMsg::FindControllerID(events, x4_contEvent) ==
            CGuiPhysicalMsg::EControllerState::NotPressed)
            x28_pressed[cIdx] = false;
    }
}

}
