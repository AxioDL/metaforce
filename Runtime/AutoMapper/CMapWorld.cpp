#include "CMapWorld.hpp"
#include "CMapWorldInfo.hpp"

namespace urde
{

CMapWorld::CMapWorld(CInputStream& in)
{
}

void CMapWorld::IsMapAreaInBFSInfoVector(const CMapWorld::CMapAreaData*, const std::vector<CMapWorld::CMapAreaBFSInfo>&) const
{

}

void CMapWorld::SetWhichMapAreasLoaded(const IWorld&, int start, int count)
{

}

bool CMapWorld::IsMapAreasStreaming() const
{
    return false;
}

void CMapWorld::MoveMapAreaToList(CMapWorld::CMapAreaData*, CMapWorld::EMapAreaList)
{

}

s32 CMapWorld::GetCurrentMapAreaDepth(const IWorld& wld, TAreaId aid) const
{
    ClearTraversedFlags();
    std::vector<CMapAreaBFSInfo> info;
    info.reserve(x0_areas.size());
    DoBFS(wld, aid, 9999, 9999.f, 9999.f, false, info);
    if (info.empty())
        return 0;
    return info.back().GetDepth();
}

std::vector<TAreaId> CMapWorld::GetVisibleAreas(const IWorld& wld, const CMapWorldInfo& mwInfo) const
{
    std::vector<TAreaId> ret;
    ret.reserve(x0_areas.size());
    for (int i=0 ; i<x0_areas.size() ; ++i)
    {
        if (!IsMapAreaValid(wld, i, true))
            continue;
        const CMapArea* area = GetMapArea(i);
        bool areaVis = mwInfo.IsAreaVisible(i);
        bool worldVis = mwInfo.IsWorldVisible(i);
        if (area->GetIsVisibleToAutoMapper(worldVis, areaVis))
            ret.push_back(i);
    }
    return ret;
}

void CMapWorld::Draw(const CMapWorld::CMapWorldDrawParms&, int, int, float, float, bool) const
{

}

void CMapWorld::DoBFS(const IWorld&, TAreaId, int, float, float, bool, std::vector<CMapAreaBFSInfo>&) const
{

}

bool CMapWorld::IsMapAreaValid(const IWorld &, int, bool) const
{
    return false;
}

void CMapWorld::DrawAreas(const CMapWorld::CMapWorldDrawParms&, int, const std::vector<CMapWorld::CMapAreaBFSInfo>&, bool) const
{

}

void CMapWorld::RecalculateWorldSphere(const CMapWorldInfo&, const IWorld&) const
{

}

zeus::CVector3f CMapWorld::ConstrainToWorldVolume(const zeus::CVector3f&, const zeus::CVector3f&) const
{
    return {};
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
