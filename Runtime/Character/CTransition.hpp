#pragma once

#include <memory>
#include <utility>

#include "Runtime/Streams/IOStreams.hpp"
#include "Runtime/Character/CMetaTransFactory.hpp"

namespace metaforce {

class CTransition {
  u32 x0_id;
  u32 x4_animA;
  u32 x8_animB;
  std::shared_ptr<IMetaTrans> xc_trans;

public:
  explicit CTransition(CInputStream& in);
  u32 GetAnimA() const { return x4_animA; }
  u32 GetAnimB() const { return x8_animB; }
  std::pair<u32, u32> GetAnimPair() const { return {x4_animA, x8_animB}; }
  const std::shared_ptr<IMetaTrans>& GetMetaTrans() const { return xc_trans; }
};

} // namespace metaforce
