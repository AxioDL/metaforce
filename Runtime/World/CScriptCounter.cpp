#include "CScriptCounter.hpp"

namespace urde
{

CScriptCounter::CScriptCounter(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                               u32, u32, bool, bool active)
: CEntity(uid, info, active, name)
{
}

void CScriptCounter::Accept(IVisitor&) {}

}
