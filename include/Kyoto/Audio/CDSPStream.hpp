#ifndef _CDSPSTREAM
#define _CDSPSTREAM

#include "types.h"

#include "dolphin/dvd.h"
#include "musyx/musyx.h"

struct dspadpcm_header {
  uint mNumSamples;
  uint mNumNibbles;
  uint mSampleRate;
  ushort mLoopFlag;
  ushort mFormat;
  uint mLoopStartNibble;
  uint mLoopEndNibble;
  uint mCurrentAddress;
  short mCoef[8][2];
  short mGain;
  short mPredScale;
  short mHist1;
  short mHist2;
  short mLoopPredScale;
  short mLoopHist1;
  short mLoopHist2;
  ushort mPad[11];
};
CHECK_SIZEOF(dspadpcm_header, 0x60);

struct SStreamInfo {
  const char* mFileName;
  ushort mSampleRate;
  ushort mPad0;
  uint mHeaderSize;
  uint mAdpcmBytes;
  bool mLoopFlag;
  uchar mPad1[3];
  uint mLoopStartByte;
  uint mLoopEndByte;
  SND_ADPCMSTREAM_INFO mAdpcmInfo;
};
CHECK_SIZEOF(SStreamInfo, 0x3C);

class CDSPStream {
public:
  static void ReadCompleted(s32 result, DVDFileInfo* fileInfo);

  int InitializeStream();
  static u32 UpdateStream(void* dest, u32 destOffset, void* src, u32 len, u32 user);
  void BufferStream();
  void StopStream();

  static bool IsStreamAvailable(int handle);
  static bool IsStreamActive(int handle);
  static void UpdateVolume(int handle, int vol);
  void UpdateStreamVolume(int vol);

  static void Silence(int handle);
  void SilenceStream();

  static int AllocateStereo(const SStreamInfo& leftInfo, const SStreamInfo& rightInfo, char vol,
                            int oneshot);
  void DeallocateStream();
  static int AllocateMono(const SStreamInfo& info, char vol, char pan, int oneshot);
  uint AllocateStream(const SStreamInfo& info, char vol, char pan);

  static void FreeAllStreams();
  static void Initialize();

private:
  void DoAllocateStream();
  static int FindStreamIdx(int handle);
  static int PickFreeStream(CDSPStream*& streamOut, int oneshot);
  void CloseFiles();
  static void OpenFiles(const char* fileName, CDSPStream& stream);

private:
  uchar mState;
  uchar mOneshot;
  ushort mPad0;
  uint mHandle;
  CDSPStream* mRight;
  CDSPStream* mLeft;
  const char* mFileName;
  ushort mSampleRate;
  ushort mPad1;
  uint mHeaderSize;
  uint mAdpcmBytes;
  bool mLoopFlag;
  uchar mPad2[3];
  uint mLoopStartByte;
  uint mLoopEndByte;
  SND_ADPCMSTREAM_INFO mAdpcmInfo;
  uchar mVol;
  uchar mPan;
  ushort mPad3;
  DVDFileInfo mFileInfo1;
  DVDFileInfo mFileInfo2;
  uint mStreamId;
  uint mFileCur;
  uint mRemaining;
  void* mBuffer;
  uint mBufferBytes;
  uint mStreamSamples;
  uchar mCurBuffer;
  uchar mPad4[3];
  int mNeedsPrime;
  int mSilenced;
  uchar mReadsPending;
  uchar mPad5[3];
  int mStopRequested;
};
CHECK_SIZEOF(CDSPStream, 0xF4);

#endif // _CDSPSTREAM
