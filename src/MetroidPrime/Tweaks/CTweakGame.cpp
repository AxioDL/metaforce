#include "MetroidPrime/Tweaks/CTweakGame.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CTweakGame::~CTweakGame() {}

CTweakGame::CTweakGame(CInputStream& in)
: mWorldPrefix(in)
, mDefaultRoom(in)
, mFov(in.ReadFloat())
, mUnknown1(in.ReadBool())
, mUnknown2(in.ReadBool())
, mUnknown3(in.ReadBool())
, mSplashScreensDisabled(in.ReadBool())
, mUnknown5(in.ReadFloat())
, mPressStartDelay(in.ReadFloat())
, mWavecapIntensityNormal(in.ReadFloat())
, mWavecapIntensityPoison(in.ReadFloat())
, mWavecapIntensityLava(in.ReadFloat())
, mRippleIntensityNormal(in.ReadFloat())
, mRippleIntensityPoison(in.ReadFloat())
, mRippleIntensityLava(in.ReadFloat())
, mFluidEnvBumpScale(in.ReadFloat())
, mWaterFogDistanceBase(in.ReadFloat())
, mWaterFogDistanceRange(in.ReadFloat())
, mGravityWaterFogDistanceBase(in.ReadFloat())
, mGravityWaterFogDistanceRange(in.ReadFloat())
, mHardmodeDamageMult(in.ReadFloat())
, mHardmodeWeaponMult(in.ReadFloat()) {
  mSplashScreensDisabled = false;
}
