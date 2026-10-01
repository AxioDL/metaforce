#pragma once

#include <borealis/ui/menu_bar.hpp>

namespace metaforce::ui {

class MenuBar : public borealis::ui::MenuBar {
public:
  MenuBar();

protected:
  void build_tabs() override;
};

} // namespace metaforce::ui
