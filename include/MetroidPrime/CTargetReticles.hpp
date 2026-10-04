#ifndef _CTARGETRETICLES
#define _CTARGETRETICLES

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CAbsAngle.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/vector.hpp"
#include "rstl/reserved_vector.hpp"

class CActor;
class CMatrix3f;
class CModel;
class CScriptGrapplePoint;
class CStateManager;

enum EReticleState {
  kRS_Combat,
  kRS_Scan,
  kRS_XRay,
  kRS_Thermal,
  kRS_Four,
#if VERSION >= VERSION_R3IJ_00
  kRS_Selector,
  kRS_Six,
#endif
  kRS_Unspecified,
};

class CTargetReticleRenderState {
public:
  CTargetReticleRenderState(TUniqueId target, float radiusWorld, CVector3f positionWorld,
                            float factor, float minVpClampScale, bool orbitZoneIdlePosition);

  static void InterpolateWithClamp(const CTargetReticleRenderState& a,
                                   CTargetReticleRenderState& out,
                                   const CTargetReticleRenderState& b, float t);

  void SetTargetId(TUniqueId id) { mTarget = id; }
  void SetFactor(float factor) { mFactor = factor; }
  void SetIsOrbitZoneIdlePosition(bool orbit) { mOrbitZoneIdlePosition = orbit; }
  void SetRadiusWorld(float radius) { mRadiusWorld = radius; }
  void SetTargetPositionWorld(const CVector3f& position) { mPositionWorld = position; }
  void SetMinViewportClampScale(float scale) { mMinVpClampScale = scale; }

  TUniqueId GetTargetId() const { return mTarget; }
  float GetRadiusWorld() const { return mRadiusWorld; }
  CVector3f GetTargetPositionWorld() const { return mPositionWorld; }
  float GetFactor() const { return mFactor; }
  float GetMinViewportClampScale() const { return mMinVpClampScale; }
  bool GetIsOrbitZoneIdlePosition() const { return mOrbitZoneIdlePosition; }

private:
  TUniqueId mTarget;
  float mRadiusWorld;
  CVector3f mPositionWorld;
  float mFactor;
  float mMinVpClampScale;
  bool mOrbitZoneIdlePosition;
};

CHECK_SIZEOF(CTargetReticleRenderState, 0x20)

class CCompoundTargetReticle {
public:
  struct SOuterItemInfo {
    TCachedToken< CModel > mModel;
#if VERSION >= VERSION_R3IJ_00
    CAbsAngle mOffshootBaseAngle;
    CAbsAngle mRotAng;
    CAbsAngle mBaseAngle;
    CRelAngle mOffshootAngleDelta;
#else
    float mOffshootBaseAngle;
    float mRotAng;
    float mBaseAngle;
    float mOffshootAngleDelta;
#endif

    explicit SOuterItemInfo(const char* modelName);
  };

  explicit CCompoundTargetReticle(const CStateManager& mgr);

  void SetLeadingOrientation(const CQuaternion& o) { mLeadingOrientation = o; }
  bool CheckLoadComplete();
  EReticleState GetDesiredReticleState(const CStateManager& mgr) const;
  void Update(float dt, const CStateManager& mgr);
#if VERSION >= VERSION_R3IJ_00
  void UpdateOffScreenReticle(float dt, const CStateManager& mgr);
  void DrawOffScreenReticle(const CMatrix3f& rot, const CStateManager& mgr) const;
  void UpdateOrbitLockPosition(float dt, const CStateManager& mgr);
  bool IsHostileTarget(TUniqueId id, const CStateManager& mgr) const;
  void UpdateCombatAimingReticle(float dt, const CStateManager& mgr);
  void DrawCombatAimingReticle(const CMatrix3f& rot, const CStateManager& mgr) const;
  static bool IsActiveGrappleTarget(TUniqueId id, const CStateManager& mgr);
  void UpdateNextLockOnGroupRS5(float dt, const CStateManager& mgr);
  void UpdateCurrLockOnGroupRS5(float dt, const CStateManager& mgr);
  void DrawNextLockOnGroupRS5(const CMatrix3f& rot, const CStateManager& mgr) const;
  void DrawCurrLockOnGroupRS5(const CMatrix3f& rot, const CStateManager& mgr) const;
  TUniqueId ResolveScanTarget(const CStateManager& mgr, TUniqueId id) const;
  void UpdateScanTargetBounds(const CStateManager& mgr);
  void UpdateScanTargetReticle(float dt, const CStateManager& mgr);
  void DrawScanTargetReticle(const CMatrix3f& rot, const CStateManager& mgr) const;
  float CalculateOrbitZoneReticleDistance(const CStateManager& mgr) const;
  CVector3f ClampToScreenCircle(CVector3f position, const CStateManager& mgr, float radius) const;
#endif
  void UpdateCurrLockOnGroup(float dt, const CStateManager& mgr);
  void UpdateNextLockOnGroup(float dt, const CStateManager& mgr);
  void UpdateOrbitZoneGroup(float dt, const CStateManager& mgr);
  void Draw(const CStateManager& mgr, bool hideLockon) const;
  void DrawGrappleGroup(const CMatrix3f& rot, const CStateManager& mgr, bool hideLockon) const;
  void DrawGrapplePoint(const CScriptGrapplePoint& point, float t, const CStateManager& mgr,
                        const CMatrix3f& rot, bool zEqual) const;
#if VERSION < VERSION_R3IJ_00
  void DrawCurrLockOnGroup(const CMatrix3f& rot, const CStateManager& mgr) const;
  void DrawNextLockOnGroup(const CMatrix3f& rot, const CStateManager& mgr) const;
#endif
  void DrawOrbitZoneGroup(const CMatrix3f& rot, const CStateManager& mgr) const;
  void UpdateTargetParameters(CTargetReticleRenderState& state, const CStateManager& mgr);
  float CalculateRadiusWorld(const CActor& actor, const CStateManager& mgr) const;
  CVector3f CalculatePositionWorld(const CActor& actor, const CStateManager& mgr) const;
#if VERSION >= VERSION_R3IJ_00
  CVector3f CalculateOrbitZoneReticlePosition(const CStateManager& mgr, float distance) const;
#else
  CVector3f CalculateOrbitZoneReticlePosition(const CStateManager& mgr, bool lag) const;
#endif
  static bool IsGrappleTarget(TUniqueId id, const CStateManager& mgr);
  static float CalculateClampedScale(CVector3f pos, float scale, float clampMin,
                                     float clampMax, const CStateManager& mgr);
  void Touch() const;

  TUniqueId GetTargetId() const { return mTargetId; }
  TUniqueId GetNextTargetId() const { return mNextTargetId; }
  const CTargetReticleRenderState& GetCurrGroupInterp() const { return mCurrGroupInterp; }
  const CTargetReticleRenderState& GetNextGroupInterp() const { return mNextGroupInterp; }
  bool GetIsOrbitZoneIdlePosition() const { return mCurrGroupInterp.GetIsOrbitZoneIdlePosition(); }

private:
  CQuaternion mLeadingOrientation;
  CQuaternion mLaggingOrientation;
  EReticleState mPrevState;
  EReticleState mNextState;
  mutable int mNoDrawTicks;
  float mOvershootOffsetHalf;
  float mPremultOvershootOffset;
  TCachedToken< CModel > mCrosshairs;
  TCachedToken< CModel > mSeeker;
  TCachedToken< CModel > mLockConfirm;
  TCachedToken< CModel > mTargetFlower;
  TCachedToken< CModel > mMissileBracket;
  TCachedToken< CModel > mInnerBeamIcon;
  TCachedToken< CModel > mLockFire;
  TCachedToken< CModel > mLockDagger;
  TCachedToken< CModel > mGrapple;
  TCachedToken< CModel > mChargeTickFirst;
  TCachedToken< CModel > mXrayRetRing;
  TCachedToken< CModel > mThermalReticle;
  SOuterItemInfo mChargeGauge;
  rstl::vector< SOuterItemInfo > mOuterBeamIconSquares;
  TUniqueId mTargetId;
  TUniqueId mNextTargetId;
  CVector3f mTargetPos;
  CVector3f mLaggingTargetPos;
  CTargetReticleRenderState mCurrGroupInterp;
  CTargetReticleRenderState mCurrGroupA;
  CTargetReticleRenderState mCurrGroupB;
  float mCurrGroupDur;
  float mCurrGroupTimer;
  CTargetReticleRenderState mNextGroupInterp;
  CTargetReticleRenderState mNextGroupA;
  CTargetReticleRenderState mNextGroupB;
  float mNextGroupDur;
  float mNextGroupTimer;
  TUniqueId mGrapplePoint0;
  TUniqueId mGrapplePoint1;
  float mGrapplePoint0T;
  float mGrapplePoint1T;
  float mCrosshairsScale;
  float mSeekerAngle;
  float mXrayRetAngle;
  bool mMissileActive;
  float mMissileBracketTimer;
  float mMissileBracketScaleTimer;
  CPlayerState::EBeamId mBeam;
  float mChargeGaugeOvershootTimer;
  float mLockonTimer;
  float mUnk;
  float mLockFireTimer;
  float mFullChargeFadeTimer;
#if VERSION >= VERSION_R3IJ_00
  float x214_;
  bool mBeamShot : 1;
  bool mMissileShot : 1;
  bool mFullyCharged : 1;
  float mOrbitPresenceAlpha;
  float x220_;
  float mOrbitTransientAlpha;
  CVector3f mCursorWorldPosition;
  CVector3f mOrbitTargetPosition;
  CVector3f mCursorPlanePosition;
  float x24c_;
  float mAimingScale;
  CColor mAimingArmColor;
  float mAimingArmAlpha;
  CColor mAimingCenterColor;
  float mAimingCenterAlpha;
  float mCursorTargetColorBlend;
  CVector3f mAimingArmOffset;
  float mAimingArmLengthScale;
  float mAimingCenterScale;
  TUniqueId mNextReticleTargetId;
  CVector3f mNextReticlePosition;
  CRelAngle mNextReticleAngle;
  CVector3f mNextReticleScale;
  CColor mNextReticleColor;
  float mNextReticleAlpha;
  CVector3f mNextReticleArmOffset;
  float x2b0_;
  CVector3f mNextReticleTargetPosition;
  CVector3f mNextReticleWorldPosition;
  bool mNextReticleInterpolating;
  float mNextReticleInterpDuration;
  float mNextReticleInterpTime;
  CVector3f mNextReticleDestPosition;
  CVector3f mNextReticleFromPosition;
  TUniqueId mCurrReticleTargetId;
  CVector3f mCurrReticlePosition;
  CColor mCurrReticleArmColor;
  float mCurrReticleArmAlpha;
  CColor mCurrReticleDetailColor;
  float mCurrReticleDetailAlpha;
  float x310_;
  float x314_;
  CVector3f x318_;
  float mCurrReticleLockTime;
  CVector3f mCurrReticleBaseScale;
  float mCurrReticleBracketAlpha;
  CRelAngle mCurrReticleBracketAngle;
  CAbsAngle mCurrReticleBracketHeading;
  float mCurrReticleTechTime;
  CRelAngle mCurrReticleTechAngle;
  CAbsAngle mCurrReticleTechHeading;
  bool mCurrReticleAimHeld;
  float mCurrReticleReleaseAlpha;
  CVector3f x354_;
  CVector3f x360_;
  CColor x36c_;
  float x370_;
  float x374_;
  float x378_;
  float x37c_;
  float x380_;
  float mOffScreenBlinkTime;
  uint mOffScreenFrameCount;
  TLockedToken< CModel > mCombatAimingCenter;
  TLockedToken< CModel > mCombatAimingArm;
  TLockedToken< CModel > mOrbitLockArm;
  TLockedToken< CModel > mOrbitLockTech;
  TLockedToken< CModel > mOrbitLockBrackets;
  TLockedToken< CModel > mOrbitLockBase;
  TLockedToken< CModel > mOffScreen;
  TCachedToken< CModel > mScanReticleRing;
  TCachedToken< CModel > mScanReticleBracket;
  TCachedToken< CModel > mScanReticleProgress;
  float mScanTargetBlend;
  float mScanTargetInterpFactor;
  float mScanningBlend;
  CVector3f mScanTargetFromPosition;
  CVector3f mScanTargetPosition;
  CVector3f mScanTargetFromExtent;
  CVector3f mScanTargetExtent;
  rstl::reserved_vector< TUniqueId, 8 > mScanTargets;
  TUniqueId mScanOrbitTargetId;
  TUniqueId mResolvedScanTargetId;
#else
  bool mBeamShot;
  bool mMissileShot;
  bool mFullyCharged;
#endif
};

#if VERSION >= VERSION_R3IJ_00
CHECK_SIZEOF(CCompoundTargetReticle, 0x458)
#else
CHECK_SIZEOF(CCompoundTargetReticle, 0x21C)
#endif

class COrbitPointMarker {
public:
  COrbitPointMarker();

  bool CheckLoadComplete();
  void Update(float dt, const CStateManager& mgr);
#if VERSION < VERSION_R3IJ_00
  void Draw(const CStateManager& mgr) const;
#endif
  void ResetInterpolationTimer(float time);
  bool IsInterpolating() const { return mInterpTimer > 0.f; }

private:
  float mZOffset;
  bool mCamRelZPos;
  float mLagAzimuth;
  float mAzimuth;
  CVector3f mLagTargetPos;
  bool mLastFreeOrbit;
  float mInterpTimer;
  float mCurTime;
  TCachedToken< CModel > mOrbitPointModel;
};

CHECK_SIZEOF(COrbitPointMarker, 0x34)

class CTargetingManager {
public:
  explicit CTargetingManager(const CStateManager& mgr);

  bool CheckLoadComplete();
  void Update(float dt, const CStateManager& mgr);
  void Draw(const CStateManager& mgr, bool hideLockon) const;
  void Touch() const;

  CCompoundTargetReticle& CompoundTargetReticle() { return mTargetReticle; }
  const CCompoundTargetReticle& GetCompoundTargetReticle() const { return mTargetReticle; }

private:
  CCompoundTargetReticle mTargetReticle;
  COrbitPointMarker mOrbitPointMarker;
};

#if VERSION >= VERSION_R3IJ_00
CHECK_SIZEOF(CTargetingManager, 0x48C)
#else
CHECK_SIZEOF(CTargetingManager, 0x250)
#endif

#endif // _CTARGETRETICLES
