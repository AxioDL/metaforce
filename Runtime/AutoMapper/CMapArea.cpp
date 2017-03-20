#include "CMapArea.hpp"
#include "GameGlobalObjects.hpp"
#include "CMappableObject.hpp"
#include "CToken.hpp"
#include "World/CWorld.hpp"
#include "World/CGameArea.hpp"
#include "CResFactory.hpp"

namespace urde
{
CMapArea::CMapArea(CInputStream& in, u32 size)
    : x0_magic(in.readUint32()),
      x4_version(in.readUint32Big()),
      x8_(in.readUint32Big()),
      xc_(in.readUint32Big()),
      x10_box(zeus::CAABox::ReadBoundingBoxBig(in)),
      x28_mappableObjCount(in.readUint32Big()),
      x2c_vertexCount(in.readUint32Big()),
      x30_surfaceCount(in.readUint32Big()),
      x34_size(size - 52)
{
    x44_buf.reset(new u8[x34_size]);
    in.readUBytesToBuf(x44_buf.get(), x34_size);
    PostConstruct();
}

void CMapArea::PostConstruct()
{
    x38_moStart = x44_buf.get();
    x3c_vertexStart = x38_moStart + (x28_mappableObjCount * 0x50);
    x40_surfaceStart = x40_surfaceStart + (x2c_vertexCount * 12);
    for (u32 i = 0, j=0 ; i<x28_mappableObjCount ; ++i, j += 0x50)
        (reinterpret_cast<CMappableObject*>(x38_moStart + j))->PostConstruct(x44_buf.get());

#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
    u8* tmp = x3c_vertexStart;
    for (u32 i = 0 ; i<(x2c_vertexCount*3) ; ++i)
    {
        u32* fl = reinterpret_cast<u32*>(tmp);
        *fl = SBIG(*fl);
        tmp += 4;
    }
#endif


    for (u32 i = 0, j = 0 ; i<x30_surfaceCount ; ++i, j += 32)
        (reinterpret_cast<CMapAreaSurface*>(x40_surfaceStart + j))->PostConstruct(x44_buf.get());
}

static const zeus::CVector3f MinesPostTransforms[3] =
{
    {0.f, 0.f, 200.f},
    {0.f, 0.f, 0.f},
    {0.f, 0.f, -200.f}
};
static const u8 MinesPostTransformIndices[] =
{
    0, // 00 Transport to Tallon Overworld South
    0, // 01 Quarry Access
    0, // 02 Main Quarry
    0, // 03 Waste Disposal
    0, // 04 Save Station Mines A
    0, // 05 Security Access A
    0, // 06 Ore Processing
    0, // 07 Mine Security Station
    0, // 08 Research Access
    0, // 09 Storage Depot B
    0, // 10 Elevator Access A
    0, // 11 Security Access B
    0, // 12 Storage Depot A
    0, // 13 Elite Research
    0, // 14 Elevator A
    1, // 15 Elite Control Access
    1, // 16 Elite Control
    1, // 17 Maintenance Tunnel
    1, // 18 Ventilation Shaft
    2, // 19 Phazon Processing Center
    1, // 20 Omega Research
    2, // 21 Transport Access
    2, // 22 Processing Center Access
    1, // 23 Map Station Mines
    1, // 24 Dynamo Access
    2, // 25 Transport to Magmoor Caverns South
    2, // 26 Elite Quarters
    1, // 27 Central Dynamo
    2, // 28 Elite Quarters Access
    1, // 29 Quarantine Access A
    1, // 30 Save Station Mines B
    2, // 31 Metroid Quarantine B
    1, // 32 Metroid Quarantine A
    2, // 33 Quarantine Access B
    2, // 34 Save Station Mines C
    1, // 35 Elevator Access B
    2, // 36 Fungal Hall B
    1, // 37 Elevator B
    2, // 38 Missile Station Mines
    2, // 39 Phazon Mining Tunnel
    2, // 40 Fungal Hall Access
    2, // 41 Fungal Hall A
};

zeus::CTransform CMapArea::GetAreaPostTransform(const CWorld& world, TAreaId aid) const
{
    if (world.IGetWorldAssetId() == g_ResFactory->TranslateOriginalToNew(0xB1AC4D65)) // Phazon Mines
    {
        const zeus::CTransform& areaXf = world.IGetAreaAlways(aid)->IGetTM();
        const zeus::CVector3f& postVec = MinesPostTransforms[MinesPostTransformIndices[aid]];
        return zeus::CTransform::Translate(postVec) * areaXf;
    }
    else
    {
        return world.IGetAreaAlways(aid)->IGetTM();
    }
}

void CMapArea::CMapAreaSurface::PostConstruct(const void *)
{
}

CFactoryFnReturn FMapAreaFactory(const SObjectTag& objTag, CInputStream& in, const CVParamTransfer&)
{
    u32 size = g_ResFactory->ResourceSize(objTag);
    return TToken<CMapArea>::GetIObjObjectFor(std::make_unique<CMapArea>(in, size));
}

}
