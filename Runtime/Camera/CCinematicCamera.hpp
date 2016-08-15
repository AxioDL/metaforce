#ifndef __URDE_CCINEMATICCAMERA_HPP__
#define __URDE_CCINEMATICCAMERA_HPP__

#include "CGameCamera.hpp"

namespace urde
{

class CCinematicCamera : public CGameCamera
{
public:
    CCinematicCamera(TUniqueId, const std::string& name, const CEntityInfo& info,
                     const zeus::CTransform& xf, bool, float, float, float, float, float, u32);

    void ProcessInput(const CFinalInput&, CStateManager& mgr);
    void Reset(const zeus::CTransform&, CStateManager& mgr);
};

}

#endif // __URDE_CCINEMATICCAMERA_HPP__
