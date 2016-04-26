#include "CScriptGenerator.hpp"

namespace urde
{

CScriptGenerator::CScriptGenerator(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                   u32, bool, const zeus::CVector3f&, bool, bool active, float, float)
: CEntity(uid, info, active, name)
{
}
}
