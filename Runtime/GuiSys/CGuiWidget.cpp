#include "CGuiWidget.hpp"
#include "CGuiFrame.hpp"

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

CGuiWidget::CGuiWidget(const CGuiWidgetParms& parms)
{
}

CGuiWidget::CGuiWidgetParms
CGuiWidget::ReadWidgetHeader(CGuiFrame* frame, CInputStream& in, bool flag)
{
    std::string name = in.readString(-1);
    s16 selfId = frame->GetWidgetIdDB().AddWidget(name);
    std::string parent = in.readString(-1);
    s16 parentId = frame->GetWidgetIdDB().AddWidget(parent);

    bool a = in.readBool();
    bool d = in.readBool();
    bool e = in.readBool();
    bool f = in.readBool();
    zeus::CColor color;
    color.readRGBA(in);
    EGuiModelDrawFlags df = EGuiModelDrawFlags(in.readUint32Big());

    return CGuiWidget::CGuiWidgetParms(frame, a, selfId, parentId, d, e, f,
                                       color, df, true, flag);
}

CGuiWidget* CGuiWidget::Create(CGuiFrame* frame, CInputStream& in, bool)
{
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
