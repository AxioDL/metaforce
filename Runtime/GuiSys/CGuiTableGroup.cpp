#include "CGuiTableGroup.hpp"

namespace urde
{

CGuiTableGroup::CGuiTableGroup(const CGuiWidgetParms& parms, int a, int b, bool c)
: CGuiCompoundWidget(parms),
  xc0_(a), xc4_(b),
  xc8_(b), xcc_(b),
  xd0_(c)
{}

CGuiTableGroup* CGuiTableGroup::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);

    int a = in.readInt16Big();
    in.readInt16Big();
    in.readUint32Big();
    int b = in.readInt16Big();
    in.readInt16Big();
    bool c = in.readBool();
    in.readBool();
    in.readFloatBig();
    in.readFloatBig();
    in.readBool();
    in.readFloatBig();
    in.readInt16Big();
    in.readInt16Big();
    in.readInt16Big();
    in.readInt16Big();

    CGuiTableGroup* ret = new CGuiTableGroup(parms, a, b, c);
    ret->ParseBaseInfo(frame, in, parms);
    return ret;
}

}
