#include "CGuiPane.hpp"

namespace urde
{

CGuiPane::CGuiPane(const CGuiWidgetParms& parms, float a, float b, const zeus::CVector3f& vec)
: CGuiWidget(parms), xf8_a(a), xfc_b(b), x108_vec(vec)
{
    InitializeBuffers();
}

CGuiPane* CGuiPane::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);
    float a = in.readFloatBig();
    float b = in.readFloatBig();
    zeus::CVector3f vec;
    vec.read(in);
    return new CGuiPane(parms, a, b, vec);
}

}
