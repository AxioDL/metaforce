#ifndef __URDE_CGAMECAMERA_HPP__
#define __URDE_CGAMECAMERA_HPP__

#include "World/CActor.hpp"
#include "zeus/CTransform.hpp"

namespace urde
{

class CGameCamera : public CActor
{
public:
    const zeus::CTransform& GetTransform() const {return x34_transform;}
};

}

#endif // __URDE_CGAMECAMERA_HPP__
