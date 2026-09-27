#ifndef _CDSPSTREAMMANAGER
#define _CDSPSTREAMMANAGER

#include "types.h"

#include "Kyoto/Audio/CDSPStream.hpp"
#include "rstl/string.hpp"

class CDSPStreamManager {
  friend SStreamInfo MakeDSPStreamInfo(const CDSPStreamManager& stream);

public:
  enum EState { kCDSPSM_Looping, kCDSPSM_Oneshot, kCDSPSM_Preparing };

  CDSPStreamManager(int unused = 0);
  CDSPStreamManager(const rstl::string& fileName, int handle, int volume, bool oneshot);
  CDSPStreamManager& operator=(const CDSPStreamManager& other);

  static void Initialize();
  static void Shutdown();
  static int StartStreaming(const rstl::string& fileName, int volume, int oneshot);
  static void StopStreaming(int handle);
  static void UpdateVolume(int handle, int volume);
  static bool IsStreamAvailable(int handle);
  static bool CanStop(int handle);
  static EState GetStreamState(int handle);

private:
  static int FindUnclaimedStreamIdx();
  static bool FindUnclaimedStereoPair(int& left, int& right);
  static int GetFreeHandleId();
  static int FindClaimedStreamIdx(int handle);
  static void AllocateStream(int idx);
  static void HeaderReadComplete(s32 result, DVDFileInfo* fileInfo);

  bool StartHeaderRead(DVDCallback callback);
  bool HasSupportedSampleRate();
  void WaitForReadCompletion();

private:
  enum EHeaderReadState { kHRS_Unread, kHRS_Reading, kHRS_Read };

  char CompanionRight() { return mCompanionRight; }

  ATTRIBUTE_ALIGN_DECL(32, dspadpcm_header mHeader);
  rstl::string mFileName;
  bool mUnclaimed : 1;
  volatile bool mHeaderReadCancelled : 1;
  uchar mHeaderReadState : 2;
  char mCompanionRight;
  char mCompanionLeft;
  char mVolume;
  char mOneshot;
  int mHandleId;
  uint mStreamId;
  DVDFileInfo mDvdFile;
};
CHECK_SIZEOF(CDSPStreamManager, 0xC0);

#endif // _CDSPSTREAMMANAGER
