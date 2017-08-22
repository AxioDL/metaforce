#include "CPlayerGun.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "Character/CPrimitive.hpp"

namespace urde
{

static const zeus::CVector3f sGunScale(2.f);

static float kVerticalAngleTable[] = { -30.f, 0.f, 30.f };
static float kHorizontalAngleTable[] = { 30.f, 30.f, 30.f };
static float kVerticalVarianceTable[] = { 30.f, 30.f, 30.f };

float CPlayerGun::CMotionState::gGunExtendDistance = 0.125f;

CPlayerGun::CPlayerGun(TUniqueId playerId)
: x0_lights(8, zeus::CVector3f{-30.f, 0.f, 30.f}, 4, 4, 0, 0, 0, 0.1f), x538_thisId(playerId),
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

void CPlayerGun::AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&)
{

}

void CPlayerGun::AsyncLoadSuit(CStateManager& mgr)
{
    x72c_currentBeam->AsyncLoadSuitArm(mgr);
    x740_grappleArm->AsyncLoadSuit(mgr);
}

void CPlayerGun::TouchModel(const CStateManager& stateMgr)
{

}

void CPlayerGun::DamageRumble(const zeus::CVector3f& location, float damage, const CStateManager& mgr)
{
    x398_damageAmt = damage;
    x3dc_damageLocation = location;
}

void CPlayerGun::ProcessInput(const CFinalInput& input, CStateManager& mgr)
{

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
