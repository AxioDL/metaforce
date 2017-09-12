#include "GameGlobalObjects.hpp"
#include "CDependencyGroup.hpp"
#include "CMorphBall.hpp"
#include "CPlayer.hpp"
#include "CSimplePool.hpp"
#include "CGameLight.hpp"
#include "World/CWorld.hpp"
#include "World/CScriptAreaAttributes.hpp"
#include "TCastTo.hpp"

namespace urde
{

static float kSpiderBallCollisionRadius;

const u8 CMorphBall::BallTransFlashColors[9][3] =
{
    {0xc2, 0x7e, 0x10},
    {0x66, 0xc4, 0xff},
    {0x60, 0xff, 0x90},
    {0x33, 0x33, 0xff},
    {0xff, 0x20, 0x20},
    {0x0, 0x9d, 0xb6},
    {0xd3, 0xf1, 0x0},
    {0xa6, 0x86, 0xd8},
    {0xfb, 0x98, 0x21}
};

CMorphBall::CMorphBall(CPlayer& player, float radius)
: x0_player(player), xc_radius(radius),
  x38_collisionSphere({{0.f, 0.f, radius}, radius},
                      {EMaterialTypes::Player, EMaterialTypes::Solid, EMaterialTypes::GroundCollider}),
  x58_ballModel(GetMorphBallModel("SamusBallANCS", radius)),
  x60_spiderBallGlassModel(GetMorphBallModel("SamusSpiderBallGlassCMDL", radius)),
  x68_lowPolyBallModel(GetMorphBallModel("SamusBallLowPolyCMDL", radius)),
  x70_frozenBallModel(GetMorphBallModel("SamusBallFrozenCMDL", radius)),
  x1968_slowBlueTailSwoosh(g_SimplePool->GetObj("SlowBlueTailSwoosh")),
  x1970_slowBlueTailSwoosh2(g_SimplePool->GetObj("SlowBlueTailSwoosh2")),
  x1978_jaggyTrail(g_SimplePool->GetObj("JaggyTrail")),
  x1980_wallSpark(g_SimplePool->GetObj("WallSpark")),
  x1988_ballInnerGlow(g_SimplePool->GetObj("BallInnerGlow")),
  x1990_spiderBallMagnetEffect(g_SimplePool->GetObj("SpiderBallMagnetEffect")),
  x1998_boostBallGlow(g_SimplePool->GetObj("BoostBallGlow")),
  x19a0_spiderElectric(g_SimplePool->GetObj("SpiderElectric")),
  x19a8_morphBallTransitionFlash(g_SimplePool->GetObj("MorphBallTransitionFlash")),
  x19b0_effect_morphBallIceBreak(g_SimplePool->GetObj("Effect_MorphBallIceBreak"))
{
    x19b8_slowBlueTailSwooshGen = std::make_unique<CParticleSwoosh>(x1968_slowBlueTailSwoosh, 0);
    x19bc_slowBlueTailSwooshGen2 = std::make_unique<CParticleSwoosh>(x1968_slowBlueTailSwoosh, 0);
    x19c0_slowBlueTailSwoosh2Gen = std::make_unique<CParticleSwoosh>(x1970_slowBlueTailSwoosh2, 0);
    x19c4_slowBlueTailSwoosh2Gen2 = std::make_unique<CParticleSwoosh>(x1970_slowBlueTailSwoosh2, 0);
    x19c8_jaggyTrailGen = std::make_unique<CParticleSwoosh>(x1978_jaggyTrail, 0);
    x19cc_wallSparkGen = std::make_unique<CElementGen>(x1980_wallSpark);
    x19d0_ballInnerGlowGen = std::make_unique<CElementGen>(x1988_ballInnerGlow);
    x19d4_spiderBallMagnetEffectGen = std::make_unique<CElementGen>(x1990_spiderBallMagnetEffect);
    x19d8_boostBallGlowGen = std::make_unique<CElementGen>(x1998_boostBallGlow);
    x1c14_worldShadow = std::make_unique<CWorldShadow>(16, 16, false);
    x1c18_actorLights = std::make_unique<CActorLights>(8, zeus::CVector3f::skZero, 4, 4, false, false, false, 0.1f);
    x1c1c_rainSplashGen = std::make_unique<CRainSplashGenerator>(x58_ballModel->GetScale(), 40, 2, 0.15f, 0.5f);
    x1de4_24 = false;
    x1de4_25 = true;
    x1df8_24_inHalfPipeMode = false;
    x1df8_25_inHalfPipeModeInAir = false;
    x1df8_26_touchedHalfPipeRecently = false;
    x1df8_27 = false;

    x19d4_spiderBallMagnetEffectGen->SetParticleEmission(false);
    x19d4_spiderBallMagnetEffectGen->Update(1.0 / 60.0);

    kSpiderBallCollisionRadius = GetBallRadius() + 0.2f;

    for (int i=0 ; i<32 ; ++i)
        x19e4_spiderElectricGens.emplace_back(std::make_unique<CParticleSwoosh>(x19a0_spiderElectric, 0), false);

    LoadAnimationTokens("SamusBallANCS");
    InitializeWakeEffects();
}

void CMorphBall::LoadAnimationTokens(const std::string& ancsName)
{
    TToken<CDependencyGroup> dgrp = g_SimplePool->GetObj((ancsName + "_DGRP").c_str());
    x1958_animationTokens.clear();
    x1958_animationTokens.reserve(dgrp->GetObjectTagVector().size());
    for (const SObjectTag& tag : dgrp->GetObjectTagVector())
    {
        if (tag.type == FOURCC('CMDL') || tag.type == FOURCC('CSKR') || tag.type == FOURCC('TXTR'))
            continue;
        x1958_animationTokens.push_back(g_SimplePool->GetObj(tag));
        x1958_animationTokens.back().Lock();
    }
}

/* Maps material index to effect in generator array */
static const s32 skWakeEffectMap[32] =
{
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    0, // Phazon
    2, // Dirt
    3, // Lava
    -1,
    4, // Snow
    5, // MudSlow
    -1,
    -1,
    -1,
    -1,
    6, // Sand
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1
};

void CMorphBall::InitializeWakeEffects()
{
    TToken<CGenDescription> nullParticle =
        CToken(TObjOwnerDerivedFromIObj<CGenDescription>::GetNewDerivedObject(std::make_unique<CGenDescription>()));
    for (int i=0 ; i<8 ; ++i)
        x1b84_wakeEffects.push_back(nullParticle);

    x1b84_wakeEffects[2] = g_SimplePool->GetObj("DirtWake");
    x1b84_wakeEffects[0] = g_SimplePool->GetObj("PhazonWake");
    x1b84_wakeEffects[1] = g_SimplePool->GetObj("PhazonWakeOrange");
    x1b84_wakeEffects[3] = g_SimplePool->GetObj("LavaWake");
    x1b84_wakeEffects[4] = g_SimplePool->GetObj("SnowWake");
    x1b84_wakeEffects[5] = g_SimplePool->GetObj("MudWake");
    x1b84_wakeEffects[6] = g_SimplePool->GetObj("SandWake");
    x1b84_wakeEffects[7] = g_SimplePool->GetObj("RainWake");

    x1bc8_wakeEffectGens.resize(8);
    x1bc8_wakeEffectGens[2] = std::make_unique<CElementGen>(x1b84_wakeEffects[2]);
    x1bc8_wakeEffectGens[0] = std::make_unique<CElementGen>(x1b84_wakeEffects[0]);
    x1bc8_wakeEffectGens[1] = std::make_unique<CElementGen>(x1b84_wakeEffects[1]);
    x1bc8_wakeEffectGens[3] = std::make_unique<CElementGen>(x1b84_wakeEffects[3]);
    x1bc8_wakeEffectGens[4] = std::make_unique<CElementGen>(x1b84_wakeEffects[4]);
    x1bc8_wakeEffectGens[5] = std::make_unique<CElementGen>(x1b84_wakeEffects[5]);
    x1bc8_wakeEffectGens[6] = std::make_unique<CElementGen>(x1b84_wakeEffects[6]);
    x1bc8_wakeEffectGens[7] = std::make_unique<CElementGen>(x1b84_wakeEffects[7]);
}

std::unique_ptr<CModelData> CMorphBall::GetMorphBallModel(const char* name, float radius)
{
    const SObjectTag* tag = g_ResFactory->GetResourceIdByName(name);
    if (tag->type == FOURCC('CMDL'))
        return std::make_unique<CModelData>(CStaticRes(tag->id, zeus::CVector3f(radius * 2.f)));
    else
        return std::make_unique<CModelData>(CAnimRes(tag->id, 0, zeus::CVector3f(radius * 2.f), 0, false));
}

void CMorphBall::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr)
{
    switch (msg)
    {
    case EScriptObjectMessage::Registered:
        if (x19d0_ballInnerGlowGen && x19d0_ballInnerGlowGen->SystemHasLight())
        {
            x1c10_ballInnerGlowLight = mgr.AllocateUniqueId();
            CGameLight* l = new CGameLight(x1c10_ballInnerGlowLight, kInvalidAreaId, false, "BallLight",
                                           GetBallToWorld(), x0_player.GetUniqueId(),
                                           x19d0_ballInnerGlowGen->GetLight(),
                                           u32(reinterpret_cast<uintptr_t>(x1988_ballInnerGlow.GetObj())), 0, 0.f);
            mgr.AddObject(l);
        }
        break;
    case EScriptObjectMessage::Deleted:
        DeleteLight(mgr);
        break;
    default:
        break;
    }
}

void CMorphBall::DrawBallShadow(const CStateManager& mgr)
{
    if (!x1e50_shadow)
        return;

    float alpha = 1.f;
    switch (x0_player.x2f8_morphBallState)
    {
    case CPlayer::EPlayerMorphBallState::Unmorphed:
        return;
    case CPlayer::EPlayerMorphBallState::Unmorphing:
        alpha = 0.f;
        if (x0_player.x578_morphDuration != 0.f)
            alpha = zeus::clamp(0.f, x0_player.x574_morphTime / x0_player.x578_morphDuration, 1.f);
        alpha = 1.f - alpha;
        break;
    case CPlayer::EPlayerMorphBallState::Morphing:
        alpha = 0.f;
        if (x0_player.x578_morphDuration != 0.f)
            alpha = zeus::clamp(0.f, x0_player.x574_morphTime / x0_player.x578_morphDuration, 1.f);
        break;
    default: break;
    }
    x1e50_shadow->Render(mgr, alpha);
}

void CMorphBall::DeleteBallShadow()
{
    x1e50_shadow.reset();
}

void CMorphBall::CreateBallShadow()
{
    x1e50_shadow = std::make_unique<CMorphBallShadow>();
}

void CMorphBall::RenderToShadowTex(CStateManager& mgr)
{
    if (x1e50_shadow)
    {
        zeus::CVector3f center =
            x0_player.GetPrimitiveOffset() + x0_player.GetTranslation() + zeus::CVector3f(0.f, 0.f, xc_radius);
        zeus::CAABox aabb(center - zeus::CVector3f(1.25f * xc_radius, 1.25f * xc_radius, 10.f),
                          center + zeus::CVector3f(1.25f * xc_radius, 1.25f * xc_radius, xc_radius));
        x1e50_shadow->RenderIdBuffer(aabb, mgr, x0_player);
    }
}

static const u16 skBallRollSfx[] =
{
    0xFFFF, 0x05DE, 0x05DD, 0x062F, 0x0786,
    0xFFFF, 0x05DC, 0x060B, 0x05C8, 0x088A,
    0x0698, 0x0787, 0x0630, 0xFFFF, 0x0628,
    0x05DD, 0x05DD, 0x05C8, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0x05FE, 0x0628
};

static const u16 skBallLandSfx[] =
{
    0xFFFF, 0x05C3, 0x05E0, 0x062C, 0x065B,
    0xFFFF, 0x05DA, 0x0609, 0x05C0, 0x0697,
    0x0697, 0x065C, 0x062D, 0xFFFF, 0x0627,
    0x05E0, 0x05E0, 0x05C0, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0x05FD, 0x0627
};

void CMorphBall::SelectMorphBallSounds(const CMaterialList& mat)
{
    u16 rollSfx;
    if (x0_player.x9c5_30_selectFluidBallSound)
    {
        if (x0_player.x82c_inLava)
            rollSfx = 2186;
        else
            rollSfx = 1481;
    }
    else
    {
        rollSfx = CPlayer::SfxIdFromMaterial(mat, skBallRollSfx, 24, 0xffff);
    }
    x0_player.x9c5_30_selectFluidBallSound = false;

    if (rollSfx != 0xffff)
    {
        if (x1e34_rollSfx != rollSfx && x1e2c_rollSfxHandle)
        {
            CSfxManager::SfxStop(x1e2c_rollSfxHandle);
            x1e2c_rollSfxHandle.reset();
        }
        x1e34_rollSfx = rollSfx;
    }

    x1e36_landSfx = CPlayer::SfxIdFromMaterial(mat, skBallLandSfx, 24, 0xffff);
}

void CMorphBall::UpdateMorphBallSounds(float dt)
{
    zeus::CVector3f velocity = x0_player.GetVelocity();
    if (x187c_spiderBallState != ESpiderBallState::Active)
        velocity.z = 0.f;

    switch (x0_player.GetPlayerMovementState())
    {
    case CPlayer::EPlayerMovementState::OnGround:
    case CPlayer::EPlayerMovementState::FallingMorphed:
    {
        float vel = velocity.magnitude();
        if (x187c_spiderBallState == ESpiderBallState::Active)
            vel += g_tweakBall->GetBallGravity() * dt * 4.f;
        if (vel > 0.8f)
        {
            if (!x1e2c_rollSfxHandle)
            {
                if (x1e34_rollSfx != 0xffff)
                {
                    x1e2c_rollSfxHandle =
                        CSfxManager::AddEmitter(x1e34_rollSfx, x0_player.GetTranslation(), zeus::CVector3f::skZero,
                                                true, true, 0x7f, kInvalidAreaId);
                }
                x0_player.ApplySubmergedPitchBend(x1e2c_rollSfxHandle);
            }
            CSfxManager::PitchBend(x1e2c_rollSfxHandle, zeus::clamp(-1.f, vel * 0.122f - 0.831f, 1.f));
            float maxVol = zeus::clamp(0.f, 0.025f * vel + 0.5f, 1.f);
            CSfxManager::UpdateEmitter(x1e2c_rollSfxHandle, x0_player.GetTranslation(),
                                       zeus::CVector3f::skZero, maxVol);
            break;
        }
    }
    default:
        if (x1e2c_rollSfxHandle)
        {
            CSfxManager::SfxStop(x1e2c_rollSfxHandle);
            x1e2c_rollSfxHandle.reset();
        }
        break;
    }

    if (x187c_spiderBallState == ESpiderBallState::Active)
    {
        if (!x1e30_spiderSfxHandle)
        {
            x1e30_spiderSfxHandle =
                CSfxManager::AddEmitter(1523, x0_player.GetTranslation(), zeus::CVector3f::skZero,
                                        true, true, 0xc8, kInvalidAreaId);
            x0_player.ApplySubmergedPitchBend(x1e30_spiderSfxHandle);
        }
        CSfxManager::UpdateEmitter(x1e30_spiderSfxHandle, x0_player.GetTranslation(),
                                   zeus::CVector3f::skZero, 1.f);
    }
    else if (x1e30_spiderSfxHandle)
    {
        CSfxManager::SfxStop(x1e30_spiderSfxHandle);
        x1e30_spiderSfxHandle.reset();
    }
}

float CMorphBall::GetBallRadius() const
{
    return g_tweakPlayer->GetPlayerBallHalfExtent();
}

float CMorphBall::GetBallTouchRadius() const
{
    return g_tweakBall->GetBallTouchRadius();
}

void CMorphBall::ForwardInput(const CFinalInput&) const
{

}

void CMorphBall::BallTurnInput(const CFinalInput&) const
{

}

void CMorphBall::ComputeBallMovement(const CFinalInput&, CStateManager&, float)
{

}

bool CMorphBall::IsMovementAllowed() const
{
    return false;
}

void CMorphBall::UpdateSpiderBall(const CFinalInput&, CStateManager&, float)
{

}

void CMorphBall::ApplySpiderBallSwingingForces(const CFinalInput&, CStateManager&, float)
{

}

void CMorphBall::ApplySpiderBallRollForces(const CFinalInput&, CStateManager&, float)
{

}

void CMorphBall::CalculateSpiderBallAttractionSurfaceForces(const CFinalInput&, CStateManager&,
                                                            const zeus::CTransform&)
{

}

void CMorphBall::CheckForSwitchToSpiderBallSwinging(CStateManager&)
{

}

void CMorphBall::FindClosestSpiderBallWaypoint(CStateManager&, const zeus::CVector3f&, zeus::CVector3f&,
                                               zeus::CVector3f&, zeus::CVector3f&, float&, zeus::CVector3f&, bool&,
                                               zeus::CTransform&) const
{

}

void CMorphBall::SetSpiderBallSwingingState(bool)
{

}

void CMorphBall::GetSpiderBallControllerMovement(const CFinalInput&, bool, bool)
{

}

void CMorphBall::ResetSpiderBallSwingControllerMovementTimer()
{

}

void CMorphBall::UpdateSpiderBallSwingControllerMovementTimer(float, float)
{

}

float CMorphBall::GetSpiderBallSwingControllerMovementScalar() const
{
    return 0.f;
}

void CMorphBall::CreateSpiderBallParticles(const zeus::CVector3f&, const zeus::CVector3f&)
{

}

void CMorphBall::ComputeMarioMovement(const CFinalInput&, CStateManager&, float)
{

}

void CMorphBall::GetSwooshToWorld() const
{

}

zeus::CTransform CMorphBall::GetBallToWorld() const
{
    return zeus::CTransform::Translate(x0_player.GetTranslation() + zeus::CVector3f(0.f, 0.f, GetBallRadius())) *
        x0_player.GetTransform().getRotation();
}

void CMorphBall::CalculateSurfaceToWorld(const zeus::CVector3f&, const zeus::CVector3f&, const zeus::CVector3f&) const
{

}

void CMorphBall::CalculateBallContactInfo(zeus::CVector3f&, zeus::CVector3f&) const
{

}

void CMorphBall::UpdateBallDynamics(CStateManager&, float)
{

}

void CMorphBall::SwitchToMarble()
{

}

void CMorphBall::SwitchToTire()
{

}

void CMorphBall::Update(float dt, CStateManager& mgr)
{
    if (x187c_spiderBallState == ESpiderBallState::Active)
        CreateSpiderBallParticles(GetBallToWorld().origin, x1890_spiderTrackPoint);

    if (x0_player.GetDeathTime() <= 0.f)
        UpdateEffects(dt, mgr);

    if (x1e44_ > 0.f)
    {
        x1e44_ -= x1e48_ * dt;
        if (x1e44_ <= 0.f)
        {
            x1e44_ = 0.f;
            x1e48_ = 0.f;
            x1e4c_ = 0.f;
        }
        else
        {
            x1e4c_ += dt;
        }
    }

    if (x58_ballModel)
        x58_ballModel->AdvanceAnimation(dt, mgr, kInvalidAreaId, true);

    if (x1c2c_)
    {
        x1c20_ += x1c28_ * dt;
        if (x1c20_ < 0.f)
        {
            x1c2c_ = false;
            x1c20_ = 0.f;
        }
        else if (x1c20_ > x1c24_)
        {
            x1c2c_ = false;
            x1c20_ = x1c24_;
        }
    }

    if (x1c1c_rainSplashGen)
        x1c1c_rainSplashGen->Update(dt, mgr);

    UpdateMorphBallSounds(dt);
}

void CMorphBall::UpdateScriptMessage(EScriptObjectMessage, TUniqueId, CStateManager&)
{

}

void CMorphBall::DeleteLight(CStateManager&)
{

}

void CMorphBall::SetBallLightActive(CStateManager&, bool)
{

}

void CMorphBall::EnterMorphBallState(CStateManager&)
{

}

void CMorphBall::LeaveMorphBallState(CStateManager&)
{

}

void CMorphBall::UpdateEffects(float, CStateManager&)
{

}

void CMorphBall::ComputeBoostBallMovement(const CFinalInput&, const CStateManager&, float)
{

}

void CMorphBall::EnterBoosting()
{

}

void CMorphBall::LeaveBoosting()
{

}

void CMorphBall::CancelBoosting()
{

}

void CMorphBall::UpdateMarbleDynamics(CStateManager&, float, const zeus::CVector3f&)
{

}

void CMorphBall::ApplyFriction(float)
{

}

void CMorphBall::DampLinearAndAngularVelocities(float, float)
{

}

zeus::CTransform CMorphBall::GetPrimitiveTransform() const
{
    return {};
}

void CMorphBall::DrawCollisionPrimitive() const
{

}

void CMorphBall::GetMinimumAlignmentSpeed() const
{

}

void CMorphBall::PreRender(CStateManager&, const zeus::CFrustum&)
{

}

void CMorphBall::Render(const CStateManager&, const CActorLights*) const
{

}

void CMorphBall::ResetMorphBallTransitionFlash()
{

}

void CMorphBall::UpdateMorphBallTransitionFlash(float)
{

}

void CMorphBall::RenderMorphBallTransitionFlash(const CStateManager&) const
{
    if (x19dc_morphBallTransitionFlashGen)
    {
        const u8* c = BallTransFlashColors[x8_];
        zeus::CColor color = {c[0] / 255.f, c[1] / 255.f, c[2] / 255.f, 1.f};
        x19dc_morphBallTransitionFlashGen->SetModulationColor(color);
        x19dc_morphBallTransitionFlashGen->Render();
    }
}

void CMorphBall::UpdateIceBreakEffect(float dt)
{

}

void CMorphBall::RenderIceBreakEffect(const CStateManager& mgr) const
{
    if (x19e0_effect_morphBallIceBreakGen)
        x19e0_effect_morphBallIceBreakGen->Render();
}

void CMorphBall::RenderDamageEffects(const CStateManager&, const zeus::CTransform&) const
{

}

void CMorphBall::UpdateHalfPipeStatus(CStateManager&, float)
{

}

void CMorphBall::DisableHalfPipeStatus()
{
    SetIsInHalfPipeMode(false);
    SetIsInHalfPipeModeInAir(false);
    SetTouchedHalfPipeRecently(false);
    x1dfc_ = 0.f;
    x1e00_ = 0.f;
    x0_player.SetCollisionAccuracyModifier(1.f);
    x1e08_ = zeus::CVector3f::skZero;
    x1e14_ = zeus::CVector3f::skZero;
}

bool CMorphBall::BallCloseToCollision(const CStateManager&, float, const CMaterialFilter& filter) const
{
    return false;
}

void CMorphBall::CollidedWith(TUniqueId id, const CCollisionInfoList& list, CStateManager& mgr)
{
    x74_collisionInfos = list;
    CMaterialList allMats;
    for (const CCollisionInfo& info : list)
        allMats.Add(info.GetMaterialLeft());

    zeus::CVector3f vel = x0_player.GetVelocity();
    float velMag = vel.magnitude();
    EMaterialTypes wakeMaterial = EMaterialTypes::Unknown;
    if (velMag > 7.f && x0_player.GetFluidCounter() == 0)
    {
        bool hitWall = false;
        for (const CCollisionInfo& info : list)
        {
            if (!hitWall)
            {
                if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::Wall))
                {
                    hitWall = true;
                    if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::Stone) ||
                        info.GetMaterialLeft().HasMaterial(EMaterialTypes::Metal))
                    {
                        x19cc_wallSparkGen->SetTranslation(info.GetPoint());
                        x19cc_wallSparkGen->SetParticleEmission(true);
                        x1e38_ = 7;
                    }
                }
            }

            if (wakeMaterial == EMaterialTypes::Unknown)
            {
                if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::Floor))
                {
                    EMaterialTypes tmpMaterial;
                    if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::Dirt))
                        tmpMaterial = EMaterialTypes::Dirt;
                    else
                        tmpMaterial = wakeMaterial;

                    if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::Sand))
                        tmpMaterial = EMaterialTypes::Sand;

                    if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::Lava))
                        tmpMaterial = EMaterialTypes::Lava;

                    if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::MudSlow))
                        tmpMaterial = EMaterialTypes::MudSlow;

                    if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::Snow))
                        tmpMaterial = EMaterialTypes::Snow;

                    if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::Phazon))
                        tmpMaterial = EMaterialTypes::Phazon;

                    wakeMaterial = tmpMaterial;
                    if (tmpMaterial != EMaterialTypes::Unknown)
                    {
                        int mappedIdx = skWakeEffectMap[int(tmpMaterial)];
                        if (mappedIdx == 0) // Phazon
                        {
                            const CGameArea* area = mgr.GetWorld()->GetAreaAlways(mgr.GetNextAreaId());
                            if (const CScriptAreaAttributes* attribs =
                                area->GetPostConstructed()->x10d8_areaAttributes)
                                if (attribs->GetPhazonType() == EPhazonType::Orange)
                                    mappedIdx = 1; // Orange Phazon
                        }

                        if (x1c0c_wakeEffectIdx != mappedIdx)
                        {
                            if (x1c0c_wakeEffectIdx != -1)
                                x1bc8_wakeEffectGens[x1c0c_wakeEffectIdx]->SetParticleEmission(false);
                            x1c0c_wakeEffectIdx = mappedIdx;
                        }

                        x1bc8_wakeEffectGens[x1c0c_wakeEffectIdx]->SetParticleEmission(true);
                        x1bc8_wakeEffectGens[x1c0c_wakeEffectIdx]->SetTranslation(info.GetPoint());
                    }
                }
            }
        }

        if (hitWall && !CSfxManager::IsPlaying(x1e28_wallHitSfxHandle))
        {
            x1e28_wallHitSfxHandle = CSfxManager::AddEmitter(1525, x0_player.GetTranslation(),
                                                             zeus::CVector3f::skZero,
                                                             true, false, 0x7f, kInvalidAreaId);
            x0_player.ApplySubmergedPitchBend(x1e28_wallHitSfxHandle);
        }
    }

    if (wakeMaterial == EMaterialTypes::Unknown && x1c0c_wakeEffectIdx != -1)
        x1bcc_[int(wakeMaterial)]->SetParticleEmission(false);

    x1954_isProjectile = false;

    if (allMats.HasMaterial(EMaterialTypes::HalfPipe))
    {
        x1dfc_ = 4.f;
        x1e04_ = 0.05f;
        for (const CCollisionInfo& info : list)
        {
            if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::HalfPipe))
            {
                if (info.GetNormalLeft().dot(x1e14_) < 0.99)
                {
                    x1e08_ = x1e14_;
                    x1e14_ = info.GetNormalLeft();
                    if (zeus::close_enough(x1e08_, zeus::CVector3f::skZero, 0.000011920929f))
                        x1e08_ = x1e14_;
                }
            }
        }
    }

    if (x28_ && allMats.HasMaterial(EMaterialTypes::Floor) && allMats.HasMaterial(EMaterialTypes::Wall))
        SwitchToMarble();

    if (!GetIsInHalfPipeMode() && x1de4_24 && velMag > 3.f)
    {
        zeus::CVector3f velNorm = vel.normalized();
        for (const CCollisionInfo& info : list)
        {
            if (!info.GetMaterialLeft().HasMaterial(EMaterialTypes::HalfPipe) &&
                info.GetNormalLeft().dot(velNorm) < -0.4f)
            {
                LeaveBoosting();
                DampLinearAndAngularVelocities(0.4f, 0.01f);
                break;
            }
        }
    }

    if (id == kInvalidUniqueId)
    {
        zeus::CVector3f cvel = x0_player.GetVelocity();
        float cvelMag = cvel.magnitude();
        zeus::CVector3f pvel = x1c_;
        if (pvel.magnitude() > 1000.f && cvelMag > 8.f)
        {
            zeus::CVector3f pvelNorm = pvel.normalized();
            zeus::CVector3f cvelNorm = cvel.normalized();
            for (const CCollisionInfo& info : list)
            {
                if (IsClimbable(info) &&
                    info.GetNormalLeft().dot(pvelNorm) < -0.4f &&
                    info.GetNormalLeft().dot(cvelNorm) < -0.6f)
                {
                    float f2 = 0.75f * cvelMag;
                    float maxSpeed = g_tweakBall->GetBallTranslationMaxSpeed(int(x0_player.GetSurfaceRestraint()));
                    float f0 = maxSpeed * 0.15f;
                    float f3 = (f2 - f0) < 0.f ? f2 : f0;
                    float f4 = maxSpeed * 0.25f;
                    f4 = (f3 - f4) < 0.f ? f4 : f3;
                    zeus::CVector3f newVel = cvel + zeus::CVector3f(0.f, 0.f, f4);
                    x1dd8_ = newVel;
                    x0_player.SetVelocityWR(newVel);
                    x1dc8_ += 1;
                    break;
                }
            }
        }
    }

    if (list.GetCount() > 2 && list.GetItem(0).GetNormalLeft().z > 0.2f &&
        std::fabs(x0_player.GetVelocity().dot(list.GetItem(0).GetNormalLeft())) > 2.f)
    {
        float accum = 0.f;
        int count = 0;
        for (auto it = list.begin() + 1 ; it != list.end() ; ++it)
        {
            const CCollisionInfo& item1 = *it;
            for (auto it2 = list.begin() + 1 ; it2 != list.end() ; ++it2)
            {
                const CCollisionInfo& item2 = *it2;
                accum += item1.GetNormalLeft().dot(item2.GetNormalLeft());
                count += 1;
            }
        }

        if (accum / float(count) < 0.5f)
            x1dc8_ += 1;
    }

    if (list.GetCount() != 0)
        SelectMorphBallSounds(list.GetItem(0).GetMaterialLeft());
}

bool CMorphBall::IsInFrustum(const zeus::CFrustum& frustum) const
{
    if (x58_ballModel->IsNull())
        return false;

    if (x58_ballModel->IsInFrustum(GetBallToWorld(), frustum))
        return true;

    auto swooshBounds = x19b8_slowBlueTailSwooshGen->GetBounds();
    return x19b8_slowBlueTailSwooshGen->GetModulationColor().a != 0.f && swooshBounds &&
           frustum.aabbFrustumTest(*swooshBounds);
}

void CMorphBall::ComputeLiftForces(const zeus::CVector3f&, const zeus::CVector3f&, const CStateManager&)
{

}

float CMorphBall::CalculateSurfaceFriction() const
{
    float friction = g_tweakBall->GetBallTranslationFriction(int(x0_player.GetSurfaceRestraint()));
    if (x0_player.GetAttachedActor() != kInvalidUniqueId)
        friction *= 2.f;
    size_t drainSourceCount = x0_player.GetEnergyDrain().GetEnergyDrainSources().size();
    if (drainSourceCount > 0)
        friction *= drainSourceCount * 1.5f;
    return friction;
}

void CMorphBall::ApplyGravity(CStateManager& mgr)
{
    if (x0_player.CheckSubmerged() && !mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::GravitySuit))
        x0_player.SetMomentumWR(zeus::CVector3f(0.f, 0.f, g_tweakBall->GetBallWaterGravity() * x0_player.GetMass()));
    else
        x0_player.SetMomentumWR(zeus::CVector3f(0.f, 0.f, g_tweakBall->GetBallGravity() * x0_player.GetMass()));
}

void CMorphBall::SpinToSpeed(float holdMag, zeus::CVector3f torque, float mag)
{
    x0_player.ApplyTorqueWR(torque *
        ((holdMag - x0_player.GetAngularVelocityWR().getVector().magnitude()) * mag));
}

float CMorphBall::ComputeMaxSpeed() const
{
    if (GetIsInHalfPipeMode())
        return std::min(x0_player.GetVelocity().magnitude() * 1.5f, 95.f);
    else
        return g_tweakBall->GetBallTranslationMaxSpeed(int(x0_player.GetSurfaceRestraint()));
}

static const CDamageInfo kBallDamage = { CWeaponMode(EWeaponType::BoostBall), 50000.f, 0.f, 0.f };

void CMorphBall::Touch(CActor& actor, CStateManager& mgr)
{
    if (TCastToPtr<CPhysicsActor> act = actor)
    {
        if (x1de4_24 &&
            (act->GetVelocity() - x0_player.GetVelocity()).magnitude() >
            g_tweakBall->GetBoostBallMinRelativeSpeedForDamage())
        {
            mgr.ApplyDamage(x0_player.GetUniqueId(), actor.GetUniqueId(), x0_player.GetUniqueId(), kBallDamage,
                CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::CVector3f::skZero);
        }
    }
}

bool CMorphBall::IsClimbable(const CCollisionInfo& cinfo) const
{
    if (std::fabs(cinfo.GetNormalLeft().z) < 0.7f)
    {
        float pointToBall = GetBallToWorld().origin.z - cinfo.GetPoint().z;
        if (pointToBall > 0.1f && pointToBall < GetBallRadius() - 0.05f)
            return true;
    }
    return false;
}

void CMorphBall::FluidFXThink(CActor::EFluidState, CScriptWater&, CStateManager&)
{

}

void CMorphBall::GetMorphBallModel(const std::string&, float)
{

}

void CMorphBall::LoadMorphBallModel(CStateManager& mgr)
{

}

void CMorphBall::AddSpiderBallElectricalEffect()
{

}

void CMorphBall::UpdateSpiderBallElectricalEffect()
{

}

void CMorphBall::RenderSpiderBallElectricalEffect() const
{

}

void CMorphBall::RenderEnergyDrainEffects(const CStateManager&) const
{

}

void CMorphBall::TouchModel(const CStateManager& mgr) const
{
    x58_ballModel->Touch(mgr, x5c_ballModelShader);
    if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::SpiderBall) && x60_spiderBallGlassModel)
        x60_spiderBallGlassModel->Touch(mgr, x64_spiderBallGlassModelShader);
    x68_lowPolyBallModel->Touch(mgr, x6c_lowPolyBallModelShader);
}

void CMorphBall::SetAsProjectile(const CDamageInfo&, const CDamageInfo&)
{

}

void CMorphBall::TakeDamage(float)
{

}

void CMorphBall::StartLandingSfx()
{
    if (x0_player.GetVelocity().z < -5.f && x1e36_landSfx != 0xffff)
    {
        float vol = zeus::clamp(0.75f, 0.0125f * x0_player.GetLastVelocity().z + 0.75f, 1.f);
        CSfxHandle hnd = CSfxManager::SfxStart(x1e36_landSfx, vol, 0.f, true, 0x7f, false, kInvalidAreaId);
        x0_player.ApplySubmergedPitchBend(hnd);
    }
}

void CMorphBall::Stop()
{
    x19b0_effect_morphBallIceBreak.Lock();
    if (x19e0_effect_morphBallIceBreakGen)
        x19e0_effect_morphBallIceBreakGen.reset();
}

void CMorphBall::StopSounds()
{
    if (x1e2c_rollSfxHandle)
    {
        CSfxManager::SfxStop(x1e2c_rollSfxHandle);
        x1e2c_rollSfxHandle.reset();
    }
    if (x1e30_spiderSfxHandle)
    {
        CSfxManager::SfxStop(x1e30_spiderSfxHandle);
        x1e30_spiderSfxHandle.reset();
    }
}

void CMorphBall::ActorAttached()
{

}

}
