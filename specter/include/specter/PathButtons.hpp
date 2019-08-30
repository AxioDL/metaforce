#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "specter/ScrollView.hpp"

#include <hecl/SystemChar.hpp>

namespace specter {
class ViewResources;

struct IPathButtonsBinding {
  virtual void pathButtonActivated(size_t idx) = 0;
};

class PathButtons : public ScrollView {
  struct ContentView;
  struct PathButton;
  friend struct PathButton;

  ViewChild<std::unique_ptr<ContentView>> m_contentView;
  int m_pathButtonPending = -1;
  IPathButtonsBinding& m_binding;
  bool m_fillContainer;
  std::vector<PathButton> m_pathButtons;

public:
  PathButtons(ViewResources& res, View& parentView, IPathButtonsBinding& binding, bool fillContainer = false);
  ~PathButtons() override;

  void setButtons(const std::vector<hecl::SystemString>& comps);
  void setMultiplyColor(const zeus::CColor& color) override;

  /* Fill all available space in container when requested */
  void containerResized(const boo::SWindowRect& root, const boo::SWindowRect& sub) override;
};

} // namespace specter
