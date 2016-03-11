#ifndef __URDE_CGUISLIDERGROUP_HPP__
#define __URDE_CGUISLIDERGROUP_HPP__

#include "CGuiCompoundWidget.hpp"

namespace urde
{

class CGuiSliderGroup : public CGuiCompoundWidget
{
public:
    static void LoadWidgetFnMap();

    bool MAF_Increment(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_Decrement(CGuiFunctionDef* def, CGuiControllerInfo* info);
};

}

#endif // __URDE_CGUISLIDERGROUP_HPP__
