#ifndef __URDE_CGUITABLEGROUP_HPP__
#define __URDE_CGUITABLEGROUP_HPP__

#include "CGuiCompoundWidget.hpp"

namespace urde
{

class CGuiTableGroup : public CGuiCompoundWidget
{
public:
    static void LoadWidgetFnMap();

    bool MAF_SelectNextColumn(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SelectPrevColumn(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SelectNextRow(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SelectPrevRow(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SelectNextColumnSkipUnActivated(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SelectPrevColumnSkipUnActivated(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SelectNextRowSkipUnActivated(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SelectPrevRowSkipUnActivated(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SelectColumnAtIndex(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_SelectRowAtIndex(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_InitializeTable(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_MenuAdvance(CGuiFunctionDef* def, CGuiControllerInfo* info);
    bool MAF_MenuCancel(CGuiFunctionDef* def, CGuiControllerInfo* info);
};

}

#endif // __URDE_CGUITABLEGROUP_HPP__
