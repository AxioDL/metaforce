#ifndef _CGRAPHICSPALETTE
#define _CGRAPHICSPALETTE

#include "types.h"

#include <rstl/single_ptr.hpp>

#include "dolphin/gx/GXEnum.h"
#include "dolphin/gx/GXStruct.h"

enum EPaletteFormat {
  kPF_IA8 = GX_TL_IA8,
  kPF_RGB565 = GX_TL_RGB565,
  kPF_RGB5A3 = GX_TL_RGB5A3,
};

class CInputStream;

class CGraphicsPalette {
public:
  CGraphicsPalette(EPaletteFormat format, int numEntries);
  CGraphicsPalette(CInputStream& in);
  ~CGraphicsPalette();

  inline GXTlutFmt GetTlutFmt() const { return static_cast< GXTlutFmt >(x0_fmt); }
  ushort* GetPaletteData() { return xc_entries.get(); }
  const ushort* GetPaletteData() const { return xc_entries.get(); }
  void Load() const;
#if defined(TARGET_PC)
  void* Lock();
#else
  void* Lock() {
    x1c_locked = true;
    return xc_entries.get();
  }
#endif
  void UnLock();

public:
  static uint sCurrentFrameCount;

private:
  EPaletteFormat x0_fmt;
  mutable uint x4_frameLoaded;
  uint x8_entryCount;
  rstl::single_ptr< ushort > xc_entries;
  GXTlutObj x10_tlutObj;
  bool x1c_locked;
};

static inline GXTlutFmt format_to_format(EPaletteFormat fmt) {
  return static_cast< GXTlutFmt >(fmt);
}

#endif // _CGRAPHICSPALETTE
