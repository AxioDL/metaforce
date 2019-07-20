#pragma once

#include <locale.hpp>
#include "SplitView.hpp"
#include <hecl/hecl.hpp>

namespace specter {
struct ISpaceController;

struct IViewManager {
public:
  virtual locale::ELocale getTranslatorLocale() const { return locale::ELocale::en_US; }
  template <typename Key, typename... Args>
  constexpr auto translate(Args&&... args) const {
    return locale::Translate<Key>(getTranslatorLocale(), std::forward<Args>(args)...);
  }

  virtual void deferSpaceSplit(ISpaceController* split, SplitView::Axis axis, int thisSlot,
                               const boo::SWindowCoord& coord) {}

  virtual const std::vector<hecl::SystemString>* recentProjects() const { return nullptr; }
  virtual void pushRecentProject(hecl::SystemStringView path) {}

  virtual const std::vector<hecl::SystemString>* recentFiles() const { return nullptr; }
  virtual void pushRecentFile(hecl::SystemStringView path) {}
};

} // namespace specter
