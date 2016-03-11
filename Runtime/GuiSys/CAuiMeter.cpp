#include "CAuiMeter.hpp"

namespace urde
{

typedef bool(CAuiMeter::*FMAF)(CGuiFunctionDef* def, CGuiControllerInfo* info);
static std::unordered_map<u32, FMAF> WidgetFnMap;

void CAuiMeter::LoadWidgetFnMap()
{
}

}
