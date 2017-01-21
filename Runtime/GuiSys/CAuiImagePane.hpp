#ifndef __URDE_CAUIIMAGEPANE_HPP__
#define __URDE_CAUIIMAGEPANE_HPP__

#include "CGuiWidget.hpp"

namespace urde
{

class CAuiImagePane : public CGuiWidget
{
public:
    CAuiImagePane(const CGuiWidgetParms&, s32, s32, const rstl::reserved_vector<zeus::CVector3f, 4>&,
                  const rstl::reserved_vector<zeus::CVector2f, 4>&, bool);

    static CGuiWidget* Create(CGuiFrame *frame, CInputStream &in, bool);
};
}

#endif // __URDE_CAUIIMAGEPANE_HPP__
