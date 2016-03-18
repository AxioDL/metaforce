#ifndef __URDE_CGUITEXTPANE_HPP__
#define __URDE_CGUITEXTPANE_HPP__

#include "CGuiPane.hpp"
#include "CGuiTextSupport.hpp"

namespace urde
{

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
