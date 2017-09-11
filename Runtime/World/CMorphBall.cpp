#include "GameGlobalObjects.hpp"
#include "CDependencyGroup.hpp"
#include "CMorphBall.hpp"
#include "CPlayer.hpp"
#include "CMorphBallShadow.hpp"
#include "CSimplePool.hpp"
#include "CGameLight.hpp"

namespace urde
{

static float kSpiderBallCollisionRadius;

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
    x1c3c_quats.resize(5);
    x1c90_vecs.resize(5);
    x1cd0_.resize(15);
    x1d10_.resize(15);
    x1de4_24 = false;
    x1de4_25 = true;
    x1df8_24 = false;
    x1df8_25 = false;
    x1df8_26 = false;
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

zeus::CVector3f CMorphBall::GetBallContactSurfaceNormal() const { return {}; }

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
            vel += g_tweakBall->GetSpiderBallRollSpeed() * dt * 4.f;
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

void CMorphBall::BallTurnInput(const CFinalInput&) const
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

}

void CMorphBall::RenderDamageEffects(const CStateManager&, const zeus::CTransform&) const
{

}

void CMorphBall::UpdateHalfPipeStatus(CStateManager&, float)
{

}

bool CMorphBall::GetIsInHalfPipeMode() const
{
    return false;
}

void CMorphBall::SetIsInHalfPipeMode(bool)
{

}

void CMorphBall::GetIsInHalfPipeModeInAir() const
{

}

void CMorphBall::SetIsInHalfPipeModeInAir(bool)
{

}

void CMorphBall::GetTouchedHalfPipeRecently() const
{

}

void CMorphBall::SetTouchedHalfPipeRecently(bool)
{

}

void CMorphBall::DisableHalfPipeStatus()
{

}

bool CMorphBall::BallCloseToCollision(const CStateManager&, float, const CMaterialFilter& filter) const
{
    return false;
}

void CMorphBall::CollidedWith(TUniqueId, const CCollisionInfoList&, CStateManager&)
{

}

bool CMorphBall::IsInFrustum(const zeus::CFrustum&) const
{
    return false;
}

void CMorphBall::ComputeLiftForces(const zeus::CVector3f&, const zeus::CVector3f&, const CStateManager&)
{

}

void CMorphBall::CalculateSurfaceFriction() const
{

}

void CMorphBall::ApplyGravity(CStateManager&)
{

}

void CMorphBall::SpinToSpeed(float, zeus::CVector3f, float)
{

}

void CMorphBall::ComputeMaxSpeed() const
{

}

void CMorphBall::Touch(CActor&, CStateManager&)
{

}

bool CMorphBall::IsClimbable(const CCollisionInfo&) const
{
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

void CMorphBall::TouchModel(const CStateManager&) const
{

}

void CMorphBall::SetAsProjectile(const CDamageInfo&, const CDamageInfo&)
{

}

void CMorphBall::TakeDamage(float)
{

}

void CMorphBall::StartLandingSfx()
{

}

void CMorphBall::Stop()
{

}

void CMorphBall::StopSounds()
{

}

void CMorphBall::ActorAttached()
{

}

}
