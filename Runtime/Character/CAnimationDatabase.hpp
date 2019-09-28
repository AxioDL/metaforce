#pragma once

#include <set>
#include <string_view>
#include <vector>

#include "Runtime/RetroTypes.hpp"

namespace urde {
class CPrimitive;
class IMetaAnim;

class CAnimationDatabase {
public:
  virtual const std::shared_ptr<IMetaAnim>& GetMetaAnim(s32) const = 0;
  virtual u32 GetNumMetaAnims() const = 0;
  virtual const char* GetMetaAnimName(s32) const = 0;
  virtual void GetAllUniquePrimitives(std::vector<CPrimitive>&) const = 0;
  virtual void GetUniquePrimitivesFromMetaAnim(std::set<CPrimitive>&, std::string_view) const = 0;
};

} // namespace urde
