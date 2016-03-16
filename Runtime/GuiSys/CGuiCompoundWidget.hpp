#ifndef __URDE_CGUICOMPOUNDWIDGET_HPP__
#define __URDE_CGUICOMPOUNDWIDGET_HPP__

#include "CGuiWidget.hpp"

namespace urde
{

class CGuiCompoundWidget : public CGuiWidget
{
public:
    CGuiCompoundWidget(const CGuiWidgetParms& parms);
    virtual FourCC GetWidgetTypeID() const {return FourCC(-1);}

    void OnInvisible();
    void OnVisible();
    void OnDeActivate();
    void OnActivate(bool);
    virtual CGuiWidget* GetWorkerWidget(int id);
};

}

#endif // __URDE_CGUICOMPOUNDWIDGET_HPP__
