#ifndef __URDE_CINTERPOLATIONCAMERA_HPP__
#define __URDE_CINTERPOLATIONCAMERA_HPP__

#include "CGameCamera.hpp"

namespace urde
{

class CInterpolationCamera : public CGameCamera
{
    TUniqueId x188_targetId = kInvalidUniqueId;
    float x18c_time = 0.f;
    float x190_maxTime = 0.f;
    zeus::CTransform x194_;
    zeus::CVector3f x1c4_lookPos;
    float x1d0_ = 0.f;
    float x1d4_ = 0.f;
    float x1d8_ = 0.f;
    bool x1d8_24_ : 1;
    float x1dc_ = M_PIF * 2.f;
public:
    CInterpolationCamera(TUniqueId uid, const zeus::CTransform& xf);
    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void ProcessInput(const CFinalInput&, CStateManager& mgr);
    void Render(const CStateManager&) const;
    void Reset(const zeus::CTransform&, CStateManager& mgr);
    void Think(float, CStateManager &);
    void SetInterpolation(const zeus::CTransform& xf, const zeus::CVector3f& lookPos,
                          float f1, float f2, float f3, TUniqueId camId, bool b1, CStateManager& mgr);
    void DeactivateInterpCamera(CStateManager&);
    bool sub802654d8(zeus::CTransform&, const zeus::CVector3f&, const zeus::CVector3f&, float, float);
    bool sub802658c0(zeus::CTransform&, const zeus::CVector3f&, const zeus::CVector3f&, float, float, float, float);
};

}

#endif // __URDE_CINTERPOLATIONCAMERA_HPP__
