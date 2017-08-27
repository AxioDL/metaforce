#include "TCastTo.hpp"
#include "CPlayerGun.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "Character/CPrimitive.hpp"
#include "World/CPlayer.hpp"
#include "CEnergyProjectile.hpp"
#include "MP1/World/CMetroid.hpp"
#include "World/CScriptWater.hpp"
#include "World/CGameLight.hpp"
#include "Input/ControlMapper.hpp"
#include "CBomb.hpp"
#include "CPowerBomb.hpp"

namespace urde
{

static const zeus::CVector3f sGunScale(2.f);

static float kVerticalAngleTable[] = { -30.f, 0.f, 30.f };
static float kHorizontalAngleTable[] = { 30.f, 30.f, 30.f };
static float kVerticalVarianceTable[] = { 30.f, 30.f, 30.f };

float CPlayerGun::CMotionState::gGunExtendDistance = 0.125f;

CPlayerGun::CPlayerGun(TUniqueId playerId)
: x0_lights(8, zeus::CVector3f{-30.f, 0.f, 30.f}, 4, 4, 0, 0, 0, 0.1f), x538_playerId(playerId),
  x550_camBob(CPlayerCameraBob::ECameraBobType::One,
              zeus::CVector2f(CPlayerCameraBob::kCameraBobExtentX, CPlayerCameraBob::kCameraBobExtentY),
              CPlayerCameraBob::kCameraBobPeriod),
  x678_morph(g_tweakPlayerGun->GetGunTransformTime(), g_tweakPlayerGun->GetHoloHoldTime()),
  x6c8_(zeus::CVector3f(-0.29329199f, 0.f, -0.2481945f),
        zeus::CVector3f(0.29329199f, 1.292392f, 0.2481945f)),
  x6e0_rightHandModel(CAnimRes(g_tweakGunRes->xc_rightHand, 0, zeus::CVector3f(3.f), 0, true))
{
    x354_bombFuseTime = g_tweakPlayerGun->GetBombFuseTime();
    x358_bombDropDelayTime = g_tweakPlayerGun->GetBombDropDelayTime();
    x668_aimVerticalSpeed = g_tweakPlayerGun->GetAimVerticalSpeed();
    x66c_aimHorizontalSpeed = g_tweakPlayerGun->GetAimHorizontalSpeed();

    x73c_gunMotion = std::make_unique<CGunMotion>(g_tweakGunRes->x4_gunMotion, sGunScale);
    x740_grappleArm = std::make_unique<CGrappleArm>(sGunScale);
    x744_auxWeapon = std::make_unique<CAuxWeapon>(playerId);
    x748_rainSplashGenerator = std::make_unique<CRainSplashGenerator>(sGunScale, 20, 2, 0.f, 0.125f);
    x74c_powerBeam = std::make_unique<CPowerBeam>(g_tweakGunRes->x10_powerBeam, EWeaponType::Power,
                                                  playerId, EMaterialTypes::Player, sGunScale);
    x750_iceBeam = std::make_unique<CIceBeam>(g_tweakGunRes->x14_iceBeam, EWeaponType::Ice,
                                              playerId, EMaterialTypes::Player, sGunScale);
    x754_waveBeam = std::make_unique<CWaveBeam>(g_tweakGunRes->x18_waveBeam, EWeaponType::Wave,
                                                playerId, EMaterialTypes::Player, sGunScale);
    x758_plasmaBeam = std::make_unique<CPlasmaBeam>(g_tweakGunRes->x1c_plasmaBeam, EWeaponType::Plasma,
                                                    playerId, EMaterialTypes::Player, sGunScale);
    x75c_phazonBeam = std::make_unique<CPhazonBeam>(g_tweakGunRes->x20_phazonBeam, EWeaponType::Phazon,
                                                    playerId, EMaterialTypes::Player, sGunScale);
    x774_holoTransitionGen = std::make_unique<CElementGen>(
        g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), g_tweakGunRes->x24_holoTransition}),
        CElementGen::EModelOrientationType::Normal, CElementGen::EOptionalSystemFlags::One);
    x82c_shadow = std::make_unique<CWorldShadow>(32, 32, true);

    x832_31_ = true;
    x833_24_isFidgeting = true;
    x833_30_ = true;
    x6e0_rightHandModel.SetSortThermal(true);

    kVerticalAngleTable[2] = g_tweakPlayerGun->GetUpLookAngle();
    kVerticalAngleTable[0] = g_tweakPlayerGun->GetDownLookAngle();
    kHorizontalAngleTable[1] = g_tweakPlayerGun->GetHorizontalSpread();
    kHorizontalAngleTable[2] = g_tweakPlayerGun->GetHighHorizontalSpread();
    kHorizontalAngleTable[0] = g_tweakPlayerGun->GetLowHorizontalSpread();
    kVerticalVarianceTable[1] = g_tweakPlayerGun->GetVerticalSpread();
    kVerticalVarianceTable[2] = g_tweakPlayerGun->GetHighVerticalSpread();
    kVerticalVarianceTable[0] = g_tweakPlayerGun->GetLowVerticalSpread();
    CMotionState::SetExtendDistance(g_tweakPlayerGun->GetGunExtendDistance());

    InitBeamData();
    InitBombData();
    InitMuzzleData();
    InitCTData();
    LoadHandAnimTokens();
    x550_camBob.SetPlayerVelocity(zeus::CVector3f::skZero);
    x550_camBob.SetBobMagnitude(0.f);
    x550_camBob.SetBobTimeScale(0.f);
}

void CPlayerGun::InitBeamData()
{
    x760_selectableBeams[0] = x74c_powerBeam.get();
    x760_selectableBeams[0] = x750_iceBeam.get();
    x760_selectableBeams[0] = x754_waveBeam.get();
    x760_selectableBeams[0] = x758_plasmaBeam.get();
    x72c_currentBeam = x760_selectableBeams[0];
    x738_nextBeam = x72c_currentBeam;
    x774_holoTransitionGen->SetParticleEmission(true);
}

void CPlayerGun::InitBombData()
{
    x784_bombEffects.resize(2);
    x784_bombEffects[0].push_back(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), g_tweakGunRes->x28_bombSet}));
    x784_bombEffects[0].push_back(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), g_tweakGunRes->x2c_bombExplode}));
    TLockedToken<CGenDescription> pbExplode =
        g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), g_tweakGunRes->x30_powerBombExplode});
    x784_bombEffects[1].push_back(pbExplode);
    x784_bombEffects[1].push_back(pbExplode);
}

void CPlayerGun::InitMuzzleData()
{
    for (int i=0 ; i<5 ; ++i)
    {
        x7c0_auxMuzzleEffects.push_back(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'),
                                        g_tweakGunRes->xa4_auxMuzzle[i]}));
        x800_auxMuzzleGenerators.emplace_back(new CElementGen(x7c0_auxMuzzleEffects.back(),
                                              CElementGen::EModelOrientationType::Normal,
                                              CElementGen::EOptionalSystemFlags::One));
        x800_auxMuzzleGenerators.back()->SetParticleEmission(false);
    }
}

void CPlayerGun::InitCTData()
{
    x77c_.reset();
}

void CPlayerGun::LoadHandAnimTokens()
{
    std::set<CPrimitive> prims;
    for (int i=0 ; i<3 ; ++i)
    {
        CAnimPlaybackParms parms(i, -1, 1.f, true);
        x6e0_rightHandModel.GetAnimationData()->GetAnimationPrimitives(parms, prims);
    }
    CAnimData::PrimitiveSetToTokenVector(prims, x540_handAnimTokens, true);
}

void CPlayerGun::TakeDamage(bool bigStrike, bool notFromMetroid, CStateManager& mgr)
{
    bool hasAngle = false;
    float angle = 0.f;
    if (x398_damageAmt >= 10.f && !bigStrike && (x2f8_ & 0x10) != 0x10 && !x832_26_ && x384_ <= 0.f)
    {
        x384_ = 20.f;
        x364_ = 0.75f;
        if (x678_morph.GetGunState() == CGunMorph::EGunState::One)
        {
            zeus::CVector3f localDamageLoc = mgr.GetPlayer().GetTransform().transposeRotate(x3dc_damageLocation);
            angle = zeus::CRelAngle(std::atan2(localDamageLoc.y, localDamageLoc.x)).asDegrees();
            hasAngle = true;
        }
    }

    if (hasAngle || bigStrike)
    {
        if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Scan)
        {
            x73c_gunMotion->PlayPasAnim(SamusGun::EAnimationState::Struck, mgr, angle, bigStrike);
            if ((bigStrike && notFromMetroid) || x833_31_)
                x740_grappleArm->EnterStruck(mgr, angle, bigStrike, !x833_31_);
        }
    }

    x398_damageAmt = 0.f;
    x3dc_damageLocation = zeus::CVector3f::skZero;
}

void CPlayerGun::CreateGunLight(CStateManager& mgr)
{
    if (x53c_lightId != kInvalidUniqueId)
        return;
    x53c_lightId = mgr.AllocateUniqueId();
    CGameLight* light = new CGameLight(x53c_lightId, kInvalidAreaId, false, "GunLite", x3e8_xf, x538_playerId,
                                       CLight::BuildDirectional(zeus::CVector3f::skForward, zeus::CColor::skBlack),
                                       x53c_lightId.Value(), 0, 0.f);
    mgr.AddObject(light);
}

void CPlayerGun::DeleteGunLight(CStateManager& mgr)
{
    if (x53c_lightId == kInvalidUniqueId)
        return;
    mgr.FreeScriptObject(x53c_lightId);
    x53c_lightId = kInvalidUniqueId;
}

void CPlayerGun::UpdateGunLight(const zeus::CTransform& pos, CStateManager& mgr)
{

}

void CPlayerGun::SetGunLightActive(bool active, CStateManager& mgr)
{
    if (x53c_lightId == kInvalidUniqueId)
        return;

    if (TCastToPtr<CGameLight> light = mgr.ObjectById(x53c_lightId))
    {
        light->SetActive(active);
        if (active)
        {
            if (CElementGen* gen = x72c_currentBeam->GetChargeMuzzleFx())
            {
                if (gen->SystemHasLight())
                {
                    CLight genLight = gen->GetLight();
                    genLight.SetColor(zeus::CColor::skBlack);
                    light->SetLight(genLight);
                }
            }
        }
    }
}

static const u32 skBeamIdMap[] = { 0, 1, 2, 3, 0 };
static const u32 skBeamAnimIds[] = { 0, 1, 2, 1 };

void CPlayerGun::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr)
{
    const CPlayer& player = mgr.GetPlayer();
    bool isUnmorphed = player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed;
    switch (msg)
    {
    case EScriptObjectMessage::Registered:
    {
        CreateGunLight(mgr);
        x320_ = x314_pendingSelectedBeam = x310_selectedBeam =
            skBeamIdMap[int(mgr.GetPlayerState()->GetCurrentBeam())];
        x72c_currentBeam = x738_nextBeam = x760_selectableBeams[x310_selectedBeam];
        x72c_currentBeam->Load(mgr, true);
        x72c_currentBeam->SetRainSplashGenerator(x748_rainSplashGenerator.get());
        x744_auxWeapon->Load(x310_selectedBeam, mgr);
        CAnimPlaybackParms parms(skBeamAnimIds[int(mgr.GetPlayerState()->GetCurrentBeam())], -1, 1.f, true);
        x6e0_rightHandModel.AnimationData()->SetAnimation(parms, false);
        break;
    }
    case EScriptObjectMessage::Deleted:
        DeleteGunLight(mgr);
        break;
    case EScriptObjectMessage::UpdateSplashInhabitant:
        if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::PhazonSuit) && isUnmorphed)
        {
            if (TCastToConstPtr<CScriptWater> water = mgr.GetObjectById(sender))
            {
                if (water->GetFluidPlane().GetFluidType() == CFluidPlane::EFluidType::PhazonFluid)
                {
                    x835_24_canFirePhazon = true;
                    x835_25_inPhazonBeam = true;
                }
            }
        }
        if (player.GetDistanceUnderWater() > player.GetEyeHeight())
        {
            x834_27_underwater = true;
            if (x744_auxWeapon->IsComboFxActive(mgr) && x310_selectedBeam != 2)
                StopContinuousBeam(mgr, false);
        }
        else
        {
            x834_27_underwater = false;
        }
        break;
    case EScriptObjectMessage::RemoveSplashInhabitant:
        x834_27_underwater = false;
        x835_24_canFirePhazon = false;
        break;
    case EScriptObjectMessage::AddPhazonPoolInhabitant:
        x835_30_inPhazonPool = true;
        if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::PhazonSuit) && isUnmorphed)
            x835_24_canFirePhazon = true;
        break;
    case EScriptObjectMessage::UpdatePhazonPoolInhabitant:
        x835_30_inPhazonPool = true;
        if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::PhazonSuit) && isUnmorphed)
        {
            x835_24_canFirePhazon = true;
            x835_25_inPhazonBeam = true;
            if (x833_28_phazonBeamActive && static_cast<CPhazonBeam*>(x72c_currentBeam)->IsFiring())
                if (TCastToPtr<CEntity> ent = mgr.ObjectById(sender))
                    mgr.SendScriptMsg(ent.GetPtr(), x538_playerId, EScriptObjectMessage::Decrement);
        }
        break;
    case EScriptObjectMessage::RemovePhazonPoolInhabitant:
        x835_30_inPhazonPool = false;
        x835_24_canFirePhazon = false;
        break;
    case EScriptObjectMessage::Damage:
    {
        bool bigStrike = false;
        bool metroidAttached = false;
        if (TCastToConstPtr<CEnergyProjectile> proj = mgr.GetObjectById(sender))
        {
            if ((proj->GetAttribField() & CGameProjectile::EProjectileAttrib::BigStrike) !=
                CGameProjectile::EProjectileAttrib::None)
            {
                x394_damageTimer = proj->GetDamageDuration();
                bigStrike = true;
            }
        }
        else if (TCastToConstPtr<CPatterned> ai = mgr.GetObjectById(sender))
        {
            if (ai->GetX402_28())
            {
                x394_damageTimer = ai->GetDamageDuration();
                bigStrike = true;
                if (player.GetAttachedActor() != kInvalidUniqueId)
                    metroidAttached = CPatterned::CastTo<MP1::CMetroid>(
                        mgr.GetObjectById(player.GetAttachedActor())) != nullptr;
            }
        }
        if (!x834_30_inBigStrike)
        {
            if (bigStrike)
            {
                x834_31_ = false;
                CancelFiring(mgr);
            }
            TakeDamage(bigStrike, !metroidAttached, mgr);
            x834_30_inBigStrike = bigStrike;
        }
        break;
    }
    case EScriptObjectMessage::OnFloor:
        if (player.GetControlsFrozen() && !x834_30_inBigStrike)
        {
            x2f4_fireButtonStates = 0;
            x2ec_lastFireButtonStates = 0;
            CancelFiring(mgr);
            TakeDamage(true, false, mgr);
            x394_damageTimer = 0.75f;
            x834_30_inBigStrike = true;
        }
        break;
    default:
        break;
    }

    x740_grappleArm->AcceptScriptMsg(msg, sender, mgr);
    x758_plasmaBeam->AcceptScriptMsg(msg, sender, mgr);
    x75c_phazonBeam->AcceptScriptMsg(msg, sender, mgr);
    x744_auxWeapon->AcceptScriptMsg(msg, sender, mgr);
}

void CPlayerGun::AsyncLoadSuit(CStateManager& mgr)
{
    x72c_currentBeam->AsyncLoadSuitArm(mgr);
    x740_grappleArm->AsyncLoadSuit(mgr);
}

void CPlayerGun::TouchModel(const CStateManager& mgr)
{
    if (mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed)
    {
        x73c_gunMotion->GetModelData().Touch(mgr, 0);
        switch (x33c_gunOverrideMode)
        {
        case EGunOverrideMode::One:
            if (x75c_phazonBeam)
                x75c_phazonBeam->Touch(mgr);
            break;
        case EGunOverrideMode::Two:
            if (x738_nextBeam)
                x738_nextBeam->Touch(mgr);
            break;
        default:
            if (!x833_28_phazonBeamActive)
                x72c_currentBeam->Touch(mgr);
            else
                x75c_phazonBeam->Touch(mgr);
            break;
        }
        x72c_currentBeam->TouchHolo(mgr);
        x740_grappleArm->TouchModel(mgr);
        x6e0_rightHandModel.Touch(mgr, 0);
    }

    if (x734_)
    {
        x734_->Touch(mgr);
        x734_->TouchHolo(mgr);
    }
}

void CPlayerGun::DamageRumble(const zeus::CVector3f& location, float damage, const CStateManager& mgr)
{
    x398_damageAmt = damage;
    x3dc_damageLocation = location;
}

void CPlayerGun::StopChargeSound(CStateManager& mgr)
{
    if (x2e0_chargeSfx)
    {
        CSfxManager::SfxStop(x2e0_chargeSfx);
        x2e0_chargeSfx.reset();
    }
    if (x830_chargeRumbleHandle != -1)
    {
        mgr.GetRumbleManager().StopRumble(x830_chargeRumbleHandle);
        x830_chargeRumbleHandle = -1;
    }
}

void CPlayerGun::ResetCharge(CStateManager& mgr, bool b1)
{
    if (x32c_)
        StopChargeSound(mgr);

    if ((x2f8_ & 0x8) != 0x8 && (x2f8_ & 0x10) != 0x10)
    {
        bool r30 = !(mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed && !b1);
        if (x832_27_ || r30)
            PlayAnim(NWeaponTypes::EGunAnimType::Zero, false);
        if (r30)
            x72c_currentBeam->EnableSecondaryFx(CGunWeapon::ESecondaryFxType::Zero);
        if ((x2f8_ & 0x2) != 0x2 || x330_chargeState != EChargeState::Normal)
        {
            if ((x2f8_ & 0x8) != 0x8)
            {
                x2f8_ |= 0x1;
                x2f8_ &= 0xFFE9;
            }
            x318_ = 0;
            x31c_missileMode = EMissleMode::Inactive;
        }
    }

    x32c_ = 0;
    x330_chargeState = EChargeState::Normal;
    x320_ = x310_selectedBeam;
    x833_30_ = true;
    x832_27_ = false;
    x832_26_ = false;
    x344_ = 0.f;
}

bool CPlayerGun::ExitMissile()
{
    if ((x2f8_ & 0x1) == 0x1)
        return true;
    if ((x2f8_ & 0x10) == 0x10 || x338_ == 2)
        return false;
    x338_ = 2;
    PlayAnim(NWeaponTypes::EGunAnimType::FromMissile, false);
    return false;
}

static const CPlayerState::EItemType skBeamArr[] =
{
    CPlayerState::EItemType::PowerBeam,
    CPlayerState::EItemType::IceBeam,
    CPlayerState::EItemType::WaveBeam,
    CPlayerState::EItemType::PlasmaBeam
};

static const CPlayerState::EItemType skBeamComboArr[] =
{
    CPlayerState::EItemType::SuperMissile,
    CPlayerState::EItemType::IceSpreader,
    CPlayerState::EItemType::Wavebuster,
    CPlayerState::EItemType::Flamethrower
};

static const ControlMapper::ECommands mBeamCtrlCmd[] =
{
    ControlMapper::ECommands::PowerBeam,
    ControlMapper::ECommands::IceBeam,
    ControlMapper::ECommands::WaveBeam,
    ControlMapper::ECommands::PlasmaBeam,
};

void CPlayerGun::HandleBeamChange(const CFinalInput& input, CStateManager& mgr)
{
    CPlayerState& playerState = *mgr.GetPlayerState();
    float maxBeamInput = 0.f;
    int selectBeam = -1;
    for (int i=0 ; i<4 ; ++i)
    {
        if (playerState.HasPowerUp(skBeamArr[i]))
        {
            float inputVal = ControlMapper::GetAnalogInput(mBeamCtrlCmd[i], input);
            if (inputVal > 0.65f && inputVal > maxBeamInput)
            {
                maxBeamInput = inputVal;
                selectBeam = i;
            }
        }
    }

    if (selectBeam < 0)
        return;

    x833_25_ = true;
    if (x310_selectedBeam != selectBeam && playerState.HasPowerUp(skBeamArr[selectBeam]))
    {
        x314_pendingSelectedBeam = u32(selectBeam);
        u32 flags = 0;
        if ((x2f8_ & 0x10) == 0x10)
            flags = 0x10;
        flags |= 0x8;
        x2f8_ = flags;
        PlayAnim(NWeaponTypes::EGunAnimType::FromBeam, false);
        if (x833_31_ || x744_auxWeapon->IsComboFxActive(mgr) || x832_26_)
        {
            x832_30_ = true;
            x740_grappleArm->EnterIdle(mgr);
        }
        x72c_currentBeam->EnableSecondaryFx(CGunWeapon::ESecondaryFxType::Zero);
        x338_ = 5;
        x2e4_invalidSfx.reset();
    }
    else if (playerState.HasPowerUp(skBeamArr[selectBeam]))
    {
        if (ExitMissile())
        {
            if (!CSfxManager::IsPlaying(x2e4_invalidSfx))
                x2e4_invalidSfx = PlaySfx(1763, x834_27_underwater, false, 0.165f);
        }
        else
        {
            x2e4_invalidSfx.reset();
        }
    }
}

void CPlayerGun::SetPhazonBeamMorph(bool intoPhazonBeam)
{
    x39c_ = intoPhazonBeam ? 0.f : 1.f;
    x835_27_intoPhazonBeam = intoPhazonBeam;
    x835_26_phazonBeamMorphing = true;
}

void CPlayerGun::Reset(CStateManager& mgr, bool b1)
{
    x72c_currentBeam->Reset(mgr);
    x832_25_ = false;
    x832_24_ = false;
    x833_26_ = false;
    x348_ = 0.f;
    SetGunLightActive(false, mgr);
    if ((x2f8_ & 0x10) != 0x10)
    {
        if (!b1 && (x2f8_ & 0x2) != 0x2)
        {
            if ((x2f8_ & 0x8) != 0x8)
            {
                x2f8_ |= 0x1;
                x2f8_ &= 0xFFE9;
            }
            x318_ = 0;
            x31c_missileMode = EMissleMode::Inactive;
        }
    }
    else
    {
        x2f8_ &= ~0x7;
    }
}

void CPlayerGun::ResetBeamParams(CStateManager& mgr, const CPlayerState& playerState, bool playSelectionSfx)
{
    StopContinuousBeam(mgr, true);
    if (playerState.ItemEnabled(CPlayerState::EItemType::ChargeBeam))
        ResetCharge(mgr, false);
    CAnimPlaybackParms parms(skBeamAnimIds[x314_pendingSelectedBeam], -1, 1.f, true);
    x6e0_rightHandModel.AnimationData()->SetAnimation(parms, false);
    Reset(mgr, false);
    if (playSelectionSfx)
        CSfxManager::SfxStart(1774, 1.f, 0.f, true, 0x7f, false, kInvalidAreaId);
    x2ec_lastFireButtonStates &= ~0x1;
    x320_ = x310_selectedBeam;
    x833_30_ = true;
}

CSfxHandle CPlayerGun::PlaySfx(u16 sfx, bool underwater, bool looped, float pan)
{
    CSfxHandle hnd = CSfxManager::SfxStart(sfx, 1.f, pan, true, 0x7f, looped, kInvalidAreaId);
    CSfxManager::SfxSpan(hnd, 0.f);
    if (underwater)
        CSfxManager::PitchBend(hnd, -1.f);
    return hnd;
}

static const u16 skFromMissileSound[] = { 1824, 1849, 1851, 1853 };
static const u16 skFromBeamSound[] = { 0, 1822, 1828, 1826 };
static const u16 skToMissileSound[] = { 1823, 1829, 1850, 1852 };

void CPlayerGun::PlayAnim(NWeaponTypes::EGunAnimType type, bool loop)
{
    if (x338_ != 5)
        x72c_currentBeam->PlayAnim(type, loop);

    u16 sfx = 0xffff;
    switch (type)
    {
    case NWeaponTypes::EGunAnimType::FromMissile:
        x2f8_ &= ~0x4;
        sfx = skFromMissileSound[x310_selectedBeam];
        break;
    case NWeaponTypes::EGunAnimType::MissileReload:
        sfx = 1769;
        break;
    case NWeaponTypes::EGunAnimType::FromBeam:
        sfx = skFromBeamSound[x310_selectedBeam];
        break;
    case NWeaponTypes::EGunAnimType::ToMissile:
        x2f8_ &= ~0x1;
        sfx = skToMissileSound[x310_selectedBeam];
        break;
    default:
        break;
    }

    if (sfx != 0xffff)
        PlaySfx(sfx, x834_27_underwater, false, 0.165f);
}

void CPlayerGun::CancelCharge(CStateManager& mgr, bool withEffect)
{
    if (withEffect)
    {
        x32c_ = 9;
        x72c_currentBeam->EnableSecondaryFx(CGunWeapon::ESecondaryFxType::Three);
    }
    else
    {
        x72c_currentBeam->EnableSecondaryFx(CGunWeapon::ESecondaryFxType::Zero);
    }

    x834_24_charging = false;
    x348_ = 0.f;
    x72c_currentBeam->ActivateCharge(false, false);
    SetGunLightActive(false, mgr);
}

void CPlayerGun::HandlePhazonBeamChange(CStateManager& mgr)
{
    bool inMorph = false;
    switch (x33c_gunOverrideMode)
    {
    case EGunOverrideMode::Normal:
        SetPhazonBeamMorph(true);
        x338_ = 8;
        inMorph = true;
        break;
    case EGunOverrideMode::Three:
        if (!x835_25_inPhazonBeam)
        {
            SetPhazonBeamMorph(true);
            x338_ = 9;
            inMorph = true;
            if (x75c_phazonBeam)
            {
                x75c_phazonBeam->SetX274_25(false);
                x75c_phazonBeam->SetX274_26(true);
            }
        }
        break;
    default:
        break;
    }

    if (inMorph)
    {
        ResetBeamParams(mgr, *mgr.GetPlayerState(), true);
        x2f8_ = 0x8;
        PlayAnim(NWeaponTypes::EGunAnimType::FromBeam, false);
        if (x833_31_)
        {
            x832_30_ = true;
            x740_grappleArm->EnterIdle(mgr);
        }
        CancelCharge(mgr, false);
    }
}

void CPlayerGun::HandleWeaponChange(const CFinalInput& input, CStateManager& mgr)
{
    x833_25_ = false;
    if (ControlMapper::GetPressInput(ControlMapper::ECommands::Morph, input))
        StopContinuousBeam(mgr, true);
    if ((x2f8_ & 0x8) != 0x8)
    {
        if (!x835_25_inPhazonBeam)
            HandleBeamChange(input, mgr);
        else
            HandlePhazonBeamChange(mgr);
    }
}

void CPlayerGun::ProcessInput(const CFinalInput& input, CStateManager& mgr)
{
    CPlayerState& state = *mgr.GetPlayerState();
    bool damageNotMorphed = (x834_30_inBigStrike &&
        mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed);
    if (x832_24_ || damageNotMorphed || (x2f8_ & 0x8) == 0x8)
        return;
    if (state.HasPowerUp(CPlayerState::EItemType::ChargeBeam))
    {
        if (!state.ItemEnabled(CPlayerState::EItemType::ChargeBeam))
            state.EnableItem(CPlayerState::EItemType::ChargeBeam);
    }
    else if (state.ItemEnabled(CPlayerState::EItemType::ChargeBeam))
    {
        state.DisableItem(CPlayerState::EItemType::ChargeBeam);
        ResetCharge(mgr, false);
    }
    switch (mgr.GetPlayer().GetMorphballTransitionState())
    {
    default:
        x2f4_fireButtonStates = 0;
        break;
    case CPlayer::EPlayerMorphBallState::Unmorphed:
        if ((x2f8_ & 0x10) != 0x10)
            HandleWeaponChange(input, mgr);
    case CPlayer::EPlayerMorphBallState::Morphed:
        x2f4_fireButtonStates = ControlMapper::GetDigitalInput(ControlMapper::ECommands::FireOrBomb, input) ? 1 : 0;
        x2f4_fireButtonStates |= ControlMapper::GetDigitalInput(ControlMapper::ECommands::MissileOrPowerBomb, input) ? 2 : 0;
        break;
    }
}

void CPlayerGun::UnLoadFidget()
{
    if ((x2fc_fidgetAnimBits & 0x1) == 0x1)
        x73c_gunMotion->GunController().UnLoadFidget();
    if ((x2fc_fidgetAnimBits & 0x2) == 0x2)
        x72c_currentBeam->UnLoadFidget();
    if ((x2fc_fidgetAnimBits & 0x4) == 0x4)
        if (CGunController* gc = x740_grappleArm->GunController())
            gc->UnLoadFidget();
    x2fc_fidgetAnimBits = 0;
}

void CPlayerGun::ReturnArmAndGunToDefault(CStateManager& mgr, bool b1)
{
    if (b1 || !x833_31_)
    {
        x73c_gunMotion->ReturnToDefault(mgr, false);
        x740_grappleArm->ReturnToDefault(mgr, 0.f, false);
    }
    if (!x834_25_)
        x72c_currentBeam->ReturnToDefault(mgr);
    x834_25_ = false;
}

void CPlayerGun::ReturnToRestPose()
{
    if (x832_31_)
        return;
    if ((x2f8_ & 0x1) == 0x1)
        PlayAnim(NWeaponTypes::EGunAnimType::Zero, false);
    else if ((x2f8_ & 0x4) == 0x4)
        PlayAnim(NWeaponTypes::EGunAnimType::ToMissile, false);
    x832_31_ = true;
}

void CPlayerGun::ResetIdle(CStateManager& mgr)
{
    x550_camBob.SetState(CPlayerCameraBob::ECameraBobState::GunFireNoBob, mgr);
    if (x3a4_fidget.GetState() != CFidget::EState::Zero)
    {
        if (x3a4_fidget.GetState() == CFidget::EState::Seven)
            UnLoadFidget();
        ReturnArmAndGunToDefault(mgr, true);
    }
    x3a4_fidget.ResetAll();
    ReturnToRestPose();
    if (x324_ != 0)
        x324_ = 0;
    if (!x740_grappleArm->GetActive())
        x834_26_ = false;
}

void CPlayerGun::CancelFiring(CStateManager& mgr)
{
    if (x32c_ == 8)
        ReturnArmAndGunToDefault(mgr, true);
    if ((x2f8_ & 0x10) == 0x10)
    {
        StopContinuousBeam(mgr, true);
        if ((x2f8_ & 0x8) == 0x8)
        {
            x2f8_ |= 0x1;
            x2f8_ &= 0xFFE9;
        }
        x318_ = 0;
        x31c_missileMode = EMissleMode::Inactive;
    }
    if (x32c_ != 0)
    {
        x72c_currentBeam->ActivateCharge(false, false);
        SetGunLightActive(false, mgr);
        ResetCharge(mgr, true);
    }
    Reset(mgr, (x2f8_ & 0x2) == 0x2);
}

float CPlayerGun::GetBeamVelocity() const
{
    if (x72c_currentBeam->IsLoaded())
        return x72c_currentBeam->GetVelocityInfo().GetVelocity(int(x330_chargeState)).y;
    return 10.f;
}

void CPlayerGun::StopContinuousBeam(CStateManager& mgr, bool b1)
{
    if ((x2f8_ & 0x10) == 0x10)
    {
        ReturnArmAndGunToDefault(mgr, false);
        x744_auxWeapon->StopComboFx(mgr, b1);
        switch (x310_selectedBeam)
        {
        case 0:
        case 2:
        case 3:
            // All except ice
            if (x310_selectedBeam != 0 || x833_28_phazonBeamActive)
            {
                x72c_currentBeam->EnableSecondaryFx(
                    b1 ? CGunWeapon::ESecondaryFxType::Zero : CGunWeapon::ESecondaryFxType::Three);
            }
            break;
        default:
            break;
        }
    }
    else if (x833_28_phazonBeamActive)
    {
        if (static_cast<CPhazonBeam*>(x72c_currentBeam)->IsFiring())
            static_cast<CPhazonBeam*>(x72c_currentBeam)->StopBeam(mgr, b1);
    }
    else if (x310_selectedBeam == 3) // Plasma
    {
        if (static_cast<CPlasmaBeam*>(x72c_currentBeam)->IsFiring())
            static_cast<CPlasmaBeam*>(x72c_currentBeam)->StopBeam(mgr, b1);
    }
}

void CPlayerGun::CMotionState::Update(bool b1, float dt, zeus::CTransform& xf, CStateManager& mgr)
{

}

void CPlayerGun::ChangeWeapon(const CPlayerState& playerState, CStateManager& mgr)
{

}

void CPlayerGun::GetLctrWithShake(zeus::CTransform& xfOut, const CModelData& mData, const std::string& lctrName,
                                  bool b1, bool b2)
{

}

void CPlayerGun::UpdateLeftArmTransform(const CModelData& mData, const CStateManager& mgr)
{

}

void CPlayerGun::ProcessGunMorph(float dt, CStateManager& mgr)
{

}

void CPlayerGun::ProcessPhazonGunMorph(float dt, CStateManager& mgr)
{

}

void CPlayerGun::UpdateChargeState(float dt, CStateManager& mgr)
{

}

void CPlayerGun::UpdateAuxWeapons(float dt, const zeus::CTransform& targetXf, CStateManager& mgr)
{

}

void CPlayerGun::DoUserAnimEvents(float dt, CStateManager& mgr)
{

}

TUniqueId CPlayerGun::GetTargetId(CStateManager& mgr) const
{
    return {};
}

void CPlayerGun::CancelLockOn()
{

}

void CPlayerGun::FireSecondary(float dt, CStateManager& mgr)
{

}

void CPlayerGun::ResetCharged(float dt, CStateManager& mgr)
{
    if (x832_26_)
        return;
    if (x32c_ >= 3)
    {
        x833_30_ = false;
        UpdateNormalShotCycle(dt, mgr);
        x832_24_ = true;
        CancelCharge(mgr, true);
    }
    else if (x32c_ != 0)
    {
        x320_ = x310_selectedBeam;
        x833_30_ = true;
        x32c_ = 10;
    }
    StopChargeSound(mgr);
}

void CPlayerGun::ActivateCombo(CStateManager& mgr)
{

}

void CPlayerGun::ProcessChargeState(u32 releasedStates, u32 pressedStates, CStateManager& mgr, float dt)
{
    if ((releasedStates & 0x1) != 0)
        ResetCharged(dt, mgr);
    if ((pressedStates & 0x1) != 0)
    {
        if (x32c_ == 0 && (pressedStates & 0x1) != 0 && x348_ == 0.f && x832_28_)
        {
            UpdateNormalShotCycle(dt, mgr);
            x32c_ = 1;
        }
    }
    else if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::Missiles) && (pressedStates & 0x2) != 0)
    {
        if (x32c_ >= 4)
        {
            if (mgr.GetPlayerState()->HasPowerUp(skBeamComboArr[x310_selectedBeam]))
                ActivateCombo(mgr);
        }
        else if (x32c_ == 0)
        {
            FireSecondary(dt, mgr);
        }
    }
}

void CPlayerGun::ResetNormal(CStateManager& mgr)
{
    Reset(mgr, false);
    x832_28_ = false;
}

void CPlayerGun::UpdateNormalShotCycle(float dt, CStateManager& mgr)
{
    if (!ExitMissile())
        return;
    if (mgr.GetCameraManager()->IsInCinematicCamera())
        return;
    x832_25_ = x833_28_phazonBeamActive || x310_selectedBeam != 3 || x32c_ != 0;
    x30c_ += 1;
    zeus::CTransform xf = x833_29_pointBlankWorldSurface ? x448_elbowWorldXf : x4a8_gunWorldXf * x418_beamLocalXf;
    if (!x833_29_pointBlankWorldSurface && x364_ <= 0.f)
    {
        zeus::CVector3f oldOrigin = xf.origin;
        xf = x478_assistAimXf;
        xf.origin = oldOrigin;
    }
    xf.origin += mgr.GetCameraManager()->GetGlobalCameraTranslation(mgr);
    x38c_ = 0.0625f;
    TUniqueId homingTarget;
    if (x72c_currentBeam->GetVelocityInfo().GetTargetHoming(int(x330_chargeState)))
        homingTarget = GetTargetId(mgr);
    else
        homingTarget = kInvalidUniqueId;
    x72c_currentBeam->Fire(x834_27_underwater, dt, x330_chargeState, xf, mgr, homingTarget);
    mgr.InformListeners(x4a8_gunWorldXf.origin, EListenNoiseType::Zero);
}

void CPlayerGun::ProcessNormalState(u32 releasedStates, u32 pressedStates, CStateManager& mgr, float dt)
{
    if ((releasedStates & 0x1) != 0)
        ResetNormal(mgr);
    if ((pressedStates & 0x1) != 0 && x348_ == 0.f && x832_28_)
        UpdateNormalShotCycle(dt, mgr);
    else if ((pressedStates & 0x2) != 0)
        FireSecondary(dt, mgr);
}

void CPlayerGun::UpdateWeaponFire(float dt, const CPlayerState& playerState, CStateManager& mgr)
{
    u32 oldFiring = x2ec_lastFireButtonStates;
    x2ec_lastFireButtonStates = x2f4_fireButtonStates;
    u32 pressedStates = x2f4_fireButtonStates & (oldFiring ^ x2f4_fireButtonStates);
    x2f0_pressedFireButtonStates = pressedStates;
    u32 releasedStates = oldFiring & (oldFiring ^ x2f4_fireButtonStates);
    x832_28_ = false;

    CPlayer& player = mgr.GetPlayer();
    if (!x832_24_ && !x834_30_inBigStrike)
    {
        float coolDown = x72c_currentBeam->GetWeaponInfo().x0_coolDown;
        if ((pressedStates & 0x1) == 0)
        {
            if (x390_cooldown >= coolDown)
            {
                x390_cooldown = coolDown;
                if (player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed &&
                    mgr.GetPlayerState()->ItemEnabled(CPlayerState::EItemType::ChargeBeam) &&
                    player.GetGunHolsterState() == CPlayer::EGunHolsterState::Drawn &&
                    player.GetGrappleState() == CPlayer::EGrappleState::None &&
                    mgr.GetPlayerState()->GetTransitioningVisor() != CPlayerState::EPlayerVisor::Scan &&
                    mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Scan &&
                    (x2ec_lastFireButtonStates & 0x1) != 0 && x32c_ == 0)
                {
                    x832_28_ = true;
                    pressedStates |= 0x1;
                    x390_cooldown = 0.f;
                }
            }
        }
        else if (x390_cooldown >= coolDown)
        {
            x832_28_ = true;
            x390_cooldown = 0.f;
        }
        x390_cooldown += dt;
    }

    if (x834_28_)
        x834_28_ = (x2ec_lastFireButtonStates & 0x1) != 0;

    if (player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed)
    {
        x835_28_bombReady = false;
        x835_29_powerBombReady = false;
        if (!x835_31_actorAttached)
        {
            x835_28_bombReady = false;
            if (x53a_powerBomb != kInvalidUniqueId &&
                !mgr.CanCreateProjectile(x538_playerId, EWeaponType::PowerBomb, 1))
            {
                auto* pb = static_cast<const CPowerBomb*>(mgr.GetObjectById(x53a_powerBomb));
                if (pb && pb->GetCurTime() <= 4.25f)
                    x835_28_bombReady = false;
                else
                    x53a_powerBomb = kInvalidUniqueId;
            }
            if (((pressedStates & 0x1) != 0 || x32c_ != 0) &&
                mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::MorphBallBombs))
            {
                if (x835_28_bombReady)
                    DropBomb(EBWeapon::Zero, mgr);
            }
            else if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::PowerBombs) &&
                     mgr.GetPlayerState()->GetItemAmount(CPlayerState::EItemType::PowerBombs) > 0)
            {
                x835_29_powerBombReady = mgr.CanCreateProjectile(x538_playerId, EWeaponType::PowerBomb, 1) &&
                                         mgr.CanCreateProjectile(x538_playerId, EWeaponType::Bomb, 1);
                if ((pressedStates & 0x2) != 0 && x835_29_powerBombReady)
                    DropBomb(EBWeapon::One, mgr);
            }
        }
    }
    else if ((x2f8_ & 0x8) != 0x8 &&
             player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed)
    {
        if ((pressedStates & 0x2) != 0 && !x318_ && (x2f8_ & 0x2) != 0x2 && !x32c_)
        {
            u32 missileCount = mgr.GetPlayerState()->GetItemAmount(CPlayerState::EItemType::Missiles);
            if (x338_ != 1 && x338_ != 2)
            {
                if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::Missiles) && missileCount > 0)
                {
                    x300_ = missileCount;
                    if (x300_ > 5)
                        x300_ = 5;
                    if (!x835_25_inPhazonBeam)
                    {
                        x2f8_ &= ~0x1;
                        x2f8_ |= 0x6;
                        x318_ = 1;
                        x31c_missileMode = EMissleMode::Active;
                    }
                    FireSecondary(dt, mgr);
                }
                else
                {
                    if (!CSfxManager::IsPlaying(x2e4_invalidSfx))
                        x2e4_invalidSfx = PlaySfx(1781, x834_27_underwater, false, 0.165f);
                    else
                        x2e4_invalidSfx.reset();
                }
            }
        }
        else
        {
            if (x3a4_fidget.GetState() == CFidget::EState::Zero)
            {
                if ((x2f8_ & 0x10) == 0x10 && x744_auxWeapon->IsComboFxActive(mgr))
                {
                    if (x2ec_lastFireButtonStates == 0 || (x310_selectedBeam == 2 && x833_29_pointBlankWorldSurface))
                    {
                        StopContinuousBeam(mgr, (x2f8_ & 0x8) == 0x8);
                    }
                }
                else
                {
                    if (mgr.GetPlayerState()->ItemEnabled(CPlayerState::EItemType::ChargeBeam) &&
                        x33c_gunOverrideMode == EGunOverrideMode::Normal)
                        ProcessChargeState(releasedStates, pressedStates, mgr, dt);
                    else
                        ProcessNormalState(releasedStates, pressedStates, mgr, dt);
                }
            }
        }
    }
}

void CPlayerGun::EnterFreeLook(CStateManager& mgr)
{
    if (!x832_30_)
        x73c_gunMotion->PlayPasAnim(SamusGun::EAnimationState::FreeLook, mgr, 0.f, false);
    x740_grappleArm->EnterFreeLook(x835_25_inPhazonBeam ? 1 : x310_selectedBeam,
                                   x73c_gunMotion->GetFreeLookSetId(), mgr);
}

void CPlayerGun::SetFidgetAnimBits(int parm2, bool beamOnly)
{
    x2fc_fidgetAnimBits = 0;
    if (beamOnly)
    {
        x2fc_fidgetAnimBits = 2;
        return;
    }

    switch (x3a4_fidget.GetType())
    {
    case SamusGun::EFidgetType::Zero:
        x2fc_fidgetAnimBits = 1;
        if (parm2 != 1)
            return;
        x2fc_fidgetAnimBits |= 4;
        break;
    case SamusGun::EFidgetType::One:
        if (parm2 >= 6 || parm2 < 4)
            x2fc_fidgetAnimBits = 2;
        else
            x2fc_fidgetAnimBits = 1;
        x2fc_fidgetAnimBits |= 4;
        break;
    default:
        break;
    }
}

void CPlayerGun::AsyncLoadFidget(CStateManager& mgr)
{
    SetFidgetAnimBits(x3a4_fidget.GetParm2(), x3a4_fidget.GetState() == CFidget::EState::Three);
    if ((x2fc_fidgetAnimBits & 0x1) == 0x1)
        x73c_gunMotion->GunController().LoadFidgetAnimAsync(mgr, s32(x3a4_fidget.GetType()),
                                                            x310_selectedBeam, x3a4_fidget.GetParm2());
    if ((x2fc_fidgetAnimBits & 0x2) == 0x2)
    {
        x72c_currentBeam->AsyncLoadFidget(mgr, (x3a4_fidget.GetState() == CFidget::EState::Three ?
            SamusGun::EFidgetType::Zero : x3a4_fidget.GetType()), x3a4_fidget.GetParm2());
        x832_31_ = false;
    }
    if ((x2fc_fidgetAnimBits & 0x4) == 0x4)
        if (CGunController* gc = x740_grappleArm->GunController())
            gc->LoadFidgetAnimAsync(mgr, s32(x3a4_fidget.GetType()),
                                    x3a4_fidget.GetType() != SamusGun::EFidgetType::Zero ? x310_selectedBeam : 0,
                                    x3a4_fidget.GetParm2());
}

bool CPlayerGun::IsFidgetLoaded() const
{
    int loadFlags = 0;
    if ((x2fc_fidgetAnimBits & 0x1) == 0x1 && x73c_gunMotion->GunController().IsFidgetLoaded())
        loadFlags |= 0x1;
    if ((x2fc_fidgetAnimBits & 0x2) == 0x2 && x72c_currentBeam->IsFidgetLoaded())
        loadFlags |= 0x2;
    if ((x2fc_fidgetAnimBits & 0x4) == 0x4)
        if (CGunController* gc = x740_grappleArm->GunController())
            if (gc->IsFidgetLoaded())
                loadFlags |= 0x4;
    return x2fc_fidgetAnimBits == loadFlags;
}

void CPlayerGun::EnterFidget(CStateManager& mgr)
{
    if ((x2fc_fidgetAnimBits & 0x1) == 0x1)
    {
        x73c_gunMotion->EnterFidget(mgr, x3a4_fidget.GetType(), x3a4_fidget.GetParm2());
        x834_25_ = true;
    }
    else
    {
        x834_25_ = false;
    }

    if ((x2fc_fidgetAnimBits & 0x2) == 0x2)
        x72c_currentBeam->EnterFidget(mgr, x3a4_fidget.GetType(), x3a4_fidget.GetParm2());

    if ((x2fc_fidgetAnimBits & 0x4) == 0x4)
        x740_grappleArm->EnterFidget(mgr, x3a4_fidget.GetType(),
                                     x3a4_fidget.GetType() != SamusGun::EFidgetType::Zero ? x310_selectedBeam : 0,
                                     x3a4_fidget.GetParm2());

    UnLoadFidget();
    x3a4_fidget.DoneLoading();
}

void CPlayerGun::UpdateGunIdle(bool b1, float camBobT, float dt, CStateManager& mgr)
{
    CPlayer& player = mgr.GetPlayer();
    if (player.IsInFreeLook() && !x832_29_ && !x740_grappleArm->IsGrappling() &&
        x3a4_fidget.GetState() != CFidget::EState::Three &&
        player.GetGunHolsterState() == CPlayer::EGunHolsterState::Drawn && !x834_30_inBigStrike)
    {
        if ((x2f8_ & 0x8) != 0x8)
        {
            if (!x833_31_ && !x834_26_)
            {
                if (x388_ < 0.25f)
                    x388_ += dt;
                if (x388_ >= 0.25f && !x740_grappleArm->IsSuitLoading())
                {
                    EnterFreeLook(mgr);
                    x833_31_ = true;
                }
            }
            else
            {
                x388_ = 0.f;
                if (x834_26_)
                    ResetIdle(mgr);
            }
        }
    }
    else
    {
        if (x833_31_)
        {
            if ((x2f8_ & 0x10) != 0x10)
            {
                x73c_gunMotion->ReturnToDefault(mgr, x834_30_inBigStrike);
                x740_grappleArm->ReturnToDefault(mgr, 0.f, false);
            }
            x833_31_ = false;
        }
        x388_ = 0.f;
        if (player.GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed)
        {
            x833_24_isFidgeting = (player.GetSurfaceRestraint() != CPlayer::ESurfaceRestraints::Water &&
                mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Scan &&
                (x2f4_fireButtonStates & 0x3) == 0 && x32c_ == 0 && !x832_29_ && (x2f8_ & 0x8) != 0x8 &&
                x364_ <= 0.f && player.GetPlayerMovementState() == CPlayer::EPlayerMovementState::OnGround &&
                !player.IsInFreeLook() && !player.GetFreeLookStickState() && x304_ == 0 &&
                std::fabs(player.GetAngularVelocityOR().angle()) <= 0.1f && camBobT <= 0.01f &&
                !mgr.GetCameraManager()->IsInCinematicCamera() &&
                player.GetGunHolsterState() == CPlayer::EGunHolsterState::Drawn &&
                player.GetGrappleState() == CPlayer::EGrappleState::None && !x834_30_inBigStrike &&
                !x835_25_inPhazonBeam);
            if (x833_24_isFidgeting)
            {
                if (!x834_30_inBigStrike)
                {
                    bool r19 = camBobT > 0.01f && (x2f4_fireButtonStates & 0x3) == 0;
                    if (r19)
                    {
                        x370_gunMotionSpeedMult = 1.f;
                        x374_ = 0.f;
                        if (x364_ <= 0.f && x368_ <= 0.f)
                        {
                            x368_ = 8.f;
                            x73c_gunMotion->PlayPasAnim(SamusGun::EAnimationState::Wander, mgr, 0.f, false);
                            x324_ = 1;
                            x550_camBob.SetState(CPlayerCameraBob::ECameraBobState::Walk, mgr);
                        }
                        x368_ -= dt;
                        x360_ -= dt;
                    }
                    if (!r19 || x834_26_)
                        ResetIdle(mgr);
                }
                if (x394_damageTimer > 0.f)
                {
                    x394_damageTimer -= dt;
                }
                else if (!x834_31_)
                {
                    x394_damageTimer = 0.f;
                    x834_31_ = true;
                    x73c_gunMotion->BasePosition(true);
                }
                else if (!x73c_gunMotion->GetModelData().GetAnimationData()->IsAnimTimeRemaining(0.001f, "Whole Body"))
                {
                    x834_30_inBigStrike = false;
                    x834_31_ = false;
                }
            }
            else
            {
                switch (x3a4_fidget.Update(x2ec_lastFireButtonStates, camBobT > 0.01f, b1, dt, mgr))
                {
                case CFidget::EState::Zero:
                    if (x324_ != 2)
                    {
                        x73c_gunMotion->PlayPasAnim(SamusGun::EAnimationState::Idle, mgr, 0.f, false);
                        x324_ = 2;
                    }
                    x550_camBob.SetState(CPlayerCameraBob::ECameraBobState::WalkNoBob, mgr);
                    break;
                case CFidget::EState::One:
                case CFidget::EState::Two:
                case CFidget::EState::Three:
                    if (x324_ != 0)
                    {
                        x73c_gunMotion->BasePosition(false);
                        x324_ = 0;
                    }
                    AsyncLoadFidget(mgr);
                    break;
                case CFidget::EState::Seven:
                    if (IsFidgetLoaded())
                        EnterFidget(mgr);
                    break;
                case CFidget::EState::Four:
                case CFidget::EState::Five:
                    x550_camBob.SetState(CPlayerCameraBob::ECameraBobState::Walk, mgr);
                    x833_24_isFidgeting = false;
                    x834_26_ = x834_25_ ? x73c_gunMotion->IsAnimPlaying() :
                    x72c_currentBeam->GetSolidModelData().GetAnimationData()->IsAnimTimeRemaining(0.001f, "Whole Body");
                    if (!x834_26_)
                    {
                        x3a4_fidget.ResetMinor();
                        ReturnToRestPose();
                    }
                    break;
                default:
                    break;
                }
            }
            x550_camBob.Update(dt, mgr);
        }
    }
}

static const float chargeShakeTbl[] = { -0.001f, 0.f, 0.001f };
static const CMaterialFilter sAimFilter =
    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {EMaterialTypes::ProjectilePassthrough});

void CPlayerGun::Update(float grappleSwingT, float cameraBobT, float dt, CStateManager& mgr)
{
    CPlayer& player = mgr.GetPlayer();
    CPlayerState& playerState = *mgr.GetPlayerState();
    bool isUnmorphed = player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed;

    bool becameFrozen;
    if (isUnmorphed)
        becameFrozen = !x834_29_frozen && player.GetFrozenState();
    else
        becameFrozen = false;

    bool becameThawed;
    if (isUnmorphed)
        becameThawed = x834_29_frozen && !player.GetFrozenState();
    else
        becameThawed = false;

    x834_29_frozen = isUnmorphed && player.GetFrozenState();
    float advDt;
    if (x834_29_frozen)
        advDt = 0.f;
    else
        advDt = dt;

    bool r23 = x678_morph.GetGunState() != CGunMorph::EGunState::One;
    if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::XRay || r23)
        x6e0_rightHandModel.AdvanceAnimation(advDt, mgr, kInvalidAreaId, true);
    if (r23 && x734_ != 0 && x734_ != x72c_currentBeam)
    {
        x744_auxWeapon->LoadIdle();
        x734_->Update(advDt, mgr);
    }
    if (!x744_auxWeapon->IsLoaded())
        x744_auxWeapon->LoadIdle();

    if (becameFrozen)
    {
        x72c_currentBeam->EnableSecondaryFx(CGunWeapon::ESecondaryFxType::Zero);
        x72c_currentBeam->BuildSecondaryEffect(CGunWeapon::ESecondaryFxType::One);
    }
    else if (becameThawed)
    {
        x72c_currentBeam->BuildSecondaryEffect(CGunWeapon::ESecondaryFxType::Two);
    }

    if (becameFrozen || becameThawed)
    {
        x2f4_fireButtonStates = 0;
        x2ec_lastFireButtonStates = 0;
        CancelFiring(mgr);
    }

    x72c_currentBeam->Update(advDt, mgr);
    x73c_gunMotion->Update(advDt * x370_gunMotionSpeedMult, mgr);
    x740_grappleArm->Update(grappleSwingT, advDt, mgr);

    if (x338_ != 0)
    {
        if (x678_morph.GetGunState() == CGunMorph::EGunState::Zero)
        {
            if (x338_ == 5)
            {
                ChangeWeapon(playerState, mgr);
                x338_ = 0;
            }
        }
        else if (!x72c_currentBeam->GetSolidModelData().GetAnimationData()->IsAnimTimeRemaining(0.001f, "Whole Body") ||
                 x832_30_)
        {
            bool r24 = true;
            switch (x338_)
            {
            case 1:
                x2f8_ &= 0x1;
                x2f8_ |= 0x6;
                x318_ = 1;
                x31c_missileMode = EMissleMode::Active;
                break;
            case 2:
                if ((x2f8_ & 0x8) != 0x8)
                {
                    x2f8_ |= 0x1;
                    x2f8_ &= 0xFFE9;
                }
                x318_ = 0;
                x31c_missileMode = EMissleMode::Inactive;
                x390_cooldown = x72c_currentBeam->GetWeaponInfo().x0_coolDown;
                break;
            case 4:
                PlayAnim(NWeaponTypes::EGunAnimType::MissileReload, false);
                x338_ = 3;
                r24 = false;
                break;
            case 3:
                x2f8_ |= 0x4;
                break;
            case 5:
                ChangeWeapon(playerState, mgr);
                break;
            case 6:
                x390_cooldown = x72c_currentBeam->GetWeaponInfo().x0_coolDown;
                x2f8_ &= ~0x8;
                if ((x2f8_ & 0x8) != 0x8)
                {
                    x2f8_ |= 0x1;
                    x2f8_ &= 0xFFE9;
                }
                x318_ = 0;
                x31c_missileMode = EMissleMode::Inactive;
                break;
            case 8:
                if (x75c_phazonBeam->IsLoaded())
                    break;
                x72c_currentBeam->SetLeavingBeam();
                x75c_phazonBeam->Load(mgr, false);
                x33c_gunOverrideMode = EGunOverrideMode::One;
                break;
            case 9:
                if (x738_nextBeam->IsLoaded())
                    break;
                x72c_currentBeam->SetLeavingBeam();
                x738_nextBeam->Load(mgr, false);
                x33c_gunOverrideMode = EGunOverrideMode::Two;
                break;
            default:
                break;
            }

            if (r24)
                x338_ = 0;
        }
    }

    if (x37c_ < 0.2f)
    {
        x37c_ += advDt;
    }
    else
    {
        x37c_ = 0.f;
        if (x30c_ > 0)
            x30c_ -= 1;
    }

    if (x32c_ != 0 && !player.GetFrozenState())
    {
        x34c_shakeX = chargeShakeTbl[mgr.GetActiveRandom()->Next() % 3] * x340_chargeBeamFactor;
        x350_shakeZ = chargeShakeTbl[mgr.GetActiveRandom()->Next() % 3] * x340_chargeBeamFactor;
    }

    if (!x72c_currentBeam->IsLoaded())
        return;

    GetLctrWithShake(x4d8_gunLocalXf, x73c_gunMotion->GetModelData(), "GBSE_SDK", true, true);
    GetLctrWithShake(x418_beamLocalXf, x72c_currentBeam->GetSolidModelData(), "LBEAM", false, true);
    GetLctrWithShake(x508_elbowLocalXf, x72c_currentBeam->GetSolidModelData(), "elbow", false, false);
    x4a8_gunWorldXf = x3e8_xf * x4d8_gunLocalXf * x550_camBob.GetCameraBobTransformation();

    if (x740_grappleArm->GetActive() && !x740_grappleArm->IsGrappling())
        UpdateLeftArmTransform(x72c_currentBeam->GetSolidModelData(), mgr);

    x6a0_motionState.Update(x2f0_pressedFireButtonStates != 0 && x832_28_ && x32c_ < 2 &&
                                !player.IsInFreeLook(), advDt, x4a8_gunWorldXf, mgr);

    x72c_currentBeam->GetSolidModelData().AdvanceParticles(x4a8_gunWorldXf, advDt, mgr);
    x72c_currentBeam->UpdateGunFx(x380_ > 2.f && x378_ > 0.15f, dt, mgr, x508_elbowLocalXf);

    zeus::CTransform beamWorldXf = x4a8_gunWorldXf * x418_beamLocalXf;

    if (player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed &&
        !mgr.GetCameraManager()->IsInCinematicCamera())
    {
        rstl::reserved_vector<TUniqueId, 1024> nearList;
        zeus::CAABox aabb = x72c_currentBeam->GetBounds().getTransformedAABox(x4a8_gunWorldXf);
        mgr.BuildNearList(nearList, aabb, sAimFilter, &player);
        TUniqueId bestId = kInvalidUniqueId;
        zeus::CVector3f dir = x4a8_gunWorldXf.basis[1].normalized();
        zeus::CVector3f pos = dir * -0.5f + x4a8_gunWorldXf.origin;
        CRayCastResult result = mgr.RayWorldIntersection(bestId, pos, dir, 3.5f, sAimFilter, nearList);
        x833_29_pointBlankWorldSurface = result.IsValid();
        if (result.IsValid())
        {
            x448_elbowWorldXf = x4a8_gunWorldXf * x508_elbowLocalXf;
            x448_elbowWorldXf.origin += dir * -0.5f;
            beamWorldXf.origin = result.GetPoint();
        }
    }
    else
    {
        x833_29_pointBlankWorldSurface = false;
    }

    zeus::CTransform beamTargetXf = x833_29_pointBlankWorldSurface ? x448_elbowWorldXf : beamWorldXf;

    zeus::CVector3f camTrans = mgr.GetCameraManager()->GetGlobalCameraTranslation(mgr);
    beamWorldXf.origin += camTrans;
    beamTargetXf.origin += camTrans;

    if (x832_25_)
    {
        bool emitting = x833_30_ ? x344_ < 1.f : false;
        zeus::CVector3f scale((emitting && x832_26_) ? (1.f - x344_) * 2.f : 2.f);
        x72c_currentBeam->UpdateMuzzleFx(advDt, scale, x418_beamLocalXf.origin, emitting);
        CElementGen& gen = *x800_auxMuzzleGenerators[x320_];
        gen.SetGlobalOrientAndTrans(x418_beamLocalXf);
        gen.SetGlobalScale(scale);
        gen.SetParticleEmission(emitting);
        gen.Update(advDt);
    }

    if (x748_rainSplashGenerator)
        x748_rainSplashGenerator->Update(advDt, mgr);

    UpdateGunLight(beamWorldXf, mgr);
    ProcessGunMorph(advDt, mgr);
    if (x835_26_phazonBeamMorphing)
        ProcessPhazonGunMorph(advDt, mgr);

    if (x832_26_ && x77c_)
    {
        x77c_->SetGlobalTranslation(x418_beamLocalXf.origin);
        x77c_->SetGlobalOrientation(x418_beamLocalXf.getRotation());
        x77c_->Update(advDt);
        x344_ += advDt * 4.f;
    }

    if (x35c_bombTime > 0.f)
    {
        x35c_bombTime -= advDt;
        if (x35c_bombTime <= 0.f)
            x308_bombCount = 3;
    }

    if (playerState.ItemEnabled(CPlayerState::EItemType::ChargeBeam) && x32c_ != 0)
    {
        UpdateChargeState(advDt, mgr);
    }
    else
    {
        x340_chargeBeamFactor -= advDt;
        if (x340_chargeBeamFactor < 0.f)
            x340_chargeBeamFactor = 0.f;
    }

    UpdateAuxWeapons(advDt, beamTargetXf, mgr);
    DoUserAnimEvents(advDt, mgr);

    if (x304_ == 1 && GetTargetId(mgr) != kInvalidUniqueId)
    {
        if (!x832_29_ && !x832_26_ && (x2f8_ & 0x10) != 0x10)
        {
            x832_29_ = true;
            x6a0_motionState.SetState(CMotionState::EMotionState::Two);
            ReturnArmAndGunToDefault(mgr, true);
        }
    }
    else
    {
        CancelLockOn();
    }

    UpdateWeaponFire(advDt, playerState, mgr);
    UpdateGunIdle(x364_ > 0.f, cameraBobT, advDt, mgr);

    if ((x2ec_lastFireButtonStates & 0x1) == 0x1)
    {
        x378_ = 0.f;
    }
    else if (x378_ < 2.f)
    {
        x378_ += advDt;
        if (x378_ > 1.f)
        {
            x30c_ = 0;
            x380_ = 0.f;
        }
    }

    if (x38c_ > 0.f)
        x38c_ -= advDt;

    if (x30c_ > 5 && x380_ < 2.f)
        x380_ += advDt;

    if (x384_ > 0.f)
        x384_ -= advDt;

    if (x364_ > 0.f)
    {
        x2f4_fireButtonStates = 0;
        x364_ -= advDt;
    }

    if (isUnmorphed && (x2f8_ & 0x4) == 0x4)
    {
        x3a0_ -= advDt;
        if (x3a0_ < 0.f)
        {
            x3a0_ = 0.f;
            ExitMissile();
        }
    }
}

void CPlayerGun::PreRender(const CStateManager& mgr, const zeus::CFrustum& frustum, const zeus::CVector3f& camPos)
{

}

void CPlayerGun::Render(const CStateManager& mgr, const zeus::CVector3f& pos, const CModelFlags& flags) const
{

}

void CPlayerGun::AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const
{

}

void CPlayerGun::DropBomb(EBWeapon weapon, CStateManager& mgr)
{

}

TUniqueId CPlayerGun::DropPowerBomb(CStateManager& mgr)
{
    return {};
}

}
