#include "CGuiWidget.hpp"

namespace urde
{

typedef bool(CGuiWidget::*FMAF)(CGuiFunctionDef* def, CGuiControllerInfo* info);
static std::unordered_map<u32, FMAF> WidgetFnMap;

void CGuiWidget::LoadWidgetFnMap()
{
    WidgetFnMap.emplace(std::make_pair(2, &CGuiWidget::MAF_StartAnimationSet));
    WidgetFnMap.emplace(std::make_pair(3, &CGuiWidget::MAF_SendMessage));
    WidgetFnMap.emplace(std::make_pair(6, &CGuiWidget::MAF_PauseAnim));
    WidgetFnMap.emplace(std::make_pair(7, &CGuiWidget::MAF_ResumeAnim));
    WidgetFnMap.emplace(std::make_pair(11, &CGuiWidget::MAF_SetState));
    WidgetFnMap.emplace(std::make_pair(12, &CGuiWidget::MAF_SetStateOfWidget));
}

bool CGuiWidget::MAF_StartAnimationSet(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

bool CGuiWidget::MAF_SendMessage(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

bool CGuiWidget::MAF_PauseAnim(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

bool CGuiWidget::MAF_ResumeAnim(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

bool CGuiWidget::MAF_SetState(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

bool CGuiWidget::MAF_SetStateOfWidget(CGuiFunctionDef* def, CGuiControllerInfo* info)
{
}

}
