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

    };
private:
    TUniqueId x3dc_tooCloseActorId = kInvalidUniqueId;
    float x3e0_ = 10000.f;
public:
    CBallCamera(TUniqueId, TUniqueId, const zeus::CTransform& xf, float, float, float, float);

    void Accept(IVisitor& visitor);
    void ProcessInput(const CFinalInput&, CStateManager& mgr);
    void Reset(const zeus::CTransform&, CStateManager& mgr);
    void SetState(EBallCameraState state, CStateManager& mgr);
    bool TransitionFromMorphBallState(CStateManager& mgr);
    TUniqueId GetTooCloseActorId() const { return x3dc_tooCloseActorId; }
    float GetX3E0() const { return x3e0_; }
    void TeleportCamera(const zeus::CVector3f& pos, CStateManager& mgr);
};

}

#endif // __URDE_CBALLCAMERA_HPP__
