#ifndef _CTWEAKTARGETING
#define _CTWEAKTARGETING

#include "MetroidPrime/Tweaks/ITweakObject.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#if VERSION >= VERSION_R3IJ_00
#include "Kyoto/Math/CMayaSpline.hpp"
#endif
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
#if VERSION >= VERSION_R3IJ_00
  float x220_;
  CColor x224_;
  float x228_;
  CColor x22c_;
  float x230_;
  CColor x234_;
  uint x238_; // Unidentified storage; no constructor initialization observed.
  CVector3f x23c_;
  CMayaSpline x248_;
  CMayaSpline x288_;
  float x2c8_;
  float x2cc_;
  CColor x2d0_;
  float x2d4_;
  CColor x2d8_;
  float x2dc_;
  CVector3f x2e0_;
  float x2ec_;
  float x2f0_;
  float x2f4_;
  float x2f8_;
  float x2fc_;
  float x300_;
  CColor x304_;
  float x308_;
  CColor x30c_;
  float x310_;
  float x314_;
  float x318_;
  float x31c_;
  CColor x320_;
  float x324_;
  CColor x328_;
  float x32c_;
  float x330_;
  float x334_;
  float x338_;
  float x33c_;
  float x340_;
  float x344_;
  float x348_;
  float x34c_;
  float x350_;
  float x354_;
  float x358_;
  float x35c_;
  float x360_;
  float x364_;
  CColor x368_;
  CColor x36c_;
  CColor x370_;
  CColor x374_;
  CColor x378_;
  CColor x37c_;
  CColor x380_;
#endif
};

#if VERSION >= VERSION_R3IJ_00
CHECK_SIZEOF(CTweakTargeting, 0x384)
#else
CHECK_SIZEOF(CTweakTargeting, 0x228)
#endif

extern CTweakTargeting* gpTweakTargeting;

#endif // _CTWEAKTARGETING
