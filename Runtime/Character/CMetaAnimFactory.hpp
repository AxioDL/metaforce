#pragma once

#include <memory>

#include "Runtime/IOStreams.hpp"
#include "Runtime/Character/IMetaAnim.hpp"

namespace urde {

class CMetaAnimFactory {
public:
  static std::shared_ptr<IMetaAnim> CreateMetaAnim(CInputStream& in);
};

} // namespace urde
