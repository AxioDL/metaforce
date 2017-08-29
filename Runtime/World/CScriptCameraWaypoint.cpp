#include "CScriptCameraWaypoint.hpp"
#include "CActorParameters.hpp"
#include "TCastTo.hpp"

namespace urde
{

CScriptCameraWaypoint::CScriptCameraWaypoint(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                             const zeus::CTransform& xf, bool active, float f1, u32 w1)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::Unknown),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_(f1)
, xec_(w1)
{

}

void CScriptCameraWaypoint::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptCameraWaypoint::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    CActor::AcceptScriptMsg(msg, uid, mgr);
    if (!GetActive() && msg == EScriptObjectMessage::Arrived)
        SendScriptMsgs(EScriptObjectState::Arrived, mgr, EScriptObjectMessage::None);
}

}
