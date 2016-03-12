#include "CAuiEnergyBarT01.hpp"

namespace urde
{

CAuiEnergyBarT01::CAuiEnergyBarT01(const CGuiWidgetParms& parms, u32)
: CGuiWidget(parms)
{
}

CAuiEnergyBarT01* CAuiEnergyBarT01::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);
    u32 a = in.readUint32Big();
    return new CAuiEnergyBarT01(parms, a);
}

}
