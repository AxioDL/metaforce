#pragma once

#include <cstddef>
#include <string>

namespace boo {
struct ITexture;
struct SWindowCoord;
} // namespace boo

namespace specter {

struct IMenuNode {
  virtual ~IMenuNode() = default;
  virtual boo::ITexture* icon() const { return nullptr; }
  virtual const std::string* text() const { return nullptr; }
  virtual size_t subNodeCount() const { return 0; }
  virtual IMenuNode* subNode(size_t idx) { return nullptr; }
  virtual void activated(const boo::SWindowCoord& coord) {}
};

} // namespace specter
