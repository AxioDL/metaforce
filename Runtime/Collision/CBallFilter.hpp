#pragma once

#include "ICollisionFilter.hpp"

namespace urde {
class CPhysicsActor;

class CBallFilter : public ICollisionFilter {
public:
  CBallFilter(CActor& actor) : ICollisionFilter(actor) {}
  void Filter(const CCollisionInfoList& in, CCollisionInfoList& out) const;
};

} // namespace urde
