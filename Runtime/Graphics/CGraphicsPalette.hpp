#pragma once

#include "RetroTypes.hpp"
#include "GX.hpp"

#include <memory>

namespace metaforce {
class CInputStream;

enum class EPaletteFormat : std::underlying_type_t<GXTlutFmt> {
  IA8 = GX_TL_IA8,
  RGB565 = GX_TL_RGB565,
  RGB5A3 = GX_TL_RGB5A3,
};

class CGraphicsPalette {
  static u32 sCurrentFrameCount;
  friend class CTextRenderBuffer;
  EPaletteFormat x0_fmt;
  u32 x4_frameLoaded{};
  u32 x8_entryCount;
  std::unique_ptr<u16[]> xc_entries;
  GXTlutObj x10_tlutObj;
  bool x1c_locked = false;

public:
  explicit CGraphicsPalette(EPaletteFormat fmt, int count);
  explicit CGraphicsPalette(CInputStream& in);

  u16* Lock() {
    x1c_locked = true;
    return xc_entries.get();
  }
  void UnLock();
  void Load();

  [[nodiscard]] const u16* GetPaletteData() const { return xc_entries.get(); }

  static void SetCurrentFrameCount(u32 frameCount) { sCurrentFrameCount = frameCount; }
};

} // namespace metaforce
