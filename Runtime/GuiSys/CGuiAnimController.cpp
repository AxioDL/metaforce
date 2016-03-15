#include "CGuiAnimController.hpp"
#include "CGuiLogicalEventTrigger.hpp"
#include "CGuiAnimBase.hpp"
#include "CGuiControllerInfo.hpp"

namespace urde
{

void CGuiAnimSet::AddAnim(CGuiAnimBase* anim)
{
    x0_map[x20_++] = anim;
    RecalcTotalTime();
}

void CGuiAnimSet::RecalcTotalTime()
{
    x28_totalTime = 0.f;
    for (std::pair<const int, CGuiAnimBase*>& anim : x0_map)
    {
        float num = anim.second->GetRandomVar().GenNum();
        if (num < x28_totalTime)
            num = x28_totalTime;
        else
            num += anim.second->GetFVal();
        if (num > x28_totalTime)
            x28_totalTime = num;
    }
}

void CGuiAnimSet::Init(CGuiAnimController* controller, float fval)
{
    x19_isDone = false;
    x1c_ = false;
    for (std::pair<const int, CGuiAnimBase*>& anim : x0_map)
    {
        x2c_ = 1.f;
        anim.second->AnimInit(controller, fval);
    }
}

void CGuiAnimSet::Update(CGuiAnimController* controller, bool flag)
{
    x19_isDone = true;
    for (std::pair<const int, CGuiAnimBase*>& anim : x0_map)
    {
        anim.second->AnimUpdate(controller, controller->x164_ * x2c_);
        if (!anim.second->x20_isDone)
            x19_isDone = false;
    }

    if (x19_isDone && !x1c_)
    {
        x1c_ = true;
        CGuiFuncParm a(intptr_t(controller->x168_widget->GetSelfId()));
        CGuiFuncParm b(intptr_t(int(x14_id) + 5));
        CGuiFunctionDef fa(0, false, a, b);
        CGuiControllerInfo cInfo;
        controller->x168_widget->MAF_SendMessage(&fa, &cInfo);
        if (x1b_)
            controller->x168_widget->SetVisibility(false, ETraversalMode::Children);
    }

    if (x19_isDone && x1a_ && flag)
        Init(controller, 0.f);
}

CGuiAnimController::CGuiAnimController(const CGuiWidget::CGuiWidgetParms& parms, CGuiWidget* widget)
: CGuiWidget(parms), x168_widget(widget)
{

}

CGuiAnimSet* CGuiAnimController::FindAnimSet(EGuiAnimBehListID id) const
{
    if (id < EGuiAnimBehListID::Zero || id >= EGuiAnimBehListID::EGuiAnimBehListIDMAX)
        return nullptr;
    return xf8_sets[int(id)].get();
}

void CGuiAnimController::AddAnimation(CGuiAnimBase* anim, EGuiAnimBehListID id)
{
    CGuiAnimSet* set = FindAnimSet(id);
    if (!set)
    {
        xf8_sets[int(id)] = std::make_unique<CGuiAnimSet>(id);
        set = xf8_sets[int(id)].get();
    }
    set->AddAnim(anim);
}

void CGuiAnimController::ResetListUpdateState()
{
    for (std::unique_ptr<CGuiAnimSet>& set : xf8_sets)
        set->x18_update = false;
}

void CGuiAnimController::SetListUpdateState(EGuiAnimBehListID id, bool state)
{
    CGuiAnimSet* set = FindAnimSet(id);
    if (set)
        set->x18_update = false;
}

float CGuiAnimController::GetAnimSetLength(EGuiAnimBehListID id) const
{
    CGuiAnimSet* set = FindAnimSet(id);
    if (set)
        return set->x28_totalTime;
    return 0.f;
}

void CGuiAnimController::IsAnimsDone(EGuiAnimBehListID id, bool& isDone) const
{
    CGuiAnimSet* set = FindAnimSet(id);
    if (set)
        isDone = set->x19_isDone;
    else
        isDone = true;
}

void CGuiAnimController::InitTransform(CGuiWidget* widget, EGuiAnimBehListID id, float fval,
                                       bool flag, EGuiAnimInitMode initMode)
{
    switch (initMode)
    {
    case EGuiAnimInitMode::One:
    case EGuiAnimInitMode::Three:
        FinishAnim();
    default: break;
    }

    x80_transform = widget->GetTransform();

    switch (initMode)
    {
    case EGuiAnimInitMode::Three:
    case EGuiAnimInitMode::Two:
        x16c_ = xb8_;
        break;
    case EGuiAnimInitMode::One:
        x16c_ = zeus::CColor::skClear;
    default: break;
    }

    if (flag)
        ResetListUpdateState();

    CGuiAnimSet* set = FindAnimSet(id);
    if (set)
    {
        set->Init(this, fval);
        set->x18_update = true;
    }

    Update(0.f);
}

void CGuiAnimController::FinishAnim()
{
    float bup = x164_;
    x164_ = 100000.f;
    Step(false);
    x164_ = bup;
}

void CGuiAnimController::Step(bool flag)
{
    x170_ = 0;
    for (std::unique_ptr<CGuiAnimSet>& set : xf8_sets)
        if (set && set->x18_update)
            set->Update(this, flag);
}

}
