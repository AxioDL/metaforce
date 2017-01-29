#include "CGuiCompoundWidget.hpp"

namespace urde
{

CGuiCompoundWidget::CGuiCompoundWidget(const CGuiWidgetParms& parms)
: CGuiWidget(parms)
{
}

void CGuiCompoundWidget::OnVisibleChange()
{
    CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
    while (child)
    {
        child->SetIsVisible(GetIsVisible());
        child = static_cast<CGuiWidget*>(child->GetNextSibling());
    }
    CGuiWidget::OnVisibleChange();
}

void CGuiCompoundWidget::OnActiveChange()
{
    CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
    while (child)
    {
        child->SetIsActive(GetIsActive());
        child = static_cast<CGuiWidget*>(child->GetNextSibling());
    }
    CGuiWidget::OnActiveChange();
}

CGuiWidget* CGuiCompoundWidget::GetWorkerWidget(int id) const
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
