#include "Runtime/Graphics/CGraphicsPalette.hpp"
#include "Runtime/Streams/CInputStream.hpp"

namespace metaforce {
u32 CGraphicsPalette::sCurrentFrameCount = 0;

CGraphicsPalette::CGraphicsPalette(EPaletteFormat fmt, int count)
: x0_fmt(fmt), x8_entryCount(count), xc_entries(new u8[count * 2]) {
  //GXInitTlutObj(&x10_tlutObj, xc_entries.get(), x8_entryCount);
}

CGraphicsPalette::CGraphicsPalette(CInputStream& in) : x0_fmt(EPaletteFormat(in.ReadLong())) {
  u16 w = in.ReadShort();
  u16 h = in.ReadShort();
  x8_entryCount = w * h;
  xc_entries.reset(new u8[x8_entryCount * 2]);
  in.Get(xc_entries.get(), x8_entryCount * 2);
  //GXInitTlutObj(&x10_tlutObj, xc_entries.get(), x8_entryCount);
  //DCFlushRange(xc_entries.get(), x8_entryCount * 2);
}


void CGraphicsPalette::Load() {
  //GXLoadTlut(x10_tlutObj, 0);
  x4_frameLoaded = sCurrentFrameCount;
}
} // namespace metaforce