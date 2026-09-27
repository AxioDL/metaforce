#ifndef _CTWEAKTARGETING
#define _CTWEAKTARGETING

#include "MetroidPrime/Tweaks/ITweakObject.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TOneStatic.hpp"

#include "rstl/vector.hpp"

class CTweakTargeting : public ITweakObject, public TOneStatic< CTweakTargeting > {
public:
  CTweakTargeting(CInputStream& in);

  uint mTargetRadiusMode;
  float mCurrLockOnExitDuration;
  float mCurrLockOnEnterDuration;
  float mCurrLockOnSwitchDuration;
  float mLockConfirmScale;
  float mNextLockOnEnterDuration;
  float mNextLockOnExitDuration;
  float mNextLockOnSwitchDuration;
  float mSeekerScale;
  float mSeekerAngleSpeed;
  float mXrayRetAngleSpeed;
  CVector3f x30_;
  CVector3f x3c_;
  float x48_;
  float x4c_;
  float mOrbitPointZOffset;
  float mOrbitPointInTime;
  float mOrbitPointOutTime;
  float x5c_;
  CVector3f x60_;
  CVector3f x6c_;
  CVector3f x78_;
  CVector3f x84_;
  float x90_;
  float x94_;
  float x98_;
  float x9c_;
  float xa0_;
  float xa4_;
  float xa8_;
  float xac_;
  CColor mThermalReticuleColor;
  float mTargetFlowerScale;
  CColor mTargetFlowerColor;
  float mMissileBracketDuration;
  float mMissileBracketScaleStart;
  float mMissileBracketScaleEnd;
  float mMissileBracketScaleDuration;
  CColor mMissileBracketColor;
  float mLockonDuration;
  float mInnerBeamScale;
  CColor mInnerBeamColorPower;
  CColor mInnerBeamColorIce;
  CColor mInnerBeamColorWave;
  CColor mInnerBeamColorPlasma;
  float mChargeGaugeOvershootOffset;
  float mChargeGaugeOvershootDuration;
  float mOuterBeamSquaresScale;
  CColor mOuterBeamSquareColor;
  rstl::vector< rstl::vector< float > > mOuterBeamSquareAngles;
  rstl::vector< float > mChargeGaugeAngles;
  float mChargeGaugeScale;
  CColor mChargeGaugeNonFullColor;
  int mChargeTickCount;
  float mChargeTickAnglePitch;
  float mLockFireScale;
  float mLockFireDuration;
  CColor mLockFireColor;
  float mLockDaggerScaleStart;
  float mLockDaggerScaleEnd;
  CColor mLockDaggerColor;
  float mLockDaggerAngle0;
  float mLockDaggerAngle1;
  float mLockDaggerAngle2;
  CColor mLockConfirmColor;
  CColor mSeekerColor;
  float mLockConfirmClampMin;
  float mLockConfirmClampMax;
  float mTargetFlowerClampMin;
  float mTargetFlowerClampMax;
  float mSeekerClampMin;
  float mSeekerClampMax;
  float mMissileBracketClampMin;
  float mMissileBracketClampMax;
  float mInnerBeamClampMin;
  float mInnerBeamClampMax;
  float mChargeGaugeClampMin;
  float mChargeGaugeClampMax;
  float mLockFireClampMin;
  float mLockFireClampMax;
  float mLockDaggerClampMin;
  float mLockDaggerClampMax;
  float mGrappleSelectScale;
  float mGrappleScale;
  float mGrappleClampMin;
  float mGrappleClampMax;
  CColor mGrapplePointSelectColor;
  CColor mGrapplePointColor;
  CColor mLockedGrapplePointSelectColor;
  float mGrappleMinClampScale;
  CColor mChargeGaugePulseColorHigh;
  float mFullChargeFadeDuration;
  CColor mOrbitPointColor;
  CColor mCrosshairsColor;
  float mCrosshairsScaleDur;
  bool mDrawOrbitPoint;
  CColor mChargeGaugePulseColorLow;
  float mChargeGaugePulsePeriod;
  CColor x1d4_;
  CColor x1d8_;
  CColor x1dc_;
  float x1e0_;
  float x1e4_;
  float x1e8_;
  float x1ec_;
  float x1f0_;
  float x1f4_;
  float x1f8_;
  float x1fc_;
  float x200_;
  float x204_;
  float x208_;
  float mReticuleClampMin;
  float mReticuleClampMax;
  CColor mXrayRetRingColor;
  float mReticuleScale;
  float mScanTargetClampMin;
  float mScanTargetClampMax;
  float mAngularLagSpeed;
};

CHECK_SIZEOF(CTweakTargeting, 0x228)

extern CTweakTargeting* gpTweakTargeting;

#endif // _CTWEAKTARGETING
