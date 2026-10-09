#ifndef _REVOLUTION_WPAD
#define _REVOLUTION_WPAD

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WPAD_MAX_DPD_OBJECTS 4

typedef s32 WPADChannel;
typedef s32 WPADResult;
typedef u32 WPADMotorCommand;

enum {
  WPAD_ERR_OK = 0,
  WPAD_ERR_NO_CONTROLLER = -1,
  WPAD_ERR_BUSY = -2,
  WPAD_ERR_TRANSFER = -3,
  WPAD_ERR_INVALID = -4,
};

enum {
  WPAD_DEV_CORE = 0,
  WPAD_DEV_FS = 1,
  WPAD_DEV_CLASSIC = 2,
};

enum { WPAD_MOTOR_STOP = 0, WPAD_MOTOR_RUMBLE = 1 };

typedef void* WPADAllocFunc(u32 size);
typedef int WPADFreeFunc(void* ptr);
typedef void WPADCallback(WPADChannel chan, WPADResult result);
typedef void WPADConnectCallback(WPADChannel chan, s32 result);
typedef void WPADExtensionCallback(WPADChannel chan, s32 devType);

typedef struct WPADInfo {
  BOOL dpd;
  BOOL speaker;
  BOOL attach;
  BOOL lowBat;
  BOOL nearempty;
  u8 battery;
  u8 led;
  u8 protocol;
  u8 firmware;
} WPADInfo;

typedef struct WPADAcc {
  s16 x;
  s16 y;
  s16 z;
} WPADAcc;

typedef struct DPDObject {
  s16 x;
  s16 y;
  u16 size;
  u8 traceId;
} DPDObject;

typedef struct WPADStatus {
  u16 button;
  s16 accX;
  s16 accY;
  s16 accZ;
  DPDObject obj[WPAD_MAX_DPD_OBJECTS];
  u8 dev;
  s8 err;
} WPADStatus;

typedef struct WPADFSStatus {
  u16 button;
  s16 accX;
  s16 accY;
  s16 accZ;
  DPDObject obj[WPAD_MAX_DPD_OBJECTS];
  u8 dev;
  s8 err;
  s16 fsAccX;
  s16 fsAccY;
  s16 fsAccZ;
  s8 fsStickX;
  s8 fsStickY;
} WPADFSStatus;

typedef struct WPADCLStatus {
  u16 button;
  s16 accX;
  s16 accY;
  s16 accZ;
  DPDObject obj[WPAD_MAX_DPD_OBJECTS];
  u8 dev;
  s8 err;
  u16 clButton;
  s16 clLStickX;
  s16 clLStickY;
  s16 clRStickX;
  s16 clRStickY;
  u8 clTriggerL;
  u8 clTriggerR;
} WPADCLStatus;

void WPADGetAccGravityUnit(s32 chan, u32 type, WPADAcc* acc);
void WPADRegisterAllocator(WPADAllocFunc* alloc, WPADFreeFunc* free);
s32 WPADGetStatus(void);
BOOL WPADSetAcceptConnection(BOOL accept);
WPADConnectCallback* WPADSetConnectCallback(WPADChannel chan, WPADConnectCallback* callback);
WPADExtensionCallback* WPADSetExtensionCallback(WPADChannel chan, WPADExtensionCallback* callback);
WPADResult WPADGetInfoAsync(WPADChannel chan, WPADInfo* info, WPADCallback* callback);
void WPADDisconnect(WPADChannel chan);
void WPADControlMotor(WPADChannel chan, WPADMotorCommand command);

#ifdef __cplusplus
}
#endif

#endif // _REVOLUTION_WPAD
