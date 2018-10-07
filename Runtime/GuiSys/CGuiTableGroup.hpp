#pragma once

#include "CGuiCompoundWidget.hpp"

namespace urde
{

class CGuiTableGroup : public CGuiCompoundWidget
{
public:
    class CRepeatState
    {
        float x0_timer = 0.f;
    public:
        bool Update(float dt, bool state);
    };

    enum class TableSelectReturn
    {
        Changed,
        Unchanged,
        WrappedAround
    };

private:
    CRepeatState xb8_decRepeat;
    CRepeatState xbc_incRepeat;
    int xc0_elementCount;
    int xc4_userSelection;
    int xc8_prevUserSelection;
    int xcc_defaultUserSelection;
    bool xd0_selectWraparound;
    bool xd1_vertical = true;
    std::function<void(CGuiTableGroup*)> xd4_doMenuAdvance;
    std::function<void(CGuiTableGroup*)> xec_doMenuCancel;
    std::function<void(CGuiTableGroup*, int)> x104_doMenuSelChange;

    bool IsWorkerSelectable(int) const;
    void SelectWorker(int);
    void DeactivateWorker(CGuiWidget* widget);
    void ActivateWorker(CGuiWidget* widget);

    TableSelectReturn DecrementSelectedRow();
    TableSelectReturn IncrementSelectedRow();
    void DoSelectPrevRow();
    void DoSelectNextRow();

    void DoCancel();
    void DoAdvance();
    bool PreDecrement();
    void DoDecrement();
    bool PreIncrement();
    void DoIncrement();

public:
    CGuiTableGroup(const CGuiWidgetParms& parms, int, int, bool);
    FourCC GetWidgetTypeID() const { return FOURCC('TBGP'); }

    void SetMenuAdvanceCallback(std::function<void(CGuiTableGroup*)>&& cb)
    {
        xd4_doMenuAdvance = std::move(cb);
    }

    void SetMenuCancelCallback(std::function<void(CGuiTableGroup*)>&& cb)
    {
        xec_doMenuCancel = std::move(cb);
    }

    void SetMenuSelectionChangeCallback(std::function<void(CGuiTableGroup*, int)>&& cb)
    {
        x104_doMenuSelChange = std::move(cb);
    }

    void SetColors(const zeus::CColor& selected, const zeus::CColor& unselected)
    {
        int id = -1;
        while (CGuiWidget* worker = GetWorkerWidget(++id))
        {
            if (id == xc4_userSelection)
                worker->SetColor(selected);
            else
                worker->SetColor(unselected);
        }
    }

    void SetVertical(bool v) { xd1_vertical = v; }

    void SetUserSelection(int sel)
    {
        xc8_prevUserSelection = xc4_userSelection;
        xc4_userSelection = sel;
    }

    int GetElementCount() const { return xc0_elementCount; }

    int GetUserSelection() const { return xc4_userSelection; }

    void ProcessUserInput(const CFinalInput& input);

    bool AddWorkerWidget(CGuiWidget* worker) { return true; }

    static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);
};

}

