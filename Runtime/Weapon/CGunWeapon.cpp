#include "CGunWeapon.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "Particle/CGenDescription.hpp"

namespace urde
{
const char* CGunWeapon::skBeamXferNames[5] =
{
    "PowerXfer",
    "IceXfer",
    "WaveXfer",
    "PlasmaXfer",
    "PhazonXfer"
};

const char* CGunWeapon::skSuitArmNames[8] =
{
    "PowerArm",
    "GravityArm",
    "VariaArm",
    "PhazonArm",
    "FusionArm",
    "FusionArmG",
    "FusionArmV",
    "FusionArmP",
};

s32 GetWeaponIndex(EWeaponType type)
{
    if (type == EWeaponType::Power)
        return 0;
    else if (type == EWeaponType::Ice)
        return 1;
    else if (type == EWeaponType::Wave)
        return 2;
    else if (type == EWeaponType::Plasma)
        return 3;
    else if (type == EWeaponType::Phazon)
        return 4;
    return 0;
}

CGunWeapon::CGunWeapon(CAssetId ancsId, EWeaponType type, TUniqueId uid, EMaterialTypes mType, const zeus::CVector3f& vec)
    : x4_(vec),
      x104_gunCharacter(g_SimplePool->GetObj(SObjectTag{FOURCC('ANCS'), ancsId})),
      x13c_armCharacter(g_SimplePool->GetObj(skSuitArmNames[0])),
      x1c0_weaponType(type),
      x1c4_uid(uid),
      x1c8_matType(mType)
{
}

void CGunWeapon::AsyncLoadSuitArm(CStateManager& mgr)
{
}

}
