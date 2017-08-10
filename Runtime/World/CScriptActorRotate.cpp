#include "CScriptActorRotate.hpp"
#include "TCastTo.hpp"

namespace urde
{
CScriptActorRotate::CScriptActorRotate(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                       const zeus::CVector3f& rotation, float f1, bool b1, bool b2, bool active)
: CEntity(uid, info, active, name)
, x34_rotation(rotation)
, x40_(f1)
, x58_24_(false)
, x58_25_(false)
, x58_26_(b1)
, x58_27_(b2)
{
}

void CScriptActorRotate::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptActorRotate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    CEntity::AcceptScriptMsg(msg, uid, mgr);
}

}
