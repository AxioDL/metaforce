#include "CGuiWidget.hpp"
#include "CGuiFrame.hpp"
#include "CGuiAnimController.hpp"
#include "CGuiLogicalEventTrigger.hpp"

namespace urde
{
static logvisor::Module Log("urde::CGuiWidget");

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
: x7c_selfId(parms.x6_selfId), x7e_parentId(parms.x8_parentId),
  xbc_color(parms.x10_color), xc0_color2(parms.x10_color),
  xc4_drawFlags(parms.x14_drawFlags), xc8_frame(parms.x0_frame),
  xf6_24_pg(parms.xd_g), xf6_25_pd(parms.xa_d), xf6_26_isActive(parms.xb_defaultActive),
  xf6_27_(true), xf6_28_(false), xf6_29_pf(parms.xc_f), xf6_30_(false),
  xf6_31_(true), xf7_24_(false), xf7_25_(true)
{
    if (parms.x4_a)
        EnsureHasAnimController();
    RecalcWidgetColor(ETraversalMode::NonRecursive);
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
    color.readRGBABig(in);
    EGuiModelDrawFlags df = EGuiModelDrawFlags(in.readUint32Big());

    return CGuiWidget::CGuiWidgetParms(frame, a, selfId, parentId, d, e, f,
                                       color, df, true, flag);
}

CGuiWidget* CGuiWidget::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);
    CGuiWidget* ret = new CGuiWidget(parms);
    ret->ParseBaseInfo(frame, in, parms);
    return ret;
}

void CGuiWidget::ParseBaseInfo(CGuiFrame* frame, CInputStream& in, const CGuiWidgetParms& parms)
{
    CGuiWidget* parent = frame->FindWidget(parms.x8_parentId);
    bool a = in.readBool();
    if (a)
        xf4_workerId = in.readInt16Big();
    zeus::CVector3f trans;
    trans.readBig(in);
    zeus::CMatrix3f orient;
    orient.readBig(in);
    x80_transform = zeus::CTransform(orient, trans);
    ReapplyXform();
    zeus::CVector3f rotCenter;
    rotCenter.readBig(in);
    SetRotationCenter(rotCenter);
    ParseMessages(in, parms);
    ParseAnimations(in, parms);
    if (a)
        if (!parent->AddWorkerWidget(this))
        {
            Log.report(logvisor::Warning,
            "Warning: Discarding useless worker id. Parent is not a compound widget.");
            xf4_workerId = -1;
        }
    parent->AddChildWidget(this, false, true);
}

void CGuiWidget::ParseMessages(CInputStream& in, const CGuiWidgetParms& parms)
{
    s16 count = in.readInt16Big();
    assert(count == 0);
    count = in.readInt16Big();
    assert(count == 0);
}

void CGuiWidget::ParseAnimations(CInputStream& in, const CGuiWidgetParms& parms)
{
    s16 count = in.readInt16Big();
    assert(count == 0);
}

void CGuiWidget::ReapplyXform()
{
    RotateReset();
    SetLocalPosition(zeus::CVector3f::skZero);
    MultiplyO2P(x80_transform);
}

void CGuiWidget::EnsureHasAnimController()
{
    if (!xb0_animController)
    {
        xb0_animController.reset(new CGuiAnimController(
            CGuiWidgetParms(xc8_frame, false, -1, -1, true, false, false,
                            zeus::CColor::skWhite, EGuiModelDrawFlags::Two,
                            true, false), this));
    }
}

std::vector<std::unique_ptr<CGuiLogicalEventTrigger>>* CGuiWidget::FindTriggerList(int id)
{
    auto search = xcc_functionMap.find(id);
    if (search == xcc_functionMap.cend())
        return nullptr;
    return &search->second;
}

void CGuiWidget::AddTrigger(std::unique_ptr<CGuiLogicalEventTrigger>&& trigger)
{
    int tid = trigger->GetTriggerId();
    std::vector<std::unique_ptr<CGuiLogicalEventTrigger>>* list = FindTriggerList(tid);
    if (!list)
    {
        auto it =
        xcc_functionMap.emplace(std::make_pair(tid,
            std::vector<std::unique_ptr<CGuiLogicalEventTrigger>>()));
        list = &it.first->second;
    }
    list->push_back(std::move(trigger));
}

void CGuiWidget::UnregisterEventHandler()
{
    bool flag = DoUnregisterEventHandler();
    if (!flag)
    {
        CGuiWidget* ch = static_cast<CGuiWidget*>(GetChildObject());
        if (ch)
            ch->UnregisterEventHandler();
    }
    CGuiWidget* sib = static_cast<CGuiWidget*>(GetNextSibling());
    if (sib && flag)
        sib->UnregisterEventHandler();
}

void CGuiWidget::UnregisterEventHandler(ETraversalMode mode)
{
    switch (mode)
    {
    case ETraversalMode::Recursive:
        if (!DoUnregisterEventHandler())
        {
            CGuiWidget* ch = static_cast<CGuiWidget*>(GetChildObject());
            if (ch)
                ch->UnregisterEventHandler();
        }
        break;
    case ETraversalMode::NonRecursive:
        DoUnregisterEventHandler();
        break;
    default:
        UnregisterEventHandler();
        break;
    }
}

bool CGuiWidget::DoUnregisterEventHandler()
{
    for (auto& item : xcc_functionMap)
        for (std::unique_ptr<CGuiLogicalEventTrigger>& trigger : item.second)
            xc8_frame->ClearMessageMap(trigger.get(), x7c_selfId);
    return false;
}

void CGuiWidget::RegisterEventHandler()
{
    bool flag = DoRegisterEventHandler();
    if (!flag)
    {
        CGuiWidget* ch = static_cast<CGuiWidget*>(GetChildObject());
        if (ch)
            ch->RegisterEventHandler();
    }
    CGuiWidget* sib = static_cast<CGuiWidget*>(GetNextSibling());
    if (sib && flag)
        sib->RegisterEventHandler();
}

void CGuiWidget::RegisterEventHandler(ETraversalMode mode)
{
    switch (mode)
    {
    case ETraversalMode::Recursive:
        if (!DoRegisterEventHandler())
        {
            CGuiWidget* ch = static_cast<CGuiWidget*>(GetChildObject());
            if (ch)
                ch->RegisterEventHandler();
        }
        break;
    case ETraversalMode::NonRecursive:
        DoRegisterEventHandler();
        break;
    default:
        RegisterEventHandler();
        break;
    }
}

bool CGuiWidget::DoRegisterEventHandler()
{
    if (xf6_28_ || !GetIsActive())
        return false;
    for (auto& item : xcc_functionMap)
        for (std::unique_ptr<CGuiLogicalEventTrigger>& trigger : item.second)
            xc8_frame->AddMessageMap(trigger.get(), x7c_selfId);
    return false;
}

void CGuiWidget::Initialize() {}

void CGuiWidget::InitializeRecursive()
{
    Initialize();
    CGuiWidget* ch = static_cast<CGuiWidget*>(GetChildObject());
    if (ch)
        ch->InitializeRecursive();
    CGuiWidget* sib = static_cast<CGuiWidget*>(GetNextSibling());
    if (sib)
        sib->InitializeRecursive();
}

void CGuiWidget::SetColor(const zeus::CColor& color)
{
    xbc_color = color;
    RecalcWidgetColor(ETraversalMode::Recursive);
}

void CGuiWidget::OnDeActivate() {}
void CGuiWidget::OnActivate(bool) {}
void CGuiWidget::OnInvisible() {}
void CGuiWidget::OnVisible() {}

void CGuiWidget::SetIsActive(bool a, bool b)
{
    if (a == xf6_26_isActive)
        return;
    xf6_26_isActive = a;
    if (a)
    {
        RegisterEventHandler(ETraversalMode::Recursive);
        OnActivate(b);
    }
    else
    {
        RegisterEventHandler(ETraversalMode::Recursive);
        OnDeActivate();
    }
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
