#include "CGunWeapon.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "Character/CModelData.hpp"

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

CGunWeapon::CGunWeapon(CAssetId ancsId, EWeaponType type, TUniqueId playerId,
                       EMaterialTypes playerMaterial, const zeus::CVector3f& scale)
: x4_scale(scale),
  x104_gunCharacter(g_SimplePool->GetObj(SObjectTag{FOURCC('ANCS'), ancsId})),
  x13c_armCharacter(g_SimplePool->GetObj(skSuitArmNames[0])),
  x1c0_weaponType(type),
  x1c4_playerId(playerId),
  x1c8_playerMaterial(playerMaterial)
{
}

void CGunWeapon::AsyncLoadSuitArm(CStateManager& mgr)
{
}

void CGunWeapon::AllocResPools(CPlayerState::EBeamId)
{

}

static const s32 skAnimTypeList[] = { 0, 4, 1, 2, 3, 5, 6, 7, 8, 9, 10 };

void CGunWeapon::PlayAnim(NWeaponTypes::EGunAnimType type, bool loop)
{
    if (x218_26 && type >= NWeaponTypes::EGunAnimType::BasePosition &&
        type <= NWeaponTypes::EGunAnimType::ToBeam)
    {
        x10_solidModelData->AnimationData()->EnableLooping(loop);
        CAnimPlaybackParms parms(skAnimTypeList[int(type)], -1, 1.f, true);
        x10_solidModelData->AnimationData()->SetAnimation(parms, false);
    }
}

void CGunWeapon::BuildSecondaryEffect(ESecondaryFxType type)
{
    switch (type)
    {
    case ESecondaryFxType::Two:
        if (x204_secondaryEffectType == ESecondaryFxType::Two)
            break;
        x1b8_secondaryEffect = std::make_unique<CElementGen>(x188_secondaryEffects[1],
                                                             CElementGen::EModelOrientationType::Normal,
                                                             CElementGen::EOptionalSystemFlags::One);
        x1b8_secondaryEffect->SetGlobalScale(x4_scale);
        break;
    case ESecondaryFxType::One:
        if (x204_secondaryEffectType == ESecondaryFxType::One)
            break;
        x1b8_secondaryEffect = std::make_unique<CElementGen>(x188_secondaryEffects[0],
                                                             CElementGen::EModelOrientationType::Normal,
                                                             CElementGen::EOptionalSystemFlags::One);
        x1b8_secondaryEffect->SetGlobalScale(x4_scale);
        break;
    default:
        break;
    }
    x204_secondaryEffectType = type;
}

void CGunWeapon::ActivateCharge(bool b1, bool b2)
{
}

void CGunWeapon::Touch(const CStateManager& mgr)
{

}

void CGunWeapon::TouchHolo(const CStateManager& mgr)
{

}

void CGunWeapon::DrawHologram(const CStateManager& mgr, const zeus::CTransform& xf, const CModelFlags& flags) const
{

}

void CGunWeapon::UpdateMuzzleFx(float dt, const zeus::CVector3f& scale, const zeus::CVector3f& pos, bool emitting)
{

}

const SWeaponInfo& CGunWeapon::GetWeaponInfo() const
{
    return g_tweakPlayerGun->GetBeamInfo(s32(x200_beamId));
}

zeus::CAABox CGunWeapon::GetBounds() const
{
    if (x10_solidModelData)
        return x10_solidModelData->GetBounds();
    return zeus::CAABox::skNullBox;
}

zeus::CAABox CGunWeapon::GetBounds(const zeus::CTransform& xf) const
{
    if (x10_solidModelData)
        return x10_solidModelData->GetBounds(xf);
    return zeus::CAABox::skNullBox;
}

bool CGunWeapon::IsChargeAnimOver() const
{
    return false;
}

}
