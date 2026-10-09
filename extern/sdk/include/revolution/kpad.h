#ifndef _REVOLUTION_KPAD
#define _REVOLUTION_KPAD

#include "GameVersions.h"

#include <dolphin/mtx/GeoTypes.h>
#include <revolution/wpad.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef s32 KPADChannel;

typedef struct Vec2 {
  f32 x;
  f32 y;
} Vec2;

typedef union KPADEXStatus {
  struct {
    Vec2 stick;
    Vec acc;
    f32 acc_value;
    f32 acc_speed;
  } fs;
  struct {
    u32 hold;
    u32 trig;
    u32 release;
    Vec2 lstick;
    Vec2 rstick;
    f32 ltrigger;
    f32 rtrigger;
  } cl;
#if VERSION >= VERSION_R3MP_00
  struct {
    f64 tgc_weight;
    f64 weight[4];
    f64 weight_ave[4];
    s32 weight_err;
  } bl;
#endif
} KPADEXStatus;

// PAL's balance-board extension grows the US/Japanese 0x84-byte status to 0xb0.
typedef struct KPADStatus {
  u32 hold;
  u32 trig;
  u32 release;
  Vec acc;
  f32 acc_value;
  f32 acc_speed;
  Vec2 pos;
  Vec2 vec;
  f32 speed;
  Vec2 horizon;
  Vec2 hori_vec;
  f32 hori_speed;
  f32 dist;
  f32 dist_vec;
  f32 dist_speed;
  Vec2 acc_vertical;
  u8 dev_type;
  s8 wpad_err;
  s8 dpd_valid_fg;
  u8 data_format;
  KPADEXStatus ex_status;
} KPADStatus;

typedef struct KPADUnifiedWpadStatus {
  union {
    WPADStatus core;
    WPADFSStatus fs;
    WPADCLStatus cl;
  } u;
  u8 fmt;
  u8 padding;
} KPADUnifiedWpadStatus;

void KPADInit(void);
s32 KPADRead(KPADChannel chan, KPADStatus* status, s32 count);
void KPADGetUnifiedWpadStatus(KPADChannel chan, KPADUnifiedWpadStatus* status, u32 count);
void KPADSetPosParam(KPADChannel chan, f32 playRadius, f32 sensitivity);
void KPADSetDistParam(KPADChannel chan, f32 playRadius, f32 sensitivity);
void KPADDisableAimingMode(KPADChannel chan);

#ifdef __cplusplus
}
#endif

#endif // _REVOLUTION_KPAD
