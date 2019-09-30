#pragma once

#include "Runtime/Collision/ICollisionFilter.hpp"

namespace urde {
class CCollisionInfoList;
class CPhysicsActor;

class CBallFilter : public ICollisionFilter {
public:
  CBallFilter(CActor& actor) : ICollisionFilter(actor) {}
  void Filter(const CCollisionInfoList& in, CCollisionInfoList& out) const override;
};

} // namespace urde
