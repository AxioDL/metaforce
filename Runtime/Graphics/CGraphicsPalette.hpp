#pragma once

#include <memory>
#include "Runtime/RetroTypes.hpp"

namespace metaforce {
class CInputStream;

enum class EPaletteFormat {
  IA8 = 0x0,
  RGB565 = 0x1,
  RGB5A3 = 0x2,
};

class CGraphicsPalette {
  static u32 sCurrentFrameCount;
  friend class CTextRenderBuffer;
  EPaletteFormat x0_fmt;
  u32 x4_frameLoaded{};
  u32 x8_entryCount;
  std::unique_ptr<u8[]> xc_entries;
  /* GXTlutObj x10_; */
  bool x1c_locked = false;

public:
  explicit CGraphicsPalette(EPaletteFormat fmt, int count);

  explicit CGraphicsPalette(CInputStream& in);

  void Load();
  static void SetCurrentFrameCount(u32 frameCount) { sCurrentFrameCount = frameCount; }
};

} // namespace metaforce
