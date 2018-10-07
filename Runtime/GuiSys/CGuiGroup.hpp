#pragma once

#include "CGuiCompoundWidget.hpp"

namespace urde
{

class CGuiGroup : public CGuiCompoundWidget
{
    u32 xb8_workerCount = 0;
    int xbc_selectedWorker;
    bool xc0_b;
public:
    CGuiGroup(const CGuiWidgetParms& parms, int defaultWorker, bool b);
    FourCC GetWidgetTypeID() const { return FOURCC('GRUP'); }

    void SelectWorkerWidget(int workerId, bool setActive, bool setVisible);
    CGuiWidget* GetSelectedWidget();
    bool AddWorkerWidget(CGuiWidget* worker);
    void OnActiveChange();

    static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);
    static void LoadWidgetFnMap();
};

}

