#ifndef __URDE_CCINEMATICCAMERA_HPP__
#define __URDE_CCINEMATICCAMERA_HPP__

#include "CGameCamera.hpp"

namespace urde
{

class CCinematicCamera : public CGameCamera
{
    u32 x21c_w1;
public:
    CCinematicCamera(TUniqueId, const std::string& name, const CEntityInfo& info,
                     const zeus::CTransform& xf, bool, float, float, float, float, float, u32 w1);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr);
    void ProcessInput(const CFinalInput&, CStateManager& mgr);
    void Reset(const zeus::CTransform&, CStateManager& mgr);
    u32 GetW1() const { return x21c_w1; }
    void WasDeactivated(CStateManager& mgr);
    void CalculateWaypoints(CStateManager& mgr);
    void SendArrivedMsg(TUniqueId reciever, CStateManager& mgr);
};

}

#endif // __URDE_CCINEMATICCAMERA_HPP__
