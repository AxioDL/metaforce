#ifndef __URDE_CGAMECAMERA_HPP__
#define __URDE_CGAMECAMERA_HPP__

#include "World/CActor.hpp"
#include "zeus/CTransform.hpp"

namespace urde
{

class CGameCamera : public CActor
{
public:
    CGameCamera(TUniqueId, bool active, const std::string& name, const CEntityInfo& info,
                const zeus::CTransform& xf, float, float, float, float, TUniqueId, bool, u32);
    const zeus::CTransform& GetTransform() const {return x34_transform;}
};

}

#endif // __URDE_CGAMECAMERA_HPP__
