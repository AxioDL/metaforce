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
    void SetInterpolation(const zeus::CTransform& xf, const zeus::CVector3f& lookDir,
                          float f1, float f2, float f3, TUniqueId camId, bool b1, CStateManager& mgr);
};

}

#endif // __URDE_CINTERPOLATIONCAMERA_HPP__
