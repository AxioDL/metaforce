#include "CGuiMenuRadial.hpp"

namespace urde
{

typedef bool(CGuiMenuRadial::*FMAF)(CGuiFunctionDef* def, CGuiControllerInfo* info);
static std::unordered_map<u32, FMAF> WidgetFnMap;

void CGuiMenuRadial::LoadWidgetFnMap()
{
    WidgetFnMap.emplace(std::make_pair(34, &CGuiMenuRadial::MAF_SelectWithStick));
}

bool CGuiMenuRadial::MAF_SelectWithStick(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

}
