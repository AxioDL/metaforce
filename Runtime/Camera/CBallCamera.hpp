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
        Zero
    };
private:
public:
    CBallCamera(TUniqueId, TUniqueId, const zeus::CTransform& xf, float, float, float, float);

    void Accept(IVisitor& visitor);
    void ProcessInput(const CFinalInput&, CStateManager& mgr);
    void Reset(const zeus::CTransform&, CStateManager& mgr);
    void SetState(EBallCameraState state, CStateManager& mgr);
};

}

#endif // __URDE_CBALLCAMERA_HPP__
