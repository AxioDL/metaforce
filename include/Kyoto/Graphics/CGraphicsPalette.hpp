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

  inline GXTlutFmt GetTlutFmt() const { return static_cast< GXTlutFmt >(mFmt); }
  ushort* GetPaletteData() { return mEntries.get(); }
  const ushort* GetPaletteData() const { return mEntries.get(); }
  void Load() const;
#if defined(TARGET_PC)
  void* Lock();
#else
  void* Lock() {
    mLocked = true;
    return mEntries.get();
  }
#endif
  void UnLock();

public:
  static uint sCurrentFrameCount;

private:
  EPaletteFormat mFmt;
  mutable uint mFrameLoaded;
  uint mEntryCount;
  rstl::single_ptr< ushort > mEntries;
  GXTlutObj mTlutObj;
  bool mLocked;
};

static inline GXTlutFmt format_to_format(EPaletteFormat fmt) {
  return static_cast< GXTlutFmt >(fmt);
}

#endif // _CGRAPHICSPALETTE
