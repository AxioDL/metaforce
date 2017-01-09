#include "CGuiTableGroup.hpp"
#include "Input/CFinalInput.hpp"

namespace urde
{

bool CGuiTableGroup::CRepeatState::Update(float dt, bool state)
{
    if (x0_timer == 0.f)
    {
        if (state)
        {
            x0_timer = 0.6f;
            return true;
        }
    }
    else
    {
        if (state)
        {
            x0_timer -= dt;
            if (x0_timer <= 0.f)
            {
                x0_timer = 0.05f;
                return true;
            }
        }
        else
        {
            x0_timer = 0.f;
        }
    }
    return false;
}

CGuiTableGroup::CGuiTableGroup(const CGuiWidgetParms& parms, int elementCount,
                               int defaultSel, bool selectWraparound)
: CGuiCompoundWidget(parms),
  xc0_elementCount(elementCount), xc4_userSelection(defaultSel),
  xc8_prevUserSelection(defaultSel), xcc_defaultUserSelection(defaultSel),
  xd0_selectWraparound(selectWraparound)
{}

void CGuiTableGroup::ProcessUserInput(const CFinalInput& input)
{
    if (input.PA())
    {
        DoAdvance();
    }
    else if (input.PB())
    {
        DoCancel();
    }
    else
    {
        bool decrement;
        if (xd1_vertical)
            decrement = (input.DLAUp() || input.DDPUp());
        else
            decrement = (input.DLALeft() || input.DDPLeft());

        bool increment;
        if (xd1_vertical)
            increment = (input.DLADown() || input.DDPDown());
        else
            increment = (input.DLARight() || input.DDPRight());

        if (xb8_decRepeat.Update(input.DeltaTime(), decrement) && decrement)
        {
            DoDecrement();
            return;
        }

        if (xbc_incRepeat.Update(input.DeltaTime(), increment) && increment)
        {
            DoIncrement();
            return;
        }
    }
}

bool CGuiTableGroup::IsWorkerSelectable(int idx) const
{
    CGuiWidget* widget = GetWorkerWidget(idx);
    if (widget)
        return widget->GetIsSelectable();
    return false;
}

void CGuiTableGroup::SelectWorker(int idx)
{
    idx = zeus::clamp(0, idx, xc0_elementCount - 1);
    if (idx < xc4_userSelection)
    {
        while (idx != xc4_userSelection)
            DoSelectPrevRow();
    }
    else
    {
        while (idx != xc4_userSelection)
            DoSelectNextRow();
    }
}

void CGuiTableGroup::DeactivateWorker(CGuiWidget* widget)
{
    widget->SetIsActive(false);
}

void CGuiTableGroup::ActivateWorker(CGuiWidget* widget)
{
    widget->SetIsActive(true);
}

CGuiTableGroup::TableSelectReturn CGuiTableGroup::DecrementSelectedRow()
{
    xc8_prevUserSelection = xc4_userSelection;
    --xc4_userSelection;
    if (xc4_userSelection < 0)
    {
        xc4_userSelection = xd0_selectWraparound ? xc0_elementCount - 1 : 0;
        return xd0_selectWraparound ? TableSelectReturn::WrappedAround : TableSelectReturn::Unchanged;
    }
    return TableSelectReturn::Changed;
}

CGuiTableGroup::TableSelectReturn CGuiTableGroup::IncrementSelectedRow()
{
    xc8_prevUserSelection = xc4_userSelection;
    ++xc4_userSelection;
    if (xc4_userSelection >= xc0_elementCount)
    {
        xc4_userSelection = xd0_selectWraparound ? 0 : xc0_elementCount - 1;
        return xd0_selectWraparound ? TableSelectReturn::WrappedAround : TableSelectReturn::Unchanged;
    }
    return TableSelectReturn::Changed;
}

void CGuiTableGroup::DoSelectPrevRow()
{
    DecrementSelectedRow();
    DeactivateWorker(GetWorkerWidget(xc8_prevUserSelection));
    ActivateWorker(GetWorkerWidget(xc4_userSelection));
}

void CGuiTableGroup::DoSelectNextRow()
{
    IncrementSelectedRow();
    DeactivateWorker(GetWorkerWidget(xc8_prevUserSelection));
    ActivateWorker(GetWorkerWidget(xc4_userSelection));
}

void CGuiTableGroup::DoCancel()
{
    if (xec_doMenuCancel)
        xec_doMenuCancel(this);
}

void CGuiTableGroup::DoAdvance()
{
    if (xd4_doMenuAdvance)
        xd4_doMenuAdvance(this);
}

bool CGuiTableGroup::PreDecrement()
{
    if (xd0_selectWraparound)
    {
        for (int sel = (xc4_userSelection + xc0_elementCount - 1) % xc0_elementCount;
             sel != xc4_userSelection;
             sel = (sel + xc0_elementCount - 1) % xc0_elementCount)
        {
            if (IsWorkerSelectable(sel))
            {
                SelectWorker(sel);
                return true;
            }
        }

    }
    else
    {
        for (int sel = std::max(-1, xc4_userSelection - 1) ; sel >= 0 ; --sel)
        {
            if (IsWorkerSelectable(sel))
            {
                SelectWorker(sel);
                return true;
            }
        }
    }

    return false;
}

void CGuiTableGroup::DoDecrement()
{
    if (!PreDecrement())
        return;
    if (x104_doMenuSelChange)
        x104_doMenuSelChange(this, xc4_userSelection);
}

bool CGuiTableGroup::PreIncrement()
{
    if (xd0_selectWraparound)
    {
        for (int sel = (xc4_userSelection + 1) % xc0_elementCount;
             sel != xc4_userSelection;
             sel = (sel + 1) % xc0_elementCount)
        {
            if (IsWorkerSelectable(sel))
            {
                SelectWorker(sel);
                return true;
            }
        }

    }
    else
    {
        for (int sel = std::min(xc0_elementCount, xc4_userSelection + 1) ; sel < xc0_elementCount ; ++sel)
        {
            if (IsWorkerSelectable(sel))
            {
                SelectWorker(sel);
                return true;
            }
        }
    }

    return false;
}

void CGuiTableGroup::DoIncrement()
{
    if (!PreIncrement())
        return;
    if (x104_doMenuSelChange)
        x104_doMenuSelChange(this, xc4_userSelection);
}

CGuiTableGroup* CGuiTableGroup::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);

    int elementCount = in.readInt16Big();
    in.readInt16Big();
    in.readUint32Big();
    int defaultSel = in.readInt16Big();
    in.readInt16Big();
    bool selectWraparound = in.readBool();
    in.readBool();
    in.readFloatBig();
    in.readFloatBig();
    in.readBool();
    in.readFloatBig();
    in.readInt16Big();
    in.readInt16Big();
    in.readInt16Big();
    in.readInt16Big();

    CGuiTableGroup* ret = new CGuiTableGroup(parms, elementCount, defaultSel, selectWraparound);
    ret->ParseBaseInfo(frame, in, parms);
    return ret;
}

}
