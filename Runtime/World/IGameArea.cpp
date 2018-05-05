#include "IGameArea.hpp"

namespace urde
{
IGameArea::Dock::Dock(urde::CInputStream& in, const zeus::CTransform& xf)
{
    u32 refCount = in.readUint32Big();
    x4_dockReferences.reserve(refCount);
    for (u32 i = 0 ; i < refCount ; i++)
    {
        SDockReference ref;
        ref.x0_area = in.readUint32Big();
        ref.x4_dock = in.readUint32Big();
        x4_dockReferences.push_back(ref);
    }

    u32 vertCount = in.readUint32Big();

    for (u32 i = 0 ; i < vertCount ; i++)
    {
        zeus::CVector3f vert;
        vert.readBig(in);
        x14_planeVertices.push_back(xf * vert);
    }
}

TAreaId IGameArea::Dock::GetConnectedAreaId(s32 other) const
{
    if (u32(other) >= x4_dockReferences.size() || other < 0)
        return kInvalidAreaId;

    return x4_dockReferences[other].x0_area;
}

s16 IGameArea::Dock::GetOtherDockNumber(s32 other) const
{
    if (u32(other) >= x4_dockReferences.size() || other < 0)
        return -1;

    return x4_dockReferences[other].x4_dock;
}

bool IGameArea::Dock::GetShouldLoadOther(s32 other) const
{
    if (other >= x4_dockReferences.size())
        return false;

   return x4_dockReferences[other].x6_loadOther;
}

void IGameArea::Dock::SetShouldLoadOther(s32 other, bool should)
{
    if (other >= x4_dockReferences.size())
        return;

    x4_dockReferences[other].x6_loadOther = should;
}

bool IGameArea::Dock::ShouldLoadOtherArea(s32 other) const
{
    if (x4_dockReferences.size() == 0)
        return false;

    return x4_dockReferences[other].x6_loadOther;
}

zeus::CVector3f IGameArea::Dock::GetPoint(s32 idx) const
{
    if (idx >= x14_planeVertices.size() || idx < 0)
        return zeus::CVector3f();

    return x14_planeVertices[idx];
}
}
