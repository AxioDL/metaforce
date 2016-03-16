#ifndef __URDE_CGUIGROUP_HPP__
#define __URDE_CGUIGROUP_HPP__

#include "CGuiCompoundWidget.hpp"

namespace urde
{

class CGuiGroup : public CGuiCompoundWidget
{
    u32 xf8_workerCount = 0;
    int xfc_selectedWorker;
    bool x100_b;
public:
    CGuiGroup(const CGuiWidgetParms& parms, int defaultWorker, bool b);
    virtual FourCC GetWidgetTypeID() const {return FOURCC('GRUP');}

    void SelectWorkerWidget(int workerId, bool setActive, bool setVisible);
    CGuiWidget* GetSelectedWidget();
    bool AddWorkerWidget(CGuiWidget* worker);
    void OnDeActivate();
    void OnActivate(bool flag);
    bool DoUnregisterEventHandler();
    bool DoRegisterEventHandler();

    static CGuiGroup* Create(CGuiFrame* frame, CInputStream& in, bool flag);
    static void LoadWidgetFnMap();
};

}

#endif // __URDE_CGUIGROUP_HPP__
