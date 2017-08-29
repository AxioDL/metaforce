#include "CCinematicCamera.hpp"
#include "CStateManager.hpp"
#include "Camera/CCameraManager.hpp"
#include "TCastTo.hpp"

namespace urde
{

CCinematicCamera::CCinematicCamera(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                   const zeus::CTransform& xf, bool active, float shotDuration, float fovy, float znear,
                                   float zfar, float aspect, u32 w1)
: CGameCamera(uid, active, name, info, xf, fovy, znear, zfar, aspect, kInvalidUniqueId, w1 & 0x20, 0), x21c_w1(w1)
{
}

void CCinematicCamera::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CCinematicCamera::ProcessInput(const CFinalInput&, CStateManager& mgr) {}

void CCinematicCamera::Reset(const zeus::CTransform&, CStateManager& mgr) {}

void CCinematicCamera::WasDeactivated(CStateManager& mgr)
{
    mgr.GetCameraManager()->RemoveCinemaCamera(GetUniqueId(), mgr);
}

void CCinematicCamera::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    CGameCamera::AcceptScriptMsg(msg, uid, mgr);
    if (msg == EScriptObjectMessage::Activate)
    {
        CalculateWaypoints(mgr);
        if (x21c_w1 & 1)
        {

        }
    }
    else if (msg == EScriptObjectMessage::Deactivate)
    {
        WasDeactivated(mgr);
    }
    else if (msg == EScriptObjectMessage::InitializedInArea)
    {

    }
}

void CCinematicCamera::CalculateWaypoints(CStateManager& mgr)
{

}

void CCinematicCamera::SendArrivedMsg(TUniqueId reciever, CStateManager& mgr)
{
    mgr.SendScriptMsgAlways(reciever, GetUniqueId(), EScriptObjectMessage::Arrived);
}
}
