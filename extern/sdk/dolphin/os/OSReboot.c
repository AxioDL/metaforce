#include "dolphin/dvd.h"
#include "dolphin/os.h"
#include "dolphin/os/OSBootInfo.h"
#include "dolphin/os/OSContext.h"
#include "dolphin/os/OSInterrupt.h"
#include "dolphin/types.h"

typedef struct ApploaderHeader {
  // total size: 0x20
  char date[16];  // offset 0x0, size 0x10
  u32 entry;      // offset 0x10, size 0x4
  u32 size;       // offset 0x14, size 0x4
  u32 rebootSize; // offset 0x18, size 0x4
  u32 reserved2;  // offset 0x1C, size 0x4
} ApploaderHeader;

ATTRIBUTE_ALIGN_DECL(32, static ApploaderHeader Header);

extern void *__OSSavedRegionStart;
extern void *__OSSavedRegionEnd;

static void *SaveStart = NULL;
static void *SaveEnd = NULL;

volatile u8 OS_REBOOT_BOOL : 0x800030e2;

extern u32 BOOT_REGION_START AT_ADDRESS(0x812FDFF0);
extern u32 BOOT_REGION_END AT_ADDRESS(0x812FDFEC);
extern u32 UNK_HOT_RESET1 AT_ADDRESS(0x817ffff8);
extern u32 UNK_HOT_RESET2 AT_ADDRESS(0x817ffffc);

static volatile BOOL Prepared = FALSE;

extern void __DVDPrepareResetAsync(DVDCBCallback callback);
extern BOOL DVDCheckDisk(void);
extern BOOL DVDReadAbsAsyncForBS(DVDCommandBlock *block, void *addr, s32 length, s32 offset,
                                 DVDCBCallback callback);

asm void Run(void *entrypoint) {
  // clang-format off
  nofralloc
  sync
  isync
  mtlr r3
  blr
  // clang-format on
}

static void Callback(s32 result, DVDCommandBlock *block) { Prepared = TRUE; }

static inline void ReadApploader(void *addr, long length, long offset) {
  DVDCommandBlock block;

  while (!Prepared) {
  }

  DVDReadAbsAsyncForBS(&block, addr, length, offset + 0x2440, NULL);
  while (1) {
    switch (block.state) {
    case 0:
      return;
    case 1:
      break;
    case -1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
      __OSDoHotReset(UNK_HOT_RESET2);
      break;
    default:
      break;
    }
  }
}

void __OSReboot(u32 resetCode, u32 bootDol) {
  OSContext exceptionContext;
  u32 numBytes;
  u32 offset;

  OSDisableInterrupts();
  UNK_HOT_RESET2 = 0;
  UNK_HOT_RESET1 = 0;
  OS_REBOOT_BOOL = 1;
  BOOT_REGION_START = (u32)SaveStart;
  BOOT_REGION_END = (u32)SaveEnd;
  OSClearContext(&exceptionContext);
  OSSetCurrentContext(&exceptionContext);
  DVDInit();
  DVDSetAutoInvalidation(TRUE);
  __DVDPrepareResetAsync(Callback);
  if (!DVDCheckDisk()) {
    __OSDoHotReset(UNK_HOT_RESET2);
  }

  __OSMaskInterrupts(~0x1F);
  __OSUnmaskInterrupts(0x400);
  OSEnableInterrupts();

  ReadApploader(&Header, 32, 0);
  offset = Header.size + 0x20;
  numBytes = OSRoundUp32B(Header.rebootSize);
  ReadApploader((void *)OS_BOOTROM_ADDR, numBytes, offset);

  ICInvalidateRange((void *)OS_BOOTROM_ADDR, numBytes);
  OSDisableInterrupts();
  ICFlashInvalidate();
  Run((void *)OS_BOOTROM_ADDR);
}

void OSSetSaveRegion(void *start, void *end) {
  SaveStart = start;
  SaveEnd = end;
}

void OSGetSaveRegion(void **start, void **end) {
  *start = SaveStart;
  *end = SaveEnd;
}

void OSGetSavedRegion(void **start, void **end) {
  *start = __OSSavedRegionStart;
  *end = __OSSavedRegionEnd;
}
