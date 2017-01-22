#include "CGuiGroup.hpp"

namespace urde
{

void CGuiGroup::LoadWidgetFnMap()
{
}

CGuiGroup::CGuiGroup(const CGuiWidgetParms& parms, int defaultWorker, bool b)
: CGuiCompoundWidget(parms), xbc_selectedWorker(defaultWorker), xc0_b(b)
{
}

void CGuiGroup::SelectWorkerWidget(int workerId, bool setActive, bool setVisible)
{
    CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
    while (child)
    {
        if (child->GetWorkerId() == workerId)
        {
            CGuiWidget* sel = GetSelectedWidget();
            if (setActive)
            {
                sel->SetIsActive(false);
                child->SetIsActive(true);
            }
            if (setVisible)
            {
                sel->SetVisibility(false, ETraversalMode::Single);
                child->SetVisibility(true, ETraversalMode::Single);
            }
            break;
        }
        child = static_cast<CGuiWidget*>(child->GetNextSibling());
    }
}

CGuiWidget* CGuiGroup::GetSelectedWidget()
{
    return GetWorkerWidget(xbc_selectedWorker);
}

bool CGuiGroup::AddWorkerWidget(CGuiWidget* worker)
{
    ++xb8_workerCount;
    return true;
}

void CGuiGroup::OnActiveChange()
{
    CGuiWidget* sel = GetSelectedWidget();
    if (sel)
        sel->SetIsActive(true);
}

std::shared_ptr<CGuiWidget> CGuiGroup::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
    s16 defaultWorker = in.readInt16Big();
    bool b = in.readBool();
    std::shared_ptr<CGuiWidget> ret = std::make_shared<CGuiGroup>(parms, defaultWorker, b);
    ret->ParseBaseInfo(frame, in, parms);
    return ret;
}

}
