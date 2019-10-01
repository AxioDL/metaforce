#pragma once

namespace urde {
class CActor;
class CCollisionInfoList;

class ICollisionFilter {
  CActor& x4_actor;

protected:
  ICollisionFilter(CActor& actor) : x4_actor(actor) {}

public:
  virtual void Filter(const CCollisionInfoList& in, CCollisionInfoList& out) const = 0;
};

} // namespace urde
