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

  char CompanionRight() { return x71_companionRight; }

  dspadpcm_header x0_header ATTRIBUTE_ALIGN(32);
  rstl::string x60_fileName;
  bool x70_24_unclaimed : 1;
  volatile bool x70_25_headerReadCancelled : 1;
  uchar x70_26_headerReadState : 2;
  char x71_companionRight;
  char x72_companionLeft;
  char x73_volume;
  char x74_oneshot;
  int x78_handleId;
  uint x7c_streamId;
  DVDFileInfo x80_dvdFile;
};
CHECK_SIZEOF(CDSPStreamManager, 0xC0);

#endif // _CDSPSTREAMMANAGER
