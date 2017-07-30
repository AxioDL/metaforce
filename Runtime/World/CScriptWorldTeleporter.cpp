#include "CScriptWorldTeleporter.hpp"

namespace urde
{
CScriptWorldTeleporter::CScriptWorldTeleporter(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                               bool active, u32, u32)
: CEntity(uid, info, active, name)
{
}
}
