#include "CScriptActorRotate.hpp"
#include "TCastTo.hpp"

namespace urde
{
CScriptActorRotate::CScriptActorRotate(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                       const zeus::CVector3f& rotation, float scale, bool, bool, bool active)
: CEntity(uid, info, active, name)
{
}

void CScriptActorRotate::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

}
