#include "CMapArea.hpp"
#include "GameGlobalObjects.hpp"
#include "CMappableObject.hpp"
#include "CToken.hpp"

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


void CMapArea::CMapAreaSurface::PostConstruct(const void *)
{
}

CFactoryFnReturn FMapAreaFactory(const SObjectTag& objTag, CInputStream& in, const CVParamTransfer&)
{
    u32 size = g_ResFactory->ResourceSize(objTag);
    return TToken<CMapArea>::GetIObjObjectFor(std::make_unique<CMapArea>(in, size));
}

}
