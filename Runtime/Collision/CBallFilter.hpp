#ifndef __URDE_CBALLFILTER_HPP__
#define __URDE_CBALLFILTER_HPP__

#include "ICollisionFilter.hpp"

namespace urde
{
class CPhysicsActor;

class CBallFilter : public ICollisionFilter
{
public:
    CBallFilter(CActor& actor) : ICollisionFilter(actor) {}
    void Filter(const CCollisionInfoList& in, CCollisionInfoList& out) const;
};

}

#endif // __URDE_CBALLFILTER_HPP__
