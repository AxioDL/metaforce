#ifndef _CBALLCAMERA
#define _CBALLCAMERA

#include "types.h"

#include "MetroidPrime/Cameras/CCameraSpline.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"

#include "Kyoto/Math/CMath.hpp"

class CCameraSpring {
public:
  CCameraSpring(float k, float max, float tardis)
  : mK(k), mK2Sqrt(CMath::SqrtF(k) * 2.f), mMax(max), mTardis(tardis), mDx(0.f) {}

  void Reset();
  float ApplyDistanceSpringNoMax(float targetX, float curX, float dt);
  float ApplyDistanceSpring(float targetX, float curX, float dt);

private:
  float mK;
  float mK2Sqrt;
  float mMax;
  float mTardis;
  float mDx;
};
CHECK_SIZEOF(CCameraSpring, 0x14)

class CCameraCollider {
public:
  virtual ~CCameraCollider() {}

  CCameraCollider(float radius, CVector3f vec, const CCameraSpring& spring, float scale)
  : mRadius(radius)
  , mLastLocalPos(vec)
  , mLocalPos(vec)
  , mScaledWorldPos(vec)
  , mLastWorldPos(vec)
  , mSpring(spring)
  , mOcclusionCount(0)
  , mScale(scale) {}

  CCameraCollider(const CCameraCollider& other)
  : mRadius(other.mRadius)
  , mLastLocalPos(other.mLastLocalPos)
  , mLocalPos(other.mLocalPos)
  , mScaledWorldPos(other.mScaledWorldPos)
  , mLastWorldPos(other.mLastWorldPos)
  , mSpring(other.mSpring)
  , mOcclusionCount(other.mOcclusionCount)
  , mScale(other.mScale) {}

  float GetRadius() const { return mRadius; }

  const CVector3f& GetRealPosition() const { return mLastWorldPos; }

  const CVector3f& GetDesiredPosition() const { return mLocalPos; }

  const CVector3f& GetLookAtPosition() const { return mScaledWorldPos; }

  const CVector3f& GetLineOfSight() const;
  const CVector3f& GetPosition() const { return mLastLocalPos; }

  int GetOcclusionCount() const { return mOcclusionCount; }

  float GetScale() const { return mScale; }

  void SetRadius(float radius) { this->mRadius = radius; }

  void SetPosition(CVector3f vec) { mLastLocalPos = vec; }

  void SetRealPosition(CVector3f vec) { mLastWorldPos = vec; }

  void SetDesiredPosition(CVector3f vec) { mLocalPos = vec; }

  void SetLookAtPosition(CVector3f vec) { mScaledWorldPos = vec; }

  void SetLineOfSight();
  void SetOcclusionCount(int val) { mOcclusionCount = val; }

  void SetScale(float val) { mScale = val; }

private:
  float mRadius;
  CVector3f mLastLocalPos;    // position
  CVector3f mLocalPos;       // desired position
  CVector3f mScaledWorldPos; // look at position
  CVector3f mLastWorldPos;   // real position
  CCameraSpring mSpring;
  int mOcclusionCount;
  float mScale;
};
CHECK_SIZEOF(CCameraCollider, 0x54)

class CBallCamera : public CGameCamera {
public:
  enum EBallCameraState {
    kBCS_Default,
    kBCS_One,
    kBCS_Chase,
    kBCS_Boost,
    kBCS_ToBall,
    kBCS_FromBall,
  };
  enum EBallCameraBehaviour {
    kBCB_Default,
    kBCB_FreezeLookPosition, // Unused
    kBCB_HintBallToCam,
    kBCB_HintInitializePosition,
    kBCB_HintFixedPosition,
    kBCB_HintFixedTransform,
    kBCB_PathCameraDesiredPos, // Unused
    kBCB_PathCamera,
    kBCB_SpindleCamera,
  };
  enum ESplineState {
    kBSS_Invalid,
    kBSS_Nav,
    kBSS_Arc,
  };

  // CEntity
  ~CBallCamera() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void Render(const CStateManager&) const override;

  // CGameCamera
  void ProcessInput(const CFinalInput&, CStateManager& mgr) override;
  void Reset(const CTransform4f&, CStateManager& mgr) override;

  // CBallCamera
  CBallCamera(TUniqueId uid, TUniqueId watchedId, const CTransform4f& xf, float fovY, float nearZ,
              float farZ, float aspect);

  void SetupColliders(rstl::vector< CCameraCollider >& out, float xMag, float zMag, float radius,
                      int count, float k, float max, float startAngle);
  void TeleportColliders(rstl::vector< CCameraCollider >& colliderList, CVector3f pos);
  void TeleportCamera(const CVector3f& pos, CStateManager& mgr);
  void TeleportCamera(const CTransform4f& xf, CStateManager& mgr);
  void TeleportLookAtStuff(CStateManager& mgr);
  void ResetToTweaks(CStateManager& mgr);
  CVector3f FindDesiredPosition(float distance, float elevation, CVector3f dir, CStateManager& mgr,
                                bool fullTest);
  void UpdateCollidersDistances(rstl::vector< CCameraCollider >& colliderList, float xMag,
                                float zMag, float angOffset);
  void UpdateColliders(const CTransform4f& xf, rstl::vector< CCameraCollider >& colliderList,
                       int& idx, int count, float tolerance, const TEntityList& nearList, float dt,
                       CStateManager& mgr);
  CVector3f CalculateCollidersCentroid(const rstl::vector< CCameraCollider >& colliderList,
                                       int numObscured) const;
  CVector3f ApplyColliders();
  int CountObscuredColliders(const rstl::vector< CCameraCollider >& colliderList) const;
  CAABox CalculateCollidersBoundingBox(const rstl::vector< CCameraCollider >& colliderList,
                                       const CStateManager&) const;
  CVector3f AvoidGeometryFull(const CTransform4f& xf, const TEntityList& nearList, float dt,
                              CStateManager& mgr);
  CVector3f AvoidGeometry(const CTransform4f& xf, const TEntityList& nearList, float dt,
                          CStateManager& mgr);
  static bool DetectCollision(const CVector3f& from, const CVector3f& to, float radius, float& d,
                              const CStateManager& mgr);
  const bool ConstrainElevationAndDistance(float& elevation, float& distance, float dt,
                                           CStateManager& mgr);
  void UpdateTransform(const CVector3f& lookDir, const CVector3f& pos, float dt,
                       CStateManager& mgr);
  CVector3f ConstrainYawAngle(const CPlayer& player, float distance, float yawSpeed, float dt,
                              CStateManager& mgr);
  CTransform4f FindDesiredTransform(CVector3f dir, CStateManager& mgr);
  CTransform4f UpdateCameraPositions(float dt, const CTransform4f& oldXf,
                                     const CTransform4f& newXf);

  const CVector3f& GetFixedLookPos() const { return mFixedLookPos; }
  const CVector3f& GetLookAtPosition() const { return mLookPos; }

  const CVector3f& GetLookPosAhead() const { return mLookPosAhead; }

  float GetDistance() const { return mCurMinDistance; }

  float GetElevation() const { return mElevation; }

  void SetBehaviourType(EBallCameraBehaviour type) { mBehaviour = type; }

  void SetAllowChaseCamera(bool v) { mChaseAllowed = v; }

  void SetAllowBoostCamera(bool v) { mBoostAllowed = v; }

  // void SetLineOfSightCheck(bool v);
  void SetGeometryAvoidance(bool v) { mObscureAvoidance = true; }

  void SetMinDistance(float v) { mTargetMinDistance = v; }

  void SetMaxDistance(float v) { mMaxDistance = v; }

  void SetBackwardsDistance(float v) { mBackwardsDistance = v; }

  void SetDistanceSpring(const CCameraSpring& spring) { mBallCameraSpring = spring; }

  void SetCentroidDistanceSpring(const CCameraSpring& spring) {
    mBallCameraCentroidDistanceSpring = spring;
  }

  void SetElevation(float v) { mElevation = v; }

  void SetLookAtOffset(CVector3f vec) { mLookAtOffset = vec; }

  TUniqueId GetTooCloseActorId() const { return mTooCloseActorId; }

  float GetTooCloseActorDistance() const { return mTooCloseActorDist; }

  void SetChaseLookAtOffset(CVector3f vec) { mChaseLookAtOffset = vec; }

  void SetWorldOffset(CVector3f vec) { mOverrideBallToCam = vec; }

  EBallCameraState GetState() const { return mState; }

  void SetState(EBallCameraState state, CStateManager& mgr);
  void OverrideCameraInfo(CStateManager& mgr);
  void UpdateUsingColliders(float dt, CStateManager& mgr);
  void UpdateUsingFreeLook(float dt, CStateManager& mgr);
  void UpdateUsingTransitions(float dt, CStateManager& mgr);
  bool UpdateTransitionFromBallCamera(CStateManager& mgr);
  void UpdateUsingPathCameras(float dt, CStateManager& mgr);
  void UpdateUsingSpindleCameras(float dt, CStateManager& mgr);
  void UpdateUsingFixedCameras(float dt, CStateManager& mgr);
  void UpdateLookAtPosition(float dt, CStateManager& mgr);
  CVector3f GetFixedLookTarget(const CVector3f& hintToLookDir, CStateManager& mgr) const;
  void CheckFailSafe(float dt, CStateManager& mgr);
  bool CheckFailsafeFromMorphBallState(CStateManager& mgr);
  bool TransitionFromMorphBallState(CStateManager& mgr);
  static CVector3f GetFailsafeBezierPoint(const rstl::vector< CVector3f >& points, float t);
  static bool CheckTransitionLineOfSight(const CVector3f& eyePos, const CVector3f& behindPos,
                                         float& eyeToOccDist, float colRadius,
                                         const CStateManager& mgr);
  void ActivateFailSafe(float dt, CStateManager& mgr);
  CVector3f ClampElevationToWater(CVector3f pos, CStateManager& mgr) const;
  CVector3f MoveCollisionActor(const CVector3f& pos, float dt, CStateManager& mgr);
  CVector3f TweenVelocity(const CVector3f& curVel, const CVector3f& newVel, float rate, float dt);
  void UpdateObjectTooCloseId(CStateManager& mgr);
  void UpdateAnglePerSecond(float dt);
  CVector3f ComputeVelocity(CVector3f curVel, CVector3f posDelta, float dt);
  bool SplineIntersectTest(CMaterialList& intersectMat, CStateManager& mgr) const;
  void ResetSpline(CStateManager& mgr);
  void BuildSpline(CStateManager& mgr);
  const bool ShouldResetSpline(CStateManager& mgr) const;
  CVector3f InterpolateCameraElevation(CVector3f camPos, float dt);
  void UpdatePlayerMovement(float dt, CStateManager& mgr);

  void SetClampVelTimer(float v) { mClampVelTimer = v; }

  void SetClampVelRange(float v) { mClampVelRange = v; }

  void DoorClosing(TUniqueId uid);
  void DoorClosed(TUniqueId uid);

  static bool CheckDoorProximity(const CVector3f& vec, const CStateManager& mgr);

  EBallCameraBehaviour GetBehaviourType() const { return mBehaviour; }

private:
  struct SFailsafeState {
    CTransform4f mPlayerXf;
    CTransform4f mCamXf;
    CVector3f mLookPos;
    CVector3f mBehindPos;
    CVector3f x78_;
    CVector3f mPlayerPos;
    rstl::vector< CVector3f > mSplinePoints;

    SFailsafeState();
  };
  struct SUnknown {
    SUnknown();
  };

  EBallCameraBehaviour mBehaviour;
  bool x18c_24_ : 1;
  bool mChaseAllowed : 1;
  bool mBoostAllowed : 1;
  bool mObscureAvoidance : 1;
  bool mVolumeCollider : 1;
  bool mClampAttitude : 1;
  bool mClampAzimuth : 1;
  bool mClearLOS : 1;
  bool mPrevClearLOS : 1;
  bool mAvoidGeometryFull : 1;
  bool mLookAtBall : 1;
  bool mForceProcessing : 1;
  bool mObtuseDirection : 1;
  bool mNoElevationInterp : 1;
  bool mDirectElevation : 1;
  bool mOverrideLookDir : 1;
  bool mNoElevationVelClamp : 1;
  bool mNoSpline : 1;
  bool x18e_26_ : 1;
  bool mNearbyDoorClosed : 1;
  bool mNearbyDoorClosing : 1;
  float mCurMinDistance;
  float mTargetMinDistance;
  float mMaxDistance;
  float mBackwardsDistance;
  float mElevation;
  float mCurAnglePerSecond;
  float mTargetAnglePerSecond;
  float mAttitudeRange;
  float mAzimuthRange;
  CVector3f mLookAtOffset;
  CVector3f mLookPosAhead;
  CVector3f mFixedLookPos;
  CVector3f mLookPos;
  CTransform4f mNextLookXf;
  CCameraSpring mBallCameraSpring;
  CCameraSpring mBallCameraCentroidSpring;
  CCameraSpring mBallCameraLookAtSpring;
  CCameraSpring mBallCameraCentroidDistanceSpring;
  rstl::vector< CCameraCollider > mSmallColliders;
  rstl::vector< CCameraCollider > mMediumColliders;
  rstl::vector< CCameraCollider > mLargeColliders;
  CVector3f mDampedPos;
  CVector3f mSmallCentroid;
  CVector3f mMediumCentroid;
  CVector3f mLargeCentroid;
  int mSmallCollidersObsCount;
  int mMediumCollidersObsCount;
  int mLargeCollidersObsCount;
  int mSmallColliderIt;
  int mMediumColliderIt;
  int mLargeColliderIt;
  CVector3f mPrevBallPos;
  float mBallVelFlat;
  float mMaxBallVel;
  CVector3f mBallDelta;
  CVector3f mBallDeltaFlat;
  float mSpeedFactor;
  float mSpeedingTime;
  CVector3f mIdealLookVec;
  CVector3f mPredictedLookPos;
  int mAvoidGeomCycle;
  float mColliderMag;
  float mClearColliderThreshold;
  CAABox mCollidersAABB;
  float mObscuredTime;
  CMaterialList mObscuringMaterial;
  float mUnobscureMag;
  CVector3f mSplineIntermediatePos;
  TUniqueId mObscuringObjectId;
  ESplineState mSplineState;
  bool mReevalSplineEnd : 1;
  float mSplineCtrl;
  float mSplineCtrlRange;
  CCameraSpline mCamSpline;
  CMaterialList mCollisionExcludeList;
  bool mCamBehindFloorOrWall : 1;
  float mElevInterpTimer;
  float mElevInterpStart;
  TUniqueId mTooCloseActorId;
  float mTooCloseActorDist;
  bool mPendingFailsafe;
  float x3e8_;
  float x3ec_;
  float x3f0_;
  float x3f4_;
  float x3f8_;
  float x3fc_;
  EBallCameraState mState;
  float mChaseElevation;
  float mChaseDistance;
  float mChaseAnglePerSecond;
  CVector3f mChaseLookAtOffset;
  CCameraSpring mBallCameraChaseSpring;
  float mBoostElevation;
  float mBoostDistance;
  float mBoostAnglePerSecond;
  CVector3f mBoostLookAtOffset;
  CCameraSpring mBallCameraBoostSpring;
  CVector3f mOverrideBallToCam;
  float mConservativeDoorCamDistance;
  TUniqueId mCollisionActorId;
  float mClampVelTimer;
  float mClampVelRange;
  int mShortMoveCount;
  rstl::single_ptr< SFailsafeState > mFailsafeState;
  rstl::single_ptr< SUnknown > x480_;
};
CHECK_SIZEOF(CBallCamera, (VERSION >= VERSION_GM8E_02 ? 0x498 : 0x488))

#endif // _CBALLCAMERA
