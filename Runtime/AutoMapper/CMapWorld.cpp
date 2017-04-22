#include "CMapWorld.hpp"
#include "CMapWorldInfo.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "World/CWorld.hpp"

namespace urde
{

CMapWorld::CMapAreaData::CMapAreaData(ResId areaRes, EMapAreaList list, CMapAreaData* next)
: x0_area(g_SimplePool->GetObj(SObjectTag{FOURCC('MAPA'), areaRes})), x10_list(list), x14_next(next)
{}

CMapWorld::CMapWorld(CInputStream& in)
{
    x10_listHeads.resize(3);
    in.readUint32Big();
    in.readUint32Big();
    u32 areaCount = in.readUint32Big();
    x0_areas.reserve(areaCount);
    x20_traversed.resize(areaCount);
    for (u32 i=0 ; i<areaCount ; ++i)
    {
        ResId mapaId = in.readUint32Big();
        x0_areas.emplace_back(mapaId, EMapAreaList::Unloaded, x0_areas.empty() ? nullptr : &x0_areas.back());
    }
    x10_listHeads[2] = &x0_areas.back();
}

bool CMapWorld::IsMapAreaInBFSInfoVector(const CMapWorld::CMapAreaData* area,
                                         const std::vector<CMapWorld::CMapAreaBFSInfo>& vec) const
{
    for (const CMapWorld::CMapAreaBFSInfo& bfs : vec)
    {
        if (&x0_areas[bfs.GetAreaIndex()] == area)
            return true;
    }
    return false;
}

void CMapWorld::SetWhichMapAreasLoaded(const IWorld& wld, int start, int count)
{
    ClearTraversedFlags();

    std::vector<CMapAreaBFSInfo> bfsInfos;
    bfsInfos.reserve(x0_areas.size());
    DoBFS(wld, start, count, 9999.f, 9999.f, false, bfsInfos);

    for (int i=0 ; i<2 ; ++i)
    {
        for (CMapAreaData* data = x10_listHeads[i] ; data ; data = data->NextMapAreaData())
        {
            if (!IsMapAreaInBFSInfoVector(data, bfsInfos))
            {
                data->Unlock();
                MoveMapAreaToList(data, EMapAreaList::Unloaded);
            }
        }
    }

    for (CMapAreaBFSInfo& bfs : bfsInfos)
    {
        CMapAreaData& data = x0_areas[bfs.GetAreaIndex()];
        data.Lock();
        if (data.GetContainingList() == EMapAreaList::Unloaded)
            MoveMapAreaToList(&data, EMapAreaList::Loading);
    }
}

bool CMapWorld::IsMapAreasStreaming() const
{
    bool ret = false;
    for (CMapAreaData* data = x10_listHeads[1] ; data ; data = data->NextMapAreaData())
    {
        if (data->IsLoaded())
            const_cast<CMapWorld*>(this)->MoveMapAreaToList(data, EMapAreaList::Loaded);
        else
            ret = true;
    }
    return ret;
}

void CMapWorld::MoveMapAreaToList(CMapWorld::CMapAreaData* data, CMapWorld::EMapAreaList list)
{
    CMapAreaData* last = nullptr;
    for (CMapAreaData* head = x10_listHeads[int(data->GetContainingList())] ;;
         last = head, head = head->NextMapAreaData())
    {
        if (head != data)
            continue;
        if (!last)
            x10_listHeads[int(data->GetContainingList())] = head->NextMapAreaData();
        else
            last->SetNextMapArea(head->NextMapAreaData());
        break;
    }
    data->SetNextMapArea(x10_listHeads[int(list)]);
    data->SetContainingList(list);
    x10_listHeads[int(list)] = data;
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

std::vector<int> CMapWorld::GetVisibleAreas(const IWorld& wld, const CMapWorldInfo& mwInfo) const
{
    std::vector<int> ret;
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

void CMapWorld::Draw(const CMapWorld::CMapWorldDrawParms& parms, int curArea, int otherArea,
                     float depth1, float depth2, bool inMapScreen) const
{
    if (depth1 == 0.f && depth2 == 0.f)
        return;

    ClearTraversedFlags();
    int areaDepth = std::ceil(std::max(depth1, depth2));

    std::vector<CMapAreaBFSInfo> bfsInfos;
    bfsInfos.reserve(x0_areas.size());
    if (curArea != otherArea)
    {
        const_cast<CMapWorld*>(this)->x20_traversed[otherArea] = true;
        DoBFS(parms.GetWorld(), curArea, areaDepth, depth1, depth2, true, bfsInfos);

        float lowD1 = std::ceil(depth1 - 1.f);
        float tmp;
        if (depth1 == std::floor(depth1))
            tmp = 0.f;
        else
            tmp = 1.f - std::fmod(depth1, 1.f);
        float newD1 = lowD1 + tmp;

        float lowD2 = std::ceil(depth2 - 1.f);
        if (depth2 == std::floor(depth2))
            tmp = 0.f;
        else
            tmp = 1.f - std::fmod(depth2, 1.f);
        float newD2 = lowD2 + tmp;

        int otherDepth = std::ceil(std::max(newD1, newD2));
        if (parms.GetWorld().IGetAreaAlways(otherArea)->IIsActive())
        {
            const_cast<CMapWorld*>(this)->x20_traversed[otherArea] = false;
            DoBFS(parms.GetWorld(), otherArea, otherDepth, newD1, newD2, true, bfsInfos);
        }
    }
    else
    {
        DoBFS(parms.GetWorld(), curArea, areaDepth, depth1, depth2, true, bfsInfos);
    }

    DrawAreas(parms, curArea, bfsInfos, inMapScreen);
}

void CMapWorld::DoBFS(const IWorld& wld, int startArea, int areaCount, float surfDepth, float outlineDepth,
                      bool checkLoad, std::vector<CMapAreaBFSInfo>& bfsInfos) const
{
    if (areaCount <= 0 || !IsMapAreaValid(wld, startArea, checkLoad))
        return;

    int size = bfsInfos.size();
    bfsInfos.emplace_back(startArea, 1, surfDepth, outlineDepth);
    const_cast<CMapWorld*>(this)->x20_traversed[startArea] = true;

    for (; size != bfsInfos.size() ; ++size)
    {
        CMapAreaBFSInfo& testInfo = bfsInfos[size];
        if (testInfo.GetDepth() == areaCount)
            continue;

        surfDepth = testInfo.GetSurfaceDrawDepth() - 1.f;
        outlineDepth = testInfo.GetOutlineDrawDepth() - 1.f;

        const IGameArea* area = wld.IGetAreaAlways(testInfo.GetAreaIndex());
        for (int i=0 ; i<area->IGetNumAttachedAreas() ; ++i)
        {
            TAreaId attId = area->IGetAttachedAreaId(i);
            if (IsMapAreaValid(wld, attId, checkLoad) && !x20_traversed[attId])
            {
                bfsInfos.emplace_back(attId, testInfo.GetDepth() + 1, surfDepth, outlineDepth);
                const_cast<CMapWorld*>(this)->x20_traversed[attId] = true;
            }
        }
    }
}

bool CMapWorld::IsMapAreaValid(const IWorld& wld, int areaIdx, bool checkLoad) const
{
    if (!wld.IGetAreaAlways(areaIdx)->IIsActive())
        return false;
    const CMapArea* mapa = GetMapArea(areaIdx);
    if (checkLoad)
        return mapa != nullptr;
    return true;
}

void CMapWorld::DrawAreas(const CMapWorld::CMapWorldDrawParms&, int,
                          const std::vector<CMapWorld::CMapAreaBFSInfo>&, bool) const
{

}

struct Support
{
    int x0_;
    int x4_[3];
};

struct Circle2
{
    zeus::CVector2f x0_point;
    float x8_radiusSq;
};

struct Circle
{
    zeus::CVector2f x0_point;
    float x8_radius;
    Circle(const Circle2& circ2)
    : x0_point(circ2.x0_point), x8_radius(std::sqrt(circ2.x8_radiusSq)) {}
};

static Circle2 ExactCircle1(const zeus::CVector2f* a)
{
    return {*a, 0.f};
}

static Circle2 ExactCircle2(const zeus::CVector2f* a, const zeus::CVector2f* b)
{
    Circle2 ret = {};
    ret.x0_point = 0.5f * (*a + *b);
    ret.x8_radiusSq = (*b - *a).magSquared() * 0.25f;
    return ret;
}

static Circle2 ExactCircle3(const zeus::CVector2f* a, const zeus::CVector2f* b, const zeus::CVector2f* c)
{
    Circle2 ret = {};
    zeus::CVector2f d1 = *b - *a;
    zeus::CVector2f d2 = *c - *a;
    float cross = d1.cross(d2);
    zeus::CVector2f magVec(d1.magSquared() * 0.5f, d2.magSquared() * 0.5f);
    if (std::fabs(cross) > 0.01f)
    {
        zeus::CVector2f tmp((d2.y * magVec.x - d1.y * magVec.y) / cross,
                            (d1.x * magVec.y - d2.x * magVec.x) / cross);
        ret.x0_point = *a + tmp;
        ret.x8_radiusSq = tmp.magSquared();
    }
    else
    {
        ret.x8_radiusSq = FLT_MAX;
    }
    return ret;
}

static bool PointInsideCircle(const zeus::CVector2f& point, const Circle2& circ, float& intersect)
{
    intersect = (point - circ.x0_point).magSquared() - circ.x8_radiusSq;
    return intersect <= 0.f;
}

static Circle2 UpdateSupport1(int idx, zeus::CVector2f** list, Support& support)
{
    Circle2 ret = ExactCircle2(list[support.x4_[0]], list[idx]);
    support.x0_ = 2;
    support.x4_[1] = idx;
    return ret;
}

static Circle2 UpdateSupport2(int idx, zeus::CVector2f** list, Support& support)
{
    Circle2 circs[3] = {};
    float intersect;
    int circIdx = -1;
    float minRad = FLT_MAX;

    circs[0] = ExactCircle2(list[support.x4_[0]], list[idx]);
    if (PointInsideCircle(*list[support.x4_[1]], circs[0], intersect))
    {
        minRad = circs[0].x8_radiusSq;
        circIdx = 0;
    }

    circs[1] = ExactCircle2(list[support.x4_[1]], list[idx]);
    if (circs[1].x8_radiusSq < minRad && PointInsideCircle(*list[support.x4_[1]], circs[1], intersect))
    {
        minRad = circs[1].x8_radiusSq;
        circIdx = 1;
    }

    Circle2 ret = {};
    if (circIdx != -1)
    {
        ret = circs[circIdx];
        support.x4_[1 - circIdx] = idx;
    }
    else
    {
        ret = ExactCircle3(list[support.x4_[0]], list[support.x4_[1]], list[idx]);
        support.x0_ = 3;
        support.x4_[2] = idx;
    }
    return ret;
}

static Circle2 UpdateSupport3(int idx, zeus::CVector2f** list, Support& support)
{
    Circle2 circs[6] = {};
    float intersect;
    int circIdxA = -1;
    int circIdxB = -1;
    float minRadA = FLT_MAX;
    float minRadB = FLT_MAX;

    circs[0] = ExactCircle2(list[support.x4_[0]], list[idx]);
    if (PointInsideCircle(*list[support.x4_[1]], circs[0], intersect))
    {
        if (PointInsideCircle(*list[support.x4_[2]], circs[0], intersect))
        {
            minRadA = circs[0].x8_radiusSq;
            circIdxA = 0;
        }
        else
        {
            minRadB = intersect;
            circIdxB = 0;
        }
    }
    else
    {
        minRadB = intersect;
        circIdxB = 0;
    }

    circs[1] = ExactCircle2(list[support.x4_[1]], list[idx]);
    if (circs[1].x8_radiusSq < minRadA)
    {
        if (PointInsideCircle(*list[support.x4_[0]], circs[1], intersect))
        {
            if (PointInsideCircle(*list[support.x4_[2]], circs[1], intersect))
            {
                minRadA = circs[1].x8_radiusSq;
                circIdxA = 1;
            }
            else if (intersect < minRadB)
            {
                minRadB = intersect;
                circIdxB = 1;
            }
        }
        else if (intersect < minRadB)
        {
            minRadB = intersect;
            circIdxB = 1;
        }
    }

    circs[2] = ExactCircle2(list[support.x4_[2]], list[idx]);
    if (circs[2].x8_radiusSq < minRadA)
    {
        if (PointInsideCircle(*list[support.x4_[0]], circs[2], intersect))
        {
            if (PointInsideCircle(*list[support.x4_[1]], circs[2], intersect))
            {
                minRadA = circs[2].x8_radiusSq;
                circIdxA = 2;
            }
            else if (intersect < minRadB)
            {
                minRadB = intersect;
                circIdxB = 2;
            }
        }
        else if (intersect < minRadB)
        {
            minRadB = intersect;
            circIdxB = 2;
        }
    }

    circs[3] = ExactCircle3(list[support.x4_[0]], list[support.x4_[1]], list[idx]);
    if (circs[3].x8_radiusSq < minRadA)
    {
        if (PointInsideCircle(*list[support.x4_[2]], circs[3], intersect))
        {
            minRadA = circs[3].x8_radiusSq;
            circIdxA = 3;
        }
        else if (intersect < minRadB)
        {
            minRadB = intersect;
            circIdxB = 3;
        }
    }

    circs[4] = ExactCircle3(list[support.x4_[0]], list[support.x4_[2]], list[idx]);
    if (circs[4].x8_radiusSq < minRadA)
    {
        if (PointInsideCircle(*list[support.x4_[1]], circs[4], intersect))
        {
            minRadA = circs[4].x8_radiusSq;
            circIdxA = 4;
        }
        else if (intersect < minRadB)
        {
            minRadB = intersect;
            circIdxB = 4;
        }
    }

    circs[5] = ExactCircle3(list[support.x4_[1]], list[support.x4_[2]], list[idx]);
    if (circs[5].x8_radiusSq < minRadA)
    {
        if (PointInsideCircle(*list[support.x4_[0]], circs[5], intersect))
        {
            circIdxA = 5;
        }
        else if (intersect < minRadB)
        {
            circIdxB = 5;
        }
    }

    if (circIdxA == -1)
        circIdxA = circIdxB;

    switch (circIdxA)
    {
    case 0:
        support.x0_ = 2;
        support.x4_[1] = idx;
        break;
    case 1:
        support.x0_ = 2;
        support.x4_[0] = idx;
        break;
    case 2:
        support.x0_ = 2;
        support.x4_[0] = support.x4_[2];
        support.x4_[1] = idx;
        break;
    case 3:
        support.x4_[2] = idx;
        break;
    case 4:
        support.x4_[1] = idx;
        break;
    case 5:
        support.x4_[0] = idx;
        break;
    default: break;
    }

    return circs[circIdxA];
}

typedef Circle2(*FSupport)(int idx, zeus::CVector2f** list, Support& support);
static const FSupport SupportFuncs[] =
{
    nullptr,
    UpdateSupport1,
    UpdateSupport2,
    UpdateSupport3
};

static Circle MinCircle(const std::vector<zeus::CVector2f>& coords)
{
    Circle2 ret = {};
    if (coords.size() >= 1)
    {
        std::unique_ptr<zeus::CVector2f*[]> randArr(new zeus::CVector2f*[coords.size()]);
        for (int i=0 ; i<coords.size() ; ++i)
            randArr[i] = const_cast<zeus::CVector2f*>(&coords[i]);
        for (int i=coords.size()-1 ; i>=0 ; --i)
        {
            int shuf = rand() % (i+1);
            if (shuf != i)
                std::swap(randArr[i], randArr[shuf]);
        }
        ret = ExactCircle1(randArr[0]);

        Support support = {};
        support.x0_ = 1;
        for (int i=1 ; i<coords.size() ;)
        {
            bool broke = false;
            for (int j=0 ; j<support.x0_ ; ++j)
            {
                if ((*randArr[i] - *randArr[support.x4_[j]]).magSquared() < 0.01f)
                {
                    broke = true;
                    break;
                }
            }
            float intersect;
            if (!broke && !PointInsideCircle(*randArr[i], ret, intersect))
            {
                Circle2 circ = SupportFuncs[support.x0_](i, randArr.get(), support);
                if (circ.x8_radiusSq > ret.x8_radiusSq)
                {
                    i = 0;
                    ret = circ;
                    continue;
                }
            }
            ++i;
        }
    }
    return ret;
}

void CMapWorld::RecalculateWorldSphere(const CMapWorldInfo& mwInfo, const IWorld& wld) const
{
    std::vector<zeus::CVector2f> coords;
    coords.reserve(x0_areas.size() * 8);
    float zMin = FLT_MAX;
    float zMax = FLT_MIN;
    for (int i=0 ; i<x0_areas.size() ; ++i)
    {
        if (IsMapAreaValid(wld, i, true))
        {
            const CMapArea* mapa = GetMapArea(i);
            if (mapa->GetIsVisibleToAutoMapper(mwInfo.IsWorldVisible(i), mwInfo.IsAreaVisible(i)))
            {
                zeus::CAABox aabb = mapa->GetBoundingBox().getTransformedAABox(mapa->GetAreaPostTransform(wld, i));
                for (int j=0 ; j<8 ; ++j)
                {
                    zeus::CVector3f point = aabb.getPoint(j);
                    coords.emplace_back(point.x, point.y);
                    zMin = std::min(point.z, zMin);
                    zMax = std::max(point.z, zMax);
                }
            }
        }
    }

    Circle circle = MinCircle(coords);
    const_cast<CMapWorld*>(this)->x3c_ = circle.x8_radius;
    const_cast<CMapWorld*>(this)->x30_ = zeus::CVector3f(circle.x0_point.x, circle.x0_point.y, (zMin + zMax) * 0.5f);
    const_cast<CMapWorld*>(this)->x40_ = (zMax - zMin) * 0.5f;
}

zeus::CVector3f CMapWorld::ConstrainToWorldVolume(const zeus::CVector3f&, const zeus::CVector3f&) const
{
    return {};
}

void CMapWorld::ClearTraversedFlags() const
{
    std::vector<bool>& flags = const_cast<CMapWorld*>(this)->x20_traversed;
    for (int i=0 ; i<flags.size() ; ++i)
        flags[i] = false;
}

CFactoryFnReturn FMapWorldFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& param,
                                  CObjectReference* selfRef)
{
    return TToken<CMapWorld>::GetIObjObjectFor(std::make_unique<CMapWorld>(in));
}

}
