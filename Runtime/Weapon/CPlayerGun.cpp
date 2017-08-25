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

void CPlayerGun::TakeDamage(bool attack, bool notFromMetroid, CStateManager& mgr)
{
    bool hasAngle = false;
    float angle = 0.f;
    if (x398_damageAmt >= 10.f && !attack && (x2f8_ & 0x10) != 0x10 && !x832_26_ && x384_ <= 0.f)
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

    if (hasAngle || attack)
    {
        if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Scan)
        {
            x73c_gunMotion->PlayPasAnim(SamusGun::EAnimationState::Two, mgr, angle, attack);
            if ((attack && notFromMetroid) || x833_31_)
                x740_grappleArm->EnterStruck(mgr, angle, attack, !x833_31_);
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
        bool attackDamage = false;
        bool metroidAttached = false;
        if (TCastToConstPtr<CEnergyProjectile> proj = mgr.GetObjectById(sender))
        {
            if ((proj->GetAttribField() & CGameProjectile::EProjectileAttrib::Twelve) !=
                CGameProjectile::EProjectileAttrib::None)
            {
                x394_damageTimer = proj->GetDamageDuration();
                attackDamage = true;
            }
        }
        else if (TCastToConstPtr<CPatterned> ai = mgr.GetObjectById(sender))
        {
            if (ai->GetX402_28())
            {
                x394_damageTimer = ai->GetDamageDuration();
                attackDamage = true;
                if (player.GetAttachedActor() != kInvalidUniqueId)
                    metroidAttached = CPatterned::CastTo<MP1::CMetroid>(
                        mgr.GetObjectById(player.GetAttachedActor())) != nullptr;
            }
        }
        if (!x834_30_damaged)
        {
            if (attackDamage)
            {
                x834_31_ = false;
                CancelFiring(mgr);
            }
            TakeDamage(attackDamage, !metroidAttached, mgr);
            x834_30_damaged = attackDamage;
        }
        break;
    }
    case EScriptObjectMessage::OnFloor:
        if (player.GetControlsFrozen() && !x834_30_damaged)
        {
            x2f4_ = 0;
            x2ec_firing = 0;
            CancelFiring(mgr);
            TakeDamage(true, false, mgr);
            x394_damageTimer = 0.75f;
            x834_30_damaged = true;
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

void CPlayerGun::ResetCharge(CStateManager& mgr, bool b1)
{

}

void CPlayerGun::HandleBeamChange(const CFinalInput& input, CStateManager& mgr)
{

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
    x2ec_firing &= ~0x1;
    x320_ = x310_selectedBeam;
    x833_30_ = true;
}

void CPlayerGun::PlaySfx(u16 sfx, bool underwater, bool looped, float pan)
{
    CSfxHandle hnd = CSfxManager::SfxStart(sfx, 1.f, pan, true, 0x7f, looped, kInvalidAreaId);
    CSfxManager::SfxSpan(hnd, 0.f);
    if (underwater)
        CSfxManager::PitchBend(hnd, -1.f);
}

static const u16 skFromMissileSound[] = { 1824, 1849, 1851, 1853 };
static const u16 skFromBeamSound[] = { 0, 1822, 1828, 1826 };
static const u16 skToMissileSound[] = { 1823, 1829, 1850, 1852 };

void CPlayerGun::PlayAnim(NWeaponTypes::EGunAnimType type, bool b1)
{
    if (x338_ != 5)
        x72c_currentBeam->PlayAnim(type, b1);

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
    bool damageNotMorphed = (x834_30_damaged &&
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
        x2f4_ = 0;
        break;
    case CPlayer::EPlayerMorphBallState::Unmorphed:
        if ((x2f8_ & 0x10) != 0x10)
            HandleWeaponChange(input, mgr);
    case CPlayer::EPlayerMorphBallState::Morphed:
        x2f4_ = ControlMapper::GetDigitalInput(ControlMapper::ECommands::FireOrBomb, input) ? 1 : 0;
        x2f4_ |= ControlMapper::GetDigitalInput(ControlMapper::ECommands::MissileOrPowerBomb, input) ? 2 : 0;
        break;
    }
}

void CPlayerGun::ResetIdle(CStateManager& mgr)
{

}

void CPlayerGun::CancelFiring(CStateManager& mgr)
{

}

float CPlayerGun::GetBeamVelocity() const
{
    if (x72c_currentBeam->IsLoaded())
        return x72c_currentBeam->GetVelocityInfo().GetVelocity(x330_chargeWeaponIdx).y;
    return 10.f;
}

void CPlayerGun::StopContinuousBeam(CStateManager& mgr, bool b1)
{

}

void CPlayerGun::Update(float grappleSwingT, float cameraBobT, float dt, CStateManager& mgr)
{

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

TUniqueId CPlayerGun::DropPowerBomb(CStateManager& mgr)
{
    return {};
}

}
