#ifndef __URDE_CAUIENERGYBAR_HPP__
#define __URDE_CAUIENERGYBAR_HPP__

#include "CGuiWidget.hpp"

namespace urde
{

class CAuiEnergyBarT01 : public CGuiWidget
{
public:
    CAuiEnergyBarT01(const CGuiWidgetParms& parms, u32);
    static CAuiEnergyBarT01* Create(CGuiFrame* frame, CInputStream& in, bool);
};

}

#endif // __URDE_CAUIENERGYBAR_HPP__
