#include "MetroidPrime/Tweaks/CTweakGunRes.hpp"

#include "MetroidPrime/Weapons/WeaponCommon.hpp"
#include "rstl/string.hpp"

CTweakGunRes::~CTweakGunRes() {}
CTweakGunRes::CTweakGunRes(CInputStream& in)
: mGunMotion(0)
, mGrappleArm(0)
, mRightHand(0)
, mPowerBeam(0)
, mIceBeam(0)
, mWaveBeam(0)
, mPlasmaBeam(0)
, mPhazonBeam(0)
, mHoloTransition(0)
, mBombSet(0)
, mBombExplode(0)
, mPowerBombExplode(0) {
  const rstl::string gunMotion(in);
  mGunMotion = NWeaponTypes::get_asset_id_from_name(gunMotion.data());
  const rstl::string grappleArm(in);
  mGrappleArm = NWeaponTypes::get_asset_id_from_name(grappleArm.data());
  const rstl::string rightHand(in);
  mRightHand = NWeaponTypes::get_asset_id_from_name(rightHand.data());
  const rstl::string powerBeam(in);
  mPowerBeam = NWeaponTypes::get_asset_id_from_name(powerBeam.data());
  const rstl::string iceBeam(in);
  mIceBeam = NWeaponTypes::get_asset_id_from_name(iceBeam.data());
  const rstl::string waveBeam(in);
  mWaveBeam = NWeaponTypes::get_asset_id_from_name(waveBeam.data());
  const rstl::string plasmaBeam(in);
  mPlasmaBeam = NWeaponTypes::get_asset_id_from_name(plasmaBeam.data());
  const rstl::string phazonBeam(in);
  mPhazonBeam = NWeaponTypes::get_asset_id_from_name(phazonBeam.data());
  const rstl::string holoTransition(in);
  mHoloTransition = NWeaponTypes::get_asset_id_from_name(holoTransition.data());
  const rstl::string bombSet(in);
  mBombSet = NWeaponTypes::get_asset_id_from_name(bombSet.data());
  const rstl::string bombExplode(in);
  mBombExplode = NWeaponTypes::get_asset_id_from_name(bombExplode.data());
  const rstl::string powerBombExplode(in);
  mPowerBombExplode = NWeaponTypes::get_asset_id_from_name(powerBombExplode.data());
  ResIdVec* vectors[5] = {&mWeaponPower, &mWeaponIce, &mWeaponWave,
                                          &mWeaponPlasma, &mWeaponPhazon};
  for (int i = 0, j; i < 5; ++i) {
    ResIdVec& vec = *vectors[i];
    vec.reserve(vec.size() + 2);
    for (j = 0; j < 2; ++j) {
      const rstl::string name(in);
      vec.push_back(NWeaponTypes::get_asset_id_from_name(name.data()));
    }
  }

  for (int i = 0; i < 5; ++i) {
    mMuzzle.reserve(mMuzzle.size() + 1);
    const rstl::string name(in);
    mMuzzle.push_back(NWeaponTypes::get_asset_id_from_name(name.data()));
  }

  for (int i = 0; i < 5; ++i) {
    mCharge.reserve(mCharge.size() + 1);
    const rstl::string name(in);
    mCharge.push_back(NWeaponTypes::get_asset_id_from_name(name.data()));
  }

  for (int i = 0; i < 5; ++i) {
    mAuxMuzzle.reserve(mAuxMuzzle.size() + 1);
    const rstl::string name(in);
    mAuxMuzzle.push_back(NWeaponTypes::get_asset_id_from_name(name.data()));
  }

  GetGrappleBeamFx(in);
}

inline void CTweakGunRes::GetGrappleBeamFx(CInputStream& in) {
  const rstl::string grappleSegment(in);
  mGrappleSegment = NWeaponTypes::get_asset_id_from_name(grappleSegment.data());
  const rstl::string grappleClaw(in);
  mGrappleClaw = NWeaponTypes::get_asset_id_from_name(grappleClaw.data());
  const rstl::string grappleHit(in);
  mGrappleHit = NWeaponTypes::get_asset_id_from_name(grappleHit.data());
  const rstl::string grappleMuzzle(in);
  mGrappleMuzzle = NWeaponTypes::get_asset_id_from_name(grappleMuzzle.data());
  const rstl::string grappleSwoosh(in);
  mGrappleSwoosh = NWeaponTypes::get_asset_id_from_name(grappleSwoosh.data());
}

const CTweakGunRes::ResIdVec& CTweakGunRes::GetBeamResIdVec(const CPlayerState::EBeamId beam) const {
  const ResIdVec* vectors[5] = {&mWeaponPower, &mWeaponIce, &mWeaponWave,
                                                &mWeaponPlasma, &mWeaponPhazon};
  if (beam < CPlayerState::kBI_Power || beam > CPlayerState::kBI_Phazon) {
    return mWeaponPower;
  }

  return *vectors[beam];
}
