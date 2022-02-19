#pragma once

#include <memory>

#include "Runtime/Streams/IOStreams.hpp"
#include "Runtime/Character/IMetaAnim.hpp"

namespace metaforce {

class CMetaAnimFactory {
public:
  static std::shared_ptr<IMetaAnim> CreateMetaAnim(CInputStream& in);
};

} // namespace metaforce
