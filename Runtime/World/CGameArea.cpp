#include "CGameArea.hpp"

namespace urde
{

static std::vector<SObjectTag> ReadDependencyList(CInputStream& in)
{
    std::vector<SObjectTag> ret;
    u32 count = in.readUint32Big();
    ret.reserve(count);
    for (int i=0 ; i<count ; ++i)
    {
        ret.emplace_back();
        ret.back().readMLVL(in);
    }
    return ret;
}

CGameArea::CGameArea(CInputStream& in, int mlvlVersion)
: x4_mlvlVersion(mlvlVersion), xf0_25_(true)
{
    x8_nameSTRG = in.readUint32Big();
    xc_transform.read34RowMajor(in);
    x3c_invTransform = xc_transform.inverse();
    x6c_aabb.readBoundingBoxBig(in);
    x84_mrea = in.readUint32Big();
    x88_areaId = in.readUint32Big();

    u32 attachedCount = in.readUint32Big();
    x8c_attachedAreaIndices.reserve(attachedCount);
    for (int i=0 ; i<attachedCount ; ++i)
        x8c_attachedAreaIndices.push_back(in.readUint16Big());

    x9c_deps1 = ReadDependencyList(in);
    xac_deps2 = ReadDependencyList(in);

}

}
