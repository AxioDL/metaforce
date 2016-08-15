#ifndef __URDE_CGAMECAMERA_HPP__
#define __URDE_CGAMECAMERA_HPP__

#include "World/CActor.hpp"
#include "zeus/CTransform.hpp"

namespace urde
{
class CFinalInput;

class CGameCamera : public CActor
{
public:
    CGameCamera(TUniqueId, bool active, const std::string& name, const CEntityInfo& info,
                const zeus::CTransform& xf, float fov, float nearz, float farz, float aspect,
                TUniqueId, bool, u32);
    const zeus::CTransform& GetTransform() const {return x34_transform;}

    virtual void ProcessInput(const CFinalInput&, CStateManager& mgr)=0;
    virtual void Reset(const zeus::CTransform&, CStateManager& mgr)=0;
};

}

#endif // __URDE_CGAMECAMERA_HPP__
