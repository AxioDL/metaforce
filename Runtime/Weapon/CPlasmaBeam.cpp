#include "CPlasmaBeam.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"

namespace urde
{

CPlasmaBeam::CPlasmaBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId,
                         EMaterialTypes playerMaterial, const zeus::CVector3f& scale)
: CGunWeapon(characterId, type, playerId, playerMaterial, scale)
{
    x21c_plasma2nd1 = g_SimplePool->GetObj("Plasma2nd_1");
    x22c_24 = false;
    x22c_25 = false;
}

}