#ifndef _CAABOXFILTER
#define _CAABOXFILTER

#include "MetroidPrime/ICollisionFilter.hpp"

class CActor;
class CCollisionInfoList;

class CAABoxFilter : public ICollisionFilter {
public:
  explicit CAABoxFilter(const CActor& actor) : mActor(actor) {}
  static void FilterBoxFloorCollisions(const CCollisionInfoList& in, CCollisionInfoList& out);
  void Filter(const CCollisionInfoList& in, CCollisionInfoList& out) const;

private:
  const CActor& mActor;
};

#endif // _CAABOXFILTER
