#pragma once

#include <memory>
#include "Runtime/RetroTypes.hpp"

namespace metaforce {

enum class EPaletteFormat {
  IA8 = 0x0,
  RGB565 = 0x1,
  RGB5A3 = 0x2,
};

class CGraphicsPalette {
  friend class CTextRenderBuffer;
  EPaletteFormat x0_fmt;
  u32 x4_;
  int x8_entryCount;
  std::unique_ptr<u16[]> xc_entries;
  /* x10_ GXTlutObj here */
  bool x1c_ = false;

public:
  explicit CGraphicsPalette(EPaletteFormat fmt, int count)
  : x0_fmt(fmt), x8_entryCount(count), xc_entries(new u16[count]) {}
  explicit CGraphicsPalette(CInputStream& in) : x0_fmt(EPaletteFormat(in.ReadLong())) {
    u16 w = in.ReadShort();
    u16 h = in.ReadShort();
    x8_entryCount = w * h;

    /* GX Tlut init here */
  }
};

} // namespace metaforce
