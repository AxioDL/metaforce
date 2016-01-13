#include "Specter/Control.hpp"

namespace Specter
{

Control::Control(ViewResources& res, View& parentView,
                 IControlBinding* controlBinding)
: View(res, parentView), m_controlBinding(controlBinding) {}
    
std::recursive_mutex ITextInputView::m_textInputLk;

}
