#pragma once

#include "CCollisionInfoList.hpp"

namespace urde
{
class CActor;

class ICollisionFilter
{
    CActor& x4_actor;
protected:
    ICollisionFilter(CActor& actor) : x4_actor(actor) {}
public:
    virtual void Filter(const CCollisionInfoList& in, CCollisionInfoList& out) const = 0;
};

}

