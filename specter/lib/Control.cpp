#include "Specter/Control.hpp"

namespace Specter
{

Control::Control(ViewResources& res, View& parentView,
                 IControlBinding* controlBinding)
: View(res, parentView), m_controlBinding(controlBinding) {}

IControlBinding* Control::setControlBinding(IControlBinding* controlBinding)
{
    IControlBinding* ret = m_controlBinding;
    m_controlBinding = controlBinding;
    return ret;
}

void Control::mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey)
{
}

void Control::mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey)
{
}

void Control::mouseEnter(const boo::SWindowCoord&)
{
}

void Control::mouseLeave(const boo::SWindowCoord&)
{
}
    
std::recursive_mutex ITextInputView::m_textInputLk;

}
