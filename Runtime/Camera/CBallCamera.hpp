#ifndef __URDE_CBALLCAMERA_HPP__
#define __URDE_CBALLCAMERA_HPP__

#include "CGameCamera.hpp"
#include "CCameraSpline.hpp"

namespace urde
{
class CPlayer;

class CCameraSpring
{
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

class CCameraCollider
{
    friend class CBallCamera;
    float x4_radius;
    zeus::CVector3f x8_;
    zeus::CVector3f x14_;
    zeus::CVector3f x20_;
    zeus::CVector3f x2c_;
    CCameraSpring x38_spring;
    u32 x4c_occlusionCount = 0;
    float x50_scale;
public:
    CCameraCollider(float radius, const zeus::CVector3f& vec, const CCameraSpring& spring, float scale)
    : x4_radius(radius), x8_(vec), x14_(vec), x20_(vec), x2c_(vec), x38_spring(spring), x50_scale(scale) {}
};

class CBallCamera : public CGameCamera
{
public:
    enum class EBallCameraState
    {
        Zero,
        One,
        Two,
        Three,
        Four,
        Five
    };
    enum class EBallCameraBehaviour
    {
        Zero,
        One,
        Two,
        Three,
        Four,
        Five,
        Six,
        Seven,
        Eight
    };
private:
    struct SFailsafeState
    {
        zeus::CTransform x0_;
        zeus::CTransform x30_;
        zeus::CVector3f x60_;
        zeus::CVector3f x6c_;
        zeus::CVector3f x78_;
        zeus::CVector3f x84_;
        std::vector<zeus::CVector3f> x90_bezPoints;
    };

    EBallCameraBehaviour x188_behaviour = EBallCameraBehaviour::Zero;
    bool x18c_24_ : 1;
    bool x18c_25_ : 1;
    bool x18c_26_ : 1;
    bool x18c_27_ : 1;
    bool x18c_28_ : 1;
    bool x18c_29_ : 1;
    bool x18c_30_ : 1;
    bool x18c_31_ : 1;
    bool x18d_24_ : 1;
    bool x18d_25_ : 1;
    bool x18d_26_ : 1;
    bool x18d_27_ : 1;
    bool x18d_28_ : 1;
    bool x18d_29_ : 1;
    bool x18d_30_ : 1;
    bool x18d_31_ : 1;
    bool x18e_24_ : 1;
    bool x18e_25_ : 1;
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
    float x1ac_ = 1.5533431f;
    float x1b0_ = 1.5533431f;
    zeus::CVector3f x1b4_lookAtOffset;
    zeus::CVector3f x1c0_;
    zeus::CVector3f x1cc_;
    zeus::CVector3f x1d8_;
    zeus::CTransform x1e4_;
    CCameraSpring x214_ballCameraSpring;
    CCameraSpring x228_ballCameraCentroidSpring;
    CCameraSpring x23c_ballCameraLookAtSpring;
    CCameraSpring x250_ballCameraCentroidDistanceSpring;
    std::vector<CCameraCollider> x264_smallColliders;
    std::vector<CCameraCollider> x274_mediumColliders;
    std::vector<CCameraCollider> x284_largeColliders;
    zeus::CVector3f x294_;
    zeus::CVector3f x2a0_ = zeus::CVector3f::skUp;
    zeus::CVector3f x2ac_ = zeus::CVector3f::skUp;
    zeus::CVector3f x2b8_ = zeus::CVector3f::skUp;
    int x2c4_ = 0;
    int x2c8_ = 0;
    int x2cc_ = 0;
    int x2d0_ = 0;
    int x2d4_ = 0;
    int x2d8_ = 0;
    zeus::CVector3f x2dc_;
    float x2e8_ = 0.f;
    float x2ec_ = 0.f;
    zeus::CVector3f x2f0_;
    zeus::CVector3f x2fc_;
    float x308_ = 0.f;
    float x30c_ = 0.f;
    zeus::CVector3f x310_;
    zeus::CVector3f x31c_;
    u32 x328_ = 0;
    float x32c_ = 1.f;
    float x330_ = 0.2f;
    zeus::CAABox x334_ = zeus::CAABox::skNullBox;
    float x34c_ = 0.f;
    CMaterialList x350_ = {EMaterialTypes::Unknown};
    float x358_ = 0.f;
    zeus::CVector3f x35c_;
    TUniqueId x368_ = kInvalidUniqueId;
    u32 x36c_ = 0;
    bool x370_24_ : 1;
    float x374_ = 0.f;
    float x378_;
    CCameraSpline x37c_camSpline;
    CMaterialList x3c8_ = {EMaterialTypes::Unknown};
    bool x3d0_24_ : 1;
    float x3d4_ = 0.f;
    float x3d8_ = 0.f;
    TUniqueId x3dc_tooCloseActorId = kInvalidUniqueId;
    float x3e0_tooCloseActorDist = 10000.f;
    bool x3e4_ = false;
    float x3e8_ = 0.f;
    float x3ec_ = 0.f;
    float x3f0_ = 0.f;
    float x3f4_ = 2.f;
    float x3f8_ = 0.f;
    float x3fc_ = 0.f;
    EBallCameraState x400_state = EBallCameraState::Zero;
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
    zeus::CVector3f x45c_;
    float x468_;
    TUniqueId x46c_collisionActorId = kInvalidUniqueId;
    float x470_ = 0.f;
    float x474_ = 0.f;
    u32 x478_ = 0;
    std::unique_ptr<SFailsafeState> x47c_failsafeState;
    std::unique_ptr<u32> x480_;

    void SetupColliders(std::vector<CCameraCollider>& out, float xMag, float zMag, float radius, int count,
                        float tardis, float max, float startAngle);
    void ResetSpline(CStateManager& mgr);
    void BuildSpline(CStateManager& mgr);
    bool ShouldResetSpline(CStateManager& mgr) const;
    void UpdatePlayerMovement(float dt, CStateManager& mgr);
    void UpdateTransform(const zeus::CVector3f& lookDir, const zeus::CVector3f& pos, float dt, CStateManager& mgr);
    zeus::CVector3f ConstrainYawAngle(const CPlayer& player, float distance, float yawSpeed, float dt,
                                      CStateManager& mgr) const;
    void CheckFailsafe(float dt, CStateManager& mgr);
    void UpdateObjectTooCloseId(CStateManager& mgr);
    void UpdateAnglePerSecond(float dt);
    void UpdateUsingPathCameras(float dt, CStateManager& mgr);
    zeus::CVector3f GetFixedLookTarget(const zeus::CVector3f& pos, CStateManager& mgr) const;
    void UpdateUsingFixedCameras(float dt, CStateManager& mgr);
    zeus::CVector3f ComputeVelocity(const zeus::CVector3f& curVel, const zeus::CVector3f& posDelta) const;
    zeus::CVector3f TweenVelocity(const zeus::CVector3f& curVel, const zeus::CVector3f& newVel, float rate, float dt);
    zeus::CVector3f MoveCollisionActor(const zeus::CVector3f& pos, float dt, CStateManager& mgr);
    void UpdateUsingFreeLook(float dt, CStateManager& mgr);
    zeus::CVector3f InterpolateCameraElevation(const zeus::CVector3f& camPos);
    zeus::CVector3f CalculateCollidersCentroid(const std::vector<CCameraCollider>& colliderList, int w1) const;
    zeus::CVector3f ApplyColliders();
    void UpdateColliders(const zeus::CTransform& xf, std::vector<CCameraCollider>& colliderList, int& r6, int r7,
                         float f1, const rstl::reserved_vector<TUniqueId, 1024>& nearList, float dt,
                         CStateManager& mgr);
    zeus::CVector3f AvoidGeometry(const zeus::CTransform& xf, const rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                  float dt, CStateManager& mgr);
    zeus::CVector3f AvoidGeometryFull(const zeus::CTransform& xf, const rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                      float dt, CStateManager& mgr);
    zeus::CAABox CalculateCollidersBoundingBox(const std::vector<CCameraCollider>& colliderList,
                                               CStateManager& mgr) const;
    int CountObscuredColliders(const std::vector<CCameraCollider>& colliderList) const;
    void UpdateCollidersDistances(std::vector<CCameraCollider>& colliderList, float f1, float f2, float f3);
    void UpdateUsingColliders(float dt, CStateManager& mgr);
    void UpdateUsingSpindleCameras(float dt, CStateManager& mgr);
    zeus::CVector3f ClampElevationToWater(zeus::CVector3f& pos, CStateManager& mgr) const;
    void UpdateTransitionFromBallCamera(CStateManager& mgr);
    void UpdateUsingTransitions(float dt, CStateManager& mgr);
    zeus::CTransform UpdateCameraPositions(float dt, const zeus::CTransform& oldXf, const zeus::CTransform& newXf);
    static zeus::CVector3f GetFailsafeBezierPoint(const std::vector<zeus::CVector3f>& points, float t);
    bool CheckFailsafeFromMorphBallState(CStateManager& mgr) const;
    bool SplineIntersectTest(CMaterialList& intersectMat, CStateManager& mgr) const;
    static bool IsBallNearDoor(const zeus::CVector3f& pos, CStateManager& mgr);
    void ActivateFailsafe(float dt, CStateManager& mgr);
    bool ConstrainElevationAndDistance(float& elevation, float& distance, float dt, CStateManager& mgr);
    zeus::CVector3f FindDesiredPosition(float distance, float elevation, const zeus::CVector3f& dir,
                                        CStateManager& mgr, bool b);
    static bool DetectCollision(const zeus::CVector3f& from, const zeus::CVector3f& to, float margin,
                                float& d, CStateManager& mgr);
    void TeleportColliders(std::vector<CCameraCollider>& colliderList, const zeus::CVector3f& pos);
    static bool CheckTransitionLineOfSight(const zeus::CVector3f& eyePos, const zeus::CVector3f& behindPos,
                                           float& eyeToOccDist, float colRadius, CStateManager& mgr);

public:
    CBallCamera(TUniqueId uid, TUniqueId watchedId, const zeus::CTransform& xf,
                float fovy, float znear, float zfar, float aspect);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr);
    void ProcessInput(const CFinalInput& input, CStateManager& mgr);
    void Reset(const zeus::CTransform&, CStateManager& mgr);
    void Render(const CStateManager& mgr) const;
    EBallCameraState GetState() const { return x400_state; }
    void SetState(EBallCameraState state, CStateManager& mgr);
    void Think(float dt, CStateManager& mgr);
    bool TransitionFromMorphBallState(CStateManager& mgr);
    TUniqueId GetTooCloseActorId() const { return x3dc_tooCloseActorId; }
    float GetTooCloseActorDistance() const { return x3e0_tooCloseActorDist; }
    void TeleportCamera(const zeus::CVector3f& pos, CStateManager& mgr);
    void TeleportCamera(const zeus::CTransform& xf, CStateManager& mgr);
    const zeus::CVector3f& GetX1D8() const { return x1d8_; }
    void ResetToTweaks(CStateManager& mgr);
    void UpdateLookAtPosition(float dt, CStateManager& mgr);
    zeus::CTransform UpdateLookDirection(const zeus::CVector3f& dir, CStateManager& mgr);
    void SetX470(float f) { x470_ = f; }
    void SetX474(float f) { x474_ = f; }
    void ApplyCameraHint(CStateManager& mgr);
    void ResetPosition(CStateManager& mgr);
    void DoorClosed(TUniqueId doorId);
    void DoorClosing(TUniqueId doorId);
};

}

#endif // __URDE_CBALLCAMERA_HPP__
