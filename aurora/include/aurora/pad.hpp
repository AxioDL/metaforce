#pragma once
#include "aurora/common.hpp"

namespace PAD {
struct Status {
  u16 x0_buttons;
  s8 x2_stickX;
  s8 x3_stickY;
  s8 x4_substickX;
  s8 x5_substickY;
  u8 x6_triggerL;
  u8 x7_triggerR;
  u8 x8_analogA;
  u8 x9_analogB;
  s8 xa_err;
};

enum BUTTON : u16 {
  BUTTON_LEFT = 0x0001,
  BUTTON_RIGHT = 0x0002,
  BUTTON_DOWN = 0x0004,
  BUTTON_UP = 0x0008,
  TRIGGER_Z = 0x0010,
  TRIGGER_R = 0x0020,
  TRIGGER_L = 0x0040,
  BUTTON_A = 0x0100,
  BUTTON_B = 0x0200,
  BUTTON_X = 0x0400,
  BUTTON_Y = 0x0800,
  BUTTON_START = 0x1000,
};

enum ERROR : s8 {
  ERR_NONE = 0,
  ERR_NO_CONTROLLER = -1,
  ERR_NOT_READY = -2,
  ERR_TRANSFER = -3,
};

enum MOTOR : u32 {
  MOTOR_STOP = 0,
  MOTOR_RUMBLE = 1,
  MOTOR_STOP_HARD = 2,
};
enum CHAN : u32 {
  CHAN0_BIT = 0x80000000,
  CHAN1_BIT = 0x40000000,
  CHAN2_BIT = 0x20000000,
  CHAN3_BIT = 0x10000000,
};
} // namespace PAD

namespace SI {
constexpr u32 ERROR_UNKNOWN = 0x0040;
constexpr u32 ERROR_BUSY = 0x0080;
constexpr u32 ERROR_NO_RESPONSE = 0x0008;
constexpr u32 TYPE_GC = 0x08000000;
constexpr u32 GBA = 0x00040000;
constexpr u32 GC_STANDARD = 0x01000000;
constexpr u32 GC_WIRELESS = 0x80000000;
constexpr u32 WIRELESS_STATE = 0x02000000;
constexpr u32 WIRELESS_FIX_ID = 0x00100000;
constexpr u32 GC_CONTROLLER = (TYPE_GC | GC_STANDARD);
constexpr u32 GC_RECEIVER = (TYPE_GC | GC_WIRELESS);
constexpr u32 GC_WAVEBIRD = (TYPE_GC | GC_WIRELESS | GC_STANDARD | WIRELESS_STATE | WIRELESS_FIX_ID);
} // namespace SI

using PADSamplingCallback = void (*)(void);
constexpr bool PADButtonDown(bool lastButton, bool button) { return ((lastButton ^ button) & button) != 0; }
constexpr bool PADButtonUp(bool lastButton, bool button) { return ((lastButton ^ button) & lastButton) != 0; }
void PADClamp(PAD::Status* status);
void PADClampCircle(PAD::Status* status);
void PADInit();
bool PADIsBarrel(s32 chan);
u32 PADRead(PAD::Status* status);
static bool PADRecalibrate(u32 mask) { return true; }
static bool PADReset(u32 mask) { return true; }
void PADSetAnalog(u32 mode);
void PADSetSpec(s32 spec);
void PADSetSamplingCallback(PADSamplingCallback callback);
void PADControlAllMotors(const u32* commands);

u32 SIProbe(s32 chan);
