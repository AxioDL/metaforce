#ifndef __URDE_CBALLCAMERA_HPP__
#define __URDE_CBALLCAMERA_HPP__

#include "CGameCamera.hpp"

namespace urde
{

class CBallCamera : public CGameCamera
{
public:
    CBallCamera(TUniqueId, TUniqueId, const zeus::CTransform& xf, float, float, float, float);

    void ProcessInput(const CFinalInput&, CStateManager& mgr);
    void Reset(const zeus::CTransform&, CStateManager& mgr);
};

}

#endif // __URDE_CBALLCAMERA_HPP__
