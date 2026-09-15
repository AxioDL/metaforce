#include "Kyoto/Audio/CDSPStreamManager.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CDvdFile.hpp"

#include "dolphin/dvd.h"
#include "dolphin/os.h"
#include "dolphin/os/OSThread.h"

#include <string.h>

CDSPStreamManager g_Streams[4] = {CDSPStreamManager(), CDSPStreamManager(), CDSPStreamManager(),
                                  CDSPStreamManager()};
static int sHandleCounter;

class CInterruptGuard {
  bool x0_enabled;

public:
  CInterruptGuard() : x0_enabled(OSDisableInterrupts()) {}
  ~CInterruptGuard() { OSRestoreInterrupts(x0_enabled); }
};

CDSPStreamManager::CDSPStreamManager(const rstl::string& fileName, int handle, char volume,
                                     bool oneshot)
: x60_fileName(fileName)
, x70_24_unclaimed(false)
, x70_25_headerReadCancelled(false)
, x70_26_headerReadState(kHRS_Unread)
, x71_companionRight(-1)
, x72_companionLeft(-1)
, x73_volume(volume)
, x74_oneshot(oneshot)
, x78_handleId(handle)
, x7c_streamId(-1) {
  if (!CDvdFile::FileExists(fileName.data())) {
    x70_24_unclaimed = true;
    return;
  }
}

CDSPStreamManager::CDSPStreamManager(int unused)
: x60_fileName(rstl::string_l(""))
, x70_24_unclaimed(true)
, x70_25_headerReadCancelled(false)
, x70_26_headerReadState(kHRS_Unread) {
  x71_companionRight = -1;
  x72_companionLeft = -1;
  x73_volume = 0;
  x78_handleId = -1;
  x7c_streamId = -1;
}

bool CDSPStreamManager::StartHeaderRead(DVDCallback callback) {
  CInterruptGuard interrupts;
  if (x70_26_headerReadState != 0 || x70_24_unclaimed) {
    return false;
  }
  if (DVDOpen(const_cast< char* >(x60_fileName.data()), &x80_dvdFile) == 0) {
    return false;
  }
  DVDReadAsyncPrio(&x80_dvdFile, &x0_header, 0x60, 0, callback, 1);
  x70_26_headerReadState = kHRS_Reading;
  return true;
}

bool CDSPStreamManager::HasSupportedSampleRate() { return x0_header.x8_sampleRate == 32000; }

void CDSPStreamManager::WaitForReadCompletion() {
  BOOL ints = OSEnableInterrupts();
  while (x70_26_headerReadState == 1) {
    OSYieldThread();
  }
  OSRestoreInterrupts(ints);
}

CDSPStreamManager& CDSPStreamManager::operator=(const CDSPStreamManager& other) {
  this->~CDSPStreamManager();
  new (this) CDSPStreamManager(other);
  return *this;
}

void CDSPStreamManager::Initialize() {
  CDSPStream::Initialize();
  for (int i = 0; i < 4; ++i) {
    g_Streams[i] = CDSPStreamManager();
  }
}

void CDSPStreamManager::Shutdown() {
  CDSPStream::FreeAllStreams();
  for (int i = 0; i < 4; ++i) {
    g_Streams[i] = CDSPStreamManager();
  }
}

int CDSPStreamManager::StartStreaming(const rstl::string& fileName, char volume, int oneshot) {
  CInterruptGuard interrupts;
  const bool isOneshot = oneshot;
  const int sep = fileName.find('|', 0);
  if (sep == -1) {
    const int idx = FindUnclaimedStreamIdx();
    if (idx == -1) {
      return -1;
    }

    CDSPStreamManager tmpStream(fileName, GetFreeHandleId(), volume, isOneshot);
    if (!tmpStream.x70_24_unclaimed) {
      CDSPStreamManager& stream = g_Streams[idx];
      stream = tmpStream;
      if (!stream.StartHeaderRead(HeaderReadComplete)) {
        stream = CDSPStreamManager();
        return -1;
      }
      return tmpStream.x78_handleId;
    }
    return -1;
  }

  int leftIdx = 0;
  int rightIdx = 0;
  if (!FindUnclaimedStereoPair(leftIdx, rightIdx)) {
    return -1;
  }

  rstl::string leftFile(fileName.data(), sep);
  rstl::string rightFile(fileName.data() + sep + 1, -1);
  CDSPStreamManager tmpLeft(leftFile, GetFreeHandleId(), volume, isOneshot);
  CDSPStreamManager tmpRight(rightFile, GetFreeHandleId(), volume, isOneshot);
  if (tmpLeft.x70_24_unclaimed || tmpRight.x70_24_unclaimed) {
    return -1;
  }

  tmpLeft.x71_companionRight = rightIdx;
  tmpRight.x72_companionLeft = leftIdx;
  g_Streams[leftIdx] = tmpLeft;
  g_Streams[rightIdx] = tmpRight;

  const bool rightOk = g_Streams[rightIdx].StartHeaderRead(HeaderReadComplete);
  const bool leftOk = g_Streams[leftIdx].StartHeaderRead(HeaderReadComplete);
  if (!leftOk || !rightOk) {
    CDSPStreamManager& left = g_Streams[leftIdx];
    CDSPStreamManager& right = g_Streams[rightIdx];
    left.x70_25_headerReadCancelled = true;
    right.x70_25_headerReadCancelled = true;
    left.WaitForReadCompletion();
    right.WaitForReadCompletion();
    left = CDSPStreamManager();
    right = CDSPStreamManager();
    return -1;
  }
  return tmpLeft.x78_handleId;
}

int CDSPStreamManager::FindUnclaimedStreamIdx() {
  for (int i = 0; i < 4; ++i) {
    if (g_Streams[i].x70_24_unclaimed) {
      return i;
    }
  }
  return -1;
}

bool CDSPStreamManager::FindUnclaimedStereoPair(int& left, int& right) {
  const int idx = FindUnclaimedStreamIdx();
  for (int i = 0; i < 4; ++i) {
    if (g_Streams[i].x70_24_unclaimed && idx != i) {
      left = idx;
      right = i;
      return true;
    }
  }
  return false;
}

int CDSPStreamManager::GetFreeHandleId() {
  CInterruptGuard interrupts;
  for (;;) {
    ++sHandleCounter;
    bool good = true;
    if (sHandleCounter == -1) {
      good = false;
    } else {
      for (int i = 0; i < 4; ++i) {
        if (!g_Streams[i].x70_24_unclaimed && sHandleCounter == g_Streams[i].x78_handleId) {
          good = false;
          break;
        }
      }
    }
    if (good) {
      return sHandleCounter;
    }
  }
}

int CDSPStreamManager::FindClaimedStreamIdx(int handle) {
  for (int i = 0; i < 4; ++i) {
    if (!g_Streams[i].x70_24_unclaimed && handle == g_Streams[i].x78_handleId) {
      return i;
    }
  }
  return -1;
}

void CDSPStreamManager::StopStreaming(int handle) {
  CInterruptGuard interrupts;
  int idx = FindClaimedStreamIdx(handle);
  if (idx == -1) {
    return;
  }

  CDSPStreamManager& stream = g_Streams[idx];
  if (stream.x70_24_unclaimed) {
    return;
  }

  if (static_cast< EHeaderReadState >(stream.x70_26_headerReadState) == 1) {
    stream.x70_25_headerReadCancelled = true;
    return;
  }

  const char companion = stream.CompanionRight();
  if (companion != -1) {
    g_Streams[companion] = CDSPStreamManager();
  }
  CDSPStream::Silence(stream.x7c_streamId);
  g_Streams[idx] = CDSPStreamManager();
}

SStreamInfo MakeDSPStreamInfo(const CDSPStreamManager& stream) {
  SStreamInfo info;
  info.x0_fileName = stream.x60_fileName.data();
  info.x4_sampleRate = stream.x0_header.x8_sampleRate;
  info.xc_adpcmBytes = (stream.x0_header.x4_numNibbles / 2) & 0x7FFFFFE0;
  info.x8_headerSize = 0x60;
  if (stream.x0_header.xc_loopFlag != 0) {
    info.x10_loopFlag = true;
    info.x14_loopStartByte = (stream.x0_header.x10_loopStartNibble / 2) & 0x7FFFFFE0;
    const uint loopEnd = (stream.x0_header.x14_loopEndNibble / 2) & 0x7FFFFFE0;
    if (loopEnd > info.xc_adpcmBytes) {
      info.x18_loopEndByte = info.xc_adpcmBytes;
    } else {
      info.x18_loopEndByte = loopEnd;
    }
  } else {
    info.x10_loopFlag = false;
    info.x14_loopStartByte = 0;
    info.x18_loopEndByte = 0;
  }
  memcpy(&info.x1c_adpcmInfo, stream.x0_header.x1c_coef, sizeof(info.x1c_adpcmInfo));
  return info;
}

void CDSPStreamManager::UpdateVolume(int handle, char volume) {
  CInterruptGuard interrupts;
  int idx = FindClaimedStreamIdx(handle);
  if (idx == -1) {
    return;
  }
  g_Streams[idx].x73_volume = volume;
  if (g_Streams[idx].x7c_streamId != -1) {
    CDSPStream::UpdateVolume(g_Streams[idx].x7c_streamId, volume);
  }
}

bool CDSPStreamManager::IsStreamAvailable(int handle) {
  CInterruptGuard interrupts;
  int idx = FindClaimedStreamIdx(handle);
  if (idx == -1) {
    return false;
  }
  if (static_cast< EHeaderReadState >(g_Streams[idx].x70_26_headerReadState) == 1) {
    return false;
  }
  if (g_Streams[idx].x7c_streamId == -1) {
    return false;
  }
  return CDSPStream::IsStreamAvailable(g_Streams[idx].x7c_streamId);
}

bool CDSPStreamManager::CanStop(int handle) {
  CInterruptGuard interrupts;
  int idx = FindClaimedStreamIdx(handle);
  if (idx == -1) {
    return true;
  }
  if (static_cast< EHeaderReadState >(g_Streams[idx].x70_26_headerReadState) == 1) {
    return false;
  }
  if (g_Streams[idx].x7c_streamId == -1) {
    return true;
  }
  return !CDSPStream::IsStreamActive(g_Streams[idx].x7c_streamId);
}

CDSPStreamManager::EState CDSPStreamManager::GetStreamState(int handle) {
  CInterruptGuard interrupts;
  int idx = FindClaimedStreamIdx(handle);
  if (idx == -1) {
    return kCDSPSM_Oneshot;
  }

  switch (static_cast< int >(g_Streams[idx].x70_26_headerReadState)) {
  case 0:
    return kCDSPSM_Oneshot;
  case 2:
    return g_Streams[idx].x0_header.xc_loopFlag ? kCDSPSM_Looping : kCDSPSM_Oneshot;
  default:
    return kCDSPSM_Preparing;
  }
}

void CDSPStreamManager::HeaderReadComplete(s32 result, DVDFileInfo* fileInfo) {
  DVDClose(fileInfo);

  for (int idx = 0; idx < 4; ++idx) {
    CDSPStreamManager* stream = &g_Streams[idx];
    if (&stream->x80_dvdFile == fileInfo && !stream->x70_24_unclaimed) {
      CInterruptGuard interrupts;
      if (result <= 0 || !stream->HasSupportedSampleRate()) {
        *stream = CDSPStreamManager();
        return;
      }

      stream->x70_26_headerReadState = kHRS_Read;
      int companion = -1;
      if (stream->x72_companionLeft != -1) {
        companion = stream->x72_companionLeft;
      } else if (stream->x71_companionRight != -1) {
        companion = stream->x71_companionRight;
      }

      if (companion != -1) {
        CDSPStreamManager& other = g_Streams[companion];
        const EHeaderReadState compState =
            static_cast< EHeaderReadState >(other.x70_26_headerReadState);
        if (other.x70_24_unclaimed || compState == 0 ||
            (idx != other.x71_companionRight && idx != other.x72_companionLeft)) {
          *stream = CDSPStreamManager();
          return;
        }
        if (compState == 1) {
          return;
        }
        if (other.x71_companionRight != -1) {
          AllocateStream(companion);
          return;
        }
      }

      AllocateStream(idx);
      return;
    }
  }
}

void CDSPStreamManager::AllocateStream(int idx) {
  CDSPStreamManager& stream = g_Streams[idx];
  SStreamInfo info = MakeDSPStreamInfo(stream);
  if (stream.x71_companionRight == -1) {
    if (!stream.x70_25_headerReadCancelled) {
      stream.x7c_streamId =
          CDSPStream::AllocateMono(info, stream.x73_volume, 0x40, stream.x74_oneshot);
    }
    if (stream.x7c_streamId == -1) {
      stream = CDSPStreamManager();
    }
  } else {
    CDSPStreamManager& rstream = g_Streams[stream.x71_companionRight];
    SStreamInfo rinfo = MakeDSPStreamInfo(rstream);
    if (!stream.x70_25_headerReadCancelled) {
      stream.x7c_streamId =
          CDSPStream::AllocateStereo(info, rinfo, stream.x73_volume, stream.x74_oneshot);
    }
    if (stream.x7c_streamId == -1) {
      stream = CDSPStreamManager();
      rstream = CDSPStreamManager();
    }
  }
}
