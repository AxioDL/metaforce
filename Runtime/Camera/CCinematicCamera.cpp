#include "CCinematicCamera.hpp"
#include "CStateManager.hpp"
#include "World/CPlayer.hpp"
#include "World/CScriptActor.hpp"
#include "TCastTo.hpp"

namespace urde
{

CCinematicCamera::CCinematicCamera(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                   const zeus::CTransform& xf, bool active, float shotDuration,
                                   float fovy, float znear, float zfar, float aspect, u32 flags)
: CGameCamera(uid, active, name, info, xf, fovy, znear, zfar, aspect, kInvalidUniqueId, flags & 0x20, 0),
  x1e8_duration(shotDuration), x1f0_origFovy(fovy), x1fc_(zeus::CQuaternion(xf.basis)), x21c_flags(flags)
{
    x220_24_ = false;
}

void CCinematicCamera::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CCinematicCamera::ProcessInput(const CFinalInput&, CStateManager& mgr)
{
    // Empty
}

void CCinematicCamera::Reset(const zeus::CTransform&, CStateManager& mgr)
{
    // Empty
}

void CCinematicCamera::WasDeactivated(CStateManager& mgr)
{
    mgr.GetCameraManager()->RemoveCinemaCamera(GetUniqueId(), mgr);
    mgr.GetPlayer().GetMorphBall()->LoadMorphBallModel(mgr);
    if (x21c_flags & 0x100)
        mgr.SetCinematicPause(false);
    x188_.clear();
    x198_.clear();
    x1a8_.clear();
    x1b8_.clear();
    x1c8_.clear();
    x1d8_.clear();
}

void CCinematicCamera::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    CGameCamera::AcceptScriptMsg(msg, uid, mgr);
    switch (msg)
    {
    case EScriptObjectMessage::InitializedInArea:
        if (x21c_flags & 0x4 || x21c_flags & 0x2)
        {
            for (const SConnection& conn : x20_conns)
            {
                TUniqueId id = mgr.GetIdForScript(conn.x8_objId);
                if (TCastToPtr<CScriptActor> act = mgr.ObjectById(id))
                {
                    if (act->GetX2E3_24())
                    {
                        x20c_lookAtId = id;
                        if (conn.x4_msg != EScriptObjectMessage::Deactivate &&
                            conn.x4_msg != EScriptObjectMessage::Reset)
                            break;
                    }
                }
            }
        }
        break;
    case EScriptObjectMessage::Activate:
        CalculateWaypoints(mgr);
        if ((x21c_flags & 1) == 0 && x220_24_ && x1b8_.empty())
            break;
        x1ec_ = 0.f;
        Think(0.f, mgr);
        mgr.GetCameraManager()->AddCinemaCamera(GetUniqueId(), mgr);
        x1f4_ = 0;
        if (x1a8_.size() > 0)
            SendArrivedMsg(x1a8_[x1f4_], mgr);
        x1f8_ = 0;
        if (x1c8_.size() > 0)
            SendArrivedMsg(x1c8_[x1f8_], mgr);
        if (x21c_flags & 0x100)
            mgr.SetCinematicPause(true);
        break;
    case EScriptObjectMessage::Deactivate:
        WasDeactivated(mgr);
        break;
    default:
        break;
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
