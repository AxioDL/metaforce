#pragma once

#include <memory>

#include "Runtime/IOStreams.hpp"
#include "Runtime/Character/IMetaTrans.hpp"

namespace metaforce {

class CMetaTransFactory {
public:
  static std::shared_ptr<IMetaTrans> CreateMetaTrans(CInputStream& in);
};

} // namespace metaforce
