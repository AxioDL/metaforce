#ifndef __URDE_CINTERPOLATIONCAMERA_HPP__
#define __URDE_CINTERPOLATIONCAMERA_HPP__

#include "CGameCamera.hpp"

namespace urde
{

class CInterpolationCamera : public CGameCamera
{
public:
    CInterpolationCamera(TUniqueId uid, const zeus::CTransform& xf);
    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void ProcessInput(const CFinalInput&, CStateManager& mgr);
    void Render(const CStateManager&) const;
    void Reset(const zeus::CTransform&, CStateManager& mgr);
    void Think(float, CStateManager &);
};

}

#endif // __URDE_CINTERPOLATIONCAMERA_HPP__
