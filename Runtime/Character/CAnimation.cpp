#include "Runtime/Character/CAnimation.hpp"

#include "Runtime/Character/CMetaAnimFactory.hpp"

namespace metaforce {

CAnimation::CAnimation(CInputStream& in) {
  x0_name = in.Get<std::string>();
  x10_anim = CMetaAnimFactory::CreateMetaAnim(in);
}

} // namespace metaforce
