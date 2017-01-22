#include "CAuiEnergyBarT01.hpp"

namespace urde
{

CAuiEnergyBarT01::CAuiEnergyBarT01(const CGuiWidgetParms& parms, u32)
: CGuiWidget(parms)
{
}

std::shared_ptr<CGuiWidget> CAuiEnergyBarT01::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
    u32 a = in.readUint32Big();
    std::shared_ptr<CGuiWidget> ret = std::make_shared<CAuiEnergyBarT01>(parms, a);
    ret->ParseBaseInfo(frame, in, parms);
    return ret;
}

}
