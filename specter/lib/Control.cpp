#include "Specter/Control.hpp"

namespace Specter
{

Control::Control(ViewResources& res, View& parentView,
                 std::unique_ptr<IControlBinding>&& controlBinding)
: View(res, parentView), m_controlBinding(std::move(controlBinding)) {}

void Control::setControlBinding(std::unique_ptr<IControlBinding>&& controlBinding)
{
    m_controlBinding = std::move(controlBinding);
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

}
