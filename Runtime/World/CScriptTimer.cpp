#include "CScriptTimer.hpp"

namespace urde
{

CScriptTimer::CScriptTimer(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                           float, float, bool, bool, bool active)
: CEntity(uid, info, active, name)
{
}

void CScriptTimer::Accept(IVisitor&) {}

}
