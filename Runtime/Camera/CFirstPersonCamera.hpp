#ifndef __URDE_CFIRSTPERSONCAMERA_HPP__
#define __URDE_CFIRSTPERSONCAMERA_HPP__

#include "CGameCamera.hpp"

namespace urde
{

class CFirstPersonCamera : public CGameCamera
{
public:
    CFirstPersonCamera(TUniqueId, const zeus::CTransform& xf, TUniqueId, float, float, float, float, float);
};

}

#endif // __URDE_CFIRSTPERSONCAMERA_HPP__
