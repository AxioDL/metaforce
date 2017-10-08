#ifndef __URDE_CBALLCAMERA_HPP__
#define __URDE_CBALLCAMERA_HPP__

#include "CGameCamera.hpp"
#include "CCameraSpline.hpp"

namespace urde
{

class CCameraSpring
{
    float x0_tardis;
    float x4_tardis2Sqrt;
    float x8_max;
    float xc_k;
    float x10_dx = 0.f;
public:
    CCameraSpring(float tardis, float max, float k)
    : x0_tardis(tardis), x4_tardis2Sqrt(2.f * std::sqrt(tardis)), x8_max(max), xc_k(k) {}
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
    u32 x188_ = 0;
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
    bool x18e_27_ : 1;
    bool x18e_28_ : 1;
    float x190_origMinDistance;
    float x194_minDistance;
    float x198_maxDistance;
    float x19c_backwardsDistance;
    float x1a0_elevation;
    float x1a4_origAnglePerSecond;
    float x1a8_anglePerSecond;
    float x1ac_ = 1.5533431f;
    float x1b0_ = 1.5533431f;
    zeus::CVector3f x1b4_lookAtOffset;
    float x1c0_ = 0.f;
    float x1c4_ = 0.f;
    float x1c8_ = 0.f;
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
    float x294_ = 0.f;
    float x298_ = 0.f;
    float x29c_ = 0.f;
    float x2a0_ = 0.f;
    float x2a4_ = 0.f;
    float x2a8_ = 1.f;
    float x2ac_ = 0.f;
    float x2b0_ = 0.f;
    float x2b4_ = 1.f;
    float x2b8_ = 0.f;
    float x2bc_ = 0.f;
    float x2c0_ = 1.f;
    u32 x2c4_ = 0;
    u32 x2c8_ = 0;
    u32 x2cc_ = 0;
    u32 x2d0_ = 0;
    u32 x2d4_ = 0;
    u32 x2d8_ = 0;
    float x2dc_ = 0.f;
    float x2e0_ = 0.f;
    float x2e4_ = 0.f;
    float x2e8_ = 0.f;
    float x2ec_ = 0.f;
    zeus::CVector3f x2f0_;
    zeus::CVector3f x2fc_;
    float x308_ = 0.f;
    float x30c_ = 0.f;
    float x310_ = 0.f;
    float x314_ = 0.f;
    float x318_ = 0.f;
    float x31c_ = 0.f;
    float x320_ = 0.f;
    float x324_ = 0.f;
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
    CCameraSpline x37c_camSpline;
    CMaterialList x3c8_ = {EMaterialTypes::Unknown};
    bool x3d0_24_ : 1;
    float x3d4_ = 0.f;
    float x3d8_ = 0.f;
    TUniqueId x3dc_tooCloseActorId = kInvalidUniqueId;
    float x3e0_ = 10000.f;
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
    TUniqueId x46c_ = kInvalidUniqueId;
    float x470_ = 0.f;
    float x474_ = 0.f;
    u32 x478_ = 0;
    std::unique_ptr<u32> x480_;

    void SetupColliders(std::vector<CCameraCollider>& out, float xMag, float zMag, float radius, int count,
                        float tardis, float max, float startAngle);

public:
    CBallCamera(TUniqueId uid, TUniqueId watchedId, const zeus::CTransform& xf,
                float fovy, float znear, float zfar, float aspect);

    void Accept(IVisitor& visitor);
    void ProcessInput(const CFinalInput&, CStateManager& mgr);
    void Reset(const zeus::CTransform&, CStateManager& mgr);
    EBallCameraState GetState() const { return x400_state; }
    void SetState(EBallCameraState state, CStateManager& mgr);
    bool TransitionFromMorphBallState(CStateManager& mgr);
    TUniqueId GetTooCloseActorId() const { return x3dc_tooCloseActorId; }
    float GetX3E0() const { return x3e0_; }
    void TeleportCamera(const zeus::CVector3f& pos, CStateManager& mgr);
    void TeleportCamera(const zeus::CTransform& xf, CStateManager& mgr);
    const zeus::CVector3f& GetX1D8() const { return x1d8_; }
    void ResetToTweaks(CStateManager& mgr);
    void UpdateLookAtPosition(float offset, CStateManager& mgr);
    zeus::CTransform UpdateLookDirection(const zeus::CVector3f& dir, CStateManager& mgr);
    void SetX470(float f) { x470_ = f; }
    void SetX474(float f) { x474_ = f; }
    void ApplyCameraHint(CStateManager& mgr);
    void ResetPosition();
};

}

#endif // __URDE_CBALLCAMERA_HPP__
