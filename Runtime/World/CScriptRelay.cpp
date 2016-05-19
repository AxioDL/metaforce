#include "CScriptRelay.hpp"

namespace urde
{

CScriptRelay::CScriptRelay(TUniqueId uid, const std::string& name, const CEntityInfo& info, bool active)
    : CEntity(uid, info, active, name)
{
}

}
