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
    x44_areaData.reserve(worldCount);
    for (u32 i = 0 ; i<worldCount ; ++i)
    {
        x44_areaData.emplace_back();
        x44_areaData.back().read34RowMajor(in);
    }

    if (version != 0)
        x54_.readRGBABig(in);
    else
        x54_.fromRGBA32(255 | (x10_worldAssetId & 0xFFFFFF00));

    x58_ = zeus::CColor::lerp(zeus::CColor::skWhite, x54_, 0.5f);
    x5c_ = zeus::CColor::lerp(zeus::CColor::skBlack, x54_, 0.5f);
    x60_ = zeus::CColor::lerp(zeus::CColor::skWhite, x5c_, 0.5f);

    for (const zeus::CTransform& xf : x44_areaData)
        x64_ += xf.origin;

    x64_ *= 1.0f / float(x44_areaData.size());
}

CFactoryFnReturn FMapUniverseFactory(const SObjectTag&, CInputStream& in, const CVParamTransfer&)
{
    in.readUint32Big();
    u32 version = in.readUint32Big();

    return TToken<CMapUniverse>::GetIObjObjectFor(std::make_unique<CMapUniverse>(in, version));
}

}
