#include "CMapUniverse.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"

namespace urde
{

CMapUniverse::CMapUniverse(CInputStream& in, u32 version)
    : x0_hexagonId(in.readUint32Big())
{
    x4_hexagonToken = g_SimplePool->GetObj({'MAPA', x0_hexagonId});
    u32 count = in.readUint32Big();
    x10_worldDatas.reserve(count);
    for (u32 i = 0 ; i<count ; ++i)
        x10_worldDatas.emplace_back(in, version);
}

CMapUniverse::CMapWorldData::CMapWorldData(CInputStream& in, u32 version)
    : x0_label(in.readString()),
      x10_worldAssetId(in.readUint32Big())
{
    x14_transform.read34RowMajor(in);
    u32 worldCount = in.readUint32Big();
    x44_hexagonXfs.reserve(worldCount);
    for (u32 i = 0 ; i<worldCount ; ++i)
    {
        x44_hexagonXfs.emplace_back();
        x44_hexagonXfs.back().read34RowMajor(in);
    }

    if (version != 0)
        x54_.readRGBABig(in);
    else
        x54_.fromRGBA32(255 | (x10_worldAssetId & 0xFFFFFF00));

    x58_ = zeus::CColor::lerp(zeus::CColor::skWhite, x54_, 0.5f);
    x5c_ = zeus::CColor::lerp(zeus::CColor::skBlack, x54_, 0.5f);
    x60_ = zeus::CColor::lerp(zeus::CColor::skWhite, x5c_, 0.5f);

    for (const zeus::CTransform& xf : x44_hexagonXfs)
        x64_centerPoint += xf.origin;

    x64_centerPoint *= 1.0f / float(x44_hexagonXfs.size());
}

void CMapUniverse::Draw(const CMapUniverseDrawParms& parms, const zeus::CVector3f&, float, float) const
{

}

CFactoryFnReturn FMapUniverseFactory(const SObjectTag&, CInputStream& in, const CVParamTransfer&,
                                     CObjectReference*)
{
    in.readUint32Big();
    u32 version = in.readUint32Big();

    return TToken<CMapUniverse>::GetIObjObjectFor(std::make_unique<CMapUniverse>(in, version));
}

}
