#pragma once

namespace metaforce {
class CActor;
class CCollisionInfoList;

class ICollisionFilter {
  CActor& x4_actor;

protected:
  explicit ICollisionFilter(CActor& actor) : x4_actor(actor) {}

public:
  virtual ~ICollisionFilter() = default;
  virtual void Filter(const CCollisionInfoList& in, CCollisionInfoList& out) const = 0;
};

} // namespace metaforce
