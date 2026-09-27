#include "Kyoto/Audio/CDSPStream.hpp"

#include "Kyoto/Alloc/CMemory.hpp"

#include "musyx/musyx.h"

#include "dolphin/dvd.h"
#include "dolphin/os.h"

#include <string.h>
#if defined(TARGET_PC)
#include "Metaforce/Audio.hpp"
#endif

static struct {
  CDSPStream streams[4];
  int handleCounter;
  int pad;
  int unkCounter;
  int pad2;
} g_StreamData;

#define g_Streams g_StreamData.streams
#define sHandleCounter g_StreamData.handleCounter

void CDSPStream::OpenFiles(const char* fileName, CDSPStream& stream) {
  DVDOpen(const_cast< char* >(fileName), &stream.mFileInfo1);
  DVDOpen(const_cast< char* >(fileName), &stream.mFileInfo2);
}

void CDSPStream::CloseFiles() {
  DVDClose(&mFileInfo1);
  DVDClose(&mFileInfo2);
}

int CDSPStream::PickFreeStream(CDSPStream*& streamOut, int oneshot) {
  const CDSPStream* streams = g_Streams;
  for (int i = 0; i < 4; ++i) {
    if (g_Streams[i].mState != 0 || oneshot != g_Streams[i].mOneshot) {
      continue;
    }

    uint handle;
    for (;;) {
      handle = sHandleCounter++;
      if (handle == static_cast< uint >(-1)) {
        handle = sHandleCounter++;
      }

      uint j = 0;
      for (; j < 4; ++j) {
        if (streams[j].mState != 0 && handle == streams[j].mHandle) {
          break;
        }
      }
      if (j == 4) {
        break;
      }
    }

    g_Streams[i].mState = 1;
    g_Streams[i].mHandle = handle;
    g_Streams[i].mRight = nullptr;
    g_Streams[i].mLeft = nullptr;
    streamOut = &g_Streams[i];
    return handle;
  }

  return -1;
}

int CDSPStream::FindStreamIdx(int handle) {
  for (int i = 0; i < 4; ++i) {
    if (g_Streams[i].mState != 0 && static_cast< uint >(handle) == g_Streams[i].mHandle) {
      return i;
    }
  }
  return -1;
}

void CDSPStream::DoAllocateStream() {
  mBuffer = CMemory::Alloc(0x11DC0, IAllocator::kHI_RoundUpLen, IAllocator::kSC_Unk1,
                              IAllocator::kTP_Heap, CCallStack(-1, "??(??)"));
  mStreamId = sndStreamAllocEx(
      static_cast< u8 >(0xFF), mBuffer, mStreamSamples, static_cast< u32 >(32000),
      static_cast< u8 >(0), static_cast< u8 >(0x40), static_cast< u8 >(0), static_cast< u8 >(0),
      static_cast< u8 >(0), static_cast< u8 >(0), static_cast< u32 >(0x30001), UpdateStream,
#if UINTPTR_MAX > UINT32_MAX
      static_cast< u32 >(this - g_Streams), static_cast< SND_ADPCMSTREAM_INFO* >(nullptr));
#else
      reinterpret_cast< uintptr_t >(this), static_cast< SND_ADPCMSTREAM_INFO* >(nullptr));
#endif
}

void CDSPStream::Initialize() {
  for (uint i = 0; i < 4; ++i) {
    CDSPStream& stream = g_Streams[i];
    stream.mState = 0;
    stream.mBuffer = nullptr;
    stream.mBufferBytes = 0x11DC0;
    stream.mStreamSamples = 0x1F410;
    stream.mStreamId = static_cast< uint >(-1);
    if (i < 2) {
      stream.mOneshot = 0;
      stream.DoAllocateStream();
    } else {
      stream.mOneshot = 1;
    }
  }
  sHandleCounter = 0;
  g_StreamData.unkCounter = 0;
}

void CDSPStream::FreeAllStreams() {
#if defined(TARGET_PC)
  {
    metaforce::AudioLockGuard lock;
    for (auto& stream : g_Streams) {
      stream.mSilenced = 1;
      stream.mStopRequested = 1;
      if (stream.mStreamId != SND_ID_ERROR) sndStreamDeactivate(stream.mStreamId);
    }
  }
  // DVD completion takes the same lock. Wait without holding it, and keep
  // refill buffers alive until both requests and their callbacks have retired.
  for (auto& stream : g_Streams) {
    DVDCancel(&stream.mFileInfo1.cb);
    DVDCancel(&stream.mFileInfo2.cb);
  }
  metaforce::AudioLockGuard lock;
  for (auto& stream : g_Streams) {
    if (stream.mState != 0) stream.CloseFiles();
    if (stream.mStreamId != SND_ID_ERROR) sndStreamFree(stream.mStreamId);
    CMemory::Free(stream.mBuffer);
    stream.mBuffer = nullptr;
    stream.mStreamId = SND_ID_ERROR;
    stream.mState = 0;
    stream.mRight = stream.mLeft = nullptr;
  }
#else
  for (uint i = 0; i < 4; ++i) {
    sndStreamFree(g_Streams[i].mStreamId);
    CMemory::Free(g_Streams[i].mBuffer);
  }
#endif
}

uint CDSPStream::AllocateStream(const SStreamInfo& info, char vol, char pan) {
  mFileName = info.mFileName;
  mSampleRate = info.mSampleRate;
  mHeaderSize = info.mHeaderSize;
  mAdpcmBytes = info.mAdpcmBytes;
  mLoopFlag = info.mLoopFlag;
  mLoopStartByte = info.mLoopStartByte;
  mLoopEndByte = info.mLoopEndByte;
  mAdpcmInfo = info.mAdpcmInfo;

  if (mBuffer == nullptr) {
    DoAllocateStream();
  }

  mVol = vol;
  mPan = pan;
  mNeedsPrime = 1;
  mSilenced = 0;
  mStopRequested = 0;
  mReadsPending = 1;
  mCurBuffer = 1;
  mBufferBytes = 0x11DC0;
  mStreamSamples = (mBufferBytes >> 3) * 14;
  if (mBufferBytes > 0x11DC0) {
    mBufferBytes = 0x11DC0;
  }

  uint fileCur = mBufferBytes >> 1;
  mFileCur = fileCur;
  mRemaining = mAdpcmBytes;
  uint ret;
  if (mRemaining < fileCur) {
    ret = mRemaining;
    memset(static_cast< char* >(mBuffer) + ret, 0, fileCur - ret);
  } else {
    ret = fileCur;
  }

  mState = 2;
  return ret;
}

int CDSPStream::AllocateMono(const SStreamInfo& info, char vol, char pan, int oneshot) {
  int handle;
  BOOL ints = OSDisableInterrupts();
  CDSPStream* stream;
  handle = PickFreeStream(stream, oneshot);
  if (static_cast< uint >(handle) != static_cast< uint >(-1)) {
    uint readLen = stream->AllocateStream(info, vol, pan);
    OSRestoreInterrupts(ints);
    OpenFiles(stream->mFileName, *stream);
    DVDReadAsyncPrio(&stream->mFileInfo1, stream->mBuffer, static_cast< s32 >(readLen),
                     static_cast< s32 >(stream->mHeaderSize), ReadCompleted, 1);
    return handle;
  }
  OSRestoreInterrupts(ints);
  return handle;
}

void CDSPStream::DeallocateStream() {
  mState = 0;
  switch (mOneshot) {
  case 0:
    if (mStreamId != static_cast< uint >(-1)) {
      sndStreamDeactivate(mStreamId);
    }
    break;
  case 1:
    if (mBuffer != nullptr) {
      CMemory::Free(mBuffer);
      mBuffer = nullptr;
      if (mStreamId != static_cast< uint >(-1)) {
        sndStreamDeactivate(mStreamId);
        sndStreamFree(mStreamId);
        mStreamId = static_cast< uint >(-1);
      }
    }
    break;
  }
}

int CDSPStream::AllocateStereo(const SStreamInfo& leftInfo, const SStreamInfo& rightInfo, char vol,
                               int oneshot) {
  BOOL ints = OSDisableInterrupts();
  CDSPStream* streams[2];
  int handle = PickFreeStream(streams[0], oneshot);
  if (static_cast< uint >(handle) != static_cast< uint >(-1)) {
    if (static_cast< uint >(PickFreeStream(streams[1], oneshot)) != static_cast< uint >(-1)) {
      streams[0]->mRight = streams[1];
      streams[1]->mLeft = streams[0];
      uint readLen[2];
      readLen[0] = streams[0]->AllocateStream(leftInfo, vol, 0);
      readLen[1] = streams[1]->AllocateStream(rightInfo, vol, 0x7F);
      OSRestoreInterrupts(ints);
      OpenFiles(streams[0]->mFileName, *streams[0]);
      OpenFiles(streams[1]->mFileName, *streams[1]);
      DVDReadAsyncPrio(&streams[0]->mFileInfo1, streams[0]->mBuffer,
                       static_cast< s32 >(readLen[0]),
                       static_cast< s32 >(streams[0]->mHeaderSize), ReadCompleted, 1);
      DVDReadAsyncPrio(&streams[1]->mFileInfo1, streams[1]->mBuffer,
                       static_cast< s32 >(readLen[1]),
                       static_cast< s32 >(streams[1]->mHeaderSize), ReadCompleted, 1);
      return handle;
    }
    streams[0]->DeallocateStream();
    handle = -1;
  }
  OSRestoreInterrupts(ints);
  return handle;
}

void CDSPStream::SilenceStream() {
  if (mState != 0 && mSilenced == 0) {
    sndStreamMixParameterEx(mStreamId, 0, mPan, 0, 0, 0);
    mSilenced = 1;
  }
}

void CDSPStream::Silence(int handle) {
  BOOL ints = OSDisableInterrupts();
  int idx = FindStreamIdx(handle);
  if (static_cast< uint >(idx) != 0xFFFFFFFF) {
    g_Streams[idx].SilenceStream();
    if (g_Streams[idx].mRight != nullptr) {
      g_Streams[idx].mRight->SilenceStream();
    }
    if (g_Streams[idx].mLeft != nullptr) {
      g_Streams[idx].mLeft->SilenceStream();
    }
  }
  OSRestoreInterrupts(ints);
}

void CDSPStream::UpdateStreamVolume(int vol) {
  mVol = vol;
  if (mState != 0 && mSilenced == 0) {
    sndStreamMixParameterEx(mStreamId, mVol, mPan, 0, 0, 0);
  }
}

void CDSPStream::UpdateVolume(int handle, int vol) {
  BOOL ints = OSDisableInterrupts();
  int idx = FindStreamIdx(handle);
  if (static_cast< uint >(idx) != 0xFFFFFFFF) {
    CDSPStream& stream = g_Streams[idx];
    stream.UpdateStreamVolume(vol);
    if (stream.mRight != nullptr) {
      stream.mRight->UpdateStreamVolume(vol);
    }
    if (stream.mLeft != nullptr) {
      stream.mLeft->UpdateStreamVolume(vol);
    }
  }
  OSRestoreInterrupts(ints);
}

bool CDSPStream::IsStreamActive(int handle) {
  bool ret = false;
  BOOL ints = OSDisableInterrupts();
  int idx = FindStreamIdx(handle);
  if (static_cast< uint >(idx) != 0xFFFFFFFF) {
    ret = g_Streams[idx].mState != 0;
  }
  OSRestoreInterrupts(ints);
  return ret;
}

bool CDSPStream::IsStreamAvailable(int handle) {
  bool ret = false;
  BOOL ints = OSDisableInterrupts();
  int idx = FindStreamIdx(handle);
  if (static_cast< uint >(idx) != 0xFFFFFFFF) {
    ret = g_Streams[idx].mState == 4;
  }
  OSRestoreInterrupts(ints);
  return ret;
}

void CDSPStream::StopStream() {
  if (mReadsPending == 0) {
    CloseFiles();
    DeallocateStream();
  } else {
    mStopRequested = 1;
  }
}

void CDSPStream::BufferStream() {
  void* buf;
  uint readLen = mBufferBytes >> 1;
  uint secondReadLen = 0;
  if (mCurBuffer != 0) {
    buf = static_cast< char* >(mBuffer) + readLen;
  } else {
    buf = mBuffer;
  }

  uint endByte;
  if (mLoopFlag != 0) {
    endByte = mLoopEndByte;
  } else {
    endByte = mRemaining;
  }

  if (mFileCur + readLen > endByte) {
    if (endByte > mFileCur) {
      readLen = endByte - mFileCur;
      if (mLoopFlag != 0) {
        secondReadLen = (mBufferBytes >> 1) - readLen;
      } else {
        memset(static_cast< char* >(buf) + readLen, 0, (mBufferBytes >> 1) - readLen);
      }
    } else if (mLoopFlag != 0) {
      mFileCur = mLoopStartByte;
    } else {
      memset(buf, 0, mBufferBytes >> 1);
      sndStreamARAMUpdate(mStreamId, mCurBuffer != 0 ? mStreamSamples >> 1 : 0,
                          mStreamSamples >> 1, 0, 0);
      readLen = 0;
    }
  }

  BOOL ints = OSDisableInterrupts();
  if (readLen != 0) {
    if (mReadsPending == 0) {
      DVDReadAsyncPrio(&mFileInfo1, buf, static_cast< s32 >(readLen),
                       static_cast< s32 >(mHeaderSize + mFileCur), ReadCompleted, 1);
      if (secondReadLen != 0) {
        DVDReadAsyncPrio(&mFileInfo2, static_cast< char* >(buf) + readLen,
                         static_cast< s32 >(secondReadLen),
                         static_cast< s32 >(mHeaderSize + mLoopStartByte), ReadCompleted, 1);
        mReadsPending = 2;
        mFileCur = secondReadLen + mLoopStartByte;
      } else {
        mReadsPending = 1;
        mFileCur += readLen;
      }
    }
  } else if (mNeedsPrime != 0) {
    mNeedsPrime = 0;
  } else {
    StopStream();
  }

  mCurBuffer ^= 1;
  OSRestoreInterrupts(ints);
}

u32 CDSPStream::UpdateStream(void*, u32 destOffset, void*, u32 len, u32 user) {
#if UINTPTR_MAX > UINT32_MAX
  if (user >= sizeof(g_Streams) / sizeof(g_Streams[0])) {
    return 0;
  }
  CDSPStream* stream = &g_Streams[user];
#else
  CDSPStream* stream = reinterpret_cast< CDSPStream* >(user);
#endif
  if (stream->mSilenced != 0) {
    stream->StopStream();
    return 0;
  }

  if (destOffset + len < (stream->mStreamSamples >> 1)) {
    return 0;
  }

  if (stream->mReadsPending == 0) {
    stream->BufferStream();
    return stream->mStreamSamples >> 1;
  }

  return 0;
}

int CDSPStream::InitializeStream() {
  if (mState == 0) {
    return 0;
  }

  sndStreamMixParameterEx(mStreamId, mVol, mPan, 0, 0, 0);
  sndStreamFrq(mStreamId, mSampleRate);
  sndStreamADPCMParameter(mStreamId, &mAdpcmInfo);
  sndStreamARAMUpdate(mStreamId, 0, mStreamSamples >> 1, 0, 0);
  if (sndStreamActivate(mStreamId)) {
    mState = 4;
    BufferStream();
    return 1;
  }

  DeallocateStream();
  CloseFiles();
  return 0;
}

void CDSPStream::ReadCompleted(s32, DVDFileInfo* fileInfo) {
#if defined(TARGET_PC)
  metaforce::AudioLockGuard lock;
#endif
  int idx = 0;
  CDSPStream* s = g_Streams;
  for (; idx < 4; ++idx, ++s) {
    if (&s->mFileInfo1 == fileInfo || &s->mFileInfo2 == fileInfo) {
      break;
    }
  }

#if defined(TARGET_PC)
  if (idx == 4) return;
#endif
  CDSPStream& stream = g_Streams[idx];
  stream.mReadsPending--;
  if (stream.mReadsPending != 0) {
    return;
  }

  if (stream.mStopRequested != 0) {
    stream.mStopRequested = 0;
    stream.CloseFiles();
    stream.DeallocateStream();
    return;
  }

  if (DVDGetCommandBlockStatus(&fileInfo->cb) == 0) {
    switch (stream.mState) {
    case 2:
      if (stream.mRight != nullptr) {
        if (stream.mRight->mState != 3) {
          stream.mState = 3;
          return;
        }
        if (!stream.mRight->InitializeStream()) {
          stream.mRight = nullptr;
        }
      }
      if (stream.mLeft != nullptr) {
        if (stream.mLeft->mState != 3) {
          stream.mState = 3;
          return;
        }
        if (!stream.mLeft->InitializeStream()) {
          stream.mLeft = nullptr;
        }
      }
      if (!stream.InitializeStream()) {
        if (stream.mRight != nullptr) {
          stream.mRight->mLeft = nullptr;
        }
        if (stream.mLeft != nullptr) {
          stream.mLeft->mRight = nullptr;
        }
      }
      break;
    case 4:
      sndStreamARAMUpdate(stream.mStreamId,
                          stream.mCurBuffer != 0 ? 0 : stream.mStreamSamples >> 1,
                          stream.mStreamSamples >> 1, 0, 0);
      break;
    }
  } else {
    if (stream.mState == 4) {
      sndStreamDeactivate(stream.mStreamId);
    }
    stream.CloseFiles();
    stream.DeallocateStream();
    if (stream.mRight != nullptr) {
      stream.mRight->mLeft = nullptr;
    }
    if (stream.mLeft != nullptr) {
      stream.mLeft->mRight = nullptr;
    }
  }
}
