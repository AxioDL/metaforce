#ifndef _CPLAYERVISOR
#define _CPLAYERVISOR

#include "types.h"

#include "MetroidPrime/Cameras/CCameraBlurPass.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/reserved_vector.hpp"

class CModel;
class CStateManager;
class CTargetingManager;
class CTexture;

class CPlayerVisor {
  struct SScanObjectIndicatorInfo {
    SScanObjectIndicatorInfo(TUniqueId id, float timer, float inRangeTimer)
    : mObjId(id), mTimer(timer), mInRangeTimer(inRangeTimer), mInBox(false) {}

    TUniqueId mObjId;
    float mTimer;
    float mInRangeTimer;
    bool mInBox;
  };

  enum EScanWindowState {
    kSWS_NotInScanVisor,
    kSWS_Idle,
    kSWS_Scan,
  };

public:
  CPlayerVisor(const CStateManager& mgr);
  ~CPlayerVisor();

  void Update(float dt, const CStateManager& mgr);
  void Draw(const CStateManager& mgr, const CTargetingManager* tgtMgr) const;
  void Touch() const;
  float GetDesiredViewportScaleX(const CStateManager& mgr) const;
  float GetDesiredViewportScaleY(const CStateManager& mgr) const;

private:
  void BeginTransitionOut();
  void FinishTransitionOut(const CStateManager& mgr);
  void BeginTransitionIn(const CStateManager& mgr);
  void FinishTransitionIn();
  void UpdateCurrentVisor(float transFactor);
  void DrawThermalEffect(const CStateManager& mgr) const;
  void DrawXRayEffect(const CStateManager& mgr) const;
  void DrawScanEffect(const CStateManager& mgr, const CTargetingManager* tgtMgr) const;
  void LockUnlockAssets();
  EScanWindowState GetDesiredScanWindowState(const CStateManager& mgr) const;
  void UpdateScanWindow(float dt, const CStateManager& mgr);
  void UpdateScanObjectIndicators(const CStateManager& mgr, float dt);
  bool DrawScanObjectIndicators(const CStateManager& mgr) const;
  int FindCachedInactiveScanTarget(TUniqueId uid) const;
  int FindEmptyInactiveScanTarget() const;

  rstl::reserved_vector< CVector2f, 3 > mScanWindowSizes;
  CPlayerState::EPlayerVisor mCurVisor;
  CPlayerState::EPlayerVisor mNextVisor;
  uchar mVisorSfxVol;
  bool mVisorTransitioning : 1;
  bool x25_25_ : 1;
  float mScanTimer;
  float mScanDimInterp;
  EScanWindowState mPrevState;
  EScanWindowState mNextState;
  float mWindowInterpDuration;
  float mWindowInterpTimer;
  CVector2f mPrevWindowDims;
  CVector2f mInterpWindowDims;
  CVector2f mNextWindowDims;
  float mScanMagInterp;
  CSfxHandle mVisorLoopSfx;
  CSfxHandle mScanningLoopSfx;
  CCameraFilterPass mScanDim;
  CCameraBlurPass mXrayBlur;
  float mVpScaleX;
  float mVpScaleY;
  TCachedToken< CModel > mScanFrameCorner;
  TCachedToken< CModel > mScanFrameCenterSide;
  TCachedToken< CModel > mScanFrameCenterTop;
  TCachedToken< CModel > mScanFrameStretchSide;
  TCachedToken< CModel > mScanFrameStretchTop;
  TCachedToken< CModel > mNewScanPane;
  TCachedToken< CModel > mScanShield;
  int mAssetLockCountdown;
  mutable TCachedToken< CModel > mScanIconNoncritical;
  mutable TCachedToken< CModel > mScanIconCritical;
  rstl::reserved_vector< SScanObjectIndicatorInfo, 64 > mScanTargets;
  TCachedToken< CTexture > mXrayPalette;
  float mScanFrameColorInterp;
  float mScanFrameColorImpulseInterp;
};
CHECK_SIZEOF(CPlayerVisor, 0x554)

#endif // _CPLAYERVISOR
