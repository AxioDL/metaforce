#include "CPowerBeam.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"

namespace urde
{

CPowerBeam::CPowerBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId,
                       EMaterialTypes playerMaterial, const zeus::CVector3f& scale)
: CGunWeapon(characterId, type, playerId, playerMaterial, scale)
{
    x21c_shotSmoke = g_SimplePool->GetObj("ShotSmoke");
    x228_power2nd1 = g_SimplePool->GetObj("Power2nd_1");
    x244_24 = false;
    x244_25 = false;
}

}
