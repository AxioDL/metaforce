#ifndef __URDE_CGUITEXTPANE_HPP__
#define __URDE_CGUITEXTPANE_HPP__

#include "CGuiPane.hpp"

namespace urde
{

enum class EJustification
{
};

enum class EVerticalJustification
{
};

class CGuiTextProperties
{
    bool x0_a;
    bool x1_b;
    bool x2_c;
    EJustification x4_justification;
    EVerticalJustification x8_vertJustification;
public:
    CGuiTextProperties(bool a, bool b, bool c, EJustification justification,
                       EVerticalJustification vertJustification)
        : x0_a(a), x1_b(b), x2_c(c), x4_justification(justification),
          x8_vertJustification(vertJustification) {}
};

class CGuiTextPane : public CGuiPane
{
public:
    CGuiTextPane(const CGuiWidgetParms& parms, float a, float b, const zeus::CVector3f& vec,
                 u32, const CGuiTextProperties& props, const zeus::CColor& col1, const zeus::CColor& col2,
                 int, int);
    static CGuiTextPane* Create(CGuiFrame* frame, CInputStream& in, bool);
};

}

#endif // __URDE_CGUITEXTPANE_HPP__
