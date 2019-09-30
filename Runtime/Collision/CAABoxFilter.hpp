#pragma once

#include "Runtime/Collision/ICollisionFilter.hpp"

namespace urde {
class CCollisionInfoList;

class CAABoxFilter : public ICollisionFilter {
public:
  CAABoxFilter(CActor& actor) : ICollisionFilter(actor) {}
  void Filter(const CCollisionInfoList& in, CCollisionInfoList& out) const override;
  static void FilterBoxFloorCollisions(const CCollisionInfoList& in, CCollisionInfoList& out);
};

} // namespace urde
