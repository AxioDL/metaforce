#include "CGuiTextPane.hpp"

namespace urde
{

CGuiTextPane::CGuiTextPane(const CGuiWidgetParms& parms, float a, float b,
                           const zeus::CVector3f& vec, u32 c, const CGuiTextProperties& props,
                           const zeus::CColor& col1, const zeus::CColor& col2,
                           int, int)
: CGuiPane(parms, a, b, vec)
{
}

CGuiTextPane* CGuiTextPane::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);
    float a = in.readFloatBig();
    float b = in.readFloatBig();
    zeus::CVector3f vec;
    vec.read(in);
    u32 c = in.readUint32Big();
    bool d = in.readBool();
    bool e = in.readBool();
    EJustification justification = EJustification(in.readUint32Big());
    EVerticalJustification vJustification = EVerticalJustification(in.readUint32Big());
    CGuiTextProperties props(d, e, 0, justification, vJustification);
    zeus::CColor col1;
    col1.readRGBA(in);
    zeus::CColor col2;
    col2.readRGBA(in);
    int f = in.readFloatBig();
    int g = in.readFloatBig();
    return new CGuiTextPane(parms, a, b, vec, c, props, col1, col2, f, g);
}

}
