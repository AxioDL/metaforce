#include "MetroidPrime/CGBASupport.hpp"
#include "GameVersions.h"
#include "Kyoto/Alloc/CCallStack.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "dolphin/gba.h"
#include "dolphin/os.h"
#include "dolphin/si.h"
#include "rstl/math.hpp"

void joyboot_callback(s32 chan, s32 ret) {}
#if VERSION < VERSION_GM8E_48
const uint MAGIC = 0x414d5445;
#endif

CGBASupport* g_GBA;

inline bool GetFontEncoding() { return OSGetFontEncode() == 1; }

CGBASupport::CGBASupport()
#if VERSION >= VERSION_GM8E_48
: mFile(GetFontEncoding() ? "client_jap.bin" : "client_pad.bin")
#else
: mFile("client_pad.bin")
#endif
, mFileSize(OSRoundUp32B(mFile.Length()))
, mBuffer((uchar*)CMemory::Alloc(mFileSize, IAllocator::kHI_RoundUpLen))
, mDvdReq(mFile.SyncRead(mBuffer.get(), mFileSize))
, mPhase(kP_LoadClientPad)
, mTimeout(0.f)
, mStatus(0)
, mSiChan(-1)
, mFusionLinked(false)
, mFusionBeat(false) {
  GBAInit();
  g_GBA = this;
#if VERSION >= VERSION_GM8E_48
  OSGetFontEncode();
#endif
}

CGBASupport::~CGBASupport() { g_GBA = nullptr; }

void CGBASupport::InitializeSupport() {
  mPhase = kP_Standby;
  mTimeout = 0.f;
  mStatus = 0;
  mSiChan = -1;
  mFusionLinked = false;
  mFusionBeat = false;
}

void CGBASupport::StartLink() {
  mPhase = kP_StartProbeTimeout;
  mSiChan = -1;
}

inline bool CGBASupport::CheckReadyStatus() {
  if (mPhase != kP_LoadClientPad)
    return true;
  if (mDvdReq->IsComplete()) {
    mDvdReq = nullptr;
    mPhase = kP_Standby;
    uchar* buff = mBuffer.get();
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
  switch (mPhase) {
  case kP_LoadClientPad: {
    CheckReadyStatus();
    break;
  }
  case kP_StartProbeTimeout: {
    mTimeout = 4.f;
    mPhase = kP_PollProbe;
    // [[fallthrough]];
  }
  case kP_PollProbe: {
    int channel = 1;
    do {
      uint result = SIProbe(channel);
      if (result == 0x40000) {
        mSiChan = channel;
        mPhase = kP_StartJoyBusBoot;
        mTimeout = 4.f;
        goto end_switch;
      }
      channel++;
    } while (channel < 4);
    float newT = rstl::max_val(0.f, mTimeout - dt);
    mTimeout = newT;
    if (mTimeout == 0.f) {
      mPhase = kP_Failed;
    }
    break;
  }
  case kP_StartJoyBusBoot: {
    mPhase = kP_PollJoyBusBoot;
    GBAJoyBootAsync(mSiChan, mSiChan << 1, 2, mBuffer.get(), mFile.Length(), &mStatus,
                    &joyboot_callback);
    break;
  }
  case kP_PollJoyBusBoot: {
    int status = GBAGetProcessStatus(mSiChan, &mStatus);
    if (status != GBA_BUSY) {
      if (GBAGetStatus(mSiChan, &mStatus) == GBA_NOT_READY) {
        mPhase = kP_Failed;
      } else {
        mTimeout = 4.f;
        mPhase = kP_DataTransfer;
      }
    }
    break;
  }
  case kP_DataTransfer: {
    if (PollResponse()) {
      mPhase = kP_Complete;
      break;
    }
    mTimeout = rstl::max_val(0.f, mTimeout - dt);
    if (mTimeout == 0.f)
      mPhase = kP_Failed;
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
  if (GBAReset(mSiChan, &gbaStatus) == GBA_NOT_READY &&
      GBAReset(mSiChan, &gbaStatus) == GBA_NOT_READY) {
    return false;
  }
  if (GBAGetStatus(mSiChan, &gbaStatus) == GBA_NOT_READY) {
    return false;
  }
  if (gbaStatus != 0x28) {
    return false;
  }

#if VERSION >= VERSION_GM8E_48
  const uint targetMagic = GetFontEncoding() ? 0x414D544A : 0x414D5445;
#endif

  uint magic;
  if (GBARead(mSiChan, (u8*)(&magic), &gbaStatus) == GBA_NOT_READY) {
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
  if (GBAGetStatus(mSiChan, &gbaStatus) == GBA_NOT_READY) {
    return false;
  }
  if (gbaStatus != 0x20) {
    return false;
  }
#if VERSION < VERSION_GM8E_48
  if (GBAWrite(mSiChan, (u8*)(&MAGIC), &gbaStatus) == GBA_NOT_READY) {
    return false;
  }
#else
  if (GBAWrite(mSiChan, (u8*)(&targetMagic), &gbaStatus) == GBA_NOT_READY) {
    return false;
  }
#endif
  if (GBAGetStatus(mSiChan, &gbaStatus) == GBA_NOT_READY) {
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
  } while ((GBAGetStatus(mSiChan, &gbaStatus) == GBA_NOT_READY || (gbaStatus & 0x8) == 0) ||
           (GBAGetStatus(mSiChan, &gbaStatus) != GBA_READY || gbaStatus != 0x38));

  {
    uint read;
    uchar fusionStatus[4];
    if (GBARead(mSiChan, reinterpret_cast< uchar* >(&read), &gbaStatus) != GBA_READY) {
      return false;
    }
    fusionStatus[0] = read >> 24;
    fusionStatus[1] = read >> 16;
    fusionStatus[2] = read >> 8;
    fusionStatus[3] = read;
    if (fusionStatus[3] != CalculateFusionJBusChecksum(fusionStatus, 3)) {
      return false;
    }

    mFusionLinked = (fusionStatus[2] & 0x2) == 0;
    bool fusionBeat = false;
    if (mFusionLinked != false && (fusionStatus[2] & 0x1) > 0) {
      fusionBeat = true;
    }
    mFusionBeat = fusionBeat;
  }

end:
  return true;
}
