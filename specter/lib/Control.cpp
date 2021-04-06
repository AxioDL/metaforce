#include "specter/Control.hpp"

#include <hecl/CVar.hpp>

namespace specter {

std::string_view CVarControlBinding::name([[maybe_unused]] const Control* control) const { return m_cvar->name(); }

std::string_view CVarControlBinding::help([[maybe_unused]] const Control* control) const { return m_cvar->rawHelp(); }

Control::Control(ViewResources& res, View& parentView, IControlBinding* controlBinding)
: View(res, parentView), m_controlBinding(controlBinding) {}

std::recursive_mutex ITextInputView::m_textInputLk;

} // namespace specter
