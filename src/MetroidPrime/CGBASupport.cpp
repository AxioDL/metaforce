#include "GameVersions.h"
#include "MetroidPrime/CGBASupport.hpp"
#include "Kyoto/Alloc/CCallStack.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "dolphin/gba.h"
#include "dolphin/os.h"
#include "dolphin/os/OSSerial.h"
#include "rstl/math.hpp"

void joyboot_callback(s32 chan, s32 ret) {}
#if VERSION < VERSION_GM8E_48
const uint MAGIC = 0x414d5445;
#endif

CGBASupport* g_GBA;

inline bool GetFontEncoding() { return OSGetFontEncode() == 1; }

CGBASupport::CGBASupport()
#if VERSION >= VERSION_GM8E_48
: x0_file(GetFontEncoding() ? "client_jap.bin" : "client_pad.bin")
#else
: x0_file("client_pad.bin")
#endif
, x28_fileSize(OSRoundUp32B(x0_file.Length()))
, x2c_buffer((uchar*)CMemory::Alloc(x28_fileSize, IAllocator::kHI_RoundUpLen))
, x30_dvdReq(x0_file.SyncRead(x2c_buffer.get(), x28_fileSize))
, x34_phase(kP_LoadClientPad)
, x38_timeout(0.f)
, x3c_status(0)
, x40_siChan(-1)
, x44_fusionLinked(false)
, x45_fusionBeat(false) {
  GBAInit();
  g_GBA = this;
#if VERSION >= VERSION_GM8E_48
  OSGetFontEncode();
#endif
}

CGBASupport::~CGBASupport() { g_GBA = nullptr; }

void CGBASupport::InitializeSupport() {
  x34_phase = kP_Standby;
  x38_timeout = 0.f;
  x3c_status = 0;
  x40_siChan = -1;
  x44_fusionLinked = false;
  x45_fusionBeat = false;
}

void CGBASupport::StartLink() {
  x34_phase = kP_StartProbeTimeout;
  x40_siChan = -1;
}

inline bool CGBASupport::CheckReadyStatus() {
  if (x34_phase != kP_LoadClientPad)
    return true;
  if (x30_dvdReq->IsComplete()) {
    x30_dvdReq = nullptr;
    x34_phase = kP_Standby;
    uchar* buff = x2c_buffer.get();
    u32 tick = OSGetTick();
    buff[0xc8] = (tick >> 0);
    buff[0xc9] = (tick >> 8);
    buff[0xca] = (tick >> 16);
    buff[0xcb] = (tick >> 24);
#if VERSION < VERSION_GM8E_48
    buff[0xaf] = 'E'; // set region to 'E' instead of 'J'
    buff[0xbd] = 0xc9;
#else
    if (GetFontEncoding()) {
      buff[0xaf] = 'J'; // set region to 'E' instead of 'J'
      buff[0xbd] = 0xc4;
    } else {
      buff[0xaf] = 'E'; // set region to 'E' instead of 'J'
      buff[0xbd] = 0xc9;
    }
#endif
    return true;
  }
  return false;
}

bool CGBASupport::IsReady() { return CheckReadyStatus(); }

void CGBASupport::Update(float dt) {
  switch (x34_phase) {
  case kP_LoadClientPad: {
    CheckReadyStatus();
    break;
  }
  case kP_StartProbeTimeout: {
    x38_timeout = 4.f;
    x34_phase = kP_PollProbe;
    // [[fallthrough]];
  }
  case kP_PollProbe: {
    int channel = 1;
    do {
      uint result = SIProbe(channel);
      if (result == 0x40000) {
        x40_siChan = channel;
        x34_phase = kP_StartJoyBusBoot;
        x38_timeout = 4.f;
        goto end_switch;
      }
      channel++;
    } while (channel < 4);
    float newT = rstl::max_val(0.f, x38_timeout - dt);
    x38_timeout = newT;
    if (x38_timeout == 0.f) {
      x34_phase = kP_Failed;
    }
    break;
  }
  case kP_StartJoyBusBoot: {
    x34_phase = kP_PollJoyBusBoot;
    GBAJoyBootAsync(x40_siChan, x40_siChan << 1, 2, x2c_buffer.get(), x0_file.Length(), &x3c_status,
                    &joyboot_callback);
    break;
  }
  case kP_PollJoyBusBoot: {
    int status = GBAGetProcessStatus(x40_siChan, &x3c_status);
    if (status != GBA_BUSY) {
      if (GBAGetStatus(x40_siChan, &x3c_status) == GBA_NOT_READY) {
        x34_phase = kP_Failed;
      } else {
        x38_timeout = 4.f;
        x34_phase = kP_DataTransfer;
      }
    }
    break;
  }
  case kP_DataTransfer: {
    if (PollResponse()) {
      x34_phase = kP_Complete;
      break;
    }
    x38_timeout = rstl::max_val(0.f, x38_timeout - dt);
    if (x38_timeout == 0.f)
      x34_phase = kP_Failed;
    break;
  }
  case kP_Standby:
  case kP_Complete:
  case kP_Failed:
    break;
  }
end_switch:;
}

inline uchar CalculateFusionJBusChecksum(const uchar* data, uint i) {
  // const uchar* data = reinterpret_cast< const uchar* >(dataPtr);
  // uint i = 3;
  uint sum = -1;
  do {
    uchar ch = *data++;
    sum ^= ch;
    for (int j = 0; j < 8; ++j) {
      if ((sum & 1)) {
        sum >>= 1;
        sum ^= 0xb010;
      } else
        sum >>= 1;
    }
  } while (--i);
  return sum;
}

bool CGBASupport::PollResponse() {
  uchar gbaStatus;
  uint unk;

  // Not sure why this is called twice
  if (GBAReset(x40_siChan, &gbaStatus) == GBA_NOT_READY &&
      GBAReset(x40_siChan, &gbaStatus) == GBA_NOT_READY) {
    return false;
  }
  if (GBAGetStatus(x40_siChan, &gbaStatus) == GBA_NOT_READY) {
    return false;
  }
  if (gbaStatus != 0x28) {
    return false;
  }

#if VERSION >= VERSION_GM8E_48
  const uint targetMagic = GetFontEncoding() ? 0x414D544A : 0x414D5445;
#endif

  uint magic;
  if (GBARead(x40_siChan, (u8*)(&magic), &gbaStatus) == GBA_NOT_READY) {
    return false;
  }
#if VERSION < VERSION_GM8E_48
  if (magic != 0x414d5445) { // "AMTE"
    return false;
  }
#else
  if (magic != targetMagic) { // "AMTE"
    return false;
  }
#endif
  if (GBAGetStatus(x40_siChan, &gbaStatus) == GBA_NOT_READY) {
    return false;
  }
  if (gbaStatus != 0x20) {
    return false;
  }
#if VERSION < VERSION_GM8E_48
  if (GBAWrite(x40_siChan, (u8*)(&MAGIC), &gbaStatus) == GBA_NOT_READY) {
    return false;
  }
#else
  if (GBAWrite(x40_siChan, (u8*)(&targetMagic), &gbaStatus) == GBA_NOT_READY) {
    return false;
  }
#endif
  if (GBAGetStatus(x40_siChan, &gbaStatus) == GBA_NOT_READY) {
    return false;
  }
  if ((gbaStatus & 0x30) != 0x30) {
    return false;
  }
  uint start = OSGetTick();
  do {
    uint current = OSGetTick();
    if (OSTicksToMicroseconds(current - start) > 500) {
      goto end;
    }
  } while ((GBAGetStatus(x40_siChan, &gbaStatus) == GBA_NOT_READY || (gbaStatus & 0x8) == 0) ||
           (GBAGetStatus(x40_siChan, &gbaStatus) != GBA_READY || gbaStatus != 0x38));

  {
    uint read;
    uchar fusionStatus[4];
    if (GBARead(x40_siChan, reinterpret_cast< uchar* >(&read), &gbaStatus) != GBA_READY) {
      return false;
    }
    fusionStatus[0] = read >> 24;
    fusionStatus[1] = read >> 16;
    fusionStatus[2] = read >> 8;
    fusionStatus[3] = read;
    if (fusionStatus[3] != CalculateFusionJBusChecksum(fusionStatus, 3)) {
      return false;
    }

    x44_fusionLinked = (fusionStatus[2] & 0x2) == 0;
    bool fusionBeat = false;
    if (x44_fusionLinked != false && (fusionStatus[2] & 0x1) > 0) {
      fusionBeat = true;
    }
    x45_fusionBeat = fusionBeat;
  }

end:
  return true;
}
