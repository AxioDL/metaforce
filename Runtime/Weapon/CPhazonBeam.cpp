#include "CPhazonBeam.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"

namespace urde
{

CPhazonBeam::CPhazonBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId,
                         EMaterialTypes playerMaterial, const zeus::CVector3f& scale)
: CGunWeapon(characterId, type, playerId, playerMaterial, scale),
  x238_(zeus::CVector3f(-0.14664599f, 0.f, -0.14909725f) * scale.y,
        zeus::CVector3f(0.14664599f, 0.64619601f, 0.14909725f) * scale.y),
  x250_(zeus::CVector3f(-0.0625f, 0.f, -0.09375f) * scale.y,
        zeus::CVector3f(0.0625f, -0.25f, 0.09375f) * scale.y)
{
    x21c_phazonVeins = g_SimplePool->GetObj("PhazonVeins");
    x228_phazon2nd1 = g_SimplePool->GetObj("Phazon2nd_1");
    x274_24 = false;
    x274_25 = true;
    x274_26 = false;
    x274_27 = false;
}

void CPhazonBeam::AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&)
{

}

}