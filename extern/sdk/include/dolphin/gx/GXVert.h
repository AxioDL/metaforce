#ifndef _DOLPHIN_GXVERT
#define _DOLPHIN_GXVERT

#include <dolphin/types.h>

#include "GameVersions.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GXFIFO_ADDR 0xCC008000

typedef union {
  u8 u8;
  u16 u16;
  u32 u32;
  u64 u64;
  s8 s8;
  s16 s16;
  s32 s32;
  s64 s64;
  f32 f32;
  f64 f64;
} PPCWGPipe;

#if defined(__MWERKS__) || defined(__SN__)
#if VERSION >= VERSION_R3IJ_00
volatile PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR);
#else
PPCWGPipe GXWGFifo AT_ADDRESS(GXFIFO_ADDR);
#endif
#else
#define GXWGFifo (*(volatile PPCWGPipe*)GXFIFO_ADDR)
#endif

#ifdef TARGET_PC

void GXPosition3f32(f32 x, f32 y, f32 z);
void GXPosition3u16(u16 x, u16 y, u16 z);
void GXPosition3s16(s16 x, s16 y, s16 z);
void GXPosition3u8(u8 x, u8 y, u8 z);
void GXPosition3s8(s8 x, s8 y, s8 z);

void GXPosition2f32(f32 x, f32 y);
void GXPosition2u16(u16 x, u16 y);
void GXPosition2s16(s16 x, s16 y);
void GXPosition2u8(u8 x, u8 y);
void GXPosition2s8(s8 x, s8 y);

void GXPosition1x16(u16 index);
void GXPosition1x8(u8 index);

void GXNormal3f32(f32 x, f32 y, f32 z);
void GXNormal3s16(s16 x, s16 y, s16 z);
void GXNormal3s8(s8 x, s8 y, s8 z);

void GXNormal1x16(u16 index);
void GXNormal1x8(u8 index);

void GXColor4u8(u8 r, u8 g, u8 b, u8 a);

void GXColor3u8(u8 r, u8 g, u8 b);

void GXColor1u32(u32 clr);
void GXColor1u16(u16 clr);

void GXColor1x16(u16 index);
void GXColor1x8(u8 index);

void GXTexCoord2f32(f32 s, f32 t);
void GXTexCoord2u16(u16 s, u16 t);
void GXTexCoord2s16(s16 s, s16 t);
void GXTexCoord2u8(u8 s, u8 t);
void GXTexCoord2s8(s8 s, s8 t);

void GXTexCoord1f32(f32 s, f32 t);
void GXTexCoord1u16(u16 s, u16 t);
void GXTexCoord1s16(s16 s, s16 t);
void GXTexCoord1u8(u8 s, u8 t);
void GXTexCoord1s8(s8 s, s8 t);

void GXTexCoord1x16(u16 index);
void GXTexCoord1x8(u8 index);

extern void GXEnd(void);

#else

#define FUNC_1PARAM(name, T)                                                                       \
  static inline void name##1##T(T x) { GXWGFifo.T = x; }

#define FUNC_2PARAM(name, T)                                                                       \
  static inline void name##2##T(T x, T y) {                                                        \
    GXWGFifo.T = x;                                                                                \
    GXWGFifo.T = y;                                                                                \
  }

#define FUNC_3PARAM(name, T)                                                                       \
  static inline void name##3##T(T x, T y, T z) {                                                   \
    GXWGFifo.T = x;                                                                                \
    GXWGFifo.T = y;                                                                                \
    GXWGFifo.T = z;                                                                                \
  }

#define FUNC_4PARAM(name, T)                                                                       \
  static inline void name##4##T(T x, T y, T z, T w) {                                              \
    GXWGFifo.T = x;                                                                                \
    GXWGFifo.T = y;                                                                                \
    GXWGFifo.T = z;                                                                                \
    GXWGFifo.T = w;                                                                                \
  }

#define FUNC_INDEX8(name)                                                                          \
  static inline void name##1x8(const u8 x) { GXWGFifo.u8 = x; }

#define FUNC_INDEX16(name)                                                                         \
  static inline void name##1x16(const u16 x) { GXWGFifo.u16 = x; }

// GXCmd
FUNC_1PARAM(GXCmd, u8)
FUNC_1PARAM(GXCmd, u16)
FUNC_1PARAM(GXCmd, u32)

// GXParam
FUNC_1PARAM(GXParam, u8)
FUNC_1PARAM(GXParam, u16)
FUNC_1PARAM(GXParam, u32)
FUNC_1PARAM(GXParam, s8)
FUNC_1PARAM(GXParam, s16)
FUNC_1PARAM(GXParam, s32)
FUNC_1PARAM(GXParam, f32)
FUNC_3PARAM(GXParam, f32)
FUNC_4PARAM(GXParam, f32)

// GXPosition
FUNC_3PARAM(GXPosition, f32)
FUNC_3PARAM(GXPosition, u8)
FUNC_3PARAM(GXPosition, s8)
FUNC_3PARAM(GXPosition, u16)
FUNC_3PARAM(GXPosition, s16)
FUNC_2PARAM(GXPosition, f32)
FUNC_2PARAM(GXPosition, u8)
FUNC_2PARAM(GXPosition, s8)
FUNC_2PARAM(GXPosition, u16)
FUNC_2PARAM(GXPosition, s16)
FUNC_INDEX16(GXPosition)
FUNC_INDEX8(GXPosition)

// GXNormal
FUNC_3PARAM(GXNormal, f32)
FUNC_3PARAM(GXNormal, s16)
FUNC_3PARAM(GXNormal, s8)
FUNC_INDEX16(GXNormal)
FUNC_INDEX8(GXNormal)

// GXColor
FUNC_4PARAM(GXColor, u8)
FUNC_1PARAM(GXColor, u32)
FUNC_3PARAM(GXColor, u8)
FUNC_1PARAM(GXColor, u16)
FUNC_INDEX16(GXColor)
FUNC_INDEX8(GXColor)

// GXTexCoord
FUNC_2PARAM(GXTexCoord, f32)
FUNC_2PARAM(GXTexCoord, s16)
static inline void GXTexCoord2u16(u16 s, u16 t) {
  GXWGFifo.u16 = s;
  GXWGFifo.u16 = t;
}
FUNC_2PARAM(GXTexCoord, s8)
FUNC_2PARAM(GXTexCoord, u8)
FUNC_1PARAM(GXTexCoord, f32)
FUNC_1PARAM(GXTexCoord, s16)
FUNC_1PARAM(GXTexCoord, u16)
FUNC_1PARAM(GXTexCoord, s8)
FUNC_1PARAM(GXTexCoord, u8)
FUNC_INDEX16(GXTexCoord)
FUNC_INDEX8(GXTexCoord)

// GXMatrixIndex
FUNC_1PARAM(GXMatrixIndex, u8)

#undef FUNC_1PARAM
#undef FUNC_2PARAM
#undef FUNC_3PARAM
#undef FUNC_4PARAM
#undef FUNC_INDEX8
#undef FUNC_INDEX16

static inline void GXEnd(void) {}

#endif

#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_GXVERT
