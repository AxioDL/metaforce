#ifndef _CBALLCAMERA
#define _CBALLCAMERA

#include "types.h"

#include "MetroidPrime/Cameras/CCameraSpline.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"

#include "Kyoto/Math/CMath.hpp"

class CCameraSpring {
public:
  CCameraSpring(float k, float max, float tardis)
  : x0_k(k), x4_k2Sqrt(CMath::SqrtF(k) * 2.f), x8_max(max), xc_tardis(tardis), x10_dx(0.f) {}

  void Reset();
  float ApplyDistanceSpringNoMax(float targetX, float curX, float dt);
  float ApplyDistanceSpring(float targetX, float curX, float dt);

private:
  float x0_k;
  float x4_k2Sqrt;
  float x8_max;
  float xc_tardis;
  float x10_dx;
};
CHECK_SIZEOF(CCameraSpring, 0x14)

class CCameraCollider {
public:
  virtual ~CCameraCollider() {}

  CCameraCollider(float radius, CVector3f vec, const CCameraSpring& spring, float scale)
  : x4_radius(radius)
  , x8_lastLocalPos(vec)
  , x14_localPos(vec)
  , x20_scaledWorldPos(vec)
  , x2c_lastWorldPos(vec)
  , x38_spring(spring)
  , x4c_occlusionCount(0)
  , x50_scale(scale) {}

  CCameraCollider(const CCameraCollider& other)
  : x4_radius(other.x4_radius)
  , x8_lastLocalPos(other.x8_lastLocalPos)
  , x14_localPos(other.x14_localPos)
  , x20_scaledWorldPos(other.x20_scaledWorldPos)
  , x2c_lastWorldPos(other.x2c_lastWorldPos)
  , x38_spring(other.x38_spring)
  , x4c_occlusionCount(other.x4c_occlusionCount)
  , x50_scale(other.x50_scale) {}

  float GetRadius() const { return x4_radius; }

  const CVector3f& GetRealPosition() const { return x2c_lastWorldPos; }

  const CVector3f& GetDesiredPosition() const { return x14_localPos; }

  const CVector3f& GetLookAtPosition() const { return x20_scaledWorldPos; }

  const CVector3f& GetLineOfSight() const;
  const CVector3f& GetPosition() const { return x8_lastLocalPos; }

  int GetOcclusionCount() const { return x4c_occlusionCount; }

  float GetScale() const { return x50_scale; }

  void SetRadius(float radius) { this->x4_radius = radius; }

  void SetPosition(CVector3f vec) { x8_lastLocalPos = vec; }

  void SetRealPosition(CVector3f vec) { x2c_lastWorldPos = vec; }

  void SetDesiredPosition(CVector3f vec) { x14_localPos = vec; }

  void SetLookAtPosition(CVector3f vec) { x20_scaledWorldPos = vec; }

  void SetLineOfSight();
  void SetOcclusionCount(int val) { x4c_occlusionCount = val; }

  void SetScale(float val) { x50_scale = val; }

private:
  float x4_radius;
  CVector3f x8_lastLocalPos;    // position
  CVector3f x14_localPos;       // desired position
  CVector3f x20_scaledWorldPos; // look at position
  CVector3f x2c_lastWorldPos;   // real position
  CCameraSpring x38_spring;
  int x4c_occlusionCount;
  float x50_scale;
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

  const CVector3f& GetFixedLookPos() const { return x1cc_fixedLookPos; }
  const CVector3f& GetLookAtPosition() const { return x1d8_lookPos; }

  const CVector3f& GetLookPosAhead() const { return x1c0_lookPosAhead; }

  float GetDistance() const { return x190_curMinDistance; }

  float GetElevation() const { return x1a0_elevation; }

  void SetBehaviourType(EBallCameraBehaviour type) { x188_behaviour = type; }

  void SetAllowChaseCamera(bool v) { x18c_25_chaseAllowed = v; }

  void SetAllowBoostCamera(bool v) { x18c_26_boostAllowed = v; }

  // void SetLineOfSightCheck(bool v);
  void SetGeometryAvoidance(bool v) { x18c_27_obscureAvoidance = true; }

  void SetMinDistance(float v) { x194_targetMinDistance = v; }

  void SetMaxDistance(float v) { x198_maxDistance = v; }

  void SetBackwardsDistance(float v) { x19c_backwardsDistance = v; }

  void SetDistanceSpring(const CCameraSpring& spring) { x214_ballCameraSpring = spring; }

  void SetCentroidDistanceSpring(const CCameraSpring& spring) {
    x250_ballCameraCentroidDistanceSpring = spring;
  }

  void SetElevation(float v) { x1a0_elevation = v; }

  void SetLookAtOffset(CVector3f vec) { x1b4_lookAtOffset = vec; }

  TUniqueId GetTooCloseActorId() const { return x3dc_tooCloseActorId; }

  float GetTooCloseActorDistance() const { return x3e0_tooCloseActorDist; }

  void SetChaseLookAtOffset(CVector3f vec) { x410_chaseLookAtOffset = vec; }

  void SetWorldOffset(CVector3f vec) { x45c_overrideBallToCam = vec; }

  EBallCameraState GetState() const { return x400_state; }

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

  void SetClampVelTimer(float v) { x470_clampVelTimer = v; }

  void SetClampVelRange(float v) { x474_clampVelRange = v; }

  void DoorClosing(TUniqueId uid);
  void DoorClosed(TUniqueId uid);

  static bool CheckDoorProximity(const CVector3f& vec, const CStateManager& mgr);

  EBallCameraBehaviour GetBehaviourType() const { return x188_behaviour; }

private:
  struct SFailsafeState {
    CTransform4f x0_playerXf;
    CTransform4f x30_camXf;
    CVector3f x60_lookPos;
    CVector3f x6c_behindPos;
    CVector3f x78_;
    CVector3f x84_playerPos;
    rstl::vector< CVector3f > x90_splinePoints;

    SFailsafeState();
  };
  struct SUnknown {
    SUnknown();
  };

  EBallCameraBehaviour x188_behaviour;
  bool x18c_24_ : 1;
  bool x18c_25_chaseAllowed : 1;
  bool x18c_26_boostAllowed : 1;
  bool x18c_27_obscureAvoidance : 1;
  bool x18c_28_volumeCollider : 1;
  bool x18c_29_clampAttitude : 1;
  bool x18c_30_clampAzimuth : 1;
  bool x18c_31_clearLOS : 1;
  bool x18d_24_prevClearLOS : 1;
  bool x18d_25_avoidGeometryFull : 1;
  bool x18d_26_lookAtBall : 1;
  bool x18d_27_forceProcessing : 1;
  bool x18d_28_obtuseDirection : 1;
  bool x18d_29_noElevationInterp : 1;
  bool x18d_30_directElevation : 1;
  bool x18d_31_overrideLookDir : 1;
  bool x18e_24_noElevationVelClamp : 1;
  bool x18e_25_noSpline : 1;
  bool x18e_26_ : 1;
  bool x18e_27_nearbyDoorClosed : 1;
  bool x18e_28_nearbyDoorClosing : 1;
  float x190_curMinDistance;
  float x194_targetMinDistance;
  float x198_maxDistance;
  float x19c_backwardsDistance;
  float x1a0_elevation;
  float x1a4_curAnglePerSecond;
  float x1a8_targetAnglePerSecond;
  float x1ac_attitudeRange;
  float x1b0_azimuthRange;
  CVector3f x1b4_lookAtOffset;
  CVector3f x1c0_lookPosAhead;
  CVector3f x1cc_fixedLookPos;
  CVector3f x1d8_lookPos;
  CTransform4f x1e4_nextLookXf;
  CCameraSpring x214_ballCameraSpring;
  CCameraSpring x228_ballCameraCentroidSpring;
  CCameraSpring x23c_ballCameraLookAtSpring;
  CCameraSpring x250_ballCameraCentroidDistanceSpring;
  rstl::vector< CCameraCollider > x264_smallColliders;
  rstl::vector< CCameraCollider > x274_mediumColliders;
  rstl::vector< CCameraCollider > x284_largeColliders;
  CVector3f x294_dampedPos;
  CVector3f x2a0_smallCentroid;
  CVector3f x2ac_mediumCentroid;
  CVector3f x2b8_largeCentroid;
  int x2c4_smallCollidersObsCount;
  int x2c8_mediumCollidersObsCount;
  int x2cc_largeCollidersObsCount;
  int x2d0_smallColliderIt;
  int x2d4_mediumColliderIt;
  int x2d8_largeColliderIt;
  CVector3f x2dc_prevBallPos;
  float x2e8_ballVelFlat;
  float x2ec_maxBallVel;
  CVector3f x2f0_ballDelta;
  CVector3f x2fc_ballDeltaFlat;
  float x308_speedFactor;
  float x30c_speedingTime;
  CVector3f x310_idealLookVec;
  CVector3f x31c_predictedLookPos;
  int x328_avoidGeomCycle;
  float x32c_colliderMag;
  float x330_clearColliderThreshold;
  CAABox x334_collidersAABB;
  float x34c_obscuredTime;
  CMaterialList x350_obscuringMaterial;
  float x358_unobscureMag;
  CVector3f x35c_splineIntermediatePos;
  TUniqueId x368_obscuringObjectId;
  ESplineState x36c_splineState;
  bool x370_24_reevalSplineEnd : 1;
  float x374_splineCtrl;
  float x378_splineCtrlRange;
  CCameraSpline x37c_camSpline;
  CMaterialList x3c8_collisionExcludeList;
  bool x3d0_24_camBehindFloorOrWall : 1;
  float x3d4_elevInterpTimer;
  float x3d8_elevInterpStart;
  TUniqueId x3dc_tooCloseActorId;
  float x3e0_tooCloseActorDist;
  bool x3e4_pendingFailsafe;
  float x3e8_;
  float x3ec_;
  float x3f0_;
  float x3f4_;
  float x3f8_;
  float x3fc_;
  EBallCameraState x400_state;
  float x404_chaseElevation;
  float x408_chaseDistance;
  float x40c_chaseAnglePerSecond;
  CVector3f x410_chaseLookAtOffset;
  CCameraSpring x41c_ballCameraChaseSpring;
  float x430_boostElevation;
  float x434_boostDistance;
  float x438_boostAnglePerSecond;
  CVector3f x43c_boostLookAtOffset;
  CCameraSpring x448_ballCameraBoostSpring;
  CVector3f x45c_overrideBallToCam;
  float x468_conservativeDoorCamDistance;
  TUniqueId x46c_collisionActorId;
  float x470_clampVelTimer;
  float x474_clampVelRange;
  int x478_shortMoveCount;
  rstl::single_ptr< SFailsafeState > x47c_failsafeState;
  rstl::single_ptr< SUnknown > x480_;
};
CHECK_SIZEOF(CBallCamera, (VERSION >= VERSION_GM8P_00 ? 0x498 : 0x488))

#endif // _CBALLCAMERA
