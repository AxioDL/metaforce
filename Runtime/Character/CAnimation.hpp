#pragma once

#include <memory>
#include <string>

#include "Runtime/Streams/IOStreams.hpp"
#include "Runtime/Character/CMetaAnimFactory.hpp"

namespace metaforce {
class IMetaAnim;

class CAnimation {
  std::string x0_name;
  std::shared_ptr<IMetaAnim> x10_anim;

public:
  explicit CAnimation(CInputStream& in);
  const std::shared_ptr<IMetaAnim>& GetMetaAnim() const { return x10_anim; }
  std::string_view GetMetaAnimName() const { return x0_name; }
};

} // namespace metaforce
