#include "CScriptSpawnPoint.hpp"

namespace urde
{

CScriptSpawnPoint::CScriptSpawnPoint(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                     const zeus::CTransform& xf, const std::vector<u32>& itemCounts,
                                     bool defaultSpawn, bool active, bool b3)
: CEntity(uid, info, active, name), x34_xf(xf), x64_itemCounts(itemCounts)
{
    x10c_24_firstSpawn = defaultSpawn;
    x10c_25_ = b3;
}

u32 CScriptSpawnPoint::GetPowerup(CPlayerState::EItemType item) const
{
    int idx = int(item);
    if (idx >= x64_itemCounts.size())
        return x64_itemCounts.front();
    return x64_itemCounts[idx];
}

}
