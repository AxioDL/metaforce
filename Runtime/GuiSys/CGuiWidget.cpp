#include "CGuiWidget.hpp"
#include "CGuiFrame.hpp"
#include "CGuiAnimController.hpp"
#include "CGuiLogicalEventTrigger.hpp"
#include "CGuiMessage.hpp"

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
  xf6_24_pg(parms.xd_g), xf6_25_isVisible(parms.xa_defaultVisible),
  xf6_26_isActive(parms.xb_defaultActive),
  xf6_27_(true), xf6_28_eventLock(false),
  xf6_29_cullFaces(parms.xc_cullFaces), xf6_30_(false),
  xf6_31_(true), xf7_24_(false), xf7_25_(true)
{
    if (parms.x4_useAnimController)
        EnsureHasAnimController();
    RecalcWidgetColor(ETraversalMode::Single);
}

CGuiWidget::CGuiWidgetParms
CGuiWidget::ReadWidgetHeader(CGuiFrame* frame, CInputStream& in, bool flag)
{
    std::string name = in.readString(-1);
    s16 selfId = frame->GetWidgetIdDB().AddWidget(name);
    std::string parent = in.readString(-1);
    s16 parentId = frame->GetWidgetIdDB().AddWidget(parent);

    bool useAnimController = in.readBool();
    bool defaultVis = in.readBool();
    bool defaultActive = in.readBool();
    bool cullFaces = in.readBool();
    zeus::CColor color;
    color.readRGBABig(in);
    EGuiModelDrawFlags df = EGuiModelDrawFlags(in.readUint32Big());

    return CGuiWidget::CGuiWidgetParms(frame, useAnimController, selfId,
                                       parentId, defaultVis, defaultActive,
                                       cullFaces, color, df, true, flag);
}

CGuiWidget* CGuiWidget::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);
    CGuiWidget* ret = new CGuiWidget(parms);
    ret->ParseBaseInfo(frame, in, parms);
    return ret;
}

bool CGuiWidget::Message(const CGuiMessage& msg)
{
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

std::vector<TResId> CGuiWidget::GetTextureAssets() const
{
    return {};
}

std::vector<TResId> CGuiWidget::GetModelAssets() const
{
    return {};
}

std::vector<TResId> CGuiWidget::GetFontAssets() const
{
    return {};
}

void CGuiWidget::Initialize() {}
void CGuiWidget::Touch() const {}

bool CGuiWidget::GetIsVisible() const
{
    return xf6_25_isVisible;
}

bool CGuiWidget::GetIsActive() const
{
    return xf6_26_isActive;
}

CGuiTextSupport* CGuiWidget::TextSupport()
{
    return nullptr;
}

CGuiTextSupport* CGuiWidget::GetTextSupport() const
{
    return nullptr;
}

void CGuiWidget::ModifyRGBA(CGuiWidget* widget)
{
    xb8_ += widget->xb8_;
    xb4_ = xb8_ + xc0_color2;
}

void CGuiWidget::RecalculateAllRGBA()
{
    CGuiWidget* parent = static_cast<CGuiWidget*>(GetParent());
    if (parent)
        ModifyRGBA(parent);
    CGuiWidget* nextSib = static_cast<CGuiWidget*>(GetNextSibling());
    if (nextSib)
        nextSib->RecalculateAllRGBA();
    CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
    if (child)
        child->RecalculateAllRGBA();
}

void CGuiWidget::InitializeRGBAFactor()
{
    CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
    if (child)
        child->InitializeRGBAFactor();
    CGuiWidget* nextSib = static_cast<CGuiWidget*>(GetNextSibling());
    if (nextSib)
        nextSib->InitializeRGBAFactor();
}

bool CGuiWidget::GetIsFinishedLoadingWidgetSpecific() const
{
    return true;
}

std::vector<std::unique_ptr<CGuiLogicalEventTrigger>>* CGuiWidget::FindTriggerList(int id)
{
    auto search = xcc_triggerMap.find(id);
    if (search == xcc_triggerMap.cend())
        return nullptr;
    return search->second.get();
}

void CGuiWidget::AddTrigger(std::unique_ptr<CGuiLogicalEventTrigger>&& trigger)
{
    int tid = trigger->GetTriggerId();
    std::vector<std::unique_ptr<CGuiLogicalEventTrigger>>* list = FindTriggerList(tid);
    if (!list)
    {
        auto it =
        xcc_triggerMap.emplace(std::make_pair(tid,
            std::make_unique<std::vector<std::unique_ptr<CGuiLogicalEventTrigger>>>()));
        list = it.first->second.get();
    }
    list->push_back(std::move(trigger));
}

std::vector<std::unique_ptr<CGuiFunctionDef>>* CGuiWidget::FindFunctionDefList(int id)
{
    auto search = xe0_functionMap.find(id);
    if (search == xe0_functionMap.cend())
        return nullptr;
    return search->second.get();
}

void CGuiWidget::AddFunctionDef(s32 id, std::unique_ptr<CGuiFunctionDef>&& def)
{
    std::vector<std::unique_ptr<CGuiFunctionDef>>* list = FindFunctionDefList(id);
    if (!list)
    {
        auto it =
        xe0_functionMap.emplace(std::make_pair(id,
            std::make_unique<std::vector<std::unique_ptr<CGuiFunctionDef>>>()));
        list = it.first->second.get();
    }
    list->push_back(std::move(def));
}

void CGuiWidget::SetIdlePosition(const zeus::CVector3f& pos, bool reapply)
{
    x80_transform.m_origin = pos;
    if (reapply)
        ReapplyXform();
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

void CGuiWidget::BroadcastMessage(int id, CGuiControllerInfo* info)
{
    CGuiFuncParm a((intptr_t(x7c_selfId)));
    CGuiFuncParm b((intptr_t(id)));
    CGuiFunctionDef def(0, false, a, b);
    MAF_SendMessage(&def, info);

    CGuiWidget* ch = static_cast<CGuiWidget*>(GetChildObject());
    if (ch)
        ch->BroadcastMessage(id, info);

    CGuiWidget* sib = static_cast<CGuiWidget*>(GetNextSibling());
    if (sib)
        sib->BroadcastMessage(id, info);
}

void CGuiWidget::LockEvents(bool lock)
{
    xf6_28_eventLock = lock;
    if (lock)
        DoUnregisterEventHandler();
    else
        DoRegisterEventHandler();
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
    case ETraversalMode::Children:
        if (!DoUnregisterEventHandler())
        {
            CGuiWidget* ch = static_cast<CGuiWidget*>(GetChildObject());
            if (ch)
                ch->UnregisterEventHandler();
        }
        break;
    case ETraversalMode::Single:
        DoUnregisterEventHandler();
        break;
    default:
        UnregisterEventHandler();
        break;
    }
}

bool CGuiWidget::DoUnregisterEventHandler()
{
    for (auto& item : xcc_triggerMap)
        for (std::unique_ptr<CGuiLogicalEventTrigger>& trigger : *item.second)
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
    case ETraversalMode::Children:
        if (!DoRegisterEventHandler())
        {
            CGuiWidget* ch = static_cast<CGuiWidget*>(GetChildObject());
            if (ch)
                ch->RegisterEventHandler();
        }
        break;
    case ETraversalMode::Single:
        DoRegisterEventHandler();
        break;
    default:
        RegisterEventHandler();
        break;
    }
}

bool CGuiWidget::DoRegisterEventHandler()
{
    if (xf6_28_eventLock || !GetIsActive())
        return false;
    for (auto& item : xcc_triggerMap)
        for (std::unique_ptr<CGuiLogicalEventTrigger>& trigger : *item.second)
            xc8_frame->AddMessageMap(trigger.get(), x7c_selfId);
    return false;
}

CGuiWidget* CGuiWidget::RemoveChildWidget(CGuiWidget* widget, bool makeWorldLocal)
{
    return static_cast<CGuiWidget*>(RemoveChildObject(widget, makeWorldLocal));
}

void CGuiWidget::AddChildWidget(CGuiWidget* widget, bool makeWorldLocal, bool atEnd)
{
    AddChildObject(widget, makeWorldLocal, atEnd);
}

bool CGuiWidget::AddWorkerWidget(CGuiWidget* worker)
{
    return false;
}

void CGuiWidget::AddAnim(EGuiAnimBehListID id, CGuiAnimBase* anim)
{
    if (!xb0_animController)
    {
        xb0_animController.reset(new CGuiAnimController(
            CGuiWidgetParms(xc8_frame, false, -1, -1, false, false, false,
                            zeus::CColor::skWhite, EGuiModelDrawFlags::Two, true, false), this));
    }
    xb0_animController->AddAnimation(anim, id);
}

void CGuiWidget::ResetAllAnimUpdateState()
{
    if (xb0_animController)
        xb0_animController->ResetListUpdateState();
    CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
    if (child)
        child->ResetAllAnimUpdateState();
    CGuiWidget* nextSib = static_cast<CGuiWidget*>(GetNextSibling());
    if (nextSib)
        nextSib->ResetAllAnimUpdateState();
}

void CGuiWidget::SetVisibility(bool vis, ETraversalMode mode)
{
    switch (mode)
    {
    case ETraversalMode::Children:
    {
        CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
        if (child)
            child->SetVisibility(vis, ETraversalMode::ChildrenAndSiblings);
        break;
    }
    case ETraversalMode::ChildrenAndSiblings:
    {
        CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
        if (child)
            child->SetVisibility(vis, ETraversalMode::ChildrenAndSiblings);
        CGuiWidget* nextSib = static_cast<CGuiWidget*>(GetNextSibling());
        if (nextSib)
            nextSib->SetVisibility(vis, ETraversalMode::ChildrenAndSiblings);
        break;
    }
    default: break;
    }
    SetIsVisible(vis);
}

void CGuiWidget::SetAnimUpdateState(EGuiAnimBehListID id, bool state)
{
    if (xb0_animController)
        xb0_animController->SetListUpdateState(id, state);
    CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
    if (child)
        child->SetAnimUpdateState(id, state);
    CGuiWidget* nextSib = static_cast<CGuiWidget*>(GetNextSibling());
    if (nextSib)
        nextSib->SetAnimUpdateState(id, state);
}

void CGuiWidget::SetAnimUpdateState(EGuiAnimBehListID id, bool state, ETraversalMode mode)
{
    switch (mode)
    {
    case ETraversalMode::Children:
    {
        if (xb0_animController)
            xb0_animController->SetListUpdateState(id, state);
        CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
        if (child)
            child->SetAnimUpdateState(id, state);
        break;
    }
    case ETraversalMode::Single:
    {
        if (xb0_animController)
            xb0_animController->SetListUpdateState(id, state);
        break;
    }
    default:
        SetAnimUpdateState(id, state);
        break;
    }
}

void CGuiWidget::GetBranchAnimLen(EGuiAnimBehListID id, float& len)
{
    if (xb0_animController)
    {
        float aLen = xb0_animController->GetAnimSetLength(id);
        if (aLen > len)
            len = aLen;
    }
    CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
    if (child)
        child->GetBranchAnimLen(id, len);
    CGuiWidget* nextSib = static_cast<CGuiWidget*>(GetNextSibling());
    if (nextSib)
        nextSib->GetBranchAnimLen(id, len);
}

void CGuiWidget::GetBranchAnimLen(EGuiAnimBehListID id, float& len, ETraversalMode mode)
{
    switch (mode)
    {
    case ETraversalMode::Children:
    {
        if (xb0_animController)
        {
            float aLen = xb0_animController->GetAnimSetLength(id);
            if (aLen > len)
                len = aLen;
        }
        CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
        if (child)
            child->GetBranchAnimLen(id, len);
        break;
    }
    case ETraversalMode::Single:
    {
        if (xb0_animController)
        {
            float aLen = xb0_animController->GetAnimSetLength(id);
            if (aLen > len)
                len = aLen;
        }
        break;
    }
    default:
        GetBranchAnimLen(id, len);
        break;
    }
}

void CGuiWidget::IsAllAnimsDone(EGuiAnimBehListID id, bool& isDone)
{
    if (xb0_animController)
    {
        if (!isDone)
            return;
        xb0_animController->IsAnimsDone(id, isDone);
    }
    CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
    if (child)
        child->IsAllAnimsDone(id, isDone);
    CGuiWidget* nextSib = static_cast<CGuiWidget*>(GetNextSibling());
    if (nextSib)
        nextSib->IsAllAnimsDone(id, isDone);
}

void CGuiWidget::IsAllAnimsDone(EGuiAnimBehListID id, bool& isDone, ETraversalMode mode)
{
    switch (mode)
    {
    case ETraversalMode::Children:
    {
        if (xb0_animController)
        {
            xb0_animController->IsAnimsDone(id, isDone);
            if (!isDone)
                return;
        }
        CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
        if (child)
            child->IsAllAnimsDone(id, isDone);
        break;
    }
    case ETraversalMode::Single:
    {
        if (xb0_animController)
            xb0_animController->IsAnimsDone(id, isDone);
        break;
    }
    default:
        IsAllAnimsDone(id, isDone);
        break;
    }
}

void CGuiWidget::InitializeAnimControllers(EGuiAnimBehListID id, float fval, bool flag,
                                           EGuiAnimInitMode initMode)
{
    if (xb0_animController)
        xb0_animController->InitTransform(this, id, fval, flag, initMode);
    CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
    if (child)
        child->InitializeAnimControllers(id, fval, flag, initMode);
    CGuiWidget* nextSib = static_cast<CGuiWidget*>(GetNextSibling());
    if (nextSib)
        nextSib->InitializeAnimControllers(id, fval, flag, initMode);
}

void CGuiWidget::InitializeAnimControllers(EGuiAnimBehListID id, float fval, bool flag,
                                           EGuiAnimInitMode initMode, ETraversalMode mode)
{
    switch (mode)
    {
    case ETraversalMode::Children:
    {
        if (xb0_animController)
            xb0_animController->InitTransform(this, id, fval, flag, initMode);
        CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
        if (child)
            child->InitializeAnimControllers(id, fval, flag, initMode);
        break;
    }
    case ETraversalMode::Single:
    {
        if (xb0_animController)
            xb0_animController->InitTransform(this, id, fval, flag, initMode);
        break;
    }
    default:
        InitializeAnimControllers(id, fval, flag, initMode);
        break;
    }
}

void CGuiWidget::RecalcWidgetColor(ETraversalMode mode)
{
    CGuiWidget* parent = static_cast<CGuiWidget*>(GetParent());
    if (parent)
        xc0_color2 = xbc_color * parent->xc0_color2;
    else
        xc0_color2 = xbc_color;
    xb4_ = xb8_ + xc0_color2;

    switch (mode)
    {
    case ETraversalMode::ChildrenAndSiblings:
    {
        CGuiWidget* nextSib = static_cast<CGuiWidget*>(GetNextSibling());
        if (nextSib)
            nextSib->RecalcWidgetColor(ETraversalMode::ChildrenAndSiblings);
    }
    case ETraversalMode::Children:
    {
        CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
        if (child)
            child->RecalcWidgetColor(ETraversalMode::ChildrenAndSiblings);
    }
    default: break;
    }
}

CGuiWidget* CGuiWidget::FindWidget(s16 id)
{
    if (x7c_selfId == id)
        return this;
    CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
    if (child)
    {
        CGuiWidget* found = child->FindWidget(id);
        if (found)
            return found;
    }
    CGuiWidget* nextSib = static_cast<CGuiWidget*>(GetNextSibling());
    if (nextSib)
    {
        CGuiWidget* found = nextSib->FindWidget(id);
        if (found)
            return found;
    }
    return nullptr;
}

bool CGuiWidget::GetIsFinishedLoading() const
{
    bool widgetFinished = GetIsFinishedLoadingWidgetSpecific();
    if (!xb0_animController)
        return widgetFinished;
    return widgetFinished && xb0_animController->GetIsFinishedLoading();
}

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
    RecalcWidgetColor(ETraversalMode::Children);
}

void CGuiWidget::OnDeActivate() {}
void CGuiWidget::OnActivate(bool) {}
void CGuiWidget::OnInvisible() {}
void CGuiWidget::OnVisible() {}

void CGuiWidget::SetIsVisible(bool vis)
{
    xf6_25_isVisible = vis;
    if (vis)
        OnVisible();
    else
        OnInvisible();
}

void CGuiWidget::SetIsActive(bool a, bool b)
{
    if (a == xf6_26_isActive)
        return;
    xf6_26_isActive = a;
    if (a)
    {
        RegisterEventHandler(ETraversalMode::Children);
        OnActivate(b);
    }
    else
    {
        RegisterEventHandler(ETraversalMode::Children);
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
