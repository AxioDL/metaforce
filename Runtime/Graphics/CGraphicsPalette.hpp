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
  std::unique_ptr<u8[]> xc_entries;
  GXTlutObj x10_tlutObj;
  bool x1c_locked = false;

public:
  explicit CGraphicsPalette(EPaletteFormat fmt, int count);
  explicit CGraphicsPalette(CInputStream& in);

  void Lock() { x1c_locked = true; }
  void UnLock();
  void Load();

  [[nodiscard]] u8* GetPaletteData() { return xc_entries.get(); }
  [[nodiscard]] const u8* GetPaletteData() const { return xc_entries.get(); }

  static void SetCurrentFrameCount(u32 frameCount) { sCurrentFrameCount = frameCount; }
};

} // namespace metaforce
