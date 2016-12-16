#include "CGuiTableGroup.hpp"

namespace urde
{

CGuiTableGroup::CGuiTableGroup(const CGuiWidgetParms& parms, int elementCount,
                               int defaultSel, bool selectWraparound)
: CGuiCompoundWidget(parms),
  xc0_elementCount(elementCount), xc4_userSelection(defaultSel),
  xc8_prevUserSelection(defaultSel), xcc_defaultUserSelection(defaultSel),
  xd0_selectWraparound(selectWraparound)
{}

CGuiTableGroup* CGuiTableGroup::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);

    int elementCount = in.readInt16Big();
    in.readInt16Big();
    in.readUint32Big();
    int defaultSel = in.readInt16Big();
    in.readInt16Big();
    bool selectWraparound = in.readBool();
    in.readBool();
    in.readFloatBig();
    in.readFloatBig();
    in.readBool();
    in.readFloatBig();
    in.readInt16Big();
    in.readInt16Big();
    in.readInt16Big();
    in.readInt16Big();

    CGuiTableGroup* ret = new CGuiTableGroup(parms, elementCount, defaultSel, selectWraparound);
    ret->ParseBaseInfo(frame, in, parms);
    return ret;
}

}
