#pragma once

#include "Translator.hpp"
#include "SplitView.hpp"
#include <hecl/hecl.hpp>

namespace specter {
struct ISpaceController;

struct IViewManager {
public:
  virtual const Translator* getTranslator() const { return nullptr; }
  virtual std::string_view translateOr(std::string_view key, std::string_view vor) const {
    const Translator* trans = getTranslator();
    if (trans)
      return trans->translateOr(key, vor);
    return vor;
  }

  virtual void deferSpaceSplit(ISpaceController* split, SplitView::Axis axis, int thisSlot,
                               const boo::SWindowCoord& coord) {}

  virtual const std::vector<hecl::SystemString>* recentProjects() const { return nullptr; }
  virtual void pushRecentProject(hecl::SystemStringView path) {}

  virtual const std::vector<hecl::SystemString>* recentFiles() const { return nullptr; }
  virtual void pushRecentFile(hecl::SystemStringView path) {}
};

} // namespace specter
