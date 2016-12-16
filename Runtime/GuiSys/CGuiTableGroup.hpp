#ifndef __URDE_CGUITABLEGROUP_HPP__
#define __URDE_CGUITABLEGROUP_HPP__

#include "CGuiCompoundWidget.hpp"

namespace urde
{

class CGuiTableGroup : public CGuiCompoundWidget
{
public:
    struct SomeState
    {
        float x0_ = 0.f;
    };
    enum class TableSelectReturn
    {
        Changed,
        Unchanged,
        WrappedAround
    };

private:
    SomeState xb8_;
    SomeState xbc_;
    int xc0_elementCount;
    int xc4_userSelection;
    int xc8_prevUserSelection;
    int xcc_defaultUserSelection;
    bool xd0_selectWraparound;
    bool xd1_ = true;
    std::function<void(const CGuiTableGroup*)> xd4_doMenuAdvance;
    std::function<void(const CGuiTableGroup*)> xec_doMenuCancel;
    std::function<void(const CGuiTableGroup*)> x104_doMenuSelChange;

public:
    CGuiTableGroup(const CGuiWidgetParms& parms, int, int, bool);
    FourCC GetWidgetTypeID() const {return FOURCC('TBGP');}

    void SetMenuAdvanceCallback(std::function<void(const CGuiTableGroup*)>&& cb)
    {
        xd4_doMenuAdvance = std::move(cb);
    }

    void SetMenuCancelCallback(std::function<void(const CGuiTableGroup*)>&& cb)
    {
        xec_doMenuCancel = std::move(cb);
    }

    void SetMenuSelectionChangeCallback(std::function<void(const CGuiTableGroup*)>&& cb)
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

    void SetD1(bool v) { xd1_ = v; }

    void SetUserSelection(int sel)
    {
        xc8_prevUserSelection = xc4_userSelection;
        xc4_userSelection = sel;
    }

    TableSelectReturn DecrementSelectedRow()
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

    TableSelectReturn IncrementSelectedRow()
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

    int GetUserSelection() const { return xc4_userSelection; }

    static CGuiTableGroup* Create(CGuiFrame* frame, CInputStream& in, bool);
};

}

#endif // __URDE_CGUITABLEGROUP_HPP__
