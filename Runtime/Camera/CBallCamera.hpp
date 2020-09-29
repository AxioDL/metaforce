#pragma once

#include <cmath>
#include <memory>
#include <vector>

#include "Runtime/Camera/CCameraSpline.hpp"
#include "Runtime/Camera/CGameCamera.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CPlayer;

class CCameraSpring {
  float x0_k;
  float x4_k2Sqrt;
  float x8_max;
  float xc_tardis;
  float x10_dx = 0.f;

public:
  CCameraSpring(float k, float max, float tardis)
  : x0_k(k), x4_k2Sqrt(2.f * std::sqrt(k)), x8_max(max), xc_tardis(tardis) {}
  void Reset();
  float ApplyDistanceSpringNoMax(float targetX, float curX, float dt);
  float ApplyDistanceSpring(float targetX, float curX, float dt);
};

class CCameraCollider {
  friend class CBallCamera;
  float x4_radius;
  zeus::CVector3f x8_lastLocalPos;
  zeus::CVector3f x14_localPos;
  zeus::CVector3f x20_scaledWorldPos;
  zeus::CVector3f x2c_lastWorldPos;
  CCameraSpring x38_spring;
  u32 x4c_occlusionCount = 0;
  float x50_scale;

public:
  CCameraCollider(float radius, const zeus::CVector3f& vec, const CCameraSpring& spring, float scale)
  : x4_radius(radius)
  , x8_lastLocalPos(vec)
  , x14_localPos(vec)
  , x20_scaledWorldPos(vec)
  , x2c_lastWorldPos(vec)
  , x38_spring(spring)
  , x50_scale(scale) {}
};

class CBallCamera : public CGameCamera {
public:
  enum class EBallCameraState { Default, One, Chase, Boost, ToBall, FromBall };
  enum class EBallCameraBehaviour {
    Default,
    FreezeLookPosition, // Unused
    HintBallToCam,
    HintInitializePosition,
    HintFixedPosition,
    HintFixedTransform,
    PathCameraDesiredPos, // Unused
    PathCamera,
    SpindleCamera
  };
  enum class ESplineState { Invalid, Nav, Arc };

private:
  struct SFailsafeState {
    zeus::CTransform x0_playerXf;
    zeus::CTransform x30_camXf;
    zeus::CVector3f x60_lookPos;
    zeus::CVector3f x6c_behindPos;
    zeus::CVector3f x78_;
    zeus::CVector3f x84_playerPos;
    std::vector<zeus::CVector3f> x90_splinePoints;
  };

  EBallCameraBehaviour x188_behaviour = EBallCameraBehaviour::Default;
  bool x18c_24_ : 1 = true;
  bool x18c_25_chaseAllowed : 1 = true;
  bool x18c_26_boostAllowed : 1 = true;
  bool x18c_27_obscureAvoidance : 1 = true;
  bool x18c_28_volumeCollider : 1 = true;
  bool x18c_29_clampAttitude : 1 = false;
  bool x18c_30_clampAzimuth : 1 = false;
  bool x18c_31_clearLOS : 1 = true;
  bool x18d_24_prevClearLOS : 1 = true;
  bool x18d_25_avoidGeometryFull : 1 = false;
  bool x18d_26_lookAtBall : 1 = false;
  bool x18d_27_forceProcessing : 1 = false;
  bool x18d_28_obtuseDirection : 1 = false;
  bool x18d_29_noElevationInterp : 1 = false;
  bool x18d_30_directElevation : 1 = false;
  bool x18d_31_overrideLookDir : 1 = false;
  bool x18e_24_noElevationVelClamp : 1 = false;
  bool x18e_25_noSpline : 1 = false;
  bool x18e_26_ : 1 = false;
  bool x18e_27_nearbyDoorClosed : 1 = false;
  bool x18e_28_nearbyDoorClosing : 1 = false;
  float x190_curMinDistance;
  float x194_targetMinDistance;
  float x198_maxDistance;
  float x19c_backwardsDistance;
  float x1a0_elevation;
  float x1a4_curAnglePerSecond;
  float x1a8_targetAnglePerSecond;
  float x1ac_attitudeRange = zeus::degToRad(89.f);
  float x1b0_azimuthRange = zeus::degToRad(89.f);
  zeus::CVector3f x1b4_lookAtOffset;
  zeus::CVector3f x1c0_lookPosAhead;
  zeus::CVector3f x1cc_fixedLookPos;
  zeus::CVector3f x1d8_lookPos;
  zeus::CTransform x1e4_nextLookXf;
  CCameraSpring x214_ballCameraSpring;
  CCameraSpring x228_ballCameraCentroidSpring;
  CCameraSpring x23c_ballCameraLookAtSpring;
  CCameraSpring x250_ballCameraCentroidDistanceSpring;
  std::vector<CCameraCollider> x264_smallColliders;
  std::vector<CCameraCollider> x274_mediumColliders;
  std::vector<CCameraCollider> x284_largeColliders;
  zeus::CVector3f x294_dampedPos;
  zeus::CVector3f x2a0_smallCentroid = zeus::skUp;
  zeus::CVector3f x2ac_mediumCentroid = zeus::skUp;
  zeus::CVector3f x2b8_largeCentroid = zeus::skUp;
  int x2c4_smallCollidersObsCount = 0;
  int x2c8_mediumCollidersObsCount = 0;
  int x2cc_largeCollidersObsCount = 0;
  int x2d0_smallColliderIt = 0;
  int x2d4_mediumColliderIt = 0;
  int x2d8_largeColliderIt = 0;
  zeus::CVector3f x2dc_prevBallPos;
  float x2e8_ballVelFlat = 0.f;
  float x2ec_maxBallVel = 0.f;
  zeus::CVector3f x2f0_ballDelta;
  zeus::CVector3f x2fc_ballDeltaFlat;
  float x308_speedFactor = 0.f;
  float x30c_speedingTime = 0.f;
  zeus::CVector3f x310_idealLookVec;
  zeus::CVector3f x31c_predictedLookPos;
  u32 x328_avoidGeomCycle = 0;
  float x32c_colliderMag = 1.f;
  float x330_clearColliderThreshold = 0.2f;
  zeus::CAABox x334_collidersAABB = zeus::skNullBox;
  float x34c_obscuredTime = 0.f;
  CMaterialList x350_obscuringMaterial = {EMaterialTypes::NoStepLogic};
  float x358_unobscureMag = 0.f;
  zeus::CVector3f x35c_splineIntermediatePos;
  TUniqueId x368_obscuringObjectId = kInvalidUniqueId;
  ESplineState x36c_splineState = ESplineState::Invalid;
  bool x370_24_reevalSplineEnd : 1;
  float x374_splineCtrl = 0.f;
  float x378_splineCtrlRange;
  CCameraSpline x37c_camSpline{false};
  CMaterialList x3c8_collisionExcludeList = {EMaterialTypes::NoStepLogic};
  bool x3d0_24_camBehindFloorOrWall : 1;
  float x3d4_elevInterpTimer = 0.f;
  float x3d8_elevInterpStart = 0.f;
  TUniqueId x3dc_tooCloseActorId = kInvalidUniqueId;
  float x3e0_tooCloseActorDist = 10000.f;
  bool x3e4_pendingFailsafe = false;
  float x3e8_ = 0.f;
  float x3ec_ = 0.f;
  float x3f0_ = 0.f;
  float x3f4_ = 2.f;
  float x3f8_ = 0.f;
  float x3fc_ = 0.f;
  EBallCameraState x400_state = EBallCameraState::Default;
  float x404_chaseElevation;
  float x408_chaseDistance;
  float x40c_chaseAnglePerSecond;
  zeus::CVector3f x410_chaseLookAtOffset;
  CCameraSpring x41c_ballCameraChaseSpring;
  float x430_boostElevation;
  float x434_boostDistance;
  float x438_boostAnglePerSecond;
  zeus::CVector3f x43c_boostLookAtOffset;
  CCameraSpring x448_ballCameraBoostSpring;
  zeus::CVector3f x45c_overrideBallToCam;
  float x468_conservativeDoorCamDistance;
  TUniqueId x46c_collisionActorId = kInvalidUniqueId;
  float x470_clampVelTimer = 0.f;
  float x474_clampVelRange = 0.f;
  u32 x478_shortMoveCount = 0;
  std::unique_ptr<SFailsafeState> x47c_failsafeState;
  std::unique_ptr<u32> x480_;

  void SetupColliders(std::vector<CCameraCollider>& out, float xMag, float zMag, float radius, int count, float k,
                      float max, float startAngle);
  void BuildSplineNav(CStateManager& mgr);
  void BuildSplineArc(CStateManager& mgr);
  bool ShouldResetSpline(CStateManager& mgr) const;
  void UpdatePlayerMovement(float dt, CStateManager& mgr);
  void UpdateTransform(const zeus::CVector3f& lookDir, const zeus::CVector3f& pos, float dt, CStateManager& mgr);
  zeus::CVector3f ConstrainYawAngle(const CPlayer& player, float distance, float yawSpeed, float dt,
                                    CStateManager& mgr) const;
  void CheckFailsafe(float dt, CStateManager& mgr);
  void UpdateObjectTooCloseId(CStateManager& mgr);
  void UpdateAnglePerSecond(float dt);
  void UpdateUsingPathCameras(float dt, CStateManager& mgr);
  zeus::CVector3f GetFixedLookTarget(const zeus::CVector3f& hintToLookDir, CStateManager& mgr) const;
  void UpdateUsingFixedCameras(float dt, CStateManager& mgr);
  zeus::CVector3f ComputeVelocity(const zeus::CVector3f& curVel, const zeus::CVector3f& posDelta) const;
  zeus::CVector3f TweenVelocity(const zeus::CVector3f& curVel, const zeus::CVector3f& newVel, float rate, float dt);
  zeus::CVector3f MoveCollisionActor(const zeus::CVector3f& pos, float dt, CStateManager& mgr);
  void UpdateUsingFreeLook(float dt, CStateManager& mgr);
  zeus::CVector3f InterpolateCameraElevation(const zeus::CVector3f& camPos, float dt);
  zeus::CVector3f CalculateCollidersCentroid(const std::vector<CCameraCollider>& colliderList, int numObscured) const;
  zeus::CVector3f ApplyColliders();
  void UpdateColliders(const zeus::CTransform& xf, std::vector<CCameraCollider>& colliderList, int& it, int count,
                       float tolerance, const rstl::reserved_vector<TUniqueId, 1024>& nearList, float dt,
                       CStateManager& mgr);
  zeus::CVector3f AvoidGeometry(const zeus::CTransform& xf, const rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                float dt, CStateManager& mgr);
  zeus::CVector3f AvoidGeometryFull(const zeus::CTransform& xf, const rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                    float dt, CStateManager& mgr);
  zeus::CAABox CalculateCollidersBoundingBox(const std::vector<CCameraCollider>& colliderList,
                                             CStateManager& mgr) const;
  int CountObscuredColliders(const std::vector<CCameraCollider>& colliderList) const;
  void UpdateCollidersDistances(std::vector<CCameraCollider>& colliderList, float xMag, float zMag, float angOffset);
  void UpdateUsingColliders(float dt, CStateManager& mgr);
  void UpdateUsingSpindleCameras(float dt, CStateManager& mgr);
  zeus::CVector3f ClampElevationToWater(zeus::CVector3f& pos, CStateManager& mgr) const;
  void UpdateTransitionFromBallCamera(CStateManager& mgr);
  void UpdateUsingTransitions(float dt, CStateManager& mgr);
  zeus::CTransform UpdateCameraPositions(float dt, const zeus::CTransform& oldXf, const zeus::CTransform& newXf);
  static zeus::CVector3f GetFailsafeSplinePoint(const std::vector<zeus::CVector3f>& points, float t);
  bool CheckFailsafeFromMorphBallState(CStateManager& mgr) const;
  bool SplineIntersectTest(CMaterialList& intersectMat, CStateManager& mgr) const;
  void ActivateFailsafe(float dt, CStateManager& mgr);
  bool ConstrainElevationAndDistance(float& elevation, float& distance, float dt, CStateManager& mgr);
  zeus::CVector3f FindDesiredPosition(float distance, float elevation, const zeus::CVector3f& dir, CStateManager& mgr,
                                      bool fullTest);
  static bool DetectCollision(const zeus::CVector3f& from, const zeus::CVector3f& to, float radius, float& d,
                              CStateManager& mgr);
  void TeleportColliders(std::vector<CCameraCollider>& colliderList, const zeus::CVector3f& pos);
  static bool CheckTransitionLineOfSight(const zeus::CVector3f& eyePos, const zeus::CVector3f& behindPos,
                                         float& eyeToOccDist, float colRadius, CStateManager& mgr);

public:
  CBallCamera(TUniqueId uid, TUniqueId watchedId, const zeus::CTransform& xf, float fovy, float znear, float zfar,
              float aspect);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  void ProcessInput(const CFinalInput& input, CStateManager& mgr) override;
  void Reset(const zeus::CTransform&, CStateManager& mgr) override;
  void Render(CStateManager& mgr) override;
  EBallCameraBehaviour GetBehaviour() const { return x188_behaviour; }
  EBallCameraState GetState() const { return x400_state; }
  void SetState(EBallCameraState state, CStateManager& mgr);
  void Think(float dt, CStateManager& mgr) override;
  bool TransitionFromMorphBallState(CStateManager& mgr);
  TUniqueId GetTooCloseActorId() const { return x3dc_tooCloseActorId; }
  float GetTooCloseActorDistance() const { return x3e0_tooCloseActorDist; }
  void TeleportCamera(const zeus::CVector3f& pos, CStateManager& mgr);
  void TeleportCamera(const zeus::CTransform& xf, CStateManager& mgr);
  const zeus::CVector3f& GetLookPos() const { return x1d8_lookPos; }
  void ResetToTweaks(CStateManager& mgr);
  void UpdateLookAtPosition(float dt, CStateManager& mgr);
  zeus::CTransform UpdateLookDirection(const zeus::CVector3f& dir, CStateManager& mgr);
  void SetClampVelTimer(float f) { x470_clampVelTimer = f; }
  void SetClampVelRange(float f) { x474_clampVelRange = f; }
  void ApplyCameraHint(CStateManager& mgr);
  void ResetPosition(CStateManager& mgr);
  void DoorClosed(TUniqueId doorId);
  void DoorClosing(TUniqueId doorId);
  const zeus::CVector3f& GetLookPosAhead() const { return x1c0_lookPosAhead; }
  const zeus::CVector3f& GetFixedLookPos() const { return x1cc_fixedLookPos; }

  static bool IsBallNearDoor(const zeus::CVector3f& pos, CStateManager& mgr);
};

} // namespace urde
