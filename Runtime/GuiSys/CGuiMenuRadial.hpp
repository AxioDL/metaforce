#ifndef __URDE_CGUIMENURADIAL_HPP__
#define __URDE_CGUIMENURADIAL_HPP__

#include "CGuiTableGroup.hpp"

namespace urde
{

class CGuiMenuRadial : public CGuiTableGroup
{
public:
    static void LoadWidgetFnMap();

    bool MAF_SelectWithStick(CGuiFunctionDef* def, CGuiControllerInfo* info);
};

}

#endif // __URDE_CGUIMENURADIAL_HPP__
