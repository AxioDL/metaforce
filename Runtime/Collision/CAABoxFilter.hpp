#pragma once

#include "ICollisionFilter.hpp"

namespace urde {

class CAABoxFilter : public ICollisionFilter {
public:
  CAABoxFilter(CActor& actor) : ICollisionFilter(actor) {}
  void Filter(const CCollisionInfoList& in, CCollisionInfoList& out) const override;
  static void FilterBoxFloorCollisions(const CCollisionInfoList& in, CCollisionInfoList& out);
};

} // namespace urde
