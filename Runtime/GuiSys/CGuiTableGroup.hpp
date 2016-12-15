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

private:
    SomeState xb8_;
    SomeState xbc_;
    int xc0_;
    int xc4_;
    int xc8_;
    int xcc_;
    bool xd0_;
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

    void SetD1(bool v) { xd1_ = v; }

    static CGuiTableGroup* Create(CGuiFrame* frame, CInputStream& in, bool);
};

}

#endif // __URDE_CGUITABLEGROUP_HPP__
