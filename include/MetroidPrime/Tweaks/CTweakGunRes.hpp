#ifndef _CTWEAKGUNRES
#define _CTWEAKGUNRES

#include "types.h"

#include "MetroidPrime/Tweaks/ITweakObject.hpp"

#include "MetroidPrime/Player/CPlayerState.hpp"
#include <Kyoto/TOneStatic.hpp>

#include "Kyoto/CObjectReference.hpp"
#include "rstl/string.hpp"

class CTweakGunRes : public ITweakObject, public TOneStatic< CTweakGunRes > {
public:
  typedef CAssetId ResId;
  typedef rstl::vector< ResId > ResIdVec;

  void GetGrappleBeamFx(CInputStream& in);
  CTweakGunRes(CInputStream& in);
  ~CTweakGunRes() override;

  ResId mGunMotion;
  ResId mGrappleArm;
  ResId mRightHand;

  ResId mPowerBeam;
  ResId mIceBeam;
  ResId mWaveBeam;
  ResId mPlasmaBeam;
  ResId mPhazonBeam;

  ResId mHoloTransition;

  ResId mBombSet;
  ResId mBombExplode;
  ResId mPowerBombExplode;

  /* Power, Ice, Wave, Plasma, Phazon / Beam, Ball */
  ResIdVec mWeaponPower;
  ResIdVec mWeaponIce;
  ResIdVec mWeaponWave;
  ResIdVec mWeaponPlasma;
  ResIdVec mWeaponPhazon;
  ResIdVec mMuzzle;
  ResIdVec mCharge;
  ResIdVec mAuxMuzzle;

  ResId mGrappleSegment;
  ResId mGrappleClaw;
  ResId mGrappleHit;
  ResId mGrappleMuzzle;
  ResId mGrappleSwoosh;

  CAssetId GetAuxMuzzleResId(CPlayerState::EBeamId beam) const {
    if (beam < CPlayerState::kBI_Power || beam > CPlayerState::kBI_Phazon) {
      return kInvalidAssetId;
    }
    return mAuxMuzzle[beam];
  }

  const ResIdVec& GetBeamResIdVec(CPlayerState::EBeamId beam) const;
};

extern CTweakGunRes* gpTweakGunRes;

#endif // _CTWEAKGUNRES
