#ifndef __URDE_CGUICOMPOUNDWIDGET_HPP__
#define __URDE_CGUICOMPOUNDWIDGET_HPP__

#include "CGuiWidget.hpp"

namespace urde
{

class CGuiCompoundWidget : public CGuiWidget
{
public:
    CGuiCompoundWidget(const CGuiWidgetParms& parms);
    FourCC GetWidgetTypeID() const {return FourCC(-1);}

    void OnVisibleChange();
    void OnActiveChange();
    virtual CGuiWidget* GetWorkerWidget(int id);
};

}

#endif // __URDE_CGUICOMPOUNDWIDGET_HPP__
