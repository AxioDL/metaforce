#ifndef _REVOLUTION_OS
#define _REVOLUTION_OS

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

BOOL OSDisableInterrupts(void);
BOOL OSEnableInterrupts(void);
BOOL OSRestoreInterrupts(BOOL level);

#ifdef __cplusplus
}
#endif

#endif // _REVOLUTION_OS
