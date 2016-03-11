#include "CGuiTableGroup.hpp"

namespace urde
{

typedef bool(CGuiTableGroup::*FMAF)(CGuiFunctionDef* def, CGuiControllerInfo* info);
static std::unordered_map<u32, FMAF> WidgetFnMap;

void CGuiTableGroup::LoadWidgetFnMap()
{
    WidgetFnMap.emplace(std::make_pair(21, &CGuiTableGroup::MAF_SelectNextColumn));
    WidgetFnMap.emplace(std::make_pair(22, &CGuiTableGroup::MAF_SelectPrevColumn));
    WidgetFnMap.emplace(std::make_pair(23, &CGuiTableGroup::MAF_SelectNextRow));
    WidgetFnMap.emplace(std::make_pair(24, &CGuiTableGroup::MAF_SelectPrevRow));
    WidgetFnMap.emplace(std::make_pair(25, &CGuiTableGroup::MAF_SelectNextColumnSkipUnActivated));
    WidgetFnMap.emplace(std::make_pair(26, &CGuiTableGroup::MAF_SelectPrevColumnSkipUnActivated));
    WidgetFnMap.emplace(std::make_pair(27, &CGuiTableGroup::MAF_SelectNextRowSkipUnActivated));
    WidgetFnMap.emplace(std::make_pair(28, &CGuiTableGroup::MAF_SelectPrevRowSkipUnActivated));
    WidgetFnMap.emplace(std::make_pair(29, &CGuiTableGroup::MAF_SelectColumnAtIndex));
    WidgetFnMap.emplace(std::make_pair(30, &CGuiTableGroup::MAF_SelectRowAtIndex));
    WidgetFnMap.emplace(std::make_pair(31, &CGuiTableGroup::MAF_InitializeTable));
    WidgetFnMap.emplace(std::make_pair(32, &CGuiTableGroup::MAF_MenuAdvance));
    WidgetFnMap.emplace(std::make_pair(33, &CGuiTableGroup::MAF_MenuCancel));
}

bool CGuiTableGroup::MAF_SelectNextColumn(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

bool CGuiTableGroup::MAF_SelectPrevColumn(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

bool CGuiTableGroup::MAF_SelectNextRow(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

bool CGuiTableGroup::MAF_SelectPrevRow(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

bool CGuiTableGroup::MAF_SelectNextColumnSkipUnActivated(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

bool CGuiTableGroup::MAF_SelectPrevColumnSkipUnActivated(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

bool CGuiTableGroup::MAF_SelectNextRowSkipUnActivated(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

bool CGuiTableGroup::MAF_SelectPrevRowSkipUnActivated(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

bool CGuiTableGroup::MAF_SelectColumnAtIndex(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

bool CGuiTableGroup::MAF_SelectRowAtIndex(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

bool CGuiTableGroup::MAF_InitializeTable(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

bool CGuiTableGroup::MAF_MenuAdvance(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

bool CGuiTableGroup::MAF_MenuCancel(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

}
