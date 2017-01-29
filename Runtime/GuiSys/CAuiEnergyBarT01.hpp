#ifndef __URDE_CAUIENERGYBAR_HPP__
#define __URDE_CAUIENERGYBAR_HPP__

#include "CGuiWidget.hpp"

namespace urde
{
class CSimplePool;

class CAuiEnergyBarT01 : public CGuiWidget
{
public:
    CAuiEnergyBarT01(const CGuiWidgetParms& parms, u32);
    FourCC GetWidgetTypeID() const { return FOURCC('ENRG'); }
    static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);
};

}

#endif // __URDE_CAUIENERGYBAR_HPP__
