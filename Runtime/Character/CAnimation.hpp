#pragma once

#include "IOStreams.hpp"
#include "CMetaAnimFactory.hpp"

namespace urde {
class IMetaAnim;

class CAnimation {
  std::string x0_name;
  std::shared_ptr<IMetaAnim> x10_anim;

public:
  CAnimation(CInputStream& in);
  const std::shared_ptr<IMetaAnim>& GetMetaAnim() const { return x10_anim; }
  std::string_view GetMetaAnimName() const { return x0_name; }
};

} // namespace urde
