#include "CGuiGroup.hpp"
#include "CGuiAnimController.hpp"
#include "CGuiLogicalEventTrigger.hpp"
#include "CGuiControllerInfo.hpp"

namespace urde
{

void CGuiGroup::LoadWidgetFnMap()
{
}

CGuiGroup::CGuiGroup(const CGuiWidgetParms& parms, int defaultWorker, bool b)
: CGuiCompoundWidget(parms), xfc_selectedWorker(defaultWorker), x100_b(b)
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
                sel->SetIsActive(false, false);
                child->SetIsActive(true, false);
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
    return GetWorkerWidget(xfc_selectedWorker);
}

bool CGuiGroup::AddWorkerWidget(CGuiWidget* worker)
{
    ++xf8_workerCount;
    return true;
}

void CGuiGroup::OnDeActivate()
{
    CGuiWidget* sel = GetSelectedWidget();
    if (sel)
        sel->SetIsActive(false, true);
}

void CGuiGroup::OnActivate(bool flag)
{
    CGuiWidget* sel = GetSelectedWidget();
    if (sel)
        sel->SetIsActive(true, flag);
}

bool CGuiGroup::DoUnregisterEventHandler()
{
    CGuiWidget::DoUnregisterEventHandler();
    GetSelectedWidget()->UnregisterEventHandler(ETraversalMode::Children);
    return true;
}

bool CGuiGroup::DoRegisterEventHandler()
{
    CGuiWidget::DoRegisterEventHandler();
    CGuiWidget* sel = GetSelectedWidget();
    if (GetIsActive() && sel)
    {
        CGuiFuncParm a((intptr_t(sel->GetSelfId())));
        CGuiFuncParm b((intptr_t(3)));
        CGuiFunctionDef def(0, false, a, b);
        CGuiControllerInfo cInfo;
        MAF_SendMessage(&def, &cInfo);
    }
    if (sel)
        sel->RegisterEventHandler(ETraversalMode::Children);
    return true;
}

CGuiGroup* CGuiGroup::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);
    s16 defaultWorker = in.readInt16Big();
    bool b = in.readBool();
    CGuiGroup* ret = new CGuiGroup(parms, defaultWorker, b);
    ret->ParseBaseInfo(frame, in, parms);
    return ret;
}

}
