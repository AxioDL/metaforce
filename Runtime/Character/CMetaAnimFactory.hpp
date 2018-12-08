#pragma once

#include "IOStreams.hpp"
#include "IMetaAnim.hpp"

namespace urde {

class CMetaAnimFactory {
public:
  static std::shared_ptr<IMetaAnim> CreateMetaAnim(CInputStream& in);
};

} // namespace urde
