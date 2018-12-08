#pragma once

#include "../RetroTypes.hpp"
#include <vector>
#include <set>
#include <string>

namespace urde {
class IMetaAnim;
class CPrimitive;

class CAnimationDatabase {
public:
  virtual const std::shared_ptr<IMetaAnim>& GetMetaAnim(u32) const = 0;
  virtual u32 GetNumMetaAnims() const = 0;
  virtual const char* GetMetaAnimName(u32) const = 0;
  virtual void GetAllUniquePrimitives(std::vector<CPrimitive>&) const = 0;
  virtual void GetUniquePrimitivesFromMetaAnim(std::set<CPrimitive>&, std::string_view) const = 0;
};

} // namespace urde
