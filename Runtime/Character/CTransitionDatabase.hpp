#pragma once

#include <memory>
#include "Runtime/RetroTypes.hpp"

namespace metaforce {
class IMetaTrans;

class CTransitionDatabase {
public:
  virtual ~CTransitionDatabase() = default;
  virtual const std::shared_ptr<IMetaTrans>& GetMetaTrans(u32, u32) const = 0;
};

} // namespace metaforce
