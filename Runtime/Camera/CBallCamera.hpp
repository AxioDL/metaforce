#ifndef __URDE_CBALLCAMERA_HPP__
#define __URDE_CBALLCAMERA_HPP__

#include "CGameCamera.hpp"

namespace urde
{

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
    zeus::CVector3f x1d8_;
    TUniqueId x3dc_tooCloseActorId = kInvalidUniqueId;
    float x3e0_ = 10000.f;
    EBallCameraState x400_state;
    float x470_;
    float x474_;

public:
    CBallCamera(TUniqueId, TUniqueId, const zeus::CTransform& xf, float, float, float, float);

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
