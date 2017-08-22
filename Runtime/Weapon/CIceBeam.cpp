#include "CIceBeam.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"

namespace urde
{

CIceBeam::CIceBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId,
                   EMaterialTypes playerMaterial, const zeus::CVector3f& scale)
: CGunWeapon(characterId, type, playerId, playerMaterial, scale)
{
    x21c_iceSmoke = g_SimplePool->GetObj("IceSmoke");
    x228_ice2nd1 = g_SimplePool->GetObj("Ice2nd_1");
    x234_ice2nd2 = g_SimplePool->GetObj("Ice2nd_2");
    x248_24 = false;
    x248_25 = false;
}

}