#include "CAuiImagePane.hpp"

namespace urde
{

CAuiImagePane::CAuiImagePane(const CGuiWidget::CGuiWidgetParms& parms, s32, s32,
                             const rstl::reserved_vector<zeus::CVector3f, 4>&,
                             const rstl::reserved_vector<zeus::CVector2f, 4>&, bool)
: CGuiWidget(parms)
{
}

CGuiWidget* CAuiImagePane::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);
    in.readUint32Big();
    in.readUint32Big();
    in.readUint32Big();
    rstl::reserved_vector<zeus::CVector3f, 4> coords;
    u32 coordCount = in.readUint32Big();
    for (u32 i = 0; i < coordCount; ++i)
        coords.push_back(in.readVec3fBig());
    rstl::reserved_vector<zeus::CVector2f, 4> uvs;
    u32 uvCount = in.readUint32Big();
    for (u32 i = 0; i < uvCount; ++i)
        uvs.push_back(in.readVec2fBig());

    CAuiImagePane* ret = new CAuiImagePane(parms, -1, -1, coords, uvs, false);
    ret->ParseBaseInfo(frame, in, parms);
    return ret;
}
}
