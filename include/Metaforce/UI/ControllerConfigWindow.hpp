#pragma once

#include <borealis/ui/window.hpp>

namespace borealis::ui {
class Pane;
}

namespace metaforce::ui {

struct BindingTarget;

class ControllerConfigWindow : public borealis::ui::Window {
public:
  ControllerConfigWindow();

private:
  enum class Page { Buttons, Triggers, Sticks };

  void RenderPage(borealis::ui::Pane& pane, Page page);
  void AddBinding(borealis::ui::Pane& pane, BindingTarget target);
  static Rml::String BindingLabel(BindingTarget target);
};

} // namespace metaforce::ui
