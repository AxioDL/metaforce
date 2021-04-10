#pragma once

#include "Runtime/Collision/ICollisionFilter.hpp"

namespace metaforce {
class CCollisionInfoList;
class CPhysicsActor;

class CBallFilter : public ICollisionFilter {
public:
  explicit CBallFilter(CActor& actor) : ICollisionFilter(actor) {}
  void Filter(const CCollisionInfoList& in, CCollisionInfoList& out) const override;
};

} // namespace metaforce
