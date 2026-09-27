#ifndef _CTWEAKGAME
#define _CTWEAKGAME

#include "types.h"

#include "MetroidPrime/Tweaks/ITweakObject.hpp"

#include "Kyoto/TOneStatic.hpp"

#include "rstl/string.hpp"

class CTweakGame;

class CTweakGame : public ITweakObject, public TOneStatic< CTweakGame > {
public:
  CTweakGame(CInputStream&);
  ~CTweakGame() override;

  const rstl::string& GetWorldPrefix() { return mWorldPrefix; }
  const rstl::string& GetDefaultRoom() const { return mDefaultRoom; }
  bool GetSplashScreensDisabled() const { return mSplashScreensDisabled; }
  float GetFirstPersonFOV() const { return mFov; }
  float GetPressStartDelay() const { return mPressStartDelay; }
  float GetWavecapIntensityNormal() const { return mWavecapIntensityNormal; }
  float GetWavecapIntensityPoison() const { return mWavecapIntensityPoison; }
  float GetWavecapIntensityLava() const { return mWavecapIntensityLava; }
  float GetRippleIntensityNormal() const { return mRippleIntensityNormal; }
  float GetRippleIntensityPoison() const { return mRippleIntensityPoison; }
  float GetRippleIntensityLava() const { return mRippleIntensityLava; }
  float GetFluidEnvBumpScale() const { return mFluidEnvBumpScale; }
  float GetWaterFogDistanceBase() const { return mWaterFogDistanceBase; }
  float GetWaterFogDistanceRange() const { return mWaterFogDistanceRange; }
  float GetGravityWaterFogDistanceBase() const { return mGravityWaterFogDistanceBase; }
  float GetGravityWaterFogDistanceRange() const { return mGravityWaterFogDistanceRange; }
  float GetHardModeDamageMultiplier() const { return mHardmodeDamageMult; }
  float GetHardModeWeaponMultiplier() const { return mHardmodeWeaponMult; }

// private:
  rstl::string mWorldPrefix;
  rstl::string mDefaultRoom;
  float mFov;
  bool mUnknown1;
  bool mUnknown2;
  bool mUnknown3;
  bool mSplashScreensDisabled;
  float mUnknown5;
  float mPressStartDelay;
  float mWavecapIntensityNormal;
  float mWavecapIntensityPoison;
  float mWavecapIntensityLava;
  float mRippleIntensityNormal;
  float mRippleIntensityPoison;
  float mRippleIntensityLava;
  float mFluidEnvBumpScale;
  float mWaterFogDistanceBase;
  float mWaterFogDistanceRange;
  float mGravityWaterFogDistanceBase;
  float mGravityWaterFogDistanceRange;
  float mHardmodeDamageMult;
  float mHardmodeWeaponMult;
};

extern CTweakGame* gpTweakGame;

#endif // _CTWEAKGAME
