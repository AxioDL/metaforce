#include "CPlayer.hpp"
#include "CActorParameters.hpp"
#include "CMorphBall.hpp"
#include "Weapon/CPlayerGun.hpp"
#include "CStateManager.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"
#include "Particle/CGenDescription.hpp"
#include "Camera/CFirstPersonCamera.hpp"
#include "Camera/CBallCamera.hpp"
#include "Camera/CCinematicCamera.hpp"
#include "TCastTo.hpp"
#include "CScriptGrapplePoint.hpp"
#include "CPatterned.hpp"
#include "CScriptWater.hpp"
#include "CDependencyGroup.hpp"
#include "Character/CSteeringBehaviors.hpp"
#include "Weapon/CEnergyProjectile.hpp"
#include "MP1/World/CThardusRockProjectile.hpp"
#include "MP1/World/CMetroidBeta.hpp"
#include "Collision/CMetroidAreaCollider.hpp"
#include "Collision/CGameCollision.hpp"
#include "Input/ControlMapper.hpp"
#include "CGameState.hpp"
#include "MP1/CSamusHud.hpp"
#include "CHUDBillboardEffect.hpp"

namespace urde
{

static const CMaterialFilter SolidMaterialFilter =
    CMaterialFilter::MakeInclude(CMaterialList(EMaterialTypes::Solid));

static const CMaterialFilter TargetingFilter =
    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid},
                                        {EMaterialTypes::ProjectilePassthrough,
                                         EMaterialTypes::ScanPassthrough,
                                         EMaterialTypes::Player});

static CModelData MakePlayerAnimRes(ResId resId, const zeus::CVector3f& scale)
{
    return {CAnimRes(resId, 0, scale, 0, true), 1};
}

CPlayer::CPlayer(TUniqueId uid, const zeus::CTransform& xf, const zeus::CAABox& aabb, ResId resId,
                 const zeus::CVector3f& playerScale, float mass, float stepUp, float stepDown, float f4,
                 const CMaterialList& ml)
: CPhysicsActor(uid, true, "CPlayer", CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList), xf,
                MakePlayerAnimRes(resId, playerScale), ml, aabb, SMoverData(mass), CActorParameters::None(), stepUp,
                stepDown), x7d0_animRes(resId, 0, playerScale, 0, true)
{
    x490_gun.reset(new CPlayerGun(uid));
    x49c_gunNotFiringTimeout = g_tweakPlayerGun->GetGunNotFiringTime();
    x4a0_failsafeTest.reset(new CFailsafeTest());
    x76c_cameraBob.reset(new CPlayerCameraBob(CPlayerCameraBob::ECameraBobType::One,
                                              zeus::CVector2f{CPlayerCameraBob::kCameraBobExtentX,
                                                              CPlayerCameraBob::kCameraBobExtentY},
                                              CPlayerCameraBob::kCameraBobPeriod));
    x9c4_26_ = true;
    x9c4_27_ = true;
    x9c4_28_ = true;
    x9c5_31_ = true;
    ResId beamId = g_tweakPlayerRes->GetBeamBallTransitionModel(x7ec_beam);
    x7f0_ballTransitionBeamModel = std::make_unique<CModelData>(CStaticRes(beamId, playerScale));
    x730_.reserve(3);
    x768_morphball.reset(new CMorphBall(*this, f4));

    SetInertiaTensorScalar(xe8_mass);
    x1f4_lastNonCollidingState = GetMotionState();
    x490_gun->SetTransform(x34_transform);
    x490_gun->GetGrappleArm().SetX220(x34_transform);

    InitializeBallTransition();
    zeus::CAABox ballTransAABB = x64_modelData->GetBounds();
    x2f0_ballTransHeight = ballTransAABB.max.z - ballTransAABB.min.z;

    SetCalculateLighting(true);

    x90_actorLights->SetCastShadows(true);
    x50c_.z = 0.f;
    if (x50c_.canBeNormalized())
        x50c_.normalize();
    x2b4_.push_back(20.f);
    x2b4_.push_back(80.f);
    x2b4_.push_back(80.f);
    x2b4_.push_back(270.f);
    SetMaximumCollisionVelocity(25.f);
    x354_onScreenOrbitObjects.reserve(64);
    x344_nearbyOrbitObjects.reserve(64);
    x364_offScreenOrbitObjects.reserve(64);
    x64_modelData->SetScale(playerScale);
    x7f0_ballTransitionBeamModel->SetScale(playerScale);
    LoadAnimationTokens();
}

void CPlayer::InitializeBallTransition()
{
    if (x64_modelData && x64_modelData->HasAnimData())
        x64_modelData->AnimationData()->SetAnimation(CAnimPlaybackParms(2, -1, 1.f, true), false);
}

bool CPlayer::IsTransparent() const { return x588_alpha < 1.f; }

void CPlayer::Update(float, CStateManager& mgr) {}

bool CPlayer::StartSamusVoiceSfx(u16 sfx, float vol, int prio)
{
    if (x2f8_morphTransState == EPlayerMorphBallState::Morphed)
        return false;
    bool started = true;
    if (x77c_samusVoiceSfx)
    {
        if (CSfxManager::IsPlaying(x77c_samusVoiceSfx))
        {
            started = false;
            if (prio > x780_samusVoicePriority)
            {
                CSfxManager::SfxStop(x77c_samusVoiceSfx);
                started = true;
            }
        }
        if (started)
        {
            x77c_samusVoiceSfx = CSfxManager::SfxStart(sfx, vol, 0.f, false, 0x7f, false, kInvalidAreaId);
            x780_samusVoicePriority = prio;
        }
    }
    return started;
}

bool CPlayer::IsPlayerDeadEnough() const
{
    if (x2f8_morphTransState == CPlayer::EPlayerMorphBallState::Unmorphed)
        return x9f4_deathTime > 2.5f;
    else if (x2f8_morphTransState == CPlayer::EPlayerMorphBallState::Morphed)
        return x9f4_deathTime > 6.f;

    return false;
}

void CPlayer::AsyncLoadSuit(CStateManager& mgr) { x490_gun->AsyncLoadSuit(mgr); }

void CPlayer::LoadAnimationTokens()
{
    TLockedToken<CDependencyGroup> transGroup = g_SimplePool->GetObj("BallTransition_DGRP");
    CDependencyGroup& group = *transGroup;
    x25c_ballTransitionsRes.reserve(group.GetObjectTagVector().size());
    for (const SObjectTag& tag : group.GetObjectTagVector())
    {
        if (tag.type == FOURCC('CMDL') || tag.type == FOURCC('CSKR') || tag.type == FOURCC('TXTR'))
            continue;
        x25c_ballTransitionsRes.push_back(g_SimplePool->GetObj(tag));
    }
}

bool CPlayer::HasTransitionBeamModel() const
{
    return x7f0_ballTransitionBeamModel && !x7f0_ballTransitionBeamModel->IsNull();
}

bool CPlayer::CanRenderUnsorted(CStateManager& mgr) const { return false; }

const CDamageVulnerability* CPlayer::GetDamageVulnerability(const zeus::CVector3f& v1, const zeus::CVector3f& v2,
                                                            const CDamageInfo& info) const
{
    if (x2f8_morphTransState == EPlayerMorphBallState::Morphed && x570_ > 0.f && !info.GetX18())
        return &CDamageVulnerability::ImmuneVulnerabilty();
    return &CDamageVulnerability::NormalVulnerabilty();
}

const CDamageVulnerability* CPlayer::GetDamageVulnerability() const
{
    CDamageInfo info(CWeaponMode(EWeaponType::Power, false, false, false), 0.f, 0.f, 0.f);
    return GetDamageVulnerability(zeus::CVector3f::skZero, zeus::CVector3f::skUp, info);
}

zeus::CVector3f CPlayer::GetHomingPosition(CStateManager& mgr, float dt) const
{
    if (dt > 0.f)
        return x34_transform.origin + PredictMotion(dt).x0_translation;
    return x34_transform.origin;
}

zeus::CVector3f CPlayer::GetAimPosition(CStateManager& mgr, float dt) const
{
    zeus::CVector3f ret = x34_transform.origin;
    if (dt > 0.f)
    {
        if (x304_orbitState == EPlayerOrbitState::Zero)
            ret += PredictMotion(dt).x0_translation;
        else
            ret = CSteeringBehaviors::ProjectOrbitalPosition(ret, x138_velocity, x314_orbitPoint, dt);
    }

    if (x2f8_morphTransState == EPlayerMorphBallState::Morphed)
        ret.z += g_tweakPlayer->GetPlayerBallHalfExtent();
    else
        ret.z += GetEyeHeight();

    return ret;
}

void CPlayer::FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr)
{
    if (x2f8_morphTransState == EPlayerMorphBallState::Morphed)
    {
        x768_morphball->FluidFXThink(state, water, mgr);
        if (state == EFluidState::One)
            x9c5_30_ = true;
    }
    else if (x2f8_morphTransState != EPlayerMorphBallState::Unmorphed)
    {
        if (mgr.GetFluidPlaneManager()->GetLastSplashDeltaTime(x8_uid) >= 0.2f)
        {
            zeus::CVector3f position(x34_transform.origin);
            position.z = water.GetTriggerBoundsWR().max.z;
            mgr.GetFluidPlaneManager()->CreateSplash(x8_uid, mgr, water, position, 0.1f,
                                                     state == EFluidState::Zero);
        }
    }
    else
    {
        if (mgr.GetFluidPlaneManager()->GetLastSplashDeltaTime(x8_uid) >= 0.2f)
        {
            zeus::CVector3f posOffset = x50c_;
            if (posOffset.canBeNormalized())
                posOffset = posOffset.normalized() * zeus::CVector3f(1.2f, 1.2f, 0.f);
            switch (state)
            {
            case EFluidState::Zero:
            {
                bool doSplash = true;
                if (x4fc_ > 12.5f)
                {
                    zeus::CVector3f lookDir = x34_transform.basis[1].normalized();
                    zeus::CVector3f dcVel = GetDampedClampedVelocityWR();
                    dcVel.z = 0.f;
                    if (lookDir.dot(dcVel.normalized()) > 0.75f)
                        doSplash = false;
                }
                if (doSplash)
                {
                    zeus::CVector3f position = x34_transform.origin + posOffset;
                    position.z = water.GetTriggerBoundsWR().max.z;
                    mgr.GetFluidPlaneManager()->CreateSplash(x8_uid, mgr, water, position, 0.3f, true);
                    if (water.GetFluidPlane().GetFluidType() == CFluidPlane::EFluidType::Zero)
                    {
                        float velMag = mgr.GetPlayer().GetVelocity().magnitude() / 10.f;
                        mgr.GetEnvFxManager()->SetXB54(10.f * std::max(1.f, velMag));
                    }
                }
                break;
            }
            case EFluidState::One:
            {
                if (x138_velocity.magnitude() > 1.f &&
                    mgr.GetFluidPlaneManager()->GetLastRippleDeltaTime(x8_uid) >= 0.2f)
                {
                    zeus::CVector3f position(x34_transform.origin);
                    position.z = water.GetTriggerBoundsWR().max.z;
                    water.GetFluidPlane().Ripple(0.5f, x8_uid, position, water, mgr);
                }
                break;
            }
            case EFluidState::Two:
            {
                zeus::CVector3f position = x34_transform.origin + posOffset;
                position.z = water.GetTriggerBoundsWR().max.z;
                mgr.GetFluidPlaneManager()->CreateSplash(x8_uid, mgr, water, position, 0.15f, true);
                break;
            }
            default: break;
            }
        }
    }
}

void CPlayer::TakeDamage(bool significant, const zeus::CVector3f& location,
                         float dam, EWeaponType type, CStateManager& mgr)
{
    if (!significant)
        return;
    if (dam >= 0.f)
    {
        x570_ = 0.5f;
        x55c_damageAmt = dam;
        x560_prevDamageAmt = (type == EWeaponType::AI && dam == 0.00002f) ? 10.f : dam;
        x564_damageLocation = location;
        x558_wasDamaged = true;

        bool doRumble = false;
        u16 suitDamageSfx = 0, damageLoopSfx = 0, damageSamusVoiceSfx = 0;

        switch (type)
        {
        case EWeaponType::Phazon:
        case EWeaponType::Unused2:
            damageLoopSfx = 3114;
            damageSamusVoiceSfx = 1653;
            break;
        case EWeaponType::PoisonWater:
            damageLoopSfx = 1486;
            damageSamusVoiceSfx = 1633;
            break;
        case EWeaponType::Lava:
            damageLoopSfx = 657;
        case EWeaponType::Hot:
            damageSamusVoiceSfx = 1656;
            break;
        default:
            if (x2f8_morphTransState == EPlayerMorphBallState::Unmorphed)
            {
                if (dam > 30.f)
                    damageSamusVoiceSfx = 1512;
                else if (dam > 15.f)
                    damageSamusVoiceSfx = 1511;
                else
                    damageSamusVoiceSfx = 1489;
                suitDamageSfx = 1467;
            }
            else
            {
                if (dam > 30.f)
                    suitDamageSfx = 1514;
                else if (dam > 15.f)
                    suitDamageSfx = 1513;
                else
                    suitDamageSfx = 1491;
            }
            break;
        }

        if (damageSamusVoiceSfx && x774_samusVoiceTimeout <= 0.f)
        {
            StartSamusVoiceSfx(damageSamusVoiceSfx, 1.f, 8);
            x774_samusVoiceTimeout = mgr.GetActiveRandom()->Range(3.f, 4.f);
            doRumble = true;
        }

        if (damageLoopSfx && !x9c7_24_ && x2ac_movementSurface >= EPlayerMovementSurface::Ice)
        {
            if (!x770_damageLoopSfx || x788_damageLoopSfxId != damageLoopSfx)
            {
                if (x770_damageLoopSfx && x788_damageLoopSfxId != damageLoopSfx)
                    CSfxManager::SfxStop(x770_damageLoopSfx);
                x770_damageLoopSfx = CSfxManager::SfxStart(damageLoopSfx, 1.f, 0.f, false, 0x7f, true, kInvalidAreaId);
                x788_damageLoopSfxId = damageLoopSfx;
            }
            x784_ = 0.5f;
        }

        if (suitDamageSfx)
        {
            if (x770_damageLoopSfx)
            {
                CSfxManager::SfxStop(x770_damageLoopSfx);
                x770_damageLoopSfx.reset();
            }
            x770_damageLoopSfx = CSfxManager::SfxStart(suitDamageSfx, 1.f, 0.f, false, 0x7f, true, kInvalidAreaId);
            x788_damageLoopSfxId = suitDamageSfx;
            xa2c_ = 0;
            doRumble = true;
        }

        if (doRumble)
        {
            if (x2f8_morphTransState == EPlayerMorphBallState::Unmorphed)
                x490_gun->DamageRumble(location, dam, mgr);
            float tmp = x55c_damageAmt / 25.f;
            if (std::fabs(tmp) > 1.f)
                tmp = tmp > 0.f ? 1.f : -1.f;
            mgr.GetRumbleManager().Rumble(mgr, ERumbleFxId::Eleven, ERumblePriority::One);
        }

        if (x2f8_morphTransState != EPlayerMorphBallState::Unmorphed)
        {
            x768_morphball->TakeDamage(x55c_damageAmt);
            x768_morphball->SetDamageTimer(0.4f);
        }
    }

    if (x3b8_grappleState != EGrappleState::Zero)
        BreakGrapple(EPlayerOrbitRequest::Eleven, mgr);
}

void CPlayer::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

CHealthInfo* CPlayer::HealthInfo(const CStateManager& mgr) { return &mgr.GetPlayerState()->HealthInfo(); }

bool CPlayer::IsUnderBetaMetroidAttack(CStateManager& mgr) const
{
    if (x274_energyDrain.GetEnergyDrainIntensity() > 0.f)
    {
        for (const CEnergyDrainSource& source : x274_energyDrain.GetEnergyDrainSources())
            if (CPatterned::CastTo<MP1::CMetroidBeta>(mgr.GetObjectById(source.GetEnergyDrainSourceId())))
                return true;
    }
    return false;
}

rstl::optional_object<zeus::CAABox> CPlayer::GetTouchBounds() const { return {}; }

void CPlayer::Touch(CActor&, CStateManager& mgr) {}

void CPlayer::DoPreThink(float dt, CStateManager& mgr)
{
    PreThink(dt, mgr);
    if (CEntity* ent = mgr.ObjectById(xa00_))
        ent->PreThink(dt, mgr);
}

void CPlayer::DoThink(float dt, CStateManager& mgr)
{
    Think(dt, mgr);
    if (CEntity* ent = mgr.ObjectById(xa00_))
        ent->Think(dt, mgr);
}

void CPlayer::UpdateScanningState(const CFinalInput& input, CStateManager& mgr, float) {}

void CPlayer::ValidateScanning(const CFinalInput& input, CStateManager& mgr) {}

void CPlayer::SetScanningState(EPlayerScanState, CStateManager& mgr) {}

bool CPlayer::GetExplorationMode() const { return false; }

bool CPlayer::GetCombatMode() const { return false; }

void CPlayer::RenderGun(const CStateManager& mgr, const zeus::CVector3f&) const {}

void CPlayer::Render(const CStateManager& mgr) const {}

void CPlayer::RenderReflectedPlayer(CStateManager& mgr) const {}

void CPlayer::PreRender(CStateManager& mgr, const zeus::CFrustum&) {}

void CPlayer::CalculateRenderBounds() {}

void CPlayer::AddToRenderer(const zeus::CFrustum&, const CStateManager&) {}

void CPlayer::ComputeFreeLook(const CFinalInput& input) {}

void CPlayer::UpdateFreeLookState(const CFinalInput&, float dt, CStateManager&) {}

void CPlayer::UpdateFreeLook(float dt) {}

float CPlayer::GetMaximumPlayerPositiveVerticalVelocity(CStateManager&) const { return 0.f; }

void CPlayer::StartLandingControlFreeze()
{
    x760_controlsFrozen = true;
    x764_controlsFrozenTimeout = 0.75f;
}

void CPlayer::EndLandingControlFreeze()
{
    x760_controlsFrozen = false;
    x764_controlsFrozenTimeout = 0.f;
}

void CPlayer::ProcessFrozenInput(float dt, CStateManager& mgr)
{
    x764_controlsFrozenTimeout -= dt;
    if (x764_controlsFrozenTimeout <= 0.f)
    {
        EndLandingControlFreeze();
    }
    else
    {
        CFinalInput dummy;
        if (x2f8_morphTransState == EPlayerMorphBallState::Morphed)
        {
            x768_morphball->ComputeBallMovement(dummy, mgr, dt);
            x768_morphball->UpdateBallDynamics(mgr, dt);
        }
        else
        {
            ComputeMovement(dummy, mgr, dt);
        }
    }
}

void CPlayer::ProcessInput(const CFinalInput& input, CStateManager& mgr)
{
    if (input.ControllerIdx() != 0)
        return;

    if (x2f8_morphTransState != EPlayerMorphBallState::Morphed)
        UpdateScanningState(input, mgr, input.DeltaTime());

    if (mgr.GetGameState() != CStateManager::EGameState::Running)
        return;

    if (!mgr.GetPlayerState()->IsPlayerAlive())
        return;

    if (GetFrozenState())
        UpdateFrozenState(input, mgr);

    if (GetFrozenState())
    {
        if (x258_movementState == EPlayerMovementState::OnGround ||
            x258_movementState == EPlayerMovementState::FallingMorphed)
            return;

        CFinalInput dummyInput;
        if (x2f8_morphTransState == EPlayerMorphBallState::Morphed)
        {
            x768_morphball->ComputeBallMovement(dummyInput, mgr, input.DeltaTime());
            x768_morphball->UpdateBallDynamics(mgr, input.DeltaTime());
        }
        else
        {
            ComputeMovement(dummyInput, mgr, input.DeltaTime());
        }

        return;
    }

    if (x760_controlsFrozen)
    {
        ProcessFrozenInput(input.DeltaTime(), mgr);
        return;
    }

    if (x2f8_morphTransState == EPlayerMorphBallState::Unmorphed && x4a0_failsafeTest->Passes())
    {
        const CCollidableAABox* prim = static_cast<const CCollidableAABox*>(GetCollisionPrimitive());
        zeus::CAABox tmpAABB(prim->GetAABB().min - 0.2f, prim->GetAABB().max + 0.2f);
        CCollidableAABox tmpBox(tmpAABB, prim->GetMaterial());
        CPhysicsActor::Stop();
        zeus::CAABox testBounds = prim->GetAABB().getTransformedAABox(x34_transform);
        zeus::CAABox expandedBounds(testBounds.min - 3.f, testBounds.max + 3.f);
        CAreaCollisionCache cache(expandedBounds);
        CGameCollision::BuildAreaCollisionCache(mgr, cache);
        rstl::reserved_vector<TUniqueId, 1024> nearList;
        mgr.BuildColliderList(nearList, *this, expandedBounds);
        std::experimental::optional<zeus::CVector3f> nonIntVec =
        CGameCollision::FindNonIntersectingVector(mgr, cache, *this, *prim, nearList);
        if (nonIntVec)
        {
            x4a0_failsafeTest->Reset();
            SetTranslation(GetTranslation() + *nonIntVec);
        }
    }

    UpdateGrappleState(input, mgr);
    if (x2f8_morphTransState == EPlayerMorphBallState::Morphed)
    {
        float leftDiv = g_tweakBall->GetLeftStickDivisor();
        float rightDiv = g_tweakBall->GetRightStickDivisor();
        if (x26c_ != kInvalidUniqueId || IsUnderBetaMetroidAttack(mgr))
            leftDiv = 2.f;
        CFinalInput scaledInput = input.ScaleAnalogueSticks(leftDiv, rightDiv);
        x768_morphball->ComputeBallMovement(scaledInput, mgr, input.DeltaTime());
        x768_morphball->UpdateBallDynamics(mgr, input.DeltaTime());
        x4a0_failsafeTest->Reset();
    }
    else
    {
        if (x304_orbitState == EPlayerOrbitState::Five)
        {
            ApplyGrappleForces(input, mgr, input.DeltaTime());
        }
        else
        {
            CFinalInput scaledInput = input.ScaleAnalogueSticks(IsUnderBetaMetroidAttack(mgr) ? 3.f : 1.f, 1.f);
            ComputeMovement(scaledInput, mgr, input.DeltaTime());
        }

        if (ShouldSampleFailsafe(mgr))
        {
            CFailsafeTest::EInputState inputState = CFailsafeTest::EInputState::Moving;
            if (x258_movementState == EPlayerMovementState::StartingJump)
                inputState = CFailsafeTest::EInputState::StartingJump;
            else if (x258_movementState == EPlayerMovementState::Jump)
                inputState = CFailsafeTest::EInputState::Jump;
            x4a0_failsafeTest->AddSample(inputState, GetTranslation(), x138_velocity,
                                         zeus::CVector2f(input.ALeftX(), input.ALeftY()));
        }
    }

    ComputeFreeLook(input);
    UpdateFreeLookState(input, input.DeltaTime(), mgr);
    UpdateOrbitInput(input, mgr);
    UpdateOrbitZone(mgr);
    UpdateGunState(input, mgr);
    UpdateVisorState(input, input.DeltaTime(), mgr);

    if (x2f8_morphTransState == EPlayerMorphBallState::Morphed ||
        (x2f8_morphTransState == EPlayerMorphBallState::Unmorphed &&
        x498_gunHolsterState == EGunHolsterState::Drawn))
    {
        x490_gun->ProcessInput(input, mgr);
        if (x2f8_morphTransState == EPlayerMorphBallState::Morphed && x2fc_ != kInvalidUniqueId)
        {
            if (ControlMapper::GetPressInput(ControlMapper::ECommands::TurnLeft, input) ||
                ControlMapper::GetPressInput(ControlMapper::ECommands::TurnRight, input) ||
                ControlMapper::GetPressInput(ControlMapper::ECommands::Forward, input) ||
                ControlMapper::GetPressInput(ControlMapper::ECommands::Backward, input) ||
                ControlMapper::GetPressInput(ControlMapper::ECommands::JumpOrBoost, input))
            {
                xa28_ += input.DeltaTime() * 600.f * input.DeltaTime();
                if (xa28_ > 1.f)
                    xa28_ = 1.f;
            }
            else
            {
                float tmp = 7.5f * input.DeltaTime();
                xa28_ -= input.DeltaTime() * std::min(1.f, xa28_ * tmp + tmp);
                if (xa28_ < 0.f)
                    xa28_ = 0.f;
            }
        }
    }

    UpdateCameraState(mgr);
    UpdateMorphBallState(input, mgr);
    UpdateCameraTimers(input.DeltaTime(), input);
    UpdateFootstepSounds(input, mgr, input.DeltaTime());
    x2a8_ += input.DeltaTime();

    if (CheckSubmerged())
        SetSoundEventPitchBend(0);
    else
        SetSoundEventPitchBend(8192);

    CalculateLeaveMorphBallDirection(input);
}

bool CPlayer::ShouldSampleFailsafe(CStateManager& mgr) const
{
    TCastToPtr<CCinematicCamera> cineCam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
    if (!mgr.GetPlayerState()->IsPlayerAlive())
        return false;
    return x2f4_cameraState != EPlayerCameraState::Four || !cineCam || !(cineCam->GetW1() & 0x80);
}

void CPlayer::CalculateLeaveMorphBallDirection(const CFinalInput& input)
{
    if (x2f8_morphTransState != EPlayerMorphBallState::Morphed)
    {
        x518_leaveMorphDir = x50c_;
    }
    else
    {
        if (ControlMapper::GetAnalogInput(ControlMapper::ECommands::Forward, input) > 0.3f ||
            ControlMapper::GetAnalogInput(ControlMapper::ECommands::Backward, input) > 0.3f ||
            ControlMapper::GetAnalogInput(ControlMapper::ECommands::TurnLeft, input) > 0.3f ||
            ControlMapper::GetAnalogInput(ControlMapper::ECommands::TurnRight, input) > 0.3f)
        {
            if (x138_velocity.magnitude() > 0.5f)
                x518_leaveMorphDir = x50c_;
        }
    }
}

void CPlayer::CalculatePlayerControlDirection(CStateManager& mgr)
{
    if (x9c4_30_)
    {
        if (x9d8_.canBeNormalized())
        {
            x540_ = x9d8_.normalized();
            x54c_ = x9d8_;
            x54c_.z = 0.f;
            if (x54c_.canBeNormalized())
                x54c_.normalize();
            else
                x540_ = x54c_ = zeus::CVector3f::skForward;
        }
        else
        {
            x540_ = x54c_ = zeus::CVector3f::skForward;
        }
    }
    else
    {
        zeus::CVector3f camToPlayer =
            GetTranslation() - mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTranslation();
        if (!camToPlayer.canBeNormalized())
        {
            x540_ = x54c_ = zeus::CVector3f::skForward;
        }
        else
        {
            zeus::CVector3f camToPlayerFlat(camToPlayer.x, camToPlayer.y, 0.f);
            if (camToPlayerFlat.canBeNormalized())
            {
                if (camToPlayerFlat.magnitude() > g_tweakBall->GetBallCameraControlDistance())
                {
                    x540_ = camToPlayer.normalized();
                    if (camToPlayerFlat.canBeNormalized())
                    {
                        camToPlayerFlat.normalize();
                        switch (x2f8_morphTransState)
                        {
                        case EPlayerMorphBallState::Morphed:
                            x54c_ = camToPlayerFlat;
                            break;
                        default:
                            x540_ = GetTransform().basis[1];
                            x54c_ = x540_;
                            x54c_.z = 0.f;
                            if (x54c_.canBeNormalized())
                                x54c_.normalize();
                        }
                    }
                    else if (x2f8_morphTransState != EPlayerMorphBallState::Morphed)
                    {
                        x540_ = GetTransform().basis[1];
                        x54c_.z = 0.f;
                        if (x54c_.canBeNormalized())
                            x54c_.normalize();
                    }
                }
                else
                {
                    if (x4fc_ < 0.25f)
                    {
                        x540_ = camToPlayer;
                        x54c_ = camToPlayerFlat;
                    }
                    else if (x2f8_morphTransState != EPlayerMorphBallState::Morphed)
                    {
                        x540_ = GetTransform().basis[1];
                        x54c_.z = 0.f;
                        if (x54c_.canBeNormalized())
                            x54c_.normalize();
                    }
                }
            }
        }
    }
}

void CPlayer::CalculatePlayerMovementDirection(float dt)
{
    if (x2f8_morphTransState == EPlayerMorphBallState::Morphing ||
        x2f8_morphTransState == EPlayerMorphBallState::Unmorphing)
        return;

    zeus::CVector3f delta = GetTranslation() - x524_;
    if (delta.canBeNormalized() && delta.magnitude() > 0.02f)
    {
        x53c_ += dt;
        x4f8_ = std::fabs(delta.magnitude() / dt);
        x500_ = delta.normalized();
        zeus::CVector3f flatDelta(delta.x, delta.y, 0.f);
        if (flatDelta.canBeNormalized())
        {
            x4fc_ = std::fabs(flatDelta.magnitude() / dt);
            flatDelta.normalize();
            switch (x2f8_morphTransState)
            {
            case EPlayerMorphBallState::Morphed:
                if (x4fc_ > 0.25f)
                    x50c_ = flatDelta;
                x530_ = x50c_;
                x524_ = GetTranslation();
                break;
            default:
                x500_ = GetTransform().basis[1];
                x50c_ = x500_;
                x50c_.z = 0.f;
                if (x50c_.canBeNormalized())
                    x50c_.normalize();
                x530_ = x50c_;
                x524_ = GetTranslation();
                break;
            }
        }
        else
        {
            if (x2f8_morphTransState != EPlayerMorphBallState::Morphed)
            {
                x500_ = GetTransform().basis[1];
                x50c_ = x500_;
                x50c_.z = 0.f;
                if (x50c_.canBeNormalized())
                    x50c_.normalize();
                x530_ = x50c_;
                x524_ = GetTranslation();
            }
            x4fc_ = 0.f;
        }
    }
    else
    {
        x53c_ = 0.f;
        switch (x2f8_morphTransState)
        {
        case EPlayerMorphBallState::Morphed:
        case EPlayerMorphBallState::Morphing:
        case EPlayerMorphBallState::Unmorphing:
            x500_ = x50c_;
            break;
        default:
            x500_ = GetTransform().basis[1];
            x50c_ = x500_;
            x50c_.z = 0.f;
            if (x50c_.canBeNormalized())
                x50c_.normalize();
            x530_ = x50c_;
            x524_ = GetTranslation();
            break;
        }
        x4f8_ = 0.f;
        x4fc_ = 0.f;
    }

    x50c_.z = 0.f;
    if (x50c_.canBeNormalized())
        x500_.normalize();
}

void CPlayer::Stop(CStateManager& stateMgr)
{
    if (GetFrozenState())
    {
        x750_frozenTimeout = 0.f;
        x754_iceBreakJumps = 0;
        CPhysicsActor::Stop();
        ClearForcesAndTorques();
        RemoveMaterial(EMaterialTypes::Immovable, stateMgr);
        if (!stateMgr.GetCameraManager()->IsInCinematicCamera() && xa0c_iceTextureId != kInvalidResId)
        {
            std::experimental::optional<TToken<CGenDescription>> gpsm;
            gpsm.emplace(g_SimplePool->GetObj(SObjectTag(FOURCC('PART'), xa0c_iceTextureId)));
            CHUDBillboardEffect* effect = new CHUDBillboardEffect(gpsm, {}, stateMgr.AllocateUniqueId(), true,
                "FrostExplosion", CHUDBillboardEffect::GetNearClipDistance(stateMgr),
                CHUDBillboardEffect::GetScaleForPOV(stateMgr), zeus::CColor::skWhite,
                zeus::CVector3f::skOne, zeus::CVector3f::skZero);
            stateMgr.AddObject(effect);
            CSfxHandle hnd = CSfxManager::SfxStart(3129, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
            ApplySubmergedPitchBend(hnd);
        }
        x768_morphball->Stop();
        SetVisorSteam(0.f, 0.42857146f, 0.071428575f, xa08_steamTextureId, false);
    }
}

void CPlayer::Freeze(CStateManager& stateMgr, ResId steamTxtr, u16 sfx, ResId iceTxtr)
{
    if (stateMgr.GetCameraManager()->IsInCinematicCamera() || GetFrozenState())
        return;

    bool showMsg;
    if (x2f8_morphTransState == EPlayerMorphBallState::Unmorphed)
        showMsg = g_GameState->SystemOptions().GetShowFrozenFpsMessage();
    else
        showMsg = g_GameState->SystemOptions().GetShowFrozenBallMessage();

    if (showMsg)
    {
        const char16_t* msg =
            g_MainStringTable->GetString(int(x2f8_morphTransState >= EPlayerMorphBallState::Morphed) + 19);
        CHUDMemoParms parms(5.f, true, false, false);
        MP1::CSamusHud::DisplayHudMemo(msg, parms);
    }

    x750_frozenTimeout = x758_frozenTimeoutBias + g_tweakPlayer->GetFrozenTimeout();
    x754_iceBreakJumps = -x75c_additionalIceBreakJumps;

    CPhysicsActor::Stop();
    ClearForcesAndTorques();
    if (x3b8_grappleState != EGrappleState::Zero)
        BreakGrapple(EPlayerOrbitRequest::Ten, stateMgr);
    else
        SetOrbitRequest(EPlayerOrbitRequest::Ten, stateMgr);

    AddMaterial(EMaterialTypes::Immovable, stateMgr);
    xa08_steamTextureId = steamTxtr;
    xa0c_iceTextureId = iceTxtr;
    CSfxHandle hnd = CSfxManager::SfxStart(sfx, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    ApplySubmergedPitchBend(hnd);
    EndLandingControlFreeze();
}

bool CPlayer::GetFrozenState() const { return x750_frozenTimeout > 0.f; }

void CPlayer::UpdateFrozenState(const CFinalInput& input, CStateManager& mgr)
{
    x750_frozenTimeout -= input.DeltaTime();
    if (x750_frozenTimeout > 0.f)
        SetVisorSteam(0.7f, 0.42857146f, 0.071428575f, xa08_steamTextureId, false);
    else
        Stop(mgr);
    if (x258_movementState == EPlayerMovementState::OnGround ||
        x258_movementState == EPlayerMovementState::FallingMorphed)
    {
        Stop(mgr);
        ClearForcesAndTorques();
    }
    x7a0_visorSteam.Update(input.DeltaTime());

    switch (x2f8_morphTransState)
    {
    case EPlayerMorphBallState::Morphed:
        x490_gun->ProcessInput(input, mgr);
        break;
    case EPlayerMorphBallState::Unmorphed:
    case EPlayerMorphBallState::Morphing:
    case EPlayerMorphBallState::Unmorphing:
        if (ControlMapper::GetAnalogInput(ControlMapper::ECommands::JumpOrBoost, input))
        {
            if (x754_iceBreakJumps)
            {
                /* Subsequent Breaks */
                CSfxHandle hnd = CSfxManager::SfxStart(3127, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
                ApplySubmergedPitchBend(hnd);
            }
            else
            {
                /* Initial Break */
                CSfxHandle hnd = CSfxManager::SfxStart(3128, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
                ApplySubmergedPitchBend(hnd);
            }
            x754_iceBreakJumps += 1;
            if (x754_iceBreakJumps > g_tweakPlayer->GetIceBreakJumpCount())
            {
                g_GameState->SystemOptions().IncrementFrozenFpsCount();
                CHUDMemoParms info(0.f, true, true, true);
                MP1::CSamusHud::DisplayHudMemo(u"", info);
                Stop(mgr);
            }
        }
        break;
    }
}

void CPlayer::Think(float, CStateManager&) {}

void CPlayer::PreThink(float dt, CStateManager& mgr)
{
    x558_wasDamaged = false;
    x55c_damageAmt = 0.f;
    x560_prevDamageAmt = 0.f;
    x564_damageLocation = zeus::CVector3f::skZero;
    xa04_ = dt;
}

static const u16 skPlayerLandSfxSoft[] =
{
    0xFFFF, 0x05E4, 0x05D2, 0x0621, 0x0658, 0xFFFF, 0x05E3, 0x0606,
    0x05C0, 0x088E, 0x0694, 0x0638, 0x062B, 0xFFFF, 0x0621, 0x05D2,
    0x05D2, 0x05C0, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x05FB, 0x0625
};

static const u16 skPlayerLandSfxHard[] =
{
    0xFFFF, 0x0651, 0x064B, 0x0647, 0x065A, 0xFFFF, 0x0648, 0x064E,
    0x064F, 0x08D7, 0x0696, 0x0650, 0x064C, 0xFFFF, 0x0647, 0x064B,
    0x064B, 0x064F, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0652, 0x064D
};

void CPlayer::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr)
{
    switch (msg)
    {
    case EScriptObjectMessage::OnFloor:
        if (x258_movementState != EPlayerMovementState::OnGround &&
            x2f8_morphTransState != EPlayerMorphBallState::Morphed &&
            x300_fallingTime > 0.3f)
        {
            if (x258_movementState != EPlayerMovementState::Falling)
            {
                float hardThres = 30.f * 2.f * -g_tweakPlayer->GetHardLandingVelocityThreshold();
                hardThres = (hardThres != 0.f) ? hardThres * std::sqrt(hardThres) : 0.f;
                float landVol = zeus::clamp(95.f, 1.6f * -x794_.z + 95.f, 127.f) / 127.f;
                u16 landSfx;
                if (-x794_.z < hardThres)
                {
                    landSfx = GetMaterialSoundUnderPlayer(mgr, skPlayerLandSfxSoft, 24, 0xffff);
                }
                else
                {
                    landSfx = GetMaterialSoundUnderPlayer(mgr, skPlayerLandSfxHard, 24, 0xffff);
                    StartSamusVoiceSfx(1550, 1.f, 5);
                    x55c_damageAmt = 0.f;
                    x560_prevDamageAmt = 10.f;
                    x564_damageLocation = x34_transform.origin;
                    x558_wasDamaged = true;
                    mgr.GetCameraManager()->AddCameraShaker(
                        CCameraShakeData::BuildLandingCameraShakeData(0.3f, 1.25f), false);
                    StartLandingControlFreeze();
                }
                CSfxHandle handle =
                    CSfxManager::SfxStart(landSfx, landVol, 0.f, false, 0x7f, false, kInvalidAreaId);
                ApplySubmergedPitchBend(handle);

                float rumbleMag = -x794_.z / 110.f;
                if (rumbleMag > 0.f)
                {
                    if (std::fabs(rumbleMag) > 0.8f)
                        rumbleMag = (rumbleMag > 0.f) ? 0.8f : -0.8f;
                    mgr.GetRumbleManager().Rumble(mgr, ERumbleFxId::Fifteen, rumbleMag, ERumblePriority::One);
                }

                x2a0_ = 0.f;
            }
        }
        else if (x258_movementState != EPlayerMovementState::OnGround &&
                 x2f8_morphTransState == EPlayerMorphBallState::Morphed)
        {
            if (x138_velocity.z < -40.f && !x768_morphball->GetIsInHalfPipeMode() &&
                x258_movementState == EPlayerMovementState::StartingJump &&
                x300_fallingTime > 0.75f)
                SetCoefficientOfRestitutionModifier(0.2f);
            x768_morphball->StartLandingSfx();
            if (x138_velocity.z < -5.f)
            {
                float rumbleMag = -x138_velocity.z / 110.f * 0.5f;
                if (std::fabs(rumbleMag) > 0.8f)
                    rumbleMag = (rumbleMag > 0.f) ? 0.8f : -0.8f;
                mgr.GetRumbleManager().Rumble(mgr, ERumbleFxId::Fifteen, rumbleMag, ERumblePriority::One);
                x2a0_ = 0.f;
            }
            if (x138_velocity.z < -30.f)
            {
                float rumbleMag = -x138_velocity.z / 110.f;
                if (std::fabs(rumbleMag) > 0.8f)
                    rumbleMag = (rumbleMag > 0.f) ? 0.8f : -0.8f;
                mgr.GetRumbleManager().Rumble(mgr, ERumbleFxId::Fifteen, rumbleMag, ERumblePriority::One);
                x2a0_ = 0.f;
            }
        }
        x300_fallingTime = 0.f;
        SetMoveState(EPlayerMovementState::OnGround, mgr);
        break;
    case EScriptObjectMessage::Falling:
        if (x2f8_morphTransState == EPlayerMorphBallState::Morphed)
            if (x768_morphball->GetX187c() == 1)
                break;
        if (x2f8_morphTransState != EPlayerMorphBallState::Morphed)
            SetMoveState(EPlayerMovementState::Falling, mgr);
        else if (x258_movementState == EPlayerMovementState::OnGround)
            SetMoveState(EPlayerMovementState::FallingMorphed, mgr);
        break;
    case EScriptObjectMessage::LandOnNotFloor:
        if (x2f8_morphTransState == EPlayerMorphBallState::Morphed &&
            x768_morphball->GetX187c() == 1 &&
            x258_movementState != EPlayerMovementState::StartingJump)
            SetMoveState(EPlayerMovementState::StartingJump, mgr);
        break;
    case EScriptObjectMessage::OnIceSurface:
        x2ac_movementSurface = EPlayerMovementSurface::Ice;
        break;
    case EScriptObjectMessage::OnMudSlowSurface:
        x2ac_movementSurface = EPlayerMovementSurface::MudSlow;
        break;
    case EScriptObjectMessage::OnNormalSurface:
        x2ac_movementSurface = EPlayerMovementSurface::Normal;
        break;
    case EScriptObjectMessage::InSnakeWeed:
        x2ac_movementSurface = EPlayerMovementSurface::SnakeWeed;
        break;
    case EScriptObjectMessage::AddSplashInhabitant:
    {
        SetInFluid(true, sender);
        UpdateSubmerged(mgr);
        CRayCastResult result =
            mgr.RayStaticIntersection(x34_transform.origin, zeus::CVector3f::skDown,
                                      0.5f * GetEyeHeight(), SolidMaterialFilter);
        if (result.IsInvalid())
        {
            SetVelocityWR(x138_velocity * 0.095f);
            xfc_constantForce *= 0.095f;
        }
        break;
    }
    case EScriptObjectMessage::UpdateSplashInhabitant:
        UpdateSubmerged(mgr);
        if (CheckSubmerged() && !mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::GravitySuit))
        {
            if (TCastToPtr<CScriptWater> water = mgr.ObjectById(xc4_fluidId))
            {
                switch (water->GetFluidPlane().GetFluidType())
                {
                case CFluidPlane::EFluidType::Zero:
                    x2b0_ = 0;
                    break;
                case CFluidPlane::EFluidType::Two:
                case CFluidPlane::EFluidType::Five:
                    x2ac_movementSurface = EPlayerMovementSurface::Fluid2Or5;
                    break;
                case CFluidPlane::EFluidType::One:
                    x2b0_ = 0;
                    break;
                case CFluidPlane::EFluidType::Three:
                    x2ac_movementSurface = EPlayerMovementSurface::Fluid3;
                    break;
                default: break;
                }
            }
        }
        x9c5_25_splashUpdated = true;
        break;
    case EScriptObjectMessage::RemoveSplashInhabitant:
        SetInFluid(false, kInvalidUniqueId);
        UpdateSubmerged(mgr);
        break;
    case EScriptObjectMessage::ProjectileCollide:
        x378_ = g_tweakPlayer->GetX1fc();
        SetOrbitRequest(EPlayerOrbitRequest::Nine, mgr);
        break;
    case EScriptObjectMessage::AddPlatformRider:
        x82e_ridingPlatform = sender;
        break;
    case EScriptObjectMessage::Damage:
        if (TCastToPtr<CEnergyProjectile> energ = mgr.ObjectById(sender))
        {
            if ((energ->GetAttribField() & CWeapon::EProjectileAttrib::StaticInterference) !=
                 CWeapon::EProjectileAttrib::None)
            {
                mgr.GetPlayerState()->GetStaticInterference().
                    AddSource(GetUniqueId(), 0.3f, energ->GetInterferenceDuration());
            }
        }
        break;
    case EScriptObjectMessage::Deleted:
        mgr.GetPlayerState()->ResetVisor();
        x730_.clear();
        break;
    default: break;
    }

    x490_gun->AcceptScriptMessage(msg, sender, mgr);
    x768_morphball->AcceptScriptMessage(msg, sender, mgr);
    CActor::AcceptScriptMsg(msg, sender, mgr);
}

void CPlayer::SetVisorSteam(float, float, float, ResId, bool) {}

static const u16 skLeftStepSounds[] =
{
    0xFFFF, 0x05B9, 0x05D7, 0x0622, 0x0653,
    0xFFFF, 0x05D5, 0x0611, 0x05D9, 0x0887,
    0x0699, 0x063A, 0x0631, 0xFFFF, 0x0629,
    0x05D7, 0x05D7, 0x05D9, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0x05F9, 0x0629
};

static const u16 skRightStepSounds[] =
{
    0xFFFF, 0x05BA, 0x05D8, 0x0623, 0x0654,
    0xFFFF, 0x05D6, 0x0612, 0x05DB, 0x0888,
    0x069A, 0x063B, 0x0632, 0xFFFF, 0x062A,
    0x05D8, 0x05D8, 0x05DB, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0x05FA, 0x062A
};

void CPlayer::UpdateFootstepSounds(const CFinalInput& input, CStateManager& mgr, float dt)
{
    if (x2f8_morphTransState != EPlayerMorphBallState::Unmorphed ||
        x258_movementState != EPlayerMovementState::OnGround ||
        x3dc_inFreeLook || x3dd_freeLookPitchAngleCalculated)
        return;

    float sfxVol = 1.f;
    x78c_footstepSfxTimer += dt;
    float turn = std::fabs(TurnInput(input));
    float forward = std::fabs(ForwardInput(input, dt));
    float sfxDelay = 0.f;
    if (forward > 0.05f || x304_orbitState != EPlayerOrbitState::Zero)
    {
        float vel = std::min(1.f, x138_velocity.magnitude() / GetActualFirstPersonMaxVelocity());
        if (vel > 0.05f)
        {
            sfxDelay = -0.475f * vel + 0.85f;
            if (x790_footstepSfxSel == EFootstepSfx::None)
                x790_footstepSfxSel = EFootstepSfx::Left;
        }
        else
        {
            x78c_footstepSfxTimer = 0.f;
            x790_footstepSfxSel = EFootstepSfx::None;
        }

        sfxVol = 0.3f * vel + 0.7f;
    }
    else if (turn > 0.05f)
    {
        if (x790_footstepSfxSel == EFootstepSfx::Left)
            sfxDelay = -0.813f * turn + 1.f;
        else
            sfxDelay = -2.438f * turn + 3.f;
        if (x790_footstepSfxSel == EFootstepSfx::None)
        {
            x790_footstepSfxSel = EFootstepSfx::Left;
            sfxDelay = x78c_footstepSfxTimer;
        }
        sfxVol = 0.75f;
    }
    else
    {
        x78c_footstepSfxTimer = 0.f;
        x790_footstepSfxSel = EFootstepSfx::None;
    }

    if (x790_footstepSfxSel != EFootstepSfx::None && x78c_footstepSfxTimer > sfxDelay)
    {
        static float EarHeight = GetEyeHeight() - 0.1f;
        if (xe6_24_fluidCounter != 0 &&  x828_waterLevelOnPlayer > 0.f && x828_waterLevelOnPlayer < EarHeight)
        {
            if (x82c_inLava)
            {
                if (x790_footstepSfxSel == EFootstepSfx::Left)
                {
                    CSfxHandle hnd = CSfxManager::SfxStart(2183, sfxVol, 0.f, true, 0x7f, false, kInvalidAreaId);
                    ApplySubmergedPitchBend(hnd);
                }
                else
                {
                    CSfxHandle hnd = CSfxManager::SfxStart(2184, sfxVol, 0.f, true, 0x7f, false, kInvalidAreaId);
                    ApplySubmergedPitchBend(hnd);
                }
            }
            else
            {
                if (x790_footstepSfxSel == EFootstepSfx::Left)
                {
                    CSfxHandle hnd = CSfxManager::SfxStart(1484, sfxVol, 0.f, true, 0x7f, false, kInvalidAreaId);
                    ApplySubmergedPitchBend(hnd);
                }
                else
                {
                    CSfxHandle hnd = CSfxManager::SfxStart(1485, sfxVol, 0.f, true, 0x7f, false, kInvalidAreaId);
                    ApplySubmergedPitchBend(hnd);
                }
            }
        }
        else
        {
            u16 sfx;
            if (x790_footstepSfxSel == EFootstepSfx::Left)
                sfx = GetMaterialSoundUnderPlayer(mgr, skLeftStepSounds, 24, -1);
            else
                sfx = GetMaterialSoundUnderPlayer(mgr, skRightStepSounds, 24, -1);
            CSfxHandle hnd = CSfxManager::SfxStart(sfx, sfxVol, 0.f, true, 0x7f, false, kInvalidAreaId);
            ApplySubmergedPitchBend(hnd);
        }

        x78c_footstepSfxTimer = 0.f;
        if (x790_footstepSfxSel == EFootstepSfx::Left)
            x790_footstepSfxSel = EFootstepSfx::Right;
        else
            x790_footstepSfxSel = EFootstepSfx::Left;
    }
}

u16 CPlayer::GetMaterialSoundUnderPlayer(CStateManager& mgr, const u16* table, u32 length, u16 defId)
{
    u16 ret = defId;
    zeus::CAABox aabb = GetBoundingBox();
    aabb.accumulateBounds(x34_transform.origin + zeus::CVector3f::skDown);
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    mgr.BuildNearList(nearList, aabb, SolidMaterialFilter, nullptr);
    TUniqueId collideId = kInvalidUniqueId;
    CRayCastResult result = mgr.RayWorldIntersection(collideId, x34_transform.origin,
                                                     zeus::CVector3f::skDown,
                                                     1.5f, SolidMaterialFilter, nearList);
    if (result.IsValid())
        ret = SfxIdFromMaterial(result.GetMaterial(), table, length, defId);
    return ret;
}

u16 CPlayer::SfxIdFromMaterial(const CMaterialList& mat, const u16* idList, u32 tableLen, u16 defId)
{
    u16 id = defId;
    for (u32 i = 0; i < tableLen; ++i)
    {
        if (mat.HasMaterial(EMaterialTypes(i)) && idList[i] != 0xFFFF)
            id = idList[i];
    }
    return id;
}

void CPlayer::UpdateCrosshairsState(const CFinalInput&) {}

void CPlayer::UpdateVisorTransition(float, CStateManager& mgr) {}

void CPlayer::UpdateVisorState(const CFinalInput&, float, CStateManager& mgr) {}

void CPlayer::UpdateGunState(const CFinalInput&, CStateManager& mgr)
{
}

void CPlayer::ResetGun(CStateManager& mgr)
{
    x498_gunHolsterState = EGunHolsterState::Holstered;
    x49c_gunNotFiringTimeout = 0.f;
    x490_gun->CancelFiring(mgr);
    ResetAimTargetPrediction(kInvalidUniqueId);
}

void CPlayer::UpdateArmAndGunTransforms(float dt, CStateManager& mgr)
{
    zeus::CVector3f grappleOffset;
    zeus::CVector3f gunOffset;
    if (x2f8_morphTransState == EPlayerMorphBallState::Morphed)
    {
        gunOffset = {0.f, 0.f, 0.6f};
    }
    else
    {
        gunOffset = g_tweakPlayerGun->GetGunPosition();
        grappleOffset = x490_gun->GetGrappleArm().IsArmMoving() ?
                        zeus::CVector3f::skZero : g_tweakPlayerGun->GetGrapplingArmPosition();
        gunOffset.z += GetEyeHeight();
        grappleOffset.z += GetEyeHeight();
    }

    UpdateGunTransform(gunOffset + x76c_cameraBob->GetGunBobTransformation().origin, mgr);
    UpdateGrappleArmTransform(grappleOffset, mgr, dt);
}

void CPlayer::ForceGunOrientation(const zeus::CTransform& xf, CStateManager& mgr)
{
    ResetGun(mgr);
    x530_ = xf.basis[1];
    x490_gun->SetTransform(xf);
    UpdateArmAndGunTransforms(0.01f, mgr);
}

void CPlayer::UpdateCameraState(CStateManager& mgr)
{
    UpdateCinematicState(mgr);
}

void CPlayer::UpdateDebugCamera(CStateManager& mgr)
{
    // Empty
}

void CPlayer::UpdateCameraTimers(float dt, const CFinalInput& input)
{
    if (x3dc_inFreeLook || x3dd_freeLookPitchAngleCalculated)
    {
        x294_jumpCameraPitchTimer = 0.f;
        x29c_spaceJumpCameraPitchTimer = 0.f;
        return;
    }

    if (g_tweakPlayer->GetFiringCancelsCameraPitch())
    {
        if (ControlMapper::GetDigitalInput(ControlMapper::ECommands::FireOrBomb, input) ||
            ControlMapper::GetDigitalInput(ControlMapper::ECommands::MissileOrPowerBomb, input))
        {
            if (x288_startingJumpTimeout > 0.f)
            {
                x2a4_cancelCameraPitch = true;
                return;
            }
        }
    }

    if (ControlMapper::GetPressInput(ControlMapper::ECommands::JumpOrBoost, input))
        ++x298_jumpPresses;

    if (ControlMapper::GetDigitalInput(ControlMapper::ECommands::JumpOrBoost, input) &&
        x294_jumpCameraPitchTimer > 0.f && !x2a4_cancelCameraPitch && x298_jumpPresses <= 2)
        x294_jumpCameraPitchTimer += dt;

    if (x29c_spaceJumpCameraPitchTimer > 0.f && !x2a4_cancelCameraPitch)
        x29c_spaceJumpCameraPitchTimer += dt;
}

void CPlayer::UpdateMorphBallState(const CFinalInput&, CStateManager& mgr) {}

CFirstPersonCamera& CPlayer::GetFirstPersonCamera(CStateManager& mgr)
{
    return *mgr.GetCameraManager()->GetFirstPersonCamera();
}

void CPlayer::UpdateGunTransform(const zeus::CVector3f& gunPos, CStateManager& mgr)
{
    float eyeHeight = GetEyeHeight();
    zeus::CTransform camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
    zeus::CTransform gunXf = camXf;

    zeus::CVector3f viewGunPos;
    if (x2f8_morphTransState == EPlayerMorphBallState::Morphing)
        viewGunPos = camXf * (gunPos - zeus::CVector3f(0.f, 0.f, eyeHeight));
    else
        viewGunPos = camXf.rotate(gunPos - zeus::CVector3f(0.f, 0.f, eyeHeight)) + GetEyePosition();

    zeus::CUnitVector3f rightDir(gunXf.basis[0]);
    gunXf.origin = viewGunPos;

    switch (x498_gunHolsterState)
    {
    case EGunHolsterState::Drawing:
    {
        float liftAngle = zeus::clamp(-1.f, x49c_gunNotFiringTimeout / 0.45f, 1.f);
        if (liftAngle > 0.01f)
        {
            gunXf = zeus::CQuaternion::fromAxisAngle(rightDir, -liftAngle *
                g_tweakPlayerGun->GetFixedVerticalAim()).toTransform() *
                    camXf.getRotation();
            gunXf.origin = viewGunPos;
        }
        break;
    }
    case EGunHolsterState::Holstered:
    {
        gunXf = zeus::CQuaternion::fromAxisAngle(rightDir, -g_tweakPlayerGun->GetFixedVerticalAim()).toTransform() *
                camXf.getRotation();
        gunXf.origin = viewGunPos;
        break;
    }
    case EGunHolsterState::Holstering:
    {
        float liftAngle = 1.f -
            zeus::clamp(-1.f, x49c_gunNotFiringTimeout / g_tweakPlayerGun->GetGunHolsterTime(), 1.f);
        if (x2f8_morphTransState == EPlayerMorphBallState::Morphing)
            liftAngle = 1.f - zeus::clamp(-1.f, x49c_gunNotFiringTimeout / 0.1f, 1.f);
        if (liftAngle > 0.01f)
        {
            gunXf = zeus::CQuaternion::fromAxisAngle(rightDir, -liftAngle *
                g_tweakPlayerGun->GetFixedVerticalAim()).toTransform() *
                    camXf.getRotation();
            gunXf.origin = viewGunPos;
        }
        break;
    }
    default:
        break;
    }

    x490_gun->SetTransform(gunXf);
    UpdateAimTargetPrediction(gunXf, mgr);
    UpdateAssistedAiming(gunXf, mgr);
}

void CPlayer::UpdateAssistedAiming(const zeus::CTransform& xf, const CStateManager& mgr)
{

}

void CPlayer::UpdateAimTargetPrediction(const zeus::CTransform& xf, const CStateManager& mgr)
{

}

void CPlayer::ResetAimTargetPrediction(TUniqueId target)
{
    if (target == kInvalidUniqueId || x3f4_aimTarget != target)
        x404_aimTargetAverage.Clear();
    x3f4_aimTarget = target;
}

void CPlayer::DrawGun(CStateManager& mgr)
{
    if (x498_gunHolsterState != EGunHolsterState::Holstered || InGrappleJumpCooldown())
        return;
    x498_gunHolsterState = EGunHolsterState::Drawing;
    x49c_gunNotFiringTimeout = 0.45f;
    x490_gun->ResetIdle(mgr);
}

void CPlayer::HolsterGun(CStateManager& mgr)
{
    if (x498_gunHolsterState == EGunHolsterState::Holstered ||
        x498_gunHolsterState == EGunHolsterState::Holstering)
        return;
    float time = x2f8_morphTransState == EPlayerMorphBallState::Morphing ? 0.1f :
                 g_tweakPlayerGun->GetGunHolsterTime();
    if (x498_gunHolsterState == EGunHolsterState::Drawing)
        x49c_gunNotFiringTimeout = time * (1.f - x49c_gunNotFiringTimeout / 0.45f);
    else
        x49c_gunNotFiringTimeout = time;
    x498_gunHolsterState = EGunHolsterState::Holstering;
    x490_gun->CancelFiring(mgr);
    ResetAimTargetPrediction(kInvalidUniqueId);
}

void CPlayer::UpdateGrappleArmTransform(const zeus::CVector3f&, CStateManager& mgr, float) {}

void CPlayer::ApplyGrappleForces(const CFinalInput& input, CStateManager& mgr, float) {}

bool CPlayer::ValidateFPPosition(const zeus::CVector3f& pos, CStateManager& mgr) { return false; }

void CPlayer::UpdateGrappleState(const CFinalInput& input, CStateManager& mgr)
{
    if (!mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::GrappleBeam) ||
        x2f8_morphTransState == EPlayerMorphBallState::Morphed ||
        mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Scan ||
        mgr.GetPlayerState()->GetTransitioningVisor() == CPlayerState::EPlayerVisor::Scan)
        return;

    if (x310_orbitTargetId == kInvalidUniqueId)
    {
        x3b8_grappleState = EGrappleState::Zero;
        AddMaterial(EMaterialTypes::GroundCollider, mgr);
        return;
    }

    TCastToPtr<CScriptGrapplePoint> point = mgr.ObjectById(x310_orbitTargetId);
    if (point)
    {
        zeus::CVector3f eyePosition = GetEyePosition();
        zeus::CVector3f playerToPoint = point->GetTranslation() - eyePosition;
        zeus::CVector3f playerToPointFlat = playerToPoint;
        playerToPointFlat.z = 0.f;
        if (playerToPoint.canBeNormalized() && playerToPointFlat.canBeNormalized() &&
            playerToPointFlat.magnitude() > 2.f)
        {
            switch (x304_orbitState)
            {
            case EPlayerOrbitState::Five:
                switch (g_tweakPlayer->GetGrappleJumpMode())
                {
                case 0:
                case 1:
                    if (ControlMapper::GetPressInput(ControlMapper::ECommands::FireOrBomb, input))
                    {
                        if (TCastToPtr<CScriptGrapplePoint> point2 = mgr.ObjectById(x33c_))
                        {
                            playerToPoint = point2->GetTranslation() - eyePosition;
                            playerToPoint.z = 0.f;
                            if (playerToPoint.canBeNormalized())
                            {
                                x490_gun->GetGrappleArm().GrappleBeamDisconnected();
                                x3c0_grappleSwingAxis.x = playerToPoint.y;
                                x3c0_grappleSwingAxis.y = -playerToPoint.x;
                                x3c0_grappleSwingAxis.normalize();
                                x3bc_grappleSwingTimer = 0.f;
                                SetOrbitTargetId(x33c_, mgr);
                                x3b8_grappleState = EGrappleState::Two;
                                x33c_ = kInvalidUniqueId;
                                x490_gun->GetGrappleArm().GrappleBeamConnected();
                            }
                        }
                        else
                        {
                            if (!g_tweakPlayer->GetGrappleJumpMode() && x3d8_grappleJumpTimeout <= 0.f)
                                ApplyGrappleJump(mgr);
                            BreakGrapple(EPlayerOrbitRequest::Zero, mgr);
                        }
                    }
                    break;
                default:
                    break;
                }

                break;
            case EPlayerOrbitState::One:
                if (playerToPoint.canBeNormalized())
                {
                    CRayCastResult result =
                    mgr.RayStaticIntersection(eyePosition, playerToPoint.normalized(), playerToPoint.magnitude(),
                                              TargetingFilter);
                    if (result.IsInvalid())
                    {
                        HolsterGun(mgr);
                        switch (x3b8_grappleState)
                        {
                        case EGrappleState::One:
                        case EGrappleState::Three:
                            switch (g_tweakPlayer->GetGrappleJumpMode())
                            {
                            case 0:
                                switch (x490_gun->GetGrappleArm().GetAnimState())
                                {
                                case CGrappleArm::EArmState::One:
                                    if (ControlMapper::GetDigitalInput(ControlMapper::ECommands::FireOrBomb, input))
                                        x490_gun->GetGrappleArm().SetAnimState(CGrappleArm::EArmState::Two);
                                    break;
                                case CGrappleArm::EArmState::Six:
                                    BeginGrapple(playerToPoint, mgr);
                                }
                                break;
                            case 1:
                                if (ControlMapper::GetDigitalInput(ControlMapper::ECommands::FireOrBomb, input))
                                {
                                    switch (x490_gun->GetGrappleArm().GetAnimState())
                                    {
                                    case CGrappleArm::EArmState::One:
                                        x490_gun->GetGrappleArm().SetAnimState(CGrappleArm::EArmState::Two);
                                        break;
                                    case CGrappleArm::EArmState::Six:
                                        BeginGrapple(playerToPoint, mgr);
                                    }
                                    break;
                                }
                                break;
                            case 2:
                                switch (x490_gun->GetGrappleArm().GetAnimState())
                                {
                                case CGrappleArm::EArmState::One:
                                    x490_gun->GetGrappleArm().SetAnimState(CGrappleArm::EArmState::Two);
                                    break;
                                case CGrappleArm::EArmState::Six:
                                    BeginGrapple(playerToPoint, mgr);
                                }
                                break;
                            }
                        case EGrappleState::Zero:
                            x3b8_grappleState = EGrappleState::One;
                            x490_gun->GetGrappleArm().Activate(true);
                            break;
                        default:
                            break;
                        }
                    }
                }
                break;
            default:
                break;
            }
        }
    }

    if (x304_orbitState != EPlayerOrbitState::Five)
    {
        if (x304_orbitState >= EPlayerOrbitState::Five)
            return;
        if (x304_orbitState != EPlayerOrbitState::One)
            return;
    }
    else
    {
        if (!point)
        {
            BreakGrapple(EPlayerOrbitRequest::Three, mgr);
            return;
        }

        switch (g_tweakPlayer->GetGrappleJumpMode())
        {
        case 0:
            if (x3b8_grappleState == EGrappleState::Four)
            {
                x3d8_grappleJumpTimeout -= input.DeltaTime();
                if (x3d8_grappleJumpTimeout <= 0.f)
                {
                    BreakGrapple(EPlayerOrbitRequest::Zero, mgr);
                    SetMoveState(EPlayerMovementState::StartingJump, mgr);
                    ComputeMovement(input, mgr, input.DeltaTime());
                    PreventFallingCameraPitch();
                }
            }
            break;
        case 1:
            switch (x3b8_grappleState)
            {
            case EGrappleState::Three:
                if (!ControlMapper::GetDigitalInput(ControlMapper::ECommands::FireOrBomb, input) &&
                    x3d8_grappleJumpTimeout <= 0.f)
                {
                    x3d8_grappleJumpTimeout = g_tweakPlayer->GetGrappleReleaseTime();
                    x3b8_grappleState = EGrappleState::Four;
                    ApplyGrappleJump(mgr);
                }
                break;
            case EGrappleState::Four:
                x3d8_grappleJumpTimeout -= input.DeltaTime();
                if (x3d8_grappleJumpTimeout <= 0.f)
                {
                    SetMoveState(EPlayerMovementState::StartingJump, mgr);
                    ComputeMovement(input, mgr, input.DeltaTime());
                    BreakGrapple(EPlayerOrbitRequest::Zero, mgr);
                    PreventFallingCameraPitch();
                }
                break;
            case EGrappleState::One:
            case EGrappleState::Two:
                if (!ControlMapper::GetDigitalInput(ControlMapper::ECommands::FireOrBomb, input))
                    BreakGrapple(EPlayerOrbitRequest::Zero, mgr);
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }

        zeus::CVector3f eyePos = GetEyePosition();
        zeus::CVector3f playerToPoint = point->GetTranslation() - eyePos;
        if (playerToPoint.canBeNormalized())
        {
            CRayCastResult result =
            mgr.RayStaticIntersection(eyePos, playerToPoint.normalized(), playerToPoint.magnitude(),
                                      TargetingFilter);
            if (result.IsValid())
            {
                BreakGrapple(EPlayerOrbitRequest::Twelve, mgr);
            }
        }
        return;
    }

    if (x490_gun->GetGrappleArm().BeamActive() && g_tweakPlayer->GetGrappleJumpMode() == 1 &&
        !ControlMapper::GetDigitalInput(ControlMapper::ECommands::FireOrBomb, input))
        BreakGrapple(EPlayerOrbitRequest::Zero, mgr);
}

void CPlayer::ApplyGrappleJump(CStateManager& mgr)
{
    if (TCastToPtr<CScriptGrapplePoint> point = mgr.ObjectById(x310_orbitTargetId))
    {
        zeus::CVector3f tmp = x3c0_grappleSwingAxis;
        if (x3bc_grappleSwingTimer < 0.5f * g_tweakPlayer->GetGrappleSwingPeriod())
            tmp *= zeus::CVector3f::skNegOne;
        zeus::CVector3f pointToPlayer = GetTranslation() - point->GetTranslation();
        zeus::CVector3f cross = pointToPlayer.normalized().cross(tmp);
        zeus::CVector3f pointToPlayerFlat(pointToPlayer.x, pointToPlayer.y, 0.f);
        float dot = 1.f;
        if (pointToPlayerFlat.canBeNormalized() && cross.canBeNormalized())
            dot = zeus::clamp(-1.f, std::fabs(cross.normalized().dot(pointToPlayerFlat.normalized())), 1.f);
        ApplyForceWR(g_tweakPlayer->GetGrappleJumpForce() * cross * 10000.f * dot, zeus::CAxisAngle::sIdentity);
    }
}

void CPlayer::BeginGrapple(zeus::CVector3f& vec, CStateManager& mgr)
{
    vec.z = 0.f;
    if (vec.canBeNormalized())
    {
        x3c0_grappleSwingAxis.x = vec.y;
        x3c0_grappleSwingAxis.y = -vec.x;
        x3c0_grappleSwingAxis.normalize();
        x3bc_grappleSwingTimer = 0.f;
        SetOrbitState(EPlayerOrbitState::Five, mgr);
        x3b8_grappleState = EGrappleState::Two;
        RemoveMaterial(EMaterialTypes::GroundCollider, mgr);
    }
}

void CPlayer::BreakGrapple(EPlayerOrbitRequest req, CStateManager& mgr)
{
    x294_jumpCameraPitchTimer = 0.f;
    x29c_spaceJumpCameraPitchTimer = 0.f;
    if (g_tweakPlayer->GetGrappleJumpMode() == 2 && x3b8_grappleState == EGrappleState::Three)
    {
        ApplyGrappleJump(mgr);
        PreventFallingCameraPitch();
    }

    SetOrbitRequest(req, mgr);
    x3b8_grappleState = EGrappleState::Zero;
    AddMaterial(EMaterialTypes::GroundCollider, mgr);
    x490_gun->GetGrappleArm().SetAnimState(CGrappleArm::EArmState::Eight);
    if (!InGrappleJumpCooldown() && x3b8_grappleState != EGrappleState::Four)
        DrawGun(mgr);
}

void CPlayer::SetOrbitRequest(EPlayerOrbitRequest req, CStateManager& mgr)
{
    x30c_orbitRequest = req;
    switch (req)
    {
    case EPlayerOrbitRequest::Eight:

    case EPlayerOrbitRequest::Seven:
        SetOrbitState(EPlayerOrbitState::Two, mgr);
        x314_orbitPoint = g_tweakPlayer->GetX164(int(x308_orbitType)) *
            x34_transform.basis[1] + x34_transform.origin;
        break;
    default:
        SetOrbitState(EPlayerOrbitState::Zero, mgr);
        break;
    }
}

bool CPlayer::InGrappleJumpCooldown() const
{
    if (x258_movementState == EPlayerMovementState::OnGround)
        return false;
    return x3d8_grappleJumpTimeout > 0.f || x294_jumpCameraPitchTimer == 0.f;
}

void CPlayer::PreventFallingCameraPitch()
{
    x294_jumpCameraPitchTimer = 0.f;
    x29c_spaceJumpCameraPitchTimer = 0.01f;
    x2a4_cancelCameraPitch = true;
}

void CPlayer::OrbitCarcass(CStateManager& mgr)
{
    if (x304_orbitState == EPlayerOrbitState::One)
    {
        x308_orbitType = EPlayerOrbitType::Two;
        SetOrbitState(EPlayerOrbitState::Three, mgr);
    }
}

void CPlayer::OrbitPoint(EPlayerOrbitType type, CStateManager& mgr)
{
    x308_orbitType = type;
    SetOrbitState(EPlayerOrbitState::Two, mgr);
    SetOrbitPosition(g_tweakPlayer->GetX164(int(x308_orbitType)), mgr);
}

zeus::CVector3f CPlayer::GetHUDOrbitTargetPosition() const
{
    return x314_orbitPoint + x76c_cameraBob->GetCameraBobTransformation().origin;
}

void CPlayer::SetOrbitState(EPlayerOrbitState state, CStateManager& mgr)
{
    x304_orbitState = state;
    CFirstPersonCamera* cam = mgr.GetCameraManager()->GetFirstPersonCamera();
    switch (x304_orbitState)
    {
    case EPlayerOrbitState::One:
        cam->SetX18C(false);
        break;
    case EPlayerOrbitState::Three:
    {
        cam->SetX18C(true);
        zeus::CVector3f playerToPoint = x314_orbitPoint - GetTranslation();
        if (playerToPoint.canBeNormalized())
            x340_ = playerToPoint.magnitude();
        else
            x340_ = 0.f;
        SetOrbitTargetId(kInvalidUniqueId, mgr);
        x33c_ = kInvalidUniqueId;
        break;
    }
    case EPlayerOrbitState::Zero:
        x32c_orbitDistance = g_tweakPlayer->GetOrbitNormalDistance();
        x32c_orbitDistance = 0.28f;
        SetOrbitTargetId(kInvalidUniqueId, mgr);
        x33c_ = kInvalidUniqueId;
        break;
    case EPlayerOrbitState::Two:
        SetOrbitTargetId(kInvalidUniqueId, mgr);
        x33c_ = kInvalidUniqueId;
        break;
    default:
        break;
    }
}

void CPlayer::SetOrbitTargetId(TUniqueId id, CStateManager& mgr)
{
    if (id != kInvalidUniqueId)
    {
        x394_orbitingEnemy = (TCastToPtr<CPatterned>(mgr.ObjectById(id)) ||
                              TCastToPtr<CWallCrawlerSwarm>(mgr.ObjectById(id)) ||
                              CPatterned::CastTo<CThardusRockProjectile>(mgr.ObjectById(id)) ||
                              TCastToPtr<CScriptGunTurret>(mgr.ObjectById(id)));
    }

    x310_orbitTargetId = id;
    if (x310_orbitTargetId == kInvalidUniqueId)
        x374_ = false;
}

void CPlayer::UpdateOrbitPosition(float pos, CStateManager& mgr)
{
    switch (x304_orbitState)
    {
    case EPlayerOrbitState::Two:
    case EPlayerOrbitState::Three:
        SetOrbitPosition(pos, mgr);
        break;
    case EPlayerOrbitState::One:
    case EPlayerOrbitState::Four:
    case EPlayerOrbitState::Five:
        if (TCastToPtr<CActor> act = mgr.ObjectById(x310_orbitTargetId))
            if (x310_orbitTargetId != kInvalidUniqueId)
                x314_orbitPoint = act->GetOrbitPosition(mgr);
        break;
    default: break;
    }
}

void CPlayer::UpdateOrbitZPosition()
{
    if (x304_orbitState == EPlayerOrbitState::Two)
    {
        if (std::fabs(x320_orbitVector.z) < g_tweakPlayer->GetOrbitZRange())
            x314_orbitPoint.z = x320_orbitVector.z + x34_transform.origin.z + GetEyeHeight();
    }
}

void CPlayer::UpdateOrbitFixedPosition()
{
    x314_orbitPoint = x34_transform.rotate(x320_orbitVector) + GetEyePosition();
}

void CPlayer::SetOrbitPosition(float pos, CStateManager& mgr)
{
    zeus::CTransform camXf = GetFirstPersonCameraTransform(mgr);
    if (x304_orbitState == EPlayerOrbitState::Two && x30c_orbitRequest == EPlayerOrbitRequest::Seven)
        camXf = x34_transform;
    zeus::CVector3f fwd = camXf.basis[1];
    float dot = fwd.normalized().dot(fwd);
    if (std::fabs(dot) > 1.f)
        dot = (dot > 0.f) ? 1.f : -1.f;
    x314_orbitPoint = camXf.rotate(zeus::CVector3f(0.f, pos / dot, 0.f)) + camXf.origin;
    x320_orbitVector = zeus::CVector3f(0.f, pos, x314_orbitPoint.z - camXf.origin.z);
}

void CPlayer::UpdateAimTarget(CStateManager& mgr) {}

void CPlayer::UpdateAimTargetTimer(float) {}

bool CPlayer::ValidateAimTargetId(TUniqueId, CStateManager& mgr) { return false; }

bool CPlayer::ValidateObjectForMode(TUniqueId, CStateManager& mgr) const { return false; }

TUniqueId CPlayer::FindAimTargetId(CStateManager& mgr) { return {}; }

const zeus::CTransform& CPlayer::GetFirstPersonCameraTransform(const CStateManager& mgr) const
{
    return mgr.GetCameraManager()->GetFirstPersonCamera()->GetGunFollowTransform();
}

TUniqueId CPlayer::CheckEnemiesAgainstOrbitZone(const std::vector<TUniqueId>&, EPlayerZoneInfo, EPlayerZoneType,
                                                CStateManager& mgr) const
{
    return {};
}

TUniqueId CPlayer::FindOrbitTargetId(CStateManager& mgr)
{
    return FindBestOrbitableObject(x354_onScreenOrbitObjects, x330_orbitZone, mgr);
}

static zeus::CAABox BuildNearListBox(bool cropBottom, const zeus::CTransform& xf, float x, float z, float y)
{
    zeus::CAABox aabb(-x, cropBottom ? 0.f : -y, -z, x, y, z);
    return aabb.getTransformedAABox(xf);
}

void CPlayer::UpdateOrbitableObjects(CStateManager& mgr)
{
    x354_onScreenOrbitObjects.clear();
    x344_nearbyOrbitObjects.clear();
    x354_onScreenOrbitObjects.clear();

    if (CheckOrbitDisableSourceList(mgr))
        return;

    float dist = GetOrbitMaxTargetDistance(mgr);
    if (x9c6_24_)
        dist *= 5.f;
    zeus::CAABox nearAABB =
        BuildNearListBox(true, GetFirstPersonCameraTransform(mgr),
                         g_tweakPlayer->GetOrbitNearX(), g_tweakPlayer->GetOrbitNearZ(), dist);

    CMaterialFilter filter = mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Scan ?
    CMaterialFilter::MakeInclude({EMaterialTypes::Scannable}) : CMaterialFilter::MakeInclude({EMaterialTypes::Orbit});
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    mgr.BuildNearList(nearList, nearAABB, filter, nullptr);

    FindOrbitableObjects(nearList, x344_nearbyOrbitObjects, x330_orbitZone, EPlayerZoneType::Always, mgr, true);
    FindOrbitableObjects(nearList, x354_onScreenOrbitObjects, x330_orbitZone, x334_orbitType, mgr, true);
    FindOrbitableObjects(nearList, x364_offScreenOrbitObjects, x330_orbitZone, x334_orbitType, mgr, false);
}

TUniqueId CPlayer::FindBestOrbitableObject(const std::vector<TUniqueId>&, EPlayerZoneInfo, CStateManager& mgr) const
{
    zeus::CVector3f eyePos = GetEyePosition();
    zeus::CVector3f lookDir = x34_transform.basis[1].normalized();
    /* TODO: Finish */
    return {};
}

void CPlayer::FindOrbitableObjects(const rstl::reserved_vector<TUniqueId, 1024>& nearObjects,
                                   std::vector<TUniqueId>& listOut, EPlayerZoneInfo zone,
                                   EPlayerZoneType type, CStateManager& mgr, bool onScreenTest) const
{
    CFirstPersonCamera* fpCam = mgr.GetCameraManager()->GetFirstPersonCamera();
    zeus::CVector3f eyePos = GetEyePosition();

    for (TUniqueId id : nearObjects)
    {
        if (TCastToConstPtr<CActor> act = mgr.GetObjectById(id))
        {
            if (GetUniqueId() == act->GetUniqueId())
                continue;
            if (ValidateOrbitTargetId(act->GetUniqueId(), mgr) != EOrbitObjectType::Zero)
                continue;
            zeus::CVector3f orbitPos = act->GetOrbitPosition(mgr);
            zeus::CVector3f screenPos = fpCam->ConvertToScreenSpace(orbitPos);
            screenPos.x = g_Viewport.x8_width * screenPos.x / 2.f + g_Viewport.x8_width / 2.f;
            screenPos.y = g_Viewport.xc_height * screenPos.y / 2.f + g_Viewport.xc_height / 2.f;

            bool pass = false;
            if (onScreenTest)
            {
                if (WithinOrbitScreenBox(screenPos, zone, type))
                    pass = true;
            }
            else
            {
                if (!WithinOrbitScreenBox(screenPos, zone, type))
                    pass = true;
            }

            if (pass && (!act->GetDoTargetDistanceTest() || (orbitPos - eyePos).magnitude() <= GetOrbitMaxTargetDistance(mgr)))
                listOut.push_back(id);
        }
    }
}

bool CPlayer::WithinOrbitScreenBox(const zeus::CVector3f& screenCoords, EPlayerZoneInfo zone, EPlayerZoneType type) const
{
    if (screenCoords.z >= 1.f)
        return false;

    switch (type)
    {
    case EPlayerZoneType::Box:
        return std::fabs(screenCoords.x - g_tweakPlayer->GetOrbitScreenBoxCenterX(int(zone))) <
                   g_tweakPlayer->GetOrbitScreenBoxHalfExtentX(int(zone)) &&
               std::fabs(screenCoords.y - g_tweakPlayer->GetOrbitScreenBoxCenterY(int(zone))) <
                   g_tweakPlayer->GetOrbitScreenBoxHalfExtentY(int(zone)) &&
               screenCoords.z < 1.f;
        break;
    case EPlayerZoneType::Ellipse:
        return WithinOrbitScreenEllipse(screenCoords, zone);
    default:
        return true;
    }

    return false;
}

bool CPlayer::WithinOrbitScreenEllipse(const zeus::CVector3f& screenCoords, EPlayerZoneInfo zone) const
{
    if (screenCoords.z >= 1.f)
        return false;

    float heYSq = g_tweakPlayer->GetOrbitScreenBoxHalfExtentY(int(zone));
    heYSq *= heYSq;
    float heXSq = g_tweakPlayer->GetOrbitScreenBoxHalfExtentX(int(zone));
    heXSq *= heXSq;
    float tmpY = std::fabs(screenCoords.y - g_tweakPlayer->GetOrbitScreenBoxCenterY(int(zone)));
    float tmpX = std::fabs(screenCoords.x - g_tweakPlayer->GetOrbitScreenBoxCenterX(int(zone)));
    return tmpX * tmpX <= (1.f - tmpY * tmpY / heYSq) * heXSq;
}

bool CPlayer::CheckOrbitDisableSourceList(CStateManager& mgr)
{
    for (auto it = x9e4_orbitDisableList.begin() ; it != x9e4_orbitDisableList.end() ;)
    {
        if (!mgr.GetObjectById(*it))
        {
            it = x9e4_orbitDisableList.erase(it);
            continue;
        }
        ++it;
    }
    return x9e4_orbitDisableList.size() != 0;
}

void CPlayer::RemoveOrbitDisableSource(TUniqueId uid)
{
    for (auto it = x9e4_orbitDisableList.begin() ; it != x9e4_orbitDisableList.end() ;)
    {
        if (*it == uid)
        {
            it = x9e4_orbitDisableList.erase(it);
            return;
        }
        ++it;
    }
}

void CPlayer::AddOrbitDisableSource(CStateManager& mgr, TUniqueId addId)
{
    if (x9e4_orbitDisableList.size() >= 5)
        return;
    for (TUniqueId uid : x9e4_orbitDisableList)
        if (uid == addId)
            return;
    x9e4_orbitDisableList.push_back(addId);
    ResetAimTargetPrediction(kInvalidUniqueId);
    if (!TCastToConstPtr<CScriptGrapplePoint>(mgr.GetObjectById(x310_orbitTargetId)))
        SetOrbitTargetId(kInvalidUniqueId, mgr);
}

void CPlayer::UpdateOrbitPreventionTimer(float) {}

void CPlayer::UpdateOrbitModeTimer(float) {}

void CPlayer::UpdateOrbitZone(CStateManager& mgr) {}

void CPlayer::UpdateOrbitInput(const CFinalInput& input, CStateManager& mgr) {}

void CPlayer::ActivateOrbitSource(CStateManager& mgr)
{
    switch (x390_orbitSource)
    {
    default:
        OrbitCarcass(mgr);
        break;
    case 1:
        SetOrbitRequest(EPlayerOrbitRequest::Six, mgr);
        break;
    case 2:
        if (x394_orbitingEnemy)
            OrbitPoint(EPlayerOrbitType::One, mgr);
        else
            OrbitCarcass(mgr);
        break;
    }
}

void CPlayer::UpdateOrbitSelection(const CFinalInput& input, CStateManager& mgr) {}

void CPlayer::UpdateOrbitOrientation(CStateManager& mgr) {}

void CPlayer::UpdateOrbitTarget(CStateManager& mgr) {}

float CPlayer::GetOrbitMaxLockDistance(CStateManager& mgr) const { return 0.f; }

float CPlayer::GetOrbitMaxTargetDistance(CStateManager& mgr) const { return 0.f; }

CPlayer::EOrbitObjectType CPlayer::ValidateOrbitTargetId(TUniqueId, CStateManager& mgr) const
{
    return EOrbitObjectType::Zero;
}

CPlayer::EOrbitObjectType CPlayer::ValidateCurrentOrbitTargetId(CStateManager& mgr)
{
    return EOrbitObjectType::Zero;
}

bool CPlayer::ValidateOrbitTargetIdAndPointer(TUniqueId, CStateManager& mgr) const { return false; }

zeus::CVector3f CPlayer::GetBallPosition() const { return {}; }

zeus::CVector3f CPlayer::GetEyePosition() const { return {}; }

float CPlayer::GetEyeHeight() const { return 0.f; }

float CPlayer::GetStepUpHeight() const { return 0.f; }

float CPlayer::GetStepDownHeight() const { return 0.f; }

void CPlayer::Teleport(const zeus::CTransform& xf, CStateManager& mgr, bool) {}

void CPlayer::BombJump(const zeus::CVector3f& pos, CStateManager& mgr) {}

zeus::CTransform CPlayer::CreateTransformFromMovementDirection() const { return {}; }

const CCollisionPrimitive* CPlayer::GetCollisionPrimitive() const
{
    switch (x2f8_morphTransState)
    {
    case EPlayerMorphBallState::Morphed:
        return GetCollidableSphere();
    default:
        return CPhysicsActor::GetCollisionPrimitive();
    }
}

const CCollidableSphere* CPlayer::GetCollidableSphere() const
{
    return x768_morphball->GetCollidableSphere();
}

zeus::CTransform CPlayer::GetPrimitiveTransform() const { return {}; }

void CPlayer::CollidedWith(TUniqueId, const CCollisionInfoList&, CStateManager& mgr) {}

float CPlayer::GetActualFirstPersonMaxVelocity() const { return 0.f; }

void CPlayer::SetMoveState(EPlayerMovementState, CStateManager& mgr) {}

float CPlayer::JumpInput(const CFinalInput& input, CStateManager& mgr) { return 0.f; }

float CPlayer::TurnInput(const CFinalInput& input) const { return 0.f; }

float CPlayer::StrafeInput(const CFinalInput& input) const { return 0.f; }

float CPlayer::ForwardInput(const CFinalInput& input, float) const { return 0.f; }

void CPlayer::ComputeMovement(const CFinalInput& input, CStateManager& mgr, float) {}

float CPlayer::GetWeight() const { return 0.f; }

zeus::CVector3f CPlayer::GetDampedClampedVelocityWR() const { return {}; }

void CPlayer::UpdateCinematicState(CStateManager& mgr)
{
    if (mgr.GetCameraManager()->IsInCinematicCamera())
    {
        switch (x2f4_cameraState)
        {

        }
    }
}

void CPlayer::SetCameraState(EPlayerCameraState camState, CStateManager& stateMgr)
{
    if (x2f4_cameraState == camState)
        return;
    x2f4_cameraState = camState;
    CCameraManager* camMgr = stateMgr.GetCameraManager();
    switch (camState)
    {
    case EPlayerCameraState::Zero:
        camMgr->SetCurrentCameraId(camMgr->GetFirstPersonCamera()->GetUniqueId(), stateMgr);
        x768_morphball->SetBallLightActive(stateMgr, false);
        break;
    case EPlayerCameraState::One:
    case EPlayerCameraState::Three:
        camMgr->SetCurrentCameraId(camMgr->GetBallCamera()->GetUniqueId(), stateMgr);
        x768_morphball->SetBallLightActive(stateMgr, true);
        break;
    case EPlayerCameraState::Two:
        break;
    case EPlayerCameraState::Four:
    {
        bool ballLight = false;
        if (TCastToPtr<CCinematicCamera> cineCam = camMgr->GetCurrentCamera(stateMgr))
            ballLight = x2f8_morphTransState == EPlayerMorphBallState::Morphed && cineCam->GetW1() & 0x40;
        x768_morphball->SetBallLightActive(stateMgr, ballLight);
        break;
    }
    }
}

bool CPlayer::IsEnergyLow(const CStateManager& mgr) const
{
    float lowThreshold = mgr.GetPlayerState()->GetItemCapacity(CPlayerState::EItemType::EnergyTanks) < 4 ? 30.f : 100.f;
    return HealthInfo(mgr)->GetHP() < lowThreshold;
}

bool CPlayer::ObjectInScanningRange(TUniqueId id, const CStateManager& mgr) const
{
    const CEntity* ent = mgr.GetObjectById(id);
    if (TCastToConstPtr<CActor> act = ent)
    {
        zeus::CVector3f delta = act->GetTranslation() - GetTranslation();
        if (delta.canBeNormalized())
            return delta.magnitude() < g_tweakPlayer->GetScanningRange();
    }
    return false;
}

void CPlayer::SetPlayerHitWallDuringMove()
{
    x9c5_29_hitWall = true;
    x2d0_ = 1;
}

void CPlayer::Touch() {}

void CPlayer::CVisorSteam::SetSteam(float a, float b, float c, ResId d, bool e)
{
    if (x1c_ == kInvalidResId || a > x10_)
    {
        x10_ = a;
        x14_ = b;
        x18_ = c;
        x1c_ = d;
    }
    x28_ = e;
}

ResId CPlayer::CVisorSteam::GetTextureId() const { return xc_tex; }

void CPlayer::CVisorSteam::Update(float dt)
{
    if (x1c_ == kInvalidResId)
        x0_ = 0.f;
    else
    {
        x0_ = x10_;
        x4_ = x14_;
        x8_ = x18_;
        xc_tex = x1c_;
    }

    x1c_ = kInvalidResId;
    if ((x20_alpha - x0_) < 0.000009999f || std::fabs(x20_alpha) > 0.000009999f)
        return;

    if (x20_alpha > x0_)
    {
        if (x24_ <= 0.f)
        {
            x20_alpha -= (dt / x8_);
            x20_alpha = std::min(x20_alpha, x0_);
        }
        else
        {
            x24_ = x0_ - dt;
            x24_ = zeus::max(0.f, x24_);
        }
        return;
    }

    CToken tmpTex = g_SimplePool->GetObj({SBIG('TXTR'), xc_tex});
    if (!tmpTex)
        return;

    x20_alpha += (x20_alpha + (dt / x4_));
    if (x20_alpha > x0_)
        x20_alpha = x0_;

    x24_ = 0.1f;
}

void CPlayer::CFailsafeTest::Reset()
{
    x0_stateSamples.clear();
    x54_posSamples.clear();
    x148_velSamples.clear();
    x23c_inputSamples.clear();
}

void CPlayer::CFailsafeTest::AddSample(EInputState state, const zeus::CVector3f& pos,
                                      const zeus::CVector3f& vel, const zeus::CVector2f& input)
{
    if (x0_stateSamples.size() >= 20)
        x0_stateSamples.resize(19);
    x0_stateSamples.insert(x0_stateSamples.begin(), state);
    if (x54_posSamples.size() >= 20)
        x54_posSamples.resize(19);
    x54_posSamples.insert(x54_posSamples.begin(), pos);
    if (x148_velSamples.size() >= 20)
        x148_velSamples.resize(19);
    x148_velSamples.insert(x148_velSamples.begin(), vel);
    if (x23c_inputSamples.size() >= 20)
        x23c_inputSamples.resize(19);
    x23c_inputSamples.insert(x23c_inputSamples.begin(), input);
}

bool CPlayer::CFailsafeTest::Passes() const
{
    if (x0_stateSamples.size() != 20)
        return false;
    float posMag = 0.f;

    zeus::CAABox velAABB(x148_velSamples[0], x148_velSamples[0]);
    zeus::CAABox posAABB(x54_posSamples[0], x54_posSamples[0]);
    zeus::CVector3f inputVec(x23c_inputSamples[0].x, x23c_inputSamples[0].y, 0.f);
    zeus::CAABox inputAABB(inputVec, inputVec);

    float maxVelMag = x148_velSamples[0].magnitude();
    float minVelMag = maxVelMag;
    u32 notEqualStates = 0;

    for (int i=1 ; i<20 ; ++i)
    {
        float mag = (x54_posSamples[i-1] - x54_posSamples[i]).magSquared();
        if (mag > FLT_EPSILON)
            posMag += std::sqrt(mag);

        posAABB.accumulateBounds(x54_posSamples[i]);
        velAABB.accumulateBounds(x148_velSamples[i]);
        float velMag = x148_velSamples[i].magnitude();
        minVelMag = std::min(minVelMag, velMag);
        maxVelMag = std::max(maxVelMag, velMag);

        zeus::CVector3f inputVec2(x23c_inputSamples[i].x, x23c_inputSamples[i].y, 0.f);
        inputAABB.accumulateBounds(inputVec2);

        if (x0_stateSamples[i] != x0_stateSamples[i-1])
            notEqualStates += 1;
    }

    bool test1 = true;
    if (posMag >= 1.f / 30.f && posMag >= minVelMag / 30.f)
        test1 = false;

    if (!notEqualStates && x0_stateSamples[0] == EInputState::StartingJump)
    {
        float inputMag = (inputAABB.max - inputAABB.min).magnitude();
        zeus::CAABox inputFrom0AABB(inputAABB);
        inputFrom0AABB.accumulateBounds(zeus::CVector3f::skZero);
        bool test2 = true;
        if ((inputFrom0AABB.max - inputFrom0AABB.min).magnitude() >= 0.01f &&
            inputMag <= 1.5f)
            test2 = false;
        return test1 && test2;
    }

    return false;
}

void CPlayer::SetSpawnedMorphBallState(CPlayer::EPlayerMorphBallState, CStateManager&) {}

void CPlayer::DecrementPhazon()
{
    if (xa10_phazonCounter == 0)
        return;

    xa10_phazonCounter--;
}

void CPlayer::IncrementPhazon()
{
    if (xa10_phazonCounter != 0)
        xa10_phazonCounter++;
    else
        xa14_ = 0.f;
}

bool CPlayer::CheckSubmerged() const
{
    if (!xe6_24_fluidCounter)
        return false;

    return x828_waterLevelOnPlayer >= (x2f8_morphTransState == EPlayerMorphBallState::Morphed ?
        2.f * g_tweakPlayer->GetPlayerBallHalfExtent() : 0.5f * GetEyeHeight());
}

void CPlayer::UpdateSubmerged(CStateManager& mgr)
{
    x82c_inLava = false;
    x828_waterLevelOnPlayer = 0.f;
    if (xe6_24_fluidCounter)
    {
        if (TCastToPtr<CScriptWater> water = mgr.ObjectById(xc4_fluidId))
        {
            x828_waterLevelOnPlayer =
                -(zeus::CVector3f::skUp.dot(x34_transform.origin) - water->GetTriggerBoundsWR().max.z);
            CFluidPlane::EFluidType fluidType = water->GetFluidPlane().GetFluidType();
            x82c_inLava = (fluidType == CFluidPlane::EFluidType::Two || fluidType == CFluidPlane::EFluidType::Five);
            CheckSubmerged();
        }
    }
}

void CPlayer::ApplySubmergedPitchBend(CSfxHandle& sfx)
{
    if (CheckSubmerged())
        CSfxManager::PitchBend(sfx, -1.f);
}

}
