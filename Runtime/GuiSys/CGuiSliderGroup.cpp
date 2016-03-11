#include "CGuiSliderGroup.hpp"

namespace urde
{

typedef bool(CGuiSliderGroup::*FMAF)(CGuiFunctionDef* def, CGuiControllerInfo* info);
static std::unordered_map<u32, FMAF> WidgetFnMap;

void CGuiSliderGroup::LoadWidgetFnMap()
{
    WidgetFnMap.emplace(std::make_pair(18, &CGuiSliderGroup::MAF_Increment));
    WidgetFnMap.emplace(std::make_pair(19, &CGuiSliderGroup::MAF_Decrement));
}

bool CGuiSliderGroup::MAF_Increment(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

bool CGuiSliderGroup::MAF_Decrement(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

}
