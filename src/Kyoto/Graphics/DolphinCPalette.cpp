#include "Kyoto/Graphics/CGraphicsPalette.hpp"

#include "Kyoto/Alloc/CMemorySys.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include "dolphin/gx.h"
#include "dolphin/os.h"

uint CGraphicsPalette::sCurrentFrameCount = 0;

CGraphicsPalette::CGraphicsPalette(EPaletteFormat format, int numEntries)
: x0_fmt(format)
#if NONMATCHING
, x4_frameLoaded(0)
#endif
, x8_entryCount(numEntries)
, xc_entries((ushort*)CMemory::Alloc(numEntries * sizeof(ushort), IAllocator::kHI_RoundUpLen))
#if NONMATCHING
, x10_tlutObj()
#endif
, x1c_locked(false) {
  GXInitTlutObj(&x10_tlutObj, xc_entries.get(), format_to_format(x0_fmt), x8_entryCount);
}

CGraphicsPalette::CGraphicsPalette(CInputStream& in)
: x0_fmt(EPaletteFormat(in.ReadLong()))
#if NONMATCHING
, x4_frameLoaded(0)
#endif
, x8_entryCount(in.Get< short >() * in.Get< short >())
, xc_entries((ushort*)CMemory::Alloc(x8_entryCount * sizeof(ushort), IAllocator::kHI_RoundUpLen))
#if NONMATCHING
, x10_tlutObj()
#endif
, x1c_locked(false) {
  in.Get(reinterpret_cast< uchar* >(xc_entries.get()), x8_entryCount * sizeof(ushort));
  GXInitTlutObj(&x10_tlutObj, xc_entries.get(), format_to_format(x0_fmt), x8_entryCount);
  DCFlushRange(xc_entries.get(), x8_entryCount * sizeof(ushort));
}

CGraphicsPalette::~CGraphicsPalette() {
#if defined(TARGET_PC)
  GXDestroyTlutObj(&x10_tlutObj);
  CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                        xc_entries.release());
#else
  uint frameDiff = sCurrentFrameCount - x4_frameLoaded;
  if (frameDiff < 2) {
    CFrameDelayedKiller::ScheduleDeletion(frameDiff > 0
                                              ? CFrameDelayedKiller::kWhichFrame_ThisFrame
                                              : CFrameDelayedKiller::kWhichFrame_NextFrame,
                                          xc_entries.release());
  }
#endif
}

void CGraphicsPalette::Load() const {
  GXLoadTlut(&x10_tlutObj, GX_TLUT0);
  x4_frameLoaded = sCurrentFrameCount;
}

#if defined(TARGET_PC)
void* CGraphicsPalette::Lock() {
  // AuroraGXSync();
  x1c_locked = true;
  return xc_entries.get();
}
#endif

void CGraphicsPalette::UnLock() {
  DCStoreRange(xc_entries.get(), x8_entryCount * sizeof(ushort));
#if defined(TARGET_PC)
  GXInitTlutObjData(&x10_tlutObj, xc_entries.get());
#else
  GXInitTlutObj(&x10_tlutObj, xc_entries.get(), format_to_format(x0_fmt), x8_entryCount);
#endif
  DCFlushRange(xc_entries.get(), x8_entryCount * sizeof(ushort));
  x1c_locked = false;
}
