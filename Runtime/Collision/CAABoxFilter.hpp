#pragma once

#include "ICollisionFilter.hpp"

namespace urde
{

class CAABoxFilter : public ICollisionFilter
{
public:
    CAABoxFilter(CActor& actor) : ICollisionFilter(actor) {}
    void Filter(const CCollisionInfoList& in, CCollisionInfoList& out) const;
    static void FilterBoxFloorCollisions(const CCollisionInfoList& in, CCollisionInfoList& out);
};

}

