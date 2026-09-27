#ifndef _CGBASUPPORT
#define _CGBASUPPORT

#include "Kyoto/CDvdFile.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/rc_ptr.hpp"

class CGBASupport {
public:
  enum EPhase {
    kP_LoadClientPad,
    kP_Standby,
    kP_StartProbeTimeout,
    kP_PollProbe,
    kP_StartJoyBusBoot,
    kP_PollJoyBusBoot,
    kP_DataTransfer,
    kP_Complete,
    kP_Failed
  };

private:
  CDvdFile mFile;
  uint mFileSize;
  rstl::single_ptr<uchar> mBuffer;
  rstl::single_ptr<CDvdRequest> mDvdReq;
  EPhase mPhase;
  float mTimeout;
  uchar mStatus;
  uint mSiChan;
  bool mFusionLinked;
  bool mFusionBeat;

// this is inlined weirdly... this probably should be handled differently
  bool CheckReadyStatus();
public:
  static void Initialize();
  static void GlobalPoll();

  CGBASupport();
  ~CGBASupport();
  bool PollResponse();
  void Update(float dt);
  bool IsReady();
  void InitializeSupport();
  void StartLink();
  EPhase GetPhase() const { return mPhase; }
  bool IsFusionLinked() const { return mFusionLinked; }
  bool IsFusionBeat() const { return mFusionBeat; }
};

extern CGBASupport* g_GBA;

#endif // _CGBASUPPORT
