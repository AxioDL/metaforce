#pragma once

#include <memory>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/IOStreams.hpp"
#include "Runtime/Character/IMetaTrans.hpp"

namespace urde {

class CHalfTransition {
  u32 x0_id;
  std::shared_ptr<IMetaTrans> x4_trans;

public:
  CHalfTransition(CInputStream& in);
  u32 GetId() const { return x0_id; }
  const std::shared_ptr<IMetaTrans>& GetMetaTrans() const { return x4_trans; }
};

} // namespace urde
