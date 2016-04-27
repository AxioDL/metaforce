#include "CScriptSpawnPoint.hpp"

namespace urde
{

CScriptSpawnPoint::CScriptSpawnPoint(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                     const zeus::CTransform& xf, const std::vector<u32>& itemCounts,
                                     bool, bool active, bool)
: CEntity(uid, info, active, name)
{
}

}
