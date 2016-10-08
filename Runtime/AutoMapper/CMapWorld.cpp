#include "CMapWorld.hpp"

namespace urde
{

CMapWorld::CMapWorld(CInputStream& in)
{
}

u32 CMapWorld::GetNumAreas() const
{
    return x0_areas.size();
}

void CMapWorld::GetLoadedMapArea(s32) const
{
}

void CMapWorld::GetMapArea(s32) const
{

}

void CMapWorld::IsMapAreaInBFSInfoVector(const CMapWorld::CMapAreaData *, const std::vector<CMapWorld::CMapAreaBFSInfo> &) const
{

}

void CMapWorld::SetWhichMapAreasLoaded(const IWorld &, int start, int count)
{

}

bool CMapWorld::IsMapAreasStreaming() const
{
    return false;
}

void CMapWorld::MoveMapAreaToList(CMapWorld::CMapAreaData *, CMapWorld::EMapAreaList)
{

}

void CMapWorld::GetCurrentMapAreaDepth(const IWorld &, int) const
{

}

void CMapWorld::GetVisibleAreas(const IWorld &, const CMapWorldInfo &) const
{

}

void CMapWorld::Draw(const CMapWorld::CMapWorldDrawParms &, int, int, float, float, bool) const
{

}

void CMapWorld::DoBFS(const IWorld &, int, int, float, float, bool, std::vector<CMapWorld::CMapAreaBFSInfo> &) const
{

}

bool CMapWorld::IsMapAreaValid(const IWorld &, int, bool) const
{
    return false;
}

void CMapWorld::DrawAreas(const CMapWorld::CMapWorldDrawParms &, int, const std::vector<CMapWorld::CMapAreaBFSInfo> &, bool) const
{

}

void CMapWorld::RecalculateWorldSphere(const CMapWorldInfo &, const IWorld &) const
{

}

void CMapWorld::ConstrainToWorldVolume(const zeus::CVector3f &, const zeus::CVector3f &) const
{

}

void CMapWorld::ClearTraversedFlags() const
{

}

CFactoryFnReturn FMapWorldFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& param,
                                  CObjectReference* selfRef)
{
    return TToken<CMapWorld>::GetIObjObjectFor(std::make_unique<CMapWorld>(in));
}

}
