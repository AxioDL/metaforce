#ifndef _CTWEAKPLAYERRES
#define _CTWEAKPLAYERRES

#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Tweaks/ITweakObject.hpp"

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TOneStatic.hpp"
#include "rstl/reserved_vector.hpp"
#include "types.h"

struct CTweakPlayerRes : public ITweakObject, public TOneStatic< CTweakPlayerRes > {
public:
#if VERSION >= VERSION_GM8P_00
  rstl::reserved_vector< CAssetId, 9 > mMapIcons;
#else
  CAssetId mSaveStationIcon;
  CAssetId mMissileStationIcon;
  CAssetId mElevatorIcon;
  CAssetId mMinesBreakFirstTopIcon;
  CAssetId mMinesBreakFirstBottomIcon;
  CAssetId mMinesBreakSecondTopIcon;
  CAssetId mMinesBreakSecondBottomIcon;
#endif
  rstl::reserved_vector< CAssetId, 9 > mLStick;
  rstl::reserved_vector< CAssetId, 9 > mCStick;
  rstl::reserved_vector< CAssetId, 2 > mLTrigger;
  rstl::reserved_vector< CAssetId, 2 > mRTrigger;
  rstl::reserved_vector< CAssetId, 2 > mStartButton;
  rstl::reserved_vector< CAssetId, 2 > mAButton;
  rstl::reserved_vector< CAssetId, 2 > mBButton;
  rstl::reserved_vector< CAssetId, 2 > mXButton;
  rstl::reserved_vector< CAssetId, 2 > mYButton;
  CAssetId mBallTransitionsANCS;
  CAssetId mBallTransitions[5];
  CAssetId mCineGun[5];
  float mCinematicMoveOutofIntoPlayerDistance;

  CTweakPlayerRes(CInputStream& in);

  CAssetId GetSaveStationIcon() const {
#if VERSION >= VERSION_GM8P_00
    return mMapIcons[0];
#else
    return mSaveStationIcon;
#endif
  }

  CAssetId GetMissileStationIcon() const {
#if VERSION >= VERSION_GM8P_00
    return mMapIcons[1];
#else
    return mMissileStationIcon;
#endif
  }

  CAssetId GetElevatorIcon() const {
#if VERSION >= VERSION_GM8P_00
    return mMapIcons[2];
#else
    return mElevatorIcon;
#endif
  }

  CAssetId GetMinesBreakFirstTopIcon() const {
#if VERSION >= VERSION_GM8P_00
    return mMapIcons[3];
#else
    return mMinesBreakFirstTopIcon;
#endif
  }

  CAssetId GetMinesBreakFirstBottomIcon() const {
#if VERSION >= VERSION_GM8P_00
    return mMapIcons[4];
#else
    return mMinesBreakFirstBottomIcon;
#endif
  }

  float GetCinematicMoveOutofIntoPlayerDistance() const {
    return mCinematicMoveOutofIntoPlayerDistance;
  }

  CAssetId GetBallTransitionANCSId() const { return mBallTransitionsANCS; }

  CAssetId GetBallTransitionBeamResId(CPlayerState::EBeamId id) const;
  CAssetId GetCinematicBeamResId(CPlayerState::EBeamId id) const;
};
CHECK_SIZEOF(CTweakPlayerRes, (VERSION >= VERSION_GM8P_00 ? 0x100 : 0xf4));

extern CTweakPlayerRes* gpTweakPlayerRes;

#endif // _CTWEAKPLAYERRES
