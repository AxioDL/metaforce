#ifndef _CTARGETRETICLES
#define _CTARGETRETICLES

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/vector.hpp"

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
  bool mBeamShot;
  bool mMissileShot;
  bool mFullyCharged;
};

CHECK_SIZEOF(CCompoundTargetReticle, 0x21C)

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

CHECK_SIZEOF(CTargetingManager, 0x250)

#endif // _CTARGETRETICLES
