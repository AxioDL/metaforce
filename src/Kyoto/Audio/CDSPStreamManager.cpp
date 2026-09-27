#include "Kyoto/Audio/CDSPStreamManager.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CDvdFile.hpp"

#include "dolphin/dvd.h"
#include "dolphin/os.h"
#include "dolphin/os/OSThread.h"

#include <string.h>

#if defined(TARGET_PC)
#include "Metaforce/Audio.hpp"
#endif

CDSPStreamManager g_Streams[4] = {CDSPStreamManager(), CDSPStreamManager(), CDSPStreamManager(),
                                  CDSPStreamManager()};
static int sHandleCounter;

class CInterruptGuard {
  bool mEnabled;

public:
  CInterruptGuard() : mEnabled(OSDisableInterrupts()) {}
  ~CInterruptGuard() { OSRestoreInterrupts(mEnabled); }
};

CDSPStreamManager::CDSPStreamManager(const rstl::string& fileName, int handle, int volume,
                                     bool oneshot)
: mFileName(fileName)
, mUnclaimed(false)
, mHeaderReadCancelled(false)
, mHeaderReadState(kHRS_Unread)
, mCompanionRight(-1)
, mCompanionLeft(-1)
, mVolume(volume)
, mOneshot(oneshot)
, mHandleId(handle)
, mStreamId(-1) {
  if (!CDvdFile::FileExists(fileName.data())) {
    mUnclaimed = true;
    return;
  }
}

CDSPStreamManager::CDSPStreamManager(int unused)
: mFileName(rstl::string_l(""))
, mUnclaimed(true)
, mHeaderReadCancelled(false)
, mHeaderReadState(kHRS_Unread) {
  mCompanionRight = -1;
  mCompanionLeft = -1;
  mVolume = 0;
  mHandleId = -1;
  mStreamId = -1;
}

bool CDSPStreamManager::StartHeaderRead(DVDCallback callback) {
  CInterruptGuard interrupts;
  if (mHeaderReadState != 0 || mUnclaimed) {
    return false;
  }
  if (DVDOpen(const_cast< char* >(mFileName.data()), &mDvdFile) == 0) {
    return false;
  }
  DVDReadAsyncPrio(&mDvdFile, &mHeader, 0x60, 0, callback, 1);
  mHeaderReadState = kHRS_Reading;
  return true;
}

bool CDSPStreamManager::HasSupportedSampleRate() {
#if defined(TARGET_PC)
  return mHeader.mSampleRate > 0 && mHeader.mSampleRate <= 65535;
#else
  return mHeader.mSampleRate == 32000;
#endif
}

void CDSPStreamManager::WaitForReadCompletion() {
#if defined(TARGET_PC)
  BOOL ints = OSEnableInterrupts();
  for (;;) {
    bool complete;
    {
      metaforce::AudioLockGuard lock;
      complete = mHeaderReadState != kHRS_Reading;
    }
    if (complete) {
      OSRestoreInterrupts(ints);
      return;
    }
    OSYieldThread();
  }
#else
  BOOL ints = OSEnableInterrupts();
  while (mHeaderReadState == 1) {
    OSYieldThread();
  }
  OSRestoreInterrupts(ints);
#endif
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
#if defined(TARGET_PC)
  {
    metaforce::AudioLockGuard lock;
    for (auto& stream : g_Streams) stream.mHeaderReadCancelled = true;
  }
  for (auto& stream : g_Streams) stream.WaitForReadCompletion();
#endif
  CDSPStream::FreeAllStreams();
  for (int i = 0; i < 4; ++i) {
    g_Streams[i] = CDSPStreamManager();
  }
}

int CDSPStreamManager::StartStreaming(const rstl::string& fileName, int volume, int oneshot) {
  CInterruptGuard interrupts;
  const bool isOneshot = oneshot;
  const int sep = fileName.find('|', 0);
  if (sep == -1) {
    const int idx = FindUnclaimedStreamIdx();
    if (idx == -1) {
      return -1;
    }

    CDSPStreamManager tmpStream(fileName, GetFreeHandleId(), volume, isOneshot);
    if (!tmpStream.mUnclaimed) {
      CDSPStreamManager& stream = g_Streams[idx];
      stream = tmpStream;
      if (!stream.StartHeaderRead(HeaderReadComplete)) {
        stream = CDSPStreamManager();
        return -1;
      }
      return tmpStream.mHandleId;
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
  if (tmpLeft.mUnclaimed || tmpRight.mUnclaimed) {
    return -1;
  }

  tmpLeft.mCompanionRight = rightIdx;
  tmpRight.mCompanionLeft = leftIdx;
  g_Streams[leftIdx] = tmpLeft;
  g_Streams[rightIdx] = tmpRight;

  const bool rightOk = g_Streams[rightIdx].StartHeaderRead(HeaderReadComplete);
  const bool leftOk = g_Streams[leftIdx].StartHeaderRead(HeaderReadComplete);
  if (!leftOk || !rightOk) {
    CDSPStreamManager& left = g_Streams[leftIdx];
    CDSPStreamManager& right = g_Streams[rightIdx];
    left.mHeaderReadCancelled = true;
    right.mHeaderReadCancelled = true;
    left.WaitForReadCompletion();
    right.WaitForReadCompletion();
    left = CDSPStreamManager();
    right = CDSPStreamManager();
    return -1;
  }
  return tmpLeft.mHandleId;
}

int CDSPStreamManager::FindUnclaimedStreamIdx() {
  for (int i = 0; i < 4; ++i) {
    if (g_Streams[i].mUnclaimed) {
      return i;
    }
  }
  return -1;
}

bool CDSPStreamManager::FindUnclaimedStereoPair(int& left, int& right) {
  const int idx = FindUnclaimedStreamIdx();
  for (int i = 0; i < 4; ++i) {
    if (g_Streams[i].mUnclaimed && idx != i) {
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
        if (!g_Streams[i].mUnclaimed && sHandleCounter == g_Streams[i].mHandleId) {
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
    if (!g_Streams[i].mUnclaimed && handle == g_Streams[i].mHandleId) {
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
  if (stream.mUnclaimed) {
    return;
  }

  if (static_cast< EHeaderReadState >(stream.mHeaderReadState) == 1) {
    stream.mHeaderReadCancelled = true;
    return;
  }

  const char companion = stream.CompanionRight();
  if (companion != -1) {
    g_Streams[companion] = CDSPStreamManager();
  }
  CDSPStream::Silence(stream.mStreamId);
  g_Streams[idx] = CDSPStreamManager();
}

SStreamInfo MakeDSPStreamInfo(const CDSPStreamManager& stream) {
  SStreamInfo info;
  info.mFileName = stream.mFileName.data();
  info.mSampleRate = stream.mHeader.mSampleRate;
  info.mAdpcmBytes = (stream.mHeader.mNumNibbles / 2) & 0x7FFFFFE0;
  info.mHeaderSize = 0x60;
  if (stream.mHeader.mLoopFlag != 0) {
    info.mLoopFlag = true;
    info.mLoopStartByte = (stream.mHeader.mLoopStartNibble / 2) & 0x7FFFFFE0;
    const uint loopEnd = (stream.mHeader.mLoopEndNibble / 2) & 0x7FFFFFE0;
    if (loopEnd > info.mAdpcmBytes) {
      info.mLoopEndByte = info.mAdpcmBytes;
    } else {
      info.mLoopEndByte = loopEnd;
    }
  } else {
    info.mLoopFlag = false;
    info.mLoopStartByte = 0;
    info.mLoopEndByte = 0;
  }
  memcpy(&info.mAdpcmInfo, stream.mHeader.mCoef, sizeof(info.mAdpcmInfo));
  return info;
}

void CDSPStreamManager::UpdateVolume(int handle, int volume) {
  CInterruptGuard interrupts;
  int idx = FindClaimedStreamIdx(handle);
  if (idx == -1) {
    return;
  }
  g_Streams[idx].mVolume = volume;
  if (g_Streams[idx].mStreamId != -1) {
    CDSPStream::UpdateVolume(g_Streams[idx].mStreamId, volume);
  }
}

bool CDSPStreamManager::IsStreamAvailable(int handle) {
  CInterruptGuard interrupts;
  int idx = FindClaimedStreamIdx(handle);
  if (idx == -1) {
    return false;
  }
  if (static_cast< EHeaderReadState >(g_Streams[idx].mHeaderReadState) == 1) {
    return false;
  }
  if (g_Streams[idx].mStreamId == -1) {
    return false;
  }
  return CDSPStream::IsStreamAvailable(g_Streams[idx].mStreamId);
}

bool CDSPStreamManager::CanStop(int handle) {
  CInterruptGuard interrupts;
  int idx = FindClaimedStreamIdx(handle);
  if (idx == -1) {
    return true;
  }
  if (static_cast< EHeaderReadState >(g_Streams[idx].mHeaderReadState) == 1) {
    return false;
  }
  if (g_Streams[idx].mStreamId == -1) {
    return true;
  }
  return !CDSPStream::IsStreamActive(g_Streams[idx].mStreamId);
}

CDSPStreamManager::EState CDSPStreamManager::GetStreamState(int handle) {
  CInterruptGuard interrupts;
  int idx = FindClaimedStreamIdx(handle);
  if (idx == -1) {
    return kCDSPSM_Oneshot;
  }

  switch (static_cast< int >(g_Streams[idx].mHeaderReadState)) {
  case 0:
    return kCDSPSM_Oneshot;
  case 2:
    return g_Streams[idx].mHeader.mLoopFlag ? kCDSPSM_Looping : kCDSPSM_Oneshot;
  default:
    return kCDSPSM_Preparing;
  }
}

void CDSPStreamManager::HeaderReadComplete(s32 result, DVDFileInfo* fileInfo) {
#if defined(TARGET_PC)
  metaforce::AudioLockGuard lock;
#endif
  DVDClose(fileInfo);

  for (int idx = 0; idx < 4; ++idx) {
    CDSPStreamManager* stream = &g_Streams[idx];
    if (&stream->mDvdFile == fileInfo && !stream->mUnclaimed) {
      CInterruptGuard interrupts;
#if defined(TARGET_PC)
      if (result != 0x60 || !metaforce::ReadDSPHeader(
              {reinterpret_cast<const u8*>(&stream->mHeader), 0x60}, stream->mHeader) ||
          fileInfo->length < 0x60 ||
          (u64(stream->mHeader.mNumNibbles) + 1) / 2 > fileInfo->length - 0x60) {
        *stream = CDSPStreamManager();
        return;
      }
#endif
      if (result <= 0 || !stream->HasSupportedSampleRate()) {
        *stream = CDSPStreamManager();
        return;
      }

      stream->mHeaderReadState = kHRS_Read;
      int companion = -1;
      if (stream->mCompanionLeft != -1) {
        companion = stream->mCompanionLeft;
      } else if (stream->mCompanionRight != -1) {
        companion = stream->mCompanionRight;
      }

      if (companion != -1) {
        CDSPStreamManager& other = g_Streams[companion];
        const EHeaderReadState compState =
            static_cast< EHeaderReadState >(other.mHeaderReadState);
        if (other.mUnclaimed || compState == 0 ||
            (idx != other.mCompanionRight && idx != other.mCompanionLeft)) {
          *stream = CDSPStreamManager();
          return;
        }
        if (compState == 1) {
          return;
        }
        if (other.mCompanionRight != -1) {
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
  if (stream.mCompanionRight == -1) {
    if (!stream.mHeaderReadCancelled) {
      stream.mStreamId =
          CDSPStream::AllocateMono(info, stream.mVolume, 0x40, stream.mOneshot);
    }
    if (stream.mStreamId == -1) {
      stream = CDSPStreamManager();
    }
  } else {
    CDSPStreamManager& rstream = g_Streams[stream.mCompanionRight];
    SStreamInfo rinfo = MakeDSPStreamInfo(rstream);
    if (!stream.mHeaderReadCancelled) {
      stream.mStreamId =
          CDSPStream::AllocateStereo(info, rinfo, stream.mVolume, stream.mOneshot);
    }
    if (stream.mStreamId == -1) {
      stream = CDSPStreamManager();
      rstream = CDSPStreamManager();
    }
  }
}
