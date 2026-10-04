#ifndef _CTARGETRETICLES
#define _CTARGETRETICLES

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
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
    float mOffshootBaseAngle;
    float mRotAng;
    float mBaseAngle;
    float mOffshootAngleDelta;

    explicit SOuterItemInfo(const char* modelName);
  };

  explicit CCompoundTargetReticle(const CStateManager& mgr);

  void SetLeadingOrientation(const CQuaternion& o) { mLeadingOrientation = o; }
  bool CheckLoadComplete();
  EReticleState GetDesiredReticleState(const CStateManager& mgr) const;
  void Update(float dt, const CStateManager& mgr);
  void UpdateCurrLockOnGroup(float dt, const CStateManager& mgr);
  void UpdateNextLockOnGroup(float dt, const CStateManager& mgr);
  void UpdateOrbitZoneGroup(float dt, const CStateManager& mgr);
  void Draw(const CStateManager& mgr, bool hideLockon) const;
  void DrawGrappleGroup(const CMatrix3f& rot, const CStateManager& mgr, bool hideLockon) const;
  void DrawGrapplePoint(const CScriptGrapplePoint& point, float t, const CStateManager& mgr,
                        const CMatrix3f& rot, bool zEqual) const;
  void DrawCurrLockOnGroup(const CMatrix3f& rot, const CStateManager& mgr) const;
  void DrawNextLockOnGroup(const CMatrix3f& rot, const CStateManager& mgr) const;
  void DrawOrbitZoneGroup(const CMatrix3f& rot, const CStateManager& mgr) const;
  void UpdateTargetParameters(CTargetReticleRenderState& state, const CStateManager& mgr);
  float CalculateRadiusWorld(const CActor& actor, const CStateManager& mgr) const;
  CVector3f CalculatePositionWorld(const CActor& actor, const CStateManager& mgr) const;
  CVector3f CalculateOrbitZoneReticlePosition(const CStateManager& mgr, bool lag) const;
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
  CColor mAimingCenterColor;
  float mAimingCenterAlpha;
  CColor mAimingArmColor;
  float mAimingArmAlpha;
  float mCursorTargetColorBlend;
  CVector3f x268_;
  float mCursorDistanceScaleA;
  float mCursorDistanceScaleB;
  TUniqueId x27c_;
  CVector3f x280_;
  float x28c_;
  CVector3f x290_;
  CColor x29c_;
  float x2a0_;
  CVector3f x2a4_;
  float x2b0_;
  CVector3f x2b4_;
  CVector3f x2c0_;
  bool x2cc_;
  float x2d0_;
  float x2d4_;
  CVector3f x2d8_;
  CVector3f x2e4_;
  TUniqueId x2f0_;
  CVector3f x2f4_;
  CColor x300_;
  float x304_;
  CColor x308_;
  float x30c_;
  float x310_;
  float x314_;
  CVector3f x318_;
  float x324_;
  CVector3f x328_;
  float x334_;
  float x338_;
  float x33c_;
  float x340_;
  float x344_;
  float x348_;
  bool x34c_;
  float x350_;
  CVector3f x354_;
  CVector3f x360_;
  CColor x36c_;
  float x370_;
  float x374_;
  float x378_;
  float x37c_;
  float x380_;
  float mOffScreenBlinkTime;
  int mOffScreenFrameCount;
  TCachedToken< CModel > mCombatAimingCenter;
  TCachedToken< CModel > mCombatAimingArm;
  TCachedToken< CModel > mOrbitLockArm;
  TCachedToken< CModel > mOrbitLockTech;
  TCachedToken< CModel > mOrbitLockBrackets;
  TCachedToken< CModel > mOrbitLockBase;
  TCachedToken< CModel > mOffScreen;
  TCachedToken< CModel > mScanReticleRing;
  TCachedToken< CModel > mScanReticleBracket;
  TCachedToken< CModel > mScanReticleProgress;
  float x404_;
  float x408_;
  float x40c_;
  CVector3f x410_;
  CVector3f mScanTargetPosition;
  CVector3f x428_;
  CVector3f mScanTargetExtent;
  rstl::reserved_vector< TUniqueId, 8 > mScanTargets;
  TUniqueId x454_;
  TUniqueId x456_;
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
  void Draw(const CStateManager& mgr) const;
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
