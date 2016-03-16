#include "CGuiCompoundWidget.hpp"
#include "CGuiAnimController.hpp"
#include "CGuiLogicalEventTrigger.hpp"

namespace urde
{

CGuiCompoundWidget::CGuiCompoundWidget(const CGuiWidgetParms& parms)
: CGuiWidget(parms)
{
}

void CGuiCompoundWidget::OnInvisible()
{
    CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
    while (child)
    {
        child->SetIsVisible(false);
        child = static_cast<CGuiWidget*>(child->GetNextSibling());
    }
    CGuiWidget::OnInvisible();
}

void CGuiCompoundWidget::OnVisible()
{
    CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
    while (child)
    {
        child->SetIsVisible(true);
        child = static_cast<CGuiWidget*>(child->GetNextSibling());
    }
    CGuiWidget::OnVisible();
}

void CGuiCompoundWidget::OnDeActivate()
{
    CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
    while (child)
    {
        child->SetIsActive(false, false);
        child = static_cast<CGuiWidget*>(child->GetNextSibling());
    }
    CGuiWidget::OnDeActivate();
}

void CGuiCompoundWidget::OnActivate(bool flag)
{
    CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
    while (child)
    {
        child->SetIsActive(true, flag);
        child = static_cast<CGuiWidget*>(child->GetNextSibling());
    }
    CGuiWidget::OnDeActivate();
}

CGuiWidget* CGuiCompoundWidget::GetWorkerWidget(int id)
{
    CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
    while (child)
    {
        if (child->GetWorkerId() == id)
            return child;
        child = static_cast<CGuiWidget*>(child->GetNextSibling());
    }
    return nullptr;
}

}
