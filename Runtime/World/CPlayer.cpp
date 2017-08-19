#include "CPlayer.hpp"
#include "CActorParameters.hpp"
#include "CStateManager.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"
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
#include "Audio/CStreamAudioManager.hpp"
#include "CScriptPlayerHint.hpp"

namespace urde
{

static const CMaterialFilter SolidMaterialFilter =
    CMaterialFilter::MakeInclude(CMaterialList(EMaterialTypes::Solid));

static const CMaterialFilter LineOfSightFilter =
    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid},
                                        {EMaterialTypes::ProjectilePassthrough,
                                         EMaterialTypes::ScanPassthrough,
                                         EMaterialTypes::Player});

static const CMaterialFilter OccluderFilter =
    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid, EMaterialTypes::Occluder},
        {EMaterialTypes::ProjectilePassthrough, EMaterialTypes::ScanPassthrough, EMaterialTypes::Player});

static CModelData MakePlayerAnimRes(CAssetId resId, const zeus::CVector3f& scale)
{
    return {CAnimRes(resId, 0, scale, 0, true), 1};
}

CPlayer::CPlayer(TUniqueId uid, const zeus::CTransform& xf, const zeus::CAABox& aabb, CAssetId resId,
                 const zeus::CVector3f& playerScale, float mass, float stepUp, float stepDown, float ballRadius,
                 const CMaterialList& ml)
: CPhysicsActor(uid, true, "CPlayer", CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList), xf,
                MakePlayerAnimRes(resId, playerScale), ml, aabb, SMoverData(mass), CActorParameters::None(), stepUp,
                stepDown), x7d0_animRes(resId, 0, playerScale, 0, true), x7d8_beamScale(playerScale)
{
    x490_gun.reset(new CPlayerGun(uid));
    x49c_gunHolsterRemTime = g_tweakPlayerGun->GetGunNotFiringTime();
    x4a0_failsafeTest.reset(new CFailsafeTest());
    x76c_cameraBob.reset(new CPlayerCameraBob(CPlayerCameraBob::ECameraBobType::One,
                                              zeus::CVector2f{CPlayerCameraBob::kCameraBobExtentX,
                                                              CPlayerCameraBob::kCameraBobExtentY},
                                              CPlayerCameraBob::kCameraBobPeriod));
    x9c4_26_ = true;
    x9c4_27_canEnterMorphBall = true;
    x9c4_28_canLeaveMorphBall = true;
    x9c5_31_stepCameraZBiasDirty = true;
    CAssetId beamId = g_tweakPlayerRes->GetBeamBallTransitionModel(x7ec_beam);
    x7f0_ballTransitionBeamModel = std::make_unique<CModelData>(CStaticRes(beamId, playerScale));
    x730_transitionModels.reserve(3);
    x768_morphball.reset(new CMorphBall(*this, ballRadius));

    SetInertiaTensorScalar(xe8_mass);
    x1f4_lastNonCollidingState = GetMotionState();
    x490_gun->SetTransform(x34_transform);
    x490_gun->GetGrappleArm().SetTransform(x34_transform);

    InitializeBallTransition();
    zeus::CAABox ballTransAABB = x64_modelData->GetBounds();
    x2f0_ballTransHeight = ballTransAABB.max.z - ballTransAABB.min.z;

    SetCalculateLighting(true);

    x90_actorLights->SetCastShadows(true);
    x50c_moveDir.z = 0.f;
    if (x50c_moveDir.canBeNormalized())
        x50c_moveDir.normalize();
    x2b4_accelerationTable.push_back(20.f);
    x2b4_accelerationTable.push_back(80.f);
    x2b4_accelerationTable.push_back(80.f);
    x2b4_accelerationTable.push_back(270.f);
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

float CPlayer::GetTransitionAlpha(const zeus::CVector3f& camPos, float zNear) const
{
    float zLimit = (x2d8_fpBounds.max.x - x2d8_fpBounds.min.x) * 0.5f + zNear;
    float zStart = 1.f + zLimit;
    float dist = (camPos - GetEyePosition()).magnitude();
    if (dist >= zLimit && dist <= zStart)
        return (dist - zLimit) / (zStart - zLimit);
    else if (dist > zStart)
        return 1.f;
    else
        return 0.f;
}

static const CMaterialFilter BallTransitionCollide =
    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid},
                                        {EMaterialTypes::ProjectilePassthrough,
                                         EMaterialTypes::Player,
                                         EMaterialTypes::Character,
                                         EMaterialTypes::CameraPassthrough});

s32 CPlayer::ChooseTransitionToAnimation(float dt, CStateManager& mgr) const
{
    if (x258_movementState == EPlayerMovementState::StartingJump)
        return 3; // B_airposetoball_samus
    zeus::CVector3f localVel = x34_transform.transposeRotate(x138_velocity);
    zeus::CVector3f localVelFlat = localVel;
    localVelFlat.z = 0.f;
    float localVelFlatMag = localVelFlat.magnitude();
    if (localVelFlatMag > 1.f)
    {
        zeus::CRelAngle velAng = std::atan2(localVelFlat.x, localVelFlat.y);
        float velDeg = velAng.asDegrees();
        if (velDeg < 45.f || velDeg > 315.f)
        {
            if (localVelFlatMag < 0.5f * GetActualFirstPersonMaxVelocity(dt))
                return 0; // B_forwardtoballforward_samus
            else
                return 4; // B_runtoballfoward_samus
        }
        else
        {
            return 1; // B_readytostationarybackwards_samus
        }
    }
    else
    {
        return 2; // B_readytoball_samus
    }
}

void CPlayer::TransitionToMorphBallState(float dt, CStateManager& mgr)
{
    x584_ballTransitionAnim = ChooseTransitionToAnimation(dt, mgr);
    x58c_transitionVel = x138_velocity.magnitude();
    if (x64_modelData && x64_modelData->HasAnimData())
    {
        CAnimPlaybackParms parms(x584_ballTransitionAnim, -1, 1.f, true);
        x64_modelData->AnimationData()->SetAnimation(parms, false);
        x64_modelData->AnimationData()->SetAnimDir(CAnimData::EAnimDir::Forward);

    }
    x64_modelData->EnableLooping(false);
    x64_modelData->Touch(mgr, 0);
    x150_momentum = zeus::CVector3f::skZero;
    CPhysicsActor::Stop();
    SetMorphBallState(EPlayerMorphBallState::Morphing, mgr);
    SetCameraState(EPlayerCameraState::Transitioning, mgr);
    x500_lookDir = x34_transform.basis[1];
    x50c_moveDir = x500_lookDir;
    x50c_moveDir.z = 0.f;
    if (x50c_moveDir.canBeNormalized())
    {
        x50c_moveDir.normalize();
    }
    else
    {
        x500_lookDir = zeus::CVector3f::skForward;
        x50c_moveDir = zeus::CVector3f::skForward;
    }
    CBallCamera* ballCam = mgr.GetCameraManager()->GetBallCamera();
    mgr.GetCameraManager()->SetPlayerCamera(mgr, ballCam->GetUniqueId());
    if (!mgr.GetCameraManager()->HasBallCameraInitialPositionHint(mgr))
    {
        mgr.GetCameraManager()->SetupBallCamera(mgr);
        ballCam->SetState(CBallCamera::EBallCameraState::Four, mgr);
    }
    else
    {
        ballCam->SetState(CBallCamera::EBallCameraState::Zero, mgr);
        SetCameraState(EPlayerCameraState::Ball, mgr);
        zeus::CTransform newXf = mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform();
        ballCam->SetTransform(newXf);
        ballCam->TeleportCamera(newXf.origin, mgr);
        mgr.GetCameraManager()->SetupBallCamera(mgr);
        ballCam->SetFovInterpolation(mgr.GetCameraManager()->GetFirstPersonCamera()->GetFov(),
                                     CCameraManager::ThirdPersonFOV(), 1.f, 0.f);
    }
    SetOrbitRequest(EPlayerOrbitRequest::Two, mgr);
    x490_gun->CancelFiring(mgr);
    HolsterGun(mgr);
}

void CPlayer::TransitionFromMorphBallState(CStateManager& mgr)
{
    x584_ballTransitionAnim = 14; // B_ball_unfurl
    x58c_transitionVel = zeus::CVector2f(x138_velocity.x, x138_velocity.y).magnitude();
    if (x58c_transitionVel < 1.f)
        x584_ballTransitionAnim = 5; // ballstationarytoready_random
    if (x258_movementState != EPlayerMovementState::OnGround)
    {
        zeus::CVector3f ballPos = GetBallPosition();
        if (mgr.RayCollideWorld(ballPos, ballPos + zeus::CVector3f(0.f, 0.f, -7.f), BallTransitionCollide, *this))
            x584_ballTransitionAnim = 7; // B_balljumptoairpose
    }

    if (x64_modelData && x64_modelData->HasAnimData())
    {
        CAnimPlaybackParms parms(x584_ballTransitionAnim, -1, 1.f, true);
        x64_modelData->AnimationData()->SetAnimation(parms, false);
        x64_modelData->AnimationData()->SetAnimDir(CAnimData::EAnimDir::Forward);
    }

    x64_modelData->EnableLooping(false);
    x64_modelData->Touch(mgr, 0);
    SetMorphBallState(EPlayerMorphBallState::Unmorphing, mgr);
    x768_morphball->LeaveMorphBallState(mgr);
    mgr.GetCameraManager()->SetPlayerCamera(mgr, mgr.GetCameraManager()->GetFirstPersonCamera()->GetUniqueId());
    zeus::CVector3f camToPlayer = GetTranslation() - mgr.GetCameraManager()->GetBallCamera()->GetTranslation();
    if (camToPlayer.canBeNormalized())
    {
        camToPlayer.normalize();
        zeus::CVector3f vecFlat = x500_lookDir;
        vecFlat.z = 0.f;
        zeus::CVector3f f31 =
            vecFlat.canBeNormalized() && vecFlat.magnitude() >= 0.1f ? x518_leaveMorphDir : camToPlayer;
        if (x9c6_26_outOfBallLookAtHint)
        {
            if (TCastToConstPtr<CScriptPlayerHint> hint = mgr.GetObjectById(x830_playerHint))
            {
                zeus::CVector3f deltaFlat = hint->GetTranslation() - GetTranslation();
                deltaFlat.z = 0.f;
                if (deltaFlat.canBeNormalized())
                    f31 = deltaFlat.normalized();
            }
        }
        if (x9c7_25_outOfBallLookAtHintActor)
        {
            if (TCastToConstPtr<CScriptPlayerHint> hint = mgr.GetObjectById(x830_playerHint))
            {
                if (TCastToConstPtr<CActor> act = mgr.GetObjectById(hint->GetActorId()))
                {
                    zeus::CVector3f deltaFlat = act->GetOrbitPosition(mgr) - GetTranslation();
                    deltaFlat.z = 0.f;
                    if (deltaFlat.canBeNormalized())
                        f31 = deltaFlat.normalized();
                }
            }
        }
        if (std::acos(zeus::clamp(-1.f, camToPlayer.dot(f31), 1.f)) < M_PIF / 1.2f || x9c7_25_outOfBallLookAtHintActor)
        {
            SetTransform(zeus::lookAt(GetTranslation(), GetTranslation() + f31));
        }
        else
        {
            SetTransform(zeus::lookAt(GetTranslation(), GetTranslation() + camToPlayer));
            UpdateArmAndGunTransforms(0.01f, mgr);
        }
    }
    else
    {
        SetTransform(CreateTransformFromMovementDirection());
    }

    CBallCamera* ballCam = mgr.GetCameraManager()->GetBallCamera();
    if (TCastToConstPtr<CActor> act = mgr.GetObjectById(ballCam->GetTooCloseActorId()))
    {
        if (ballCam->GetX3E0() < 20.f && ballCam->GetX3E0() > 1.f)
        {
            zeus::CVector3f deltaFlat = act->GetTranslation() - GetTranslation();
            deltaFlat.z = 0.f;
            zeus::CVector3f deltaFlat2 = act->GetTranslation() - ballCam->GetTranslation();
            deltaFlat2.z = 0.f;
            if (deltaFlat.canBeNormalized() && deltaFlat2.canBeNormalized())
            {
                deltaFlat.normalize();
                zeus::CVector3f camLookFlat = ballCam->GetTransform().basis[1];
                camLookFlat.z = 0.f;
                camLookFlat.normalize();
                deltaFlat2.normalize();
                if (deltaFlat.dot(deltaFlat2) >= 0.3f && deltaFlat2.dot(camLookFlat) >= 0.7f)
                {
                    SetTransform(zeus::lookAt(GetTranslation(), GetTranslation() + deltaFlat));
                }
            }
        }
    }

    ForceGunOrientation(x34_transform, mgr);
    DrawGun(mgr);
    ballCam->SetState(CBallCamera::EBallCameraState::Five, mgr);
    ClearForcesAndTorques();
    SetAngularVelocityWR(zeus::CAxisAngle::sIdentity);
    AddMaterial(EMaterialTypes::GroundCollider, mgr);
    x150_momentum = zeus::CVector3f::skZero;
    SetCameraState(EPlayerCameraState::Transitioning, mgr);
    x824_transitionFilterTimer = 0.01f;
    x57c_ = 0;
    x580_ = 0;
    if (!ballCam->TransitionFromMorphBallState(mgr))
    {
        x824_transitionFilterTimer = 0.95f;
        LeaveMorphBallState(mgr);
    }
}

s32 CPlayer::GetNextBallTransitionAnim(float dt, bool& loopOut, CStateManager& mgr)
{
    loopOut = false;
    zeus::CVector2f vel(x138_velocity.x, x138_velocity.y);
    if (!vel.canBeNormalized())
        return 12; // B_ball_ready_samus
    float velMag = vel.magnitude();
    float maxVel = GetActualFirstPersonMaxVelocity(dt);
    if (velMag <= 0.2f * maxVel)
        return 12; // B_ball_ready_samus
    loopOut = true;
    s32 ret = velMag >= maxVel ? 13 : 15; // B_ball_runloop_samus : B_ball_walkloop_samus
    if (x50c_moveDir.dot(mgr.GetCameraManager()->GetBallCamera()->GetTransform().basis[1]) < -0.5f)
        return 12; // B_ball_ready_samus
    return ret;
}

void CPlayer::UpdateMorphBallTransition(float dt, CStateManager& mgr)
{
    switch (x2f8_morphBallState)
    {
    case EPlayerMorphBallState::Unmorphed:
    case EPlayerMorphBallState::Morphed:
    {
        CPlayerState::EPlayerSuit suit = mgr.GetPlayerState()->GetCurrentSuit();
        if (mgr.GetPlayerState()->IsFusionEnabled())
            suit = CPlayerState::EPlayerSuit(int(suit) + 4);
        if (x7cc_transitionSuit != suit)
        {
            x7cc_transitionSuit = suit;
            CAnimRes useRes = x7d0_animRes;
            useRes.SetCharacterNodeId(s32(x7cc_transitionSuit));
            SetModelData(std::make_unique<CModelData>(x7d0_animRes));
            SetIntoBallReadyAnimation(mgr);
        }
        return;
    }
    case EPlayerMorphBallState::Unmorphing:
        if (x584_ballTransitionAnim == 14) // B_ball_unfurl
        {
            float dur = x64_modelData->GetAnimationData()->GetAnimationDuration(x584_ballTransitionAnim);
            float facRemaining = x64_modelData->GetAnimationData()->GetAnimTimeRemaining("Whole Body") / dur;
            if (facRemaining < 0.5f)
            {
                bool loop = false;
                x584_ballTransitionAnim = GetNextBallTransitionAnim(dt, loop, mgr);
                if (x64_modelData && x64_modelData->HasAnimData())
                {
                    CAnimPlaybackParms parms(x584_ballTransitionAnim, -1, 1.f, true);
                    x64_modelData->AnimationData()->SetAnimation(parms, false);
                    x64_modelData->AnimationData()->EnableLooping(loop);
                }
            }
        }
        else if (x584_ballTransitionAnim != 5 && x584_ballTransitionAnim != 7)
        // ballstationarytoready_random, B_balljumptoairpose
        {
            float velMag = zeus::CVector2f(x138_velocity.x, x138_velocity.y).magnitude();
            if (std::fabs(x58c_transitionVel - velMag) > 0.04f * GetActualFirstPersonMaxVelocity(dt) || velMag < 1.f)
            {
                bool loop = false;
                s32 nextAnim = GetNextBallTransitionAnim(dt, loop, mgr);
                if (x64_modelData && x64_modelData->HasAnimData() &&
                    x584_ballTransitionAnim != nextAnim && x584_ballTransitionAnim != 7)
                {
                    x584_ballTransitionAnim = nextAnim;
                    CAnimPlaybackParms parms(x584_ballTransitionAnim, -1, 1.f, true);
                    x64_modelData->AnimationData()->SetAnimation(parms, false);
                    x64_modelData->AnimationData()->EnableLooping(loop);
                    x58c_transitionVel = velMag;
                }
            }
        }
        break;
    default:
        break;
    }

    SAdvancementDeltas deltas = UpdateAnimation(dt, mgr, true);
    MoveInOneFrameOR(deltas.x0_posDelta, dt);
    RotateInOneFrameOR(deltas.xc_rotDelta, dt);
    x574_morphTime = std::min(x574_morphTime + dt, x578_morphDuration);
    float morphT = x574_morphTime / x578_morphDuration;
    if ((morphT >= 0.7f || x574_morphTime <= 2.f * dt) && x730_transitionModels.size() != 0)
        x730_transitionModels.erase(x730_transitionModels.begin());

    for (auto& m : x730_transitionModels)
        m->AdvanceAnimation(dt, mgr, kInvalidAreaId, true);

    CGameCamera* cam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
    x588_alpha = GetTransitionAlpha(cam->GetTranslation(), cam->GetNearClipDistance());

    if (x2f8_morphBallState == EPlayerMorphBallState::Morphing && morphT > 0.93f)
    {
        x588_alpha *= std::min(1.f, 1.f - (morphT - 0.93f) / 0.07f + 0.2f);
        xb4_drawFlags = CModelFlags(5, 0, 1, zeus::CColor(1.f, x588_alpha));
    }
    else if (x2f8_morphBallState == EPlayerMorphBallState::Unmorphing && x588_alpha < 1.f)
    {
        if (x588_alpha > 0.05f)
            xb4_drawFlags = CModelFlags(5, 0, 0x21, zeus::CColor(1.f, x588_alpha));
        else
            xb4_drawFlags = CModelFlags(5, 0, 1, zeus::CColor(1.f, x588_alpha));
    }
    else
    {
        xb4_drawFlags = CModelFlags(5, 0, 3, zeus::CColor(1.f, x588_alpha));
    }

    x594_transisionBeamXfs.AddValue(x7f4_gunWorldXf);
    x658_transitionModelXfs.AddValue(x34_transform);
    x71c_transitionModelAlphas.AddValue(x588_alpha);

    switch (x2f8_morphBallState)
    {
    case EPlayerMorphBallState::Unmorphing:
        GetCollisionPrimitive()->CalculateAABox(GetPrimitiveTransform()).center();
        ClearForcesAndTorques();
        SetAngularVelocityWR(zeus::CAxisAngle::sIdentity);
        if (x574_morphTime >= x578_morphDuration || mgr.GetCameraManager()->IsInCinematicCamera())
        {
            x824_transitionFilterTimer = std::max(x824_transitionFilterTimer, 0.95f);
            zeus::CVector3f pos;
            if (CanLeaveMorphBallState(mgr, pos))
            {
                SetTranslation(GetTranslation() + pos);
                LeaveMorphBallState(mgr);
                xb4_drawFlags = CModelFlags(0, 0, 3, zeus::CColor::skWhite);
            }
            else
            {
                x574_morphTime = x578_morphDuration - x574_morphTime;
                TransitionToMorphBallState(dt, mgr);
            }
        }
        break;
    case EPlayerMorphBallState::Morphing:
        ClearForcesAndTorques();
        ClearForcesAndTorques();
        SetAngularVelocityWR(zeus::CAxisAngle::sIdentity);
        if (x574_morphTime >= x578_morphDuration || mgr.GetCameraManager()->IsInCinematicCamera())
        {
            if (CanEnterMorphBallState(mgr, 1.f))
            {
                ActivateMorphBallCamera(mgr);
                EnterMorphBallState(mgr);
                xb4_drawFlags = CModelFlags(0, 0, 3, zeus::CColor::skWhite);
            } else
            {
                x574_morphTime = x578_morphDuration - x574_morphTime;
                TransitionFromMorphBallState(mgr);
            }
        }
        if (x578_morphDuration != 0.f)
        {
            if (zeus::clamp(0.f, x574_morphTime, 1.f) >= 0.5f)
            {
                if (!x768_morphball->IsMorphBallTransitionFlashValid())
                    x768_morphball->ResetMorphBallTransitionFlash();
            }
        }
        break;
    default:
        break;
    }
}

void CPlayer::UpdateGunAlpha()
{
    switch (x498_gunHolsterState)
    {
    case EGunHolsterState::Holstered:
        x494_gunAlpha = 0.f;
        break;
    case EGunHolsterState::Holstering:
        x494_gunAlpha = zeus::clamp(0.f, x49c_gunHolsterRemTime / g_tweakPlayerGun->GetGunHolsterTime(), 1.f);
        break;
    case EGunHolsterState::Drawing:
        x494_gunAlpha = 1.f - zeus::clamp(0.f, x49c_gunHolsterRemTime / 0.45f, 1.f);
        break;
    case EGunHolsterState::Drawn:
        x494_gunAlpha = 1.f;
        break;
    }
}

void CPlayer::UpdatePlayerSounds(float dt)
{
    if (x784_damageSfxTimer > 0.f)
    {
        x784_damageSfxTimer -= dt;
        if (x784_damageSfxTimer <= 0.f)
        {
            CSfxManager::SfxStop(x770_damageLoopSfx);
            x770_damageLoopSfx.reset();
        }
    }
}

void CPlayer::Update(float dt, CStateManager& mgr)
{
    SetCoefficientOfRestitutionModifier(0.f);
    UpdateMorphBallTransition(dt, mgr);
    CPlayerState::EBeamId newBeam = mgr.GetPlayerState()->GetCurrentBeam();
    if (newBeam != x7ec_beam)
    {
        x7ec_beam = newBeam;
        x7f0_ballTransitionBeamModel.reset(
            new CModelData(CStaticRes(g_tweakPlayerRes->GetBeamBallTransitionModel(x7ec_beam), x7d8_beamScale)));
    }

    if (!mgr.GetPlayerState()->IsPlayerAlive())
    {
        if (x9f4_deathTime == 0.f)
        {
            CSfxManager::KillAll(CSfxManager::ESfxChannels::Game);
            CStreamAudioManager::StopAll();
            if (x2f8_morphBallState == EPlayerMorphBallState::Unmorphed)
            {
                CSfxHandle hnd = CSfxManager::SfxStart(1516, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
                ApplySubmergedPitchBend(hnd);
            }
        }

        float prevDeathTime = x9f4_deathTime;
        x9f4_deathTime += dt;
        if (x2f8_morphBallState != EPlayerMorphBallState::Unmorphed)
        {
            if (x9f4_deathTime >= 1.f && prevDeathTime < 1.f)
                xa00_deathPowerBomb = x490_gun->DropPowerBomb(mgr);
            if (x9f4_deathTime >= 4.f && prevDeathTime < 4.f)
            {
                CSfxHandle hnd = CSfxManager::SfxStart(1516, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
                ApplySubmergedPitchBend(hnd);
            }
        }
    }

    switch (x2f8_morphBallState)
    {
    case EPlayerMorphBallState::Unmorphed:
    case EPlayerMorphBallState::Morphing:
    case EPlayerMorphBallState::Unmorphing:
        x7f4_gunWorldXf = x34_transform * x64_modelData->GetScaledLocatorTransform("GUN_LCTR");
    case EPlayerMorphBallState::Morphed:
        break;
    }

    if (x2f8_morphBallState == EPlayerMorphBallState::Unmorphed)
    {
        UpdateAimTargetTimer(dt);
        UpdateAimTarget(mgr);
        UpdateOrbitModeTimer(dt);
    }
    UpdateOrbitPreventionTimer(dt);
    if (x2f8_morphBallState == EPlayerMorphBallState::Morphed)
        x768_morphball->Update(dt, mgr);
    else
        x768_morphball->StopSounds();
    if (x2f8_morphBallState == EPlayerMorphBallState::Unmorphing ||
        x2f8_morphBallState == EPlayerMorphBallState::Morphing)
        x768_morphball->UpdateEffects(dt, mgr);
    UpdateGunAlpha();
    UpdateDebugCamera(mgr);
    UpdateVisorTransition(dt, mgr);
    mgr.SetActorAreaId(*this, mgr.GetWorld()->GetCurrentAreaId());
    UpdatePlayerSounds(dt);
    if (x26c_attachedActor != kInvalidUniqueId)
        x270_attachedActorTime += dt;

    x740_staticTimer = std::max(0.f, x740_staticTimer - dt);
    if (x740_staticTimer > 0.f)
        x74c_visorStaticAlpha = std::max(0.f, x74c_visorStaticAlpha - x744_staticOutSpeed * dt);
    else
        x74c_visorStaticAlpha = std::min(1.f, x74c_visorStaticAlpha + x748_staticInSpeed * dt);

    x274_energyDrain.ProcessEnergyDrain(mgr, dt);
    x4a4_moveSpeedAvg.AddValue(x4f8_moveSpeed);

    mgr.GetPlayerState()->UpdateStaticInterference(mgr, dt);
    if (!ShouldSampleFailsafe(mgr))
        CPhysicsActor::Stop();

    if (IsEnergyLow(mgr))
        xa30_samusExhaustedVoiceTimer -= dt;
    else
        xa30_samusExhaustedVoiceTimer = 4.f;

    if (!mgr.GetCameraManager()->IsInCinematicCamera() && xa30_samusExhaustedVoiceTimer <= 0.f)
    {
        StartSamusVoiceSfx(1591, 1.f, 7);
        xa30_samusExhaustedVoiceTimer = 4.f;
    }
}

static const float skStrafeDistances[] =
{
    11.8, 11.8, 11.8, 5.0, 6.0, 5.0, 5.0, 6.0
};

static const float skDashStrafeDistances[] =
{
    30.0, 22.6, 10.0, 10.0, 10.0, 10.0, 10.0
};

static const float skOrbitForwardDistances[] =
{
    11.8, 11.8, 11.8, 5.0, 6.0, 5.0, 5.0, 6.0
};

float CPlayer::UpdateCameraBob(float dt, CStateManager& mgr)
{
    float bobMag = 0.f;
    CPlayerCameraBob::ECameraBobState state;
    zeus::CVector3f backupVel = x138_velocity;
    if (x304_orbitState == EPlayerOrbitState::NoOrbit)
    {
        bobMag = std::fabs(backupVel.dot(x34_transform.basis[1]) / GetActualFirstPersonMaxVelocity(dt));
        state = CPlayerCameraBob::ECameraBobState::Walk;
        if (bobMag < 0.01f)
        {
            bobMag = 0.f;
            state = CPlayerCameraBob::ECameraBobState::WalkNoBob;
        }
    }
    else
    {
        state = CPlayerCameraBob::ECameraBobState::Orbit;
        float f29 = backupVel.dot(x34_transform.basis[0]);
        float f30 = backupVel.dot(x34_transform.basis[1]);
        float maxVel = GetActualFirstPersonMaxVelocity(dt);
        float strafeDist = skStrafeDistances[int(x2b0_outOfWaterTicks == 2 ?
                                                 x2ac_surfaceRestraint : ESurfaceRestraints::Water)];
        bobMag = std::min(std::sqrt(f30 * f30 + f29 * f29) / std::sqrt(strafeDist * strafeDist + maxVel * maxVel) *
            CPlayerCameraBob::kOrbitBobScale, CPlayerCameraBob::kMaxOrbitBobScale);
        if (bobMag < 0.01f)
            bobMag = 0.f;
    }

    if (x258_movementState != EPlayerMovementState::OnGround)
    {
        bobMag = 0.f;
        state = CPlayerCameraBob::ECameraBobState::InAir;
    }
    else if (bobMag < 0.01f)
    {
        if (x490_gun->GetFiring())
        {
            bobMag = 0.f;
            state = CPlayerCameraBob::ECameraBobState::GunFireNoBob;
        }
        else if (std::fabs(GetAngularVelocityOR().angle()) > 0.1f)
        {
            bobMag = 0.f;
            state = CPlayerCameraBob::ECameraBobState::TurningNoBob;
        }
    }

    if (x3dc_inFreeLook || x3dd_lookButtonHeld)
    {
        bobMag = 0.f;
        state = CPlayerCameraBob::ECameraBobState::FreeLookNoBob;
    }

    if (x304_orbitState == EPlayerOrbitState::Grapple)
    {
        bobMag = 0.f;
        state = CPlayerCameraBob::ECameraBobState::GrappleNoBob;
    }

    if (x3a8_scanState == EPlayerScanState::ScanComplete)
    {
        bobMag = 0.f;
    }

    if (x38c_doneSidewaysDashing)
    {
        bobMag *= 0.1f;
        state = CPlayerCameraBob::ECameraBobState::FreeLookNoBob;
        if (x258_movementState == EPlayerMovementState::OnGround)
            x38c_doneSidewaysDashing = false;
    }

    if (mgr.GetCameraManager()->IsInCinematicCamera())
        bobMag = 0.f;

    bobMag *= mgr.GetCameraManager()->GetCameraBobMagnitude();

    x76c_cameraBob->SetPlayerVelocity(backupVel);
    x76c_cameraBob->SetState(state, mgr);
    x76c_cameraBob->SetBobMagnitude(bobMag);
    x76c_cameraBob->SetBobTimeScale((1.f - CPlayerCameraBob::kSlowSpeedPeriodScale) * bobMag +
                                        CPlayerCameraBob::kSlowSpeedPeriodScale);
    x76c_cameraBob->Update(dt, mgr);

    return bobMag;
}

float CPlayer::GetAcceleration() const
{
    if (x2d0_curAcceleration >= x2b4_accelerationTable.size())
        return x2b4_accelerationTable.back();
    return x2b4_accelerationTable[x2d0_curAcceleration];
}

float CPlayer::CalculateOrbitZBasedDistance(EPlayerOrbitType type) const
{
    return zeus::clamp(1.f, std::fabs(x314_orbitPoint.z - GetTranslation().z) / 20.f, 4.f) *
        g_tweakPlayer->GetOrbitZBasedDistance(int(type));
}

void CPlayer::PostUpdate(float dt, CStateManager& mgr)
{
    UpdateArmAndGunTransforms(dt, mgr);

    float grappleSwingT;
    if (x3b8_grappleState != EGrappleState::Swinging)
        grappleSwingT = 0.f;
    else
        grappleSwingT = x3bc_grappleSwingTimer / g_tweakPlayer->GetGrappleSwingPeriod();

    float cameraBobT = 0.f;
    if (mgr.GetCameraManager()->IsInCinematicCamera())
    {
        zeus::CVector2f bobExtent(CPlayerCameraBob::kCameraBobExtentX,
                                  CPlayerCameraBob::kCameraBobExtentY);
        x76c_cameraBob.reset(new CPlayerCameraBob(CPlayerCameraBob::ECameraBobType::One, bobExtent,
                                                  CPlayerCameraBob::kCameraBobPeriod));
    }
    else
    {
        cameraBobT = UpdateCameraBob(dt, mgr);
    }

    x490_gun->Update(grappleSwingT, cameraBobT, dt, mgr);
    UpdateOrbitTarget(mgr);
    UpdateOrbitOrientation(mgr);
}

bool CPlayer::StartSamusVoiceSfx(u16 sfx, float vol, int prio)
{
    if (x2f8_morphBallState == EPlayerMorphBallState::Morphed)
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
    if (x2f8_morphBallState == CPlayer::EPlayerMorphBallState::Unmorphed)
        return x9f4_deathTime > 2.5f;
    if (x2f8_morphBallState == CPlayer::EPlayerMorphBallState::Morphed)
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
    if (x2f8_morphBallState == EPlayerMorphBallState::Morphed && x570_immuneTimer > 0.f && !info.NoImmunity())
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
        if (x304_orbitState == EPlayerOrbitState::NoOrbit)
            ret += PredictMotion(dt).x0_translation;
        else
            ret = CSteeringBehaviors::ProjectOrbitalPosition(ret, x138_velocity, x314_orbitPoint, dt);
    }

    if (x2f8_morphBallState == EPlayerMorphBallState::Morphed)
        ret.z += g_tweakPlayer->GetPlayerBallHalfExtent();
    else
        ret.z += GetEyeHeight();

    return ret;
}

void CPlayer::FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr)
{
    if (x2f8_morphBallState == EPlayerMorphBallState::Morphed)
    {
        x768_morphball->FluidFXThink(state, water, mgr);
        if (state == EFluidState::InFluid)
            x9c5_30_ = true;
    }
    else if (x2f8_morphBallState != EPlayerMorphBallState::Unmorphed)
    {
        if (mgr.GetFluidPlaneManager()->GetLastSplashDeltaTime(x8_uid) >= 0.2f)
        {
            zeus::CVector3f position(x34_transform.origin);
            position.z = water.GetTriggerBoundsWR().max.z;
            mgr.GetFluidPlaneManager()->CreateSplash(x8_uid, mgr, water, position, 0.1f,
                                                     state == EFluidState::EnteredFluid);
        }
    }
    else
    {
        if (mgr.GetFluidPlaneManager()->GetLastSplashDeltaTime(x8_uid) >= 0.2f)
        {
            zeus::CVector3f posOffset = x50c_moveDir;
            if (posOffset.canBeNormalized())
                posOffset = posOffset.normalized() * zeus::CVector3f(1.2f, 1.2f, 0.f);
            switch (state)
            {
            case EFluidState::EnteredFluid:
            {
                bool doSplash = true;
                if (x4fc_flatMoveSpeed > 12.5f)
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
                    if (water.GetFluidPlane().GetFluidType() == CFluidPlane::EFluidType::NormalWater)
                    {
                        float velMag = mgr.GetPlayer().GetVelocity().magnitude() / 10.f;
                        mgr.GetEnvFxManager()->SetXB54(10.f * std::max(1.f, velMag));
                    }
                }
                break;
            }
            case EFluidState::InFluid:
            {
                if (x138_velocity.magnitude() > 1.f &&
                    mgr.GetFluidPlaneManager()->GetLastRippleDeltaTime(x8_uid) >= 0.2f)
                {
                    zeus::CVector3f position(x34_transform.origin);
                    position.z = water.GetTriggerBoundsWR().max.z;
                    water.GetFluidPlane().AddRipple(0.5f, x8_uid, position, water, mgr);
                }
                break;
            }
            case EFluidState::LeftFluid:
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
        x570_immuneTimer = 0.5f;
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
            if (x2f8_morphBallState == EPlayerMorphBallState::Unmorphed)
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

        if (damageLoopSfx && !x9c7_24_ && x2ac_surfaceRestraint >= ESurfaceRestraints::Ice)
        {
            if (!x770_damageLoopSfx || x788_damageLoopSfxId != damageLoopSfx)
            {
                if (x770_damageLoopSfx && x788_damageLoopSfxId != damageLoopSfx)
                    CSfxManager::SfxStop(x770_damageLoopSfx);
                x770_damageLoopSfx = CSfxManager::SfxStart(damageLoopSfx, 1.f, 0.f, false, 0x7f, true, kInvalidAreaId);
                x788_damageLoopSfxId = damageLoopSfx;
            }
            x784_damageSfxTimer = 0.5f;
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
            if (x2f8_morphBallState == EPlayerMorphBallState::Unmorphed)
                x490_gun->DamageRumble(location, dam, mgr);
            float tmp = x55c_damageAmt / 25.f;
            if (std::fabs(tmp) > 1.f)
                tmp = tmp > 0.f ? 1.f : -1.f;
            mgr.GetRumbleManager().Rumble(mgr, ERumbleFxId::Eleven, ERumblePriority::One);
        }

        if (x2f8_morphBallState != EPlayerMorphBallState::Unmorphed)
        {
            x768_morphball->TakeDamage(x55c_damageAmt);
            x768_morphball->SetDamageTimer(0.4f);
        }
    }

    if (x3b8_grappleState != EGrappleState::None)
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

rstl::optional_object<zeus::CAABox> CPlayer::GetTouchBounds() const
{
    if (x2f8_morphBallState == EPlayerMorphBallState::Morphed)
    {
        float ballTouchRad = x768_morphball->GetBallTouchRadius();
        zeus::CVector3f ballCenter =
            GetTranslation() + zeus::CVector3f(0.f, 0.f, x768_morphball->GetBallRadius());
        return zeus::CAABox(ballCenter - ballTouchRad, ballCenter + ballTouchRad);
    }
    return GetBoundingBox();
}

void CPlayer::DoPreThink(float dt, CStateManager& mgr)
{
    PreThink(dt, mgr);
    if (CEntity* ent = mgr.ObjectById(xa00_deathPowerBomb))
        ent->PreThink(dt, mgr);
}

void CPlayer::DoThink(float dt, CStateManager& mgr)
{
    Think(dt, mgr);
    if (CEntity* ent = mgr.ObjectById(xa00_deathPowerBomb))
        ent->Think(dt, mgr);
}

void CPlayer::UpdateScanningState(const CFinalInput& input, CStateManager& mgr, float dt)
{
    if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Scan)
    {
        SetScanningState(EPlayerScanState::NotScanning, mgr);
        return;
    }

    if (x3a8_scanState != EPlayerScanState::NotScanning && x3b4_scanningObject != x310_orbitTargetId &&
        x310_orbitTargetId != kInvalidUniqueId)
        SetScanningState(EPlayerScanState::NotScanning, mgr);

    switch (x3a8_scanState)
    {
    case EPlayerScanState::NotScanning:
        if (ValidateScanning(input, mgr))
        {
            if (TCastToPtr<CActor> act = mgr.ObjectById(x310_orbitTargetId))
            {
                const CScannableObjectInfo* scanInfo = act->GetScannableObjectInfo();
                float scanTime = mgr.GetPlayerState()->GetScanTime(scanInfo->GetScannableObjectId());
                if (scanTime >= 1.f)
                {
                    x9c6_30_newScanScanning = false;
                    scanTime = 0.f;
                }
                else
                {
                    x9c6_30_newScanScanning = true;
                }

                SetScanningState(EPlayerScanState::Scanning, mgr);
                x3ac_scanningTime = scanTime * scanInfo->GetTotalDownloadTime();
                x3b0_curScanTime = 0.f;
            }
        }
        break;
    case EPlayerScanState::Scanning:
        if (ValidateScanning(input, mgr))
        {
            if (TCastToPtr<CActor> act = mgr.ObjectById(x310_orbitTargetId))
            {
                if (const CScannableObjectInfo* scanInfo = act->GetScannableObjectInfo())
                {
                    x3ac_scanningTime = std::min(scanInfo->GetTotalDownloadTime(), x3ac_scanningTime + dt);
                    x3b0_curScanTime += dt;
                    mgr.GetPlayerState()->SetScanTime(scanInfo->GetScannableObjectId(),
                                                      x3ac_scanningTime / scanInfo->GetTotalDownloadTime());
                    if (x3ac_scanningTime >= scanInfo->GetTotalDownloadTime() &&
                        x3b0_curScanTime >= g_tweakGui->GetScanSidesStartTime())
                        SetScanningState(EPlayerScanState::ScanComplete, mgr);
                }
            }
            else
            {
                SetScanningState(EPlayerScanState::NotScanning, mgr);
            }
        }
        else
        {
            SetScanningState(EPlayerScanState::NotScanning, mgr);
        }
        break;
    case EPlayerScanState::ScanComplete:
        if (!ValidateScanning(input, mgr))
            SetScanningState(EPlayerScanState::NotScanning, mgr);
        break;
    }
}

bool CPlayer::ValidateScanning(const CFinalInput& input, CStateManager& mgr)
{
    if (ControlMapper::GetDigitalInput(ControlMapper::ECommands::ScanItem, input))
    {
        if (x304_orbitState == EPlayerOrbitState::One)
        {
            if (TCastToPtr<CActor> act = mgr.ObjectById(x310_orbitTargetId))
            {
                if (act->GetMaterialList().HasMaterial(EMaterialTypes::Scannable))
                {
                    zeus::CVector3f targetToPlayer = GetTranslation() - act->GetTranslation();
                    if (targetToPlayer.canBeNormalized() &&
                        targetToPlayer.magnitude() < g_tweakPlayer->GetScanningRange())
                        return true;
                }
            }
        }
    }

    return false;
}

static bool IsDataLoreResearchScan(CAssetId id)
{
    auto it = g_MemoryCardSys->LookupScanState(id);
    if (it == g_MemoryCardSys->GetScanStates().cend())
        return false;
    switch (it->second)
    {
    case CSaveWorld::EScanCategory::Data:
    case CSaveWorld::EScanCategory::Lore:
    case CSaveWorld::EScanCategory::Research:
        return true;
    default:
        return false;
    }
}

static const char* UnlockMessageResBases[] =
{
    "STRG_SlideShow_Unlock1_",
    "STRG_SlideShow_Unlock2_"
};

static CAssetId UpdatePersistentScanPercent(u32 prevLogScans, u32 logScans, u32 totalLogScans)
{
    if (prevLogScans == logScans)
        return {};

    float scanPercent = logScans / float(totalLogScans) * 100.f;
    float prevScanPercent = prevLogScans / float(totalLogScans) * 100.f;
    float scanMessageInterval = g_tweakSlideShow->GetScanPercentInterval();
    auto scanPercentProgStep = int(scanPercent / scanMessageInterval);
    auto prevScanPercentProgStep = int(prevScanPercent / scanMessageInterval);
    bool firstTime = scanPercent > g_GameState->SystemOptions().GetLogScanPercent();

    if (firstTime)
        g_GameState->SystemOptions().SetLogScanPercent(u32(scanPercent));

    if (scanPercentProgStep > prevScanPercentProgStep)
    {
        auto message =
            std::string(UnlockMessageResBases[zeus::clamp(0, scanPercentProgStep - 1, 1)]) + (firstTime ? '1' : '2');
        auto id = g_ResFactory->GetResourceIdByName(message.c_str());
        if (id != nullptr)
            return id->id;
    }

    return {};
}

void CPlayer::FinishNewScan(CStateManager& mgr)
{
    if (TCastToPtr<CActor> act = mgr.ObjectById(x310_orbitTargetId))
        if (act->GetMaterialList().HasMaterial(EMaterialTypes::Scannable))
            if (auto scanInfo = act->GetScannableObjectInfo())
                if (mgr.GetPlayerState()->GetScanTime(scanInfo->GetScannableObjectId()) >= 1.f)
                    if (IsDataLoreResearchScan(scanInfo->GetScannableObjectId()))
                    {
                        auto scanCompletion = mgr.CalculateScanCompletionRate();
                        CAssetId message = UpdatePersistentScanPercent(mgr.GetPlayerState()->GetLogScans(),
                                                                    scanCompletion.first, scanCompletion.second);
                        if (message != -1)
                            mgr.ShowPausedHUDMemo(message, 0.f);
                        mgr.GetPlayerState()->SetScanCompletionRate(scanCompletion);
                    }
}

void CPlayer::SetScanningState(EPlayerScanState state, CStateManager& mgr)
{
    if (x3a8_scanState == state)
        return;

    mgr.SetGameState(CStateManager::EGameState::Running);
    if (x3a8_scanState == EPlayerScanState::ScanComplete)
        if (TCastToPtr<CActor> act = mgr.ObjectById(x3b4_scanningObject))
            act->OnScanStateChanged(EScanState::Done, mgr);

    switch (state)
    {
    case EPlayerScanState::NotScanning:
        if (x3a8_scanState == EPlayerScanState::Scanning || x3a8_scanState == EPlayerScanState::ScanComplete)
            if (x9c6_30_newScanScanning)
                FinishNewScan(mgr);
        x3ac_scanningTime = 0.f;
        x3b0_curScanTime = 0.f;
        if (!g_tweakPlayer->GetScanRetention())
            if (TCastToPtr<CActor> act = mgr.ObjectById(x310_orbitTargetId))
                if (act->GetMaterialList().HasMaterial(EMaterialTypes::Scannable))
                    if (auto scanInfo = act->GetScannableObjectInfo())
                        if (mgr.GetPlayerState()->GetScanTime(scanInfo->GetScannableObjectId()) < 1.f)
                            mgr.GetPlayerState()->SetScanTime(scanInfo->GetScannableObjectId(), 0.f);
        x3b4_scanningObject = kInvalidUniqueId;
        break;
    case EPlayerScanState::Scanning:
        x3b4_scanningObject = x310_orbitTargetId;
        break;
    case EPlayerScanState::ScanComplete:
        if (g_tweakPlayer->GetScanFreezesGame())
            mgr.SetGameState(CStateManager::EGameState::SoftPaused);
        x3b4_scanningObject = x310_orbitTargetId;
        break;
    }

    x3a8_scanState = state;
}

bool CPlayer::GetExplorationMode() const
{
    switch (x498_gunHolsterState)
    {
    case EGunHolsterState::Holstering:
    case EGunHolsterState::Holstered:
        return true;
    default:
        return false;
    }
}

bool CPlayer::GetCombatMode() const
{
    switch (x498_gunHolsterState)
    {
    case EGunHolsterState::Drawing:
    case EGunHolsterState::Drawn:
        return true;
    default:
        return false;
    }
}

void CPlayer::RenderGun(const CStateManager& mgr, const zeus::CVector3f& pos) const
{
    if (mgr.GetCameraManager()->IsInCinematicCamera())
        return;

    if (x490_gun->GetGrappleArm().InGrappleCycle() &&
        x490_gun->GetGrappleArm().GetAnimState() != CGrappleArm::EArmState::Ten)
        x490_gun->GetGrappleArm().RenderGrappleBeam(mgr, pos);

    if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::Scan &&
        mgr.GetPlayerState()->GetVisorTransitionFactor() >= 1.f)
        return;

    if ((mgr.GetCameraManager()->IsInFirstPersonCamera() && x2f4_cameraState == EPlayerCameraState::FirstPerson) ||
        (x2f8_morphBallState == EPlayerMorphBallState::Morphing &&
            x498_gunHolsterState == EGunHolsterState::Holstering))
    {
        CModelFlags flags(5, 0, 3, zeus::CColor(1.f, x494_gunAlpha));
        x490_gun->Render(mgr, pos, flags);
    }
}

void CPlayer::Render(const CStateManager& mgr) const
{
    bool doRender = x2f4_cameraState != EPlayerCameraState::Spawned;
    if (!doRender)
        if (TCastToConstPtr<CCinematicCamera> cam = mgr.GetCameraManager()->GetCurrentCamera(mgr))
            doRender = (x2f8_morphBallState == EPlayerMorphBallState::Morphed && cam->GetW1() & 0x40);
    if (x2f4_cameraState != EPlayerCameraState::FirstPerson && doRender)
    {
        bool doTransitionRender = false;
        bool doBallRender = false;
        switch (x2f8_morphBallState)
        {
        case EPlayerMorphBallState::Unmorphed:
            x64_modelData->Touch(mgr, 0);
            CPhysicsActor::Render(mgr);
            if (HasTransitionBeamModel())
            {
                x7f0_ballTransitionBeamModel->Touch(mgr, 0);
                CModelFlags flags(0, 0, 3, zeus::CColor::skWhite);
                x7f0_ballTransitionBeamModel->Render(mgr, x7f4_gunWorldXf, x90_actorLights.get(), flags);
            }
            break;
        case EPlayerMorphBallState::Morphing:
            x768_morphball->TouchModel(mgr);
            doTransitionRender = true;
            doBallRender = true;
            break;
        case EPlayerMorphBallState::Unmorphing:
            x490_gun->TouchModel(mgr);
            doTransitionRender = true;
            doBallRender = true;
            break;
        case EPlayerMorphBallState::Morphed:
            x64_modelData->Touch(mgr, 0);
            x768_morphball->Render(mgr, x90_actorLights.get());
            break;
        }

        if (doTransitionRender)
        {
            CPhysicsActor::Render(mgr);
            if (HasTransitionBeamModel())
            {
                CModelFlags flags(5, 0, 3, zeus::CColor(1.f, x588_alpha));
                x7f0_ballTransitionBeamModel->Render(CModelData::EWhichModel::Normal,
                                                     x7f4_gunWorldXf, x90_actorLights.get(), flags);
            }

            float morphFactor = x574_morphTime / x578_morphDuration;
            float transitionAlpha;
            if (morphFactor < 0.05f)
                transitionAlpha = 0.f;
            else if (morphFactor < 0.1f)
                transitionAlpha = (morphFactor - 0.05f) / 0.05f;
            else if (morphFactor < 0.8f)
                transitionAlpha = 1.f;
            else
                transitionAlpha = 1.f - (morphFactor - 0.8f) / 0.2f;

            auto mdsp1 = int(x730_transitionModels.size() + 1);
            for (int i=0 ; i<x730_transitionModels.size() ; ++i)
            {
                int ni = i + 1;
                float alpha = transitionAlpha * (1.f - (ni + 1) / float(mdsp1)) *
                              *x71c_transitionModelAlphas.GetEntry(ni);
                if (alpha != 0.f)
                {
                    CModelData& data = *x730_transitionModels[i];
                    CModelFlags flags(5, 0, 3, zeus::CColor(1.f, alpha));
                    data.Render(CModelData::GetRenderingModel(mgr), *x658_transitionModelXfs.GetEntry(ni),
                                x90_actorLights.get(), flags);
                    if (HasTransitionBeamModel())
                    {
                        CModelFlags flags(5, 0, 3, zeus::CColor(1.f, alpha));
                        x7f0_ballTransitionBeamModel->Render(CModelData::EWhichModel::Normal,
                                                             *x594_transisionBeamXfs.GetEntry(ni),
                                                             x90_actorLights.get(), flags);
                    }
                }
            }

            if (doBallRender)
            {
                float morphFactor = x574_morphTime / x578_morphDuration;
                float ballAlphaStart = 0.75f;
                float ballAlphaMag = 4.f;
                if (x2f8_morphBallState == EPlayerMorphBallState::Unmorphing)
                {
                    ballAlphaStart = 0.875f;
                    morphFactor = 1.f - morphFactor;
                    ballAlphaMag = 8.f;
                }

                if (morphFactor > ballAlphaStart)
                {
                    CModelFlags flags(5, u8(x768_morphball->GetMorphballModelShader()), 3,
                                      zeus::CColor(1.f, ballAlphaMag * (morphFactor - ballAlphaStart)));
                    x768_morphball->GetMorphballModelData().Render(mgr, x768_morphball->GetBallToWorld(),
                                                                   x90_actorLights.get(), flags);
                }

                if (x2f8_morphBallState == EPlayerMorphBallState::Morphing)
                {
                    if (morphFactor > 0.5f)
                    {
                        float tmp = (morphFactor - 0.5f) / 0.5f;
                        float rotate = 1.f - tmp;
                        float scale = 0.75f * rotate + 1.f;
                        float ballAlpha;
                        if (tmp < 0.1f)
                            ballAlpha = 0.f;
                        else if (tmp < 0.2f)
                            ballAlpha = (tmp - 0.1f) / 0.1f;
                        else if (tmp < 0.9f)
                            ballAlpha = 1.f;
                        else
                            ballAlpha = 1.f - (morphFactor - 0.9f) / 0.1f;

                        float theta = zeus::degToRad(360.f * rotate);
                        ballAlpha *= 0.5f;
                        if (ballAlpha > 0.f)
                        {
                            CModelFlags flags(7, 0, 3, zeus::CColor(1.f, ballAlpha));
                            x768_morphball->GetMorphballModelData().Render(mgr, x768_morphball->GetBallToWorld() *
                                zeus::CTransform::RotateZ(theta) * zeus::CTransform::Scale(scale),
                                x90_actorLights.get(), flags);
                        }
                    }
                    x768_morphball->RenderMorphBallTransitionFlash(mgr);
                }
            }
        }
    }
}

void CPlayer::RenderReflectedPlayer(CStateManager& mgr)
{
    zeus::CFrustum frustum;
    switch (x2f8_morphBallState)
    {
    case EPlayerMorphBallState::Unmorphed:
    case EPlayerMorphBallState::Morphing:
    case EPlayerMorphBallState::Unmorphing:
        SetCalculateLighting(true);
        if (x2f4_cameraState == EPlayerCameraState::FirstPerson)
            CActor::PreRender(mgr, frustum);
        CPhysicsActor::Render(mgr);
        if (HasTransitionBeamModel())
        {
            CModelFlags flags(0, 0, 3, zeus::CColor::skWhite);
            x7f0_ballTransitionBeamModel->Render(mgr, x7f4_gunWorldXf, nullptr, flags);
        }
        break;
    case EPlayerMorphBallState::Morphed:
        x768_morphball->Render(mgr, x90_actorLights.get());
        break;
    }
}

void CPlayer::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum)
{
    if (x2f8_morphBallState == EPlayerMorphBallState::Morphed)
    {
        SetCalculateLighting(false);
        x768_morphball->PreRender(mgr, frustum);
    }
    else
    {
        SetCalculateLighting(true);
        if (x2f8_morphBallState == EPlayerMorphBallState::Unmorphed)
        {
            x490_gun->PreRender(mgr, frustum,
                mgr.GetCameraManager()->GetGlobalCameraTranslation(mgr));
        }
    }

    if (x2f8_morphBallState == EPlayerMorphBallState::Unmorphed ||
        mgr.GetCameraManager()->IsInCinematicCamera())
    {
        x768_morphball->DeleteBallShadow();
    }
    else
    {
        x768_morphball->CreateBallShadow();
        x768_morphball->RenderToShadowTex(mgr);
    }

    for (auto& model : x730_transitionModels)
        model->AnimationData()->PreRender();

    if (x2f4_cameraState != EPlayerCameraState::FirstPerson)
        CActor::PreRender(mgr, frustum);
}

void CPlayer::CalculateRenderBounds()
{
    if (x2f8_morphBallState == EPlayerMorphBallState::Morphed)
    {
        float rad = x768_morphball->GetBallRadius();
        x9c_renderBounds = zeus::CAABox(GetTranslation() - zeus::CVector3f(rad, rad, 0.f),
                                 GetTranslation() + zeus::CVector3f(rad, rad, rad * 2.f));
    }
    else
    {
        CActor::CalculateRenderBounds();
    }
}

void CPlayer::AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const
{
    if (x2f4_cameraState != EPlayerCameraState::FirstPerson &&
        x2f8_morphBallState == EPlayerMorphBallState::Morphed)
    {
        if (x768_morphball->IsInFrustum(frustum))
            CActor::AddToRenderer(frustum, mgr);
        else
            x768_morphball->TouchModel(mgr);
    }
    else
    {
        x490_gun->AddToRenderer(frustum, mgr);
        CActor::AddToRenderer(frustum, mgr);
    }
}

void CPlayer::ComputeFreeLook(const CFinalInput& input)
{
    float lookLeft = ControlMapper::GetAnalogInput(ControlMapper::ECommands::LookLeft, input);
    float lookRight = ControlMapper::GetAnalogInput(ControlMapper::ECommands::LookRight, input);
    float lookUp = ControlMapper::GetAnalogInput(ControlMapper::ECommands::LookUp, input);
    float lookDown = ControlMapper::GetAnalogInput(ControlMapper::ECommands::LookDown, input);

    if (g_GameState->GameOptions().GetInvertYAxis())
    {
        lookUp = ControlMapper::GetAnalogInput(ControlMapper::ECommands::LookDown, input);
        lookDown = ControlMapper::GetAnalogInput(ControlMapper::ECommands::LookUp, input);
    }

    if (!g_tweakPlayer->GetStayInFreeLookWhileFiring() &&
        (ControlMapper::GetDigitalInput(ControlMapper::ECommands::FireOrBomb, input) ||
         x304_orbitState != EPlayerOrbitState::NoOrbit))
    {
        x3e8_horizFreeLookAngleVel = 0.f;
        x3f0_vertFreeLookAngleVel = 0.f;
    }
    else
    {
        if (x3dc_inFreeLook)
        {
            x3e8_horizFreeLookAngleVel = (lookLeft - lookRight) * g_tweakPlayer->GetHorizontalFreeLookAngleVel();
            x3f0_vertFreeLookAngleVel = (lookUp - lookDown) * g_tweakPlayer->GetVerticalFreeLookAngleVel();
        }
        if (!x3de_lookAnalogHeld || !x3dd_lookButtonHeld)
        {
            x3e8_horizFreeLookAngleVel = 0.f;
            x3f0_vertFreeLookAngleVel = 0.f;
        }
    }

    if (g_tweakPlayer->GetHoldButtonsForFreeLook())
    {
        if ((g_tweakPlayer->GetTwoButtonsForFreeLook() &&
            (!ControlMapper::GetDigitalInput(ControlMapper::ECommands::LookHold1, input) ||
             !ControlMapper::GetDigitalInput(ControlMapper::ECommands::LookHold2, input))) ||
            (!ControlMapper::GetDigitalInput(ControlMapper::ECommands::LookHold1, input) &&
             !ControlMapper::GetDigitalInput(ControlMapper::ECommands::LookHold2, input)))
        {
            x3e8_horizFreeLookAngleVel = 0.f;
            x3f0_vertFreeLookAngleVel = 0.f;
        }
    }

    if (IsMorphBallTransitioning())
    {
        x3e8_horizFreeLookAngleVel = 0.f;
        x3f0_vertFreeLookAngleVel = 0.f;
    }
}

void CPlayer::UpdateFreeLookState(const CFinalInput& input, float dt, CStateManager& mgr)
{
    if (x304_orbitState == EPlayerOrbitState::Four || IsMorphBallTransitioning() ||
        x2f8_morphBallState != EPlayerMorphBallState::Unmorphed ||
        (x3b8_grappleState != EGrappleState::None && x3b8_grappleState != EGrappleState::Firing))
    {
        x3dc_inFreeLook = false;
        x3dd_lookButtonHeld = false;
        x3de_lookAnalogHeld = false;
        x3e8_horizFreeLookAngleVel = 0.f;
        x3f0_vertFreeLookAngleVel = 0.f;
        x9c4_25_showCrosshairs = false;
        return;
    }

    if (g_tweakPlayer->GetHoldButtonsForFreeLook())
    {
        if ((g_tweakPlayer->GetTwoButtonsForFreeLook() &&
             (ControlMapper::GetDigitalInput(ControlMapper::ECommands::LookHold1, input) &&
              ControlMapper::GetDigitalInput(ControlMapper::ECommands::LookHold2, input))) ||
            (!g_tweakPlayer->GetTwoButtonsForFreeLook() &&
             (ControlMapper::GetDigitalInput(ControlMapper::ECommands::LookHold1, input) ||
              ControlMapper::GetDigitalInput(ControlMapper::ECommands::LookHold2, input))))
        {
            if (!x3dd_lookButtonHeld)
            {
                zeus::CVector3f lookDir = mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform().basis[1];
                zeus::CVector3f lookDirFlat = lookDir;
                lookDirFlat.z = 0.f;
                x3e4_freeLookYawAngle = 0.f;
                if (lookDirFlat.canBeNormalized())
                {
                    lookDirFlat.normalize();
                    x3ec_freeLookPitchAngle = std::acos(zeus::clamp(-1.f, lookDirFlat.dot(lookDir), 1.f));
                    if (lookDir.z < 0.f)
                        x3ec_freeLookPitchAngle = -x3ec_freeLookPitchAngle;
                }
            }
            x3dc_inFreeLook = true;
            x3dd_lookButtonHeld = true;

            x3de_lookAnalogHeld =
               (ControlMapper::GetAnalogInput(ControlMapper::ECommands::LookLeft, input) >= 0.1f ||
                ControlMapper::GetAnalogInput(ControlMapper::ECommands::LookRight, input) >= 0.1f ||
                ControlMapper::GetAnalogInput(ControlMapper::ECommands::LookDown, input) >= 0.1f ||
                ControlMapper::GetAnalogInput(ControlMapper::ECommands::LookUp, input) >= 0.1f);
        }
        else
        {
            x3dc_inFreeLook = false;
            x3dd_lookButtonHeld = false;
            x3de_lookAnalogHeld = false;
            x3e8_horizFreeLookAngleVel = 0.f;
            x3f0_vertFreeLookAngleVel = 0.f;
        }
    }
    else
    {
        x3de_lookAnalogHeld =
            (ControlMapper::GetAnalogInput(ControlMapper::ECommands::LookLeft, input) >= 0.1f ||
             ControlMapper::GetAnalogInput(ControlMapper::ECommands::LookRight, input) >= 0.1f ||
             ControlMapper::GetAnalogInput(ControlMapper::ECommands::LookDown, input) >= 0.1f ||
             ControlMapper::GetAnalogInput(ControlMapper::ECommands::LookUp, input) >= 0.1f);
        x3dd_lookButtonHeld = false;
        if (std::fabs(x3e4_freeLookYawAngle) < g_tweakPlayer->GetFreeLookCenteredThresholdAngle() &&
            std::fabs(x3ec_freeLookPitchAngle) < g_tweakPlayer->GetFreeLookCenteredThresholdAngle())
        {
            if (x3e0_curFreeLookCenteredTime > g_tweakPlayer->GetFreeLookCenteredTime())
            {
                x3dc_inFreeLook = false;
                x3e8_horizFreeLookAngleVel = 0.f;
                x3f0_vertFreeLookAngleVel = 0.f;
            }
            else
            {
                x3e0_curFreeLookCenteredTime += dt;
            }
        }
        else
        {
            x3dc_inFreeLook = true;
            x3e0_curFreeLookCenteredTime = 0.f;
        }
    }

    UpdateCrosshairsState(input);
}

void CPlayer::UpdateFreeLook(float dt)
{
    if (GetFrozenState())
        return;
    float lookDeltaAngle = dt * g_tweakPlayer->GetFreeLookSpeed();
    if (!x3de_lookAnalogHeld)
        lookDeltaAngle = dt * g_tweakPlayer->GetFreeLookSnapSpeed();
    float angleVelP = x3f0_vertFreeLookAngleVel - x3ec_freeLookPitchAngle;
    float vertLookDamp = zeus::clamp(0.f, std::fabs(angleVelP / 1.0471976f), 1.f);
    float dx = lookDeltaAngle * (2.f * vertLookDamp - std::sin((M_PIF / 2.f) * vertLookDamp));
    if (0.f <= angleVelP)
        x3ec_freeLookPitchAngle += dx;
    else
        x3ec_freeLookPitchAngle -= dx;
    angleVelP = x3e8_horizFreeLookAngleVel - x3e4_freeLookYawAngle;
    dx = lookDeltaAngle * zeus::clamp(0.f, std::fabs(angleVelP / g_tweakPlayer->GetHorizontalFreeLookAngleVel()), 1.f);
    if (0.f <= angleVelP)
        x3e4_freeLookYawAngle += dx;
    else
        x3e4_freeLookYawAngle -= dx;
    if (g_tweakPlayer->GetFreeLookTurnsPlayer())
        x3e4_freeLookYawAngle = 0.f;
}

float CPlayer::GetMaximumPlayerPositiveVerticalVelocity(CStateManager& mgr) const
{
    return mgr.GetPlayerState()->GetItemAmount(CPlayerState::EItemType::SpaceJumpBoots) ? 14.f : 11.666666f;
}

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
        if (x2f8_morphBallState == EPlayerMorphBallState::Morphed)
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

    if (x2f8_morphBallState != EPlayerMorphBallState::Morphed)
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
        if (x2f8_morphBallState == EPlayerMorphBallState::Morphed)
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

    if (x2f8_morphBallState == EPlayerMorphBallState::Unmorphed && x4a0_failsafeTest->Passes())
    {
        auto* prim = static_cast<const CCollidableAABox*>(GetCollisionPrimitive());
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
    if (x2f8_morphBallState == EPlayerMorphBallState::Morphed)
    {
        float leftDiv = g_tweakBall->GetLeftStickDivisor();
        float rightDiv = g_tweakBall->GetRightStickDivisor();
        if (x26c_attachedActor != kInvalidUniqueId || IsUnderBetaMetroidAttack(mgr))
            leftDiv = 2.f;
        CFinalInput scaledInput = input.ScaleAnalogueSticks(leftDiv, rightDiv);
        x768_morphball->ComputeBallMovement(scaledInput, mgr, input.DeltaTime());
        x768_morphball->UpdateBallDynamics(mgr, input.DeltaTime());
        x4a0_failsafeTest->Reset();
    }
    else
    {
        if (x304_orbitState == EPlayerOrbitState::Grapple)
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

    if (x2f8_morphBallState == EPlayerMorphBallState::Morphed ||
        (x2f8_morphBallState == EPlayerMorphBallState::Unmorphed &&
        x498_gunHolsterState == EGunHolsterState::Drawn))
    {
        x490_gun->ProcessInput(input, mgr);
        if (x2f8_morphBallState == EPlayerMorphBallState::Morphed &&
            x26c_attachedActor != kInvalidUniqueId)
        {
            if (ControlMapper::GetPressInput(ControlMapper::ECommands::TurnLeft, input) ||
                ControlMapper::GetPressInput(ControlMapper::ECommands::TurnRight, input) ||
                ControlMapper::GetPressInput(ControlMapper::ECommands::Forward, input) ||
                ControlMapper::GetPressInput(ControlMapper::ECommands::Backward, input) ||
                ControlMapper::GetPressInput(ControlMapper::ECommands::JumpOrBoost, input))
            {
                xa28_attachedActorStruggle += input.DeltaTime() * 600.f * input.DeltaTime();
                if (xa28_attachedActorStruggle > 1.f)
                    xa28_attachedActorStruggle = 1.f;
            }
            else
            {
                float tmp = 7.5f * input.DeltaTime();
                xa28_attachedActorStruggle -= input.DeltaTime() * std::min(1.f, xa28_attachedActorStruggle * tmp + tmp);
                if (xa28_attachedActorStruggle < 0.f)
                    xa28_attachedActorStruggle = 0.f;
            }
        }
    }

    UpdateCameraState(mgr);
    UpdateMorphBallState(input, mgr);
    UpdateCameraTimers(input.DeltaTime(), input);
    UpdateFootstepSounds(input, mgr, input.DeltaTime());
    x2a8_timeSinceJump += input.DeltaTime();

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
    return x2f4_cameraState != EPlayerCameraState::Spawned || !cineCam || (cineCam->GetW1() & 0x80) == 0;
}

void CPlayer::CalculateLeaveMorphBallDirection(const CFinalInput& input)
{
    if (x2f8_morphBallState != EPlayerMorphBallState::Morphed)
    {
        x518_leaveMorphDir = x50c_moveDir;
    }
    else
    {
        if (ControlMapper::GetAnalogInput(ControlMapper::ECommands::Forward, input) > 0.3f ||
            ControlMapper::GetAnalogInput(ControlMapper::ECommands::Backward, input) > 0.3f ||
            ControlMapper::GetAnalogInput(ControlMapper::ECommands::TurnLeft, input) > 0.3f ||
            ControlMapper::GetAnalogInput(ControlMapper::ECommands::TurnRight, input) > 0.3f)
        {
            if (x138_velocity.magnitude() > 0.5f)
                x518_leaveMorphDir = x50c_moveDir;
        }
    }
}

void CPlayer::CalculatePlayerControlDirection(CStateManager& mgr)
{
    if (x9c4_30_controlDirOverride)
    {
        if (x9d8_controlDirOverrideDir.canBeNormalized())
        {
            x540_controlDir = x9d8_controlDirOverrideDir.normalized();
            x54c_controlDirFlat = x9d8_controlDirOverrideDir;
            x54c_controlDirFlat.z = 0.f;
            if (x54c_controlDirFlat.canBeNormalized())
                x54c_controlDirFlat.normalize();
            else
                x540_controlDir = x54c_controlDirFlat = zeus::CVector3f::skForward;
        }
        else
        {
            x540_controlDir = x54c_controlDirFlat = zeus::CVector3f::skForward;
        }
    }
    else
    {
        zeus::CVector3f camToPlayer =
            GetTranslation() - mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTranslation();
        if (!camToPlayer.canBeNormalized())
        {
            x540_controlDir = x54c_controlDirFlat = zeus::CVector3f::skForward;
        }
        else
        {
            zeus::CVector3f camToPlayerFlat(camToPlayer.x, camToPlayer.y, 0.f);
            if (camToPlayerFlat.canBeNormalized())
            {
                if (camToPlayerFlat.magnitude() > g_tweakBall->GetBallCameraControlDistance())
                {
                    x540_controlDir = camToPlayer.normalized();
                    if (camToPlayerFlat.canBeNormalized())
                    {
                        camToPlayerFlat.normalize();
                        switch (x2f8_morphBallState)
                        {
                        case EPlayerMorphBallState::Morphed:
                            x54c_controlDirFlat = camToPlayerFlat;
                            break;
                        default:
                            x540_controlDir = GetTransform().basis[1];
                            x54c_controlDirFlat = x540_controlDir;
                            x54c_controlDirFlat.z = 0.f;
                            if (x54c_controlDirFlat.canBeNormalized())
                                x54c_controlDirFlat.normalize();
                        }
                    }
                    else if (x2f8_morphBallState != EPlayerMorphBallState::Morphed)
                    {
                        x540_controlDir = GetTransform().basis[1];
                        x54c_controlDirFlat.z = 0.f;
                        if (x54c_controlDirFlat.canBeNormalized())
                            x54c_controlDirFlat.normalize();
                    }
                }
                else
                {
                    if (x4fc_flatMoveSpeed < 0.25f)
                    {
                        x540_controlDir = camToPlayer;
                        x54c_controlDirFlat = camToPlayerFlat;
                    }
                    else if (x2f8_morphBallState != EPlayerMorphBallState::Morphed)
                    {
                        x540_controlDir = GetTransform().basis[1];
                        x54c_controlDirFlat.z = 0.f;
                        if (x54c_controlDirFlat.canBeNormalized())
                            x54c_controlDirFlat.normalize();
                    }
                }
            }
        }
    }
}

void CPlayer::CalculatePlayerMovementDirection(float dt)
{
    if (x2f8_morphBallState == EPlayerMorphBallState::Morphing ||
        x2f8_morphBallState == EPlayerMorphBallState::Unmorphing)
        return;

    zeus::CVector3f delta = GetTranslation() - x524_lastPosForDirCalc;
    if (delta.canBeNormalized() && delta.magnitude() > 0.02f)
    {
        x53c_timeMoving += dt;
        x4f8_moveSpeed = std::fabs(delta.magnitude() / dt);
        x500_lookDir = delta.normalized();
        zeus::CVector3f flatDelta(delta.x, delta.y, 0.f);
        if (flatDelta.canBeNormalized())
        {
            x4fc_flatMoveSpeed = std::fabs(flatDelta.magnitude() / dt);
            flatDelta.normalize();
            switch (x2f8_morphBallState)
            {
            case EPlayerMorphBallState::Morphed:
                if (x4fc_flatMoveSpeed > 0.25f)
                    x50c_moveDir = flatDelta;
                x530_gunDir = x50c_moveDir;
                x524_lastPosForDirCalc = GetTranslation();
                break;
            default:
                x500_lookDir = GetTransform().basis[1];
                x50c_moveDir = x500_lookDir;
                x50c_moveDir.z = 0.f;
                if (x50c_moveDir.canBeNormalized())
                    x50c_moveDir.normalize();
                x530_gunDir = x50c_moveDir;
                x524_lastPosForDirCalc = GetTranslation();
                break;
            }
        }
        else
        {
            if (x2f8_morphBallState != EPlayerMorphBallState::Morphed)
            {
                x500_lookDir = GetTransform().basis[1];
                x50c_moveDir = x500_lookDir;
                x50c_moveDir.z = 0.f;
                if (x50c_moveDir.canBeNormalized())
                    x50c_moveDir.normalize();
                x530_gunDir = x50c_moveDir;
                x524_lastPosForDirCalc = GetTranslation();
            }
            x4fc_flatMoveSpeed = 0.f;
        }
    }
    else
    {
        x53c_timeMoving = 0.f;
        switch (x2f8_morphBallState)
        {
        case EPlayerMorphBallState::Morphed:
        case EPlayerMorphBallState::Morphing:
        case EPlayerMorphBallState::Unmorphing:
            x500_lookDir = x50c_moveDir;
            break;
        default:
            x500_lookDir = GetTransform().basis[1];
            x50c_moveDir = x500_lookDir;
            x50c_moveDir.z = 0.f;
            if (x50c_moveDir.canBeNormalized())
                x50c_moveDir.normalize();
            x530_gunDir = x50c_moveDir;
            x524_lastPosForDirCalc = GetTranslation();
            break;
        }
        x4f8_moveSpeed = 0.f;
        x4fc_flatMoveSpeed = 0.f;
    }

    x50c_moveDir.z = 0.f;
    if (x50c_moveDir.canBeNormalized())
        x500_lookDir.normalize();
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
        if (!stateMgr.GetCameraManager()->IsInCinematicCamera() && xa0c_iceTextureId.IsValid())
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

void CPlayer::Freeze(CStateManager& stateMgr, CAssetId steamTxtr, u16 sfx, CAssetId iceTxtr)
{
    if (stateMgr.GetCameraManager()->IsInCinematicCamera() || GetFrozenState())
        return;

    bool showMsg;
    if (x2f8_morphBallState == EPlayerMorphBallState::Unmorphed)
        showMsg = g_GameState->SystemOptions().GetShowFrozenFpsMessage();
    else
        showMsg = g_GameState->SystemOptions().GetShowFrozenBallMessage();

    if (showMsg)
    {
        const char16_t* msg =
            g_MainStringTable->GetString(int(x2f8_morphBallState >= EPlayerMorphBallState::Morphed) + 19);
        CHUDMemoParms parms(5.f, true, false, false);
        MP1::CSamusHud::DisplayHudMemo(msg, parms);
    }

    x750_frozenTimeout = x758_frozenTimeoutBias + g_tweakPlayer->GetFrozenTimeout();
    x754_iceBreakJumps = -x75c_additionalIceBreakJumps;

    CPhysicsActor::Stop();
    ClearForcesAndTorques();
    if (x3b8_grappleState != EGrappleState::None)
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

    switch (x2f8_morphBallState)
    {
    case EPlayerMorphBallState::Morphed:
        x490_gun->ProcessInput(input, mgr);
        break;
    case EPlayerMorphBallState::Unmorphed:
    case EPlayerMorphBallState::Morphing:
    case EPlayerMorphBallState::Unmorphing:
        if (ControlMapper::GetPressInput(ControlMapper::ECommands::JumpOrBoost, input))
        {
            if (x754_iceBreakJumps != 0)
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

void CPlayer::UpdateStepCameraZBias(float dt)
{
    float newBias = GetTranslation().z + GetUnbiasedEyeHeight();
    if (x258_movementState == EPlayerMovementState::OnGround && !IsMorphBallTransitioning())
    {
        float oldBias = newBias;
        if (!x9c5_31_stepCameraZBiasDirty)
        {
            float delta = newBias - x9cc_stepCameraZBias;
            float newDelta = 5.f * dt;
            if (delta > 0.f)
            {
                if (delta > dt * x138_velocity.z && delta > newDelta)
                {
                    if (delta > GetStepUpHeight())
                        newDelta += delta - GetStepUpHeight();
                    newBias = x9cc_stepCameraZBias + newDelta;
                }
            }
            else
            {
                if (delta < dt * x138_velocity.z && delta < -newDelta)
                {
                    if (delta < -GetStepDownHeight())
                        newDelta += -delta - GetStepDownHeight();
                    newBias = x9cc_stepCameraZBias + newDelta;
                }
            }
        }
        x9c8_eyeZBias = newBias - oldBias;
    }
    else
    {
        x9c8_eyeZBias = 0.f;
    }
    x9cc_stepCameraZBias = newBias;
    x9c5_31_stepCameraZBiasDirty = false;
}

void CPlayer::UpdateWaterSurfaceCameraBias(CStateManager& mgr)
{
    if (TCastToConstPtr<CScriptWater> water = mgr.GetObjectById(xc4_fluidId))
    {
        float waterZ = water->GetTriggerBoundsWR().max.z;
        float biasToEyeDelta = GetEyePosition().z - x9c8_eyeZBias;
        float waterToDeltaDelta = biasToEyeDelta - waterZ;
        if (biasToEyeDelta >= waterZ && waterToDeltaDelta <= 0.25f)
            x9c8_eyeZBias += waterZ + 0.25f - biasToEyeDelta;
        else if (biasToEyeDelta < waterZ && waterToDeltaDelta >= -0.2f)
            x9c8_eyeZBias += waterZ - 0.2f - biasToEyeDelta;
    }
}

void CPlayer::UpdatePhazonCameraShake(float dt, CStateManager& mgr)
{

}

void CPlayer::UpdatePhazonDamage(float dt, CStateManager& mgr)
{

}

void CPlayer::ResetPlayerHintState()
{
    x9c4_26_ = true;
    x9c4_27_canEnterMorphBall = true;
    x9c4_28_canLeaveMorphBall = true;
    x9c4_30_controlDirOverride = false;
    x9c6_24_extendTargetDistance = false;
    x9c6_26_outOfBallLookAtHint = false;
    x9c4_29_ = false;
    x9c6_29_disableInput = false;
    x9c7_25_outOfBallLookAtHintActor = false;
    x768_morphball->SetX1DE4_25(true);
    ResetControlDirectionInterpolation();
}

bool CPlayer::SetAreaPlayerHint(const CScriptPlayerHint& hint, CStateManager& mgr)
{
    x9c4_26_ = (hint.GetOverrideFlags() & 0x1) != 0;
    x9c4_27_canEnterMorphBall = (hint.GetOverrideFlags() & 0x40) == 0;
    x9c4_28_canLeaveMorphBall = (hint.GetOverrideFlags() & 0x20) == 0;
    x9c4_30_controlDirOverride = (hint.GetOverrideFlags() & 0x2) != 0;
    if (x9c4_30_controlDirOverride)
        x9d8_controlDirOverrideDir = hint.GetTransform().basis[1];
    x9c6_24_extendTargetDistance = (hint.GetOverrideFlags() & 0x4) != 0;
    x9c6_26_outOfBallLookAtHint = (hint.GetOverrideFlags() & 0x8) != 0;
    x9c4_29_ = (hint.GetOverrideFlags() & 0x10) != 0;
    x9c6_29_disableInput = (hint.GetOverrideFlags() & 0x80) != 0;
    x9c7_25_outOfBallLookAtHintActor = (hint.GetOverrideFlags() & 0x4000) != 0;
    x768_morphball->SetX1DE4_25((hint.GetOverrideFlags() & 0x100) == 0);
    bool switchedVisor = false;
    if ((hint.GetOverrideFlags() & 0x200) != 0)
    {
        if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::CombatVisor))
            mgr.GetPlayerState()->StartVisorTransition(CPlayerState::EPlayerVisor::Combat);
        switchedVisor = true;
    }
    if ((hint.GetOverrideFlags() & 0x400) != 0)
    {
        if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::ScanVisor))
            mgr.GetPlayerState()->StartVisorTransition(CPlayerState::EPlayerVisor::Scan);
        switchedVisor = true;
    }
    if ((hint.GetOverrideFlags() & 0x800) != 0)
    {
        if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::ThermalVisor))
            mgr.GetPlayerState()->StartVisorTransition(CPlayerState::EPlayerVisor::Thermal);
        switchedVisor = true;
    }
    if ((hint.GetOverrideFlags() & 0x1000) != 0)
    {
        if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::XRayVisor))
            mgr.GetPlayerState()->StartVisorTransition(CPlayerState::EPlayerVisor::XRay);
        switchedVisor = true;
    }
    return switchedVisor;
}

void CPlayer::AddToPlayerHintRemoveList(TUniqueId id, CStateManager& mgr)
{
    if (TCastToPtr<CScriptPlayerHint> hint = mgr.ObjectById(id))
    {
        bool alreadyAdded = false;
        for (TUniqueId existId : x93c_playerHintsToRemove)
            if (id == existId)
                return;
        if (!alreadyAdded && x93c_playerHintsToRemove.size() != 32)
            x93c_playerHintsToRemove.push_back(id);
    }
}

void CPlayer::AddToPlayerHintAddList(TUniqueId id, CStateManager& mgr)
{
    if (TCastToPtr<CScriptPlayerHint> hint = mgr.ObjectById(id))
    {
        bool alreadyAdded = false;
        for (TUniqueId existId : x980_playerHintsToAdd)
            if (id == existId)
                return;
        if (!alreadyAdded && x980_playerHintsToAdd.size() != 32)
            x980_playerHintsToAdd.push_back(id);
    }
}

void CPlayer::DeactivatePlayerHint(TUniqueId id, CStateManager& mgr)
{
    if (TCastToPtr<CScriptPlayerHint> hint = mgr.ObjectById(id))
    {
        bool alreadyAdded = false;
        for (TUniqueId existId : x93c_playerHintsToRemove)
            if (id == existId)
                return;
        if (!alreadyAdded && x93c_playerHintsToRemove.size() != 32)
        {
            x93c_playerHintsToRemove.push_back(id);
            hint->ClearObjectList();
            hint->SetDeactivated();
        }
    }
}

void CPlayer::UpdatePlayerHints(CStateManager& mgr)
{
    bool removedHint = false;
    for (auto it = x838_playerHints.begin() ; it != x838_playerHints.end() ;)
    {
        auto& p = *it;
        TCastToPtr<CScriptPlayerHint> hint = mgr.ObjectById(p.second);
        if (!hint)
        {
            it = x838_playerHints.erase(it);
            removedHint = true;
        }
        else
        {
            ++it;
        }
    }

    bool needsNewHint = false;
    for (TUniqueId id : x93c_playerHintsToRemove)
    {
        for (auto it = x838_playerHints.begin() ; it != x838_playerHints.end() ;)
        {
            if (it->second == id)
            {
                it = x838_playerHints.erase(it);
                if (id == x830_playerHint)
                    needsNewHint = true;
                break;
            }
            ++it;
        }
    }
    x93c_playerHintsToRemove.clear();

    bool addedHint = false;
    for (TUniqueId id : x980_playerHintsToAdd)
    {
        if (TCastToPtr<CScriptPlayerHint> hint = mgr.ObjectById(id))
        {
            bool exists = false;
            for (auto& p : x838_playerHints)
                if (p.second == id)
                {
                    exists = true;
                    break;
                }
            if (!exists)
            {
                x838_playerHints.emplace_back(hint->GetPriority(), id);
                addedHint = true;
            }
        }
    }
    x980_playerHintsToAdd.clear();

    if (needsNewHint || addedHint || removedHint)
    {
        std::sort(x838_playerHints.begin(), x838_playerHints.end(),
        [](const auto& a, const auto& b) { return a.first < b.first; });

        if ((needsNewHint || removedHint) && x838_playerHints.empty())
        {
            x830_playerHint = kInvalidUniqueId;
            x834_ = 1000;
            ResetPlayerHintState();
            return;
        }

        CScriptPlayerHint* foundHint = nullptr;
        bool foundHintInArea = false;
        for (auto& p : x838_playerHints)
        {
            if (TCastToPtr<CScriptPlayerHint> hint = mgr.ObjectById(p.second))
            {
                foundHint = hint.GetPtr();
                if (hint->GetAreaIdAlways() == mgr.GetNextAreaId())
                {
                    foundHintInArea = true;
                    break;
                }
            }
        }

        if (!foundHintInArea)
        {
            x830_playerHint = kInvalidUniqueId;
            x834_ = 1000;
            ResetPlayerHintState();
        }

        if (foundHint != nullptr && foundHintInArea && x830_playerHint != foundHint->GetUniqueId())
        {
            x830_playerHint = foundHint->GetUniqueId();
            x834_ = foundHint->GetPriority();
            if (SetAreaPlayerHint(*foundHint, mgr))
                DeactivatePlayerHint(x830_playerHint, mgr);
        }
    }
}

void CPlayer::UpdateBombJumpStuff()
{
    if (x9d0_bombJumpCount == 0)
        return;
    x9d4_bombJumpCheckDelayFrames -= 1;
    if (x9d4_bombJumpCheckDelayFrames > 0)
        return;

    zeus::CVector3f velFlat = x138_velocity;
    velFlat.z = 0.f;
    if (x258_movementState == EPlayerMovementState::OnGround ||
        (velFlat.canBeNormalized() && velFlat.magnitude() > 6.f))
        x9d0_bombJumpCount = 0;
}

void CPlayer::UpdateTransitionFilter(float dt, CStateManager& mgr)
{
    if (x824_transitionFilterTimer <= 0.f)
    {
        mgr.GetCameraFilterPass(8).DisableFilter(0.f);
        return;
    }

    x824_transitionFilterTimer += dt;
    if (x824_transitionFilterTimer > 1.25f)
    {
        x824_transitionFilterTimer = 0.f;
        mgr.GetCameraFilterPass(8).DisableFilter(0.f);
        return;
    }

    if (x824_transitionFilterTimer < 0.95f)
        return;

    float time = x824_transitionFilterTimer - 0.95f;
    zeus::CColor color(1.f, 0.87f, 0.54f, 1.f);
    if (time < 0.1f)
        color.a = 0.3f * time / 0.1f;
    else if (time >= 0.15f)
        color.a = 1.f - zeus::clamp(-1.f, (time - 0.15f) / 0.15f, 1.f) * 0.3f;
    else
        color.a = 0.3f;

    mgr.GetCameraFilterPass(8).SetFilter(EFilterType::Add, EFilterShape::ScanLinesEven, 0.f, color, {});
}

void CPlayer::ResetControlDirectionInterpolation()
{
    x9c6_25_interpolatingControlDir = false;
    x9f8_controlDirInterpTime = 0.f;
}

void CPlayer::SetControlDirectionInterpolation(float time)
{
    x9c6_25_interpolatingControlDir = true;
    x9f8_controlDirInterpTime = 0.f;
    x9fc_controlDirInterpDur = time;
}

void CPlayer::UpdatePlayerControlDirection(float dt, CStateManager& mgr)
{
    zeus::CVector3f oldControlDir = x540_controlDir;
    zeus::CVector3f oldControlDirFlat = x54c_controlDirFlat;
    CalculatePlayerControlDirection(mgr);
    if (x9c6_25_interpolatingControlDir && x2f8_morphBallState == EPlayerMorphBallState::Morphed)
    {
        x9f8_controlDirInterpTime += dt;
        if (x9f8_controlDirInterpTime > x9fc_controlDirInterpDur)
        {
            x9f8_controlDirInterpTime = x9fc_controlDirInterpDur;
            ResetControlDirectionInterpolation();
        }
        float t = zeus::clamp(-1.f, x9f8_controlDirInterpTime / x9fc_controlDirInterpDur, 1.f);
        x540_controlDir = zeus::CVector3f::lerp(oldControlDir, x540_controlDir, t);
        x54c_controlDirFlat = zeus::CVector3f::lerp(oldControlDirFlat, x540_controlDir, t);
    }
}

void CPlayer::Think(float dt, CStateManager& mgr)
{
    UpdateStepCameraZBias(dt);
    UpdateWaterSurfaceCameraBias(mgr);
    UpdatePhazonCameraShake(dt, mgr);
    UpdatePhazonDamage(dt, mgr);
    UpdateFreeLook(dt);
    UpdatePlayerHints(mgr);

    if (x2b0_outOfWaterTicks < 2)
        x2b0_outOfWaterTicks += 1;

    x9c5_24_ = x9c4_24_visorChangeRequested;
    x9c4_31_dampUnderwaterMotion = x9c5_25_splashUpdated;
    x9c5_25_splashUpdated = false;
    UpdateBombJumpStuff();

    if (0.f < x288_startingJumpTimeout)
    {
        x288_startingJumpTimeout -= dt;
        if (0.f >= x288_startingJumpTimeout)
            SetMoveState(EPlayerMovementState::StartingJump, mgr);
    }

    if (x2a0_ > 0.f)
        x2a0_ += dt;
    if (x774_samusVoiceTimeout > 0.f)
        x774_samusVoiceTimeout -= dt;
    if (0.f < x28c_sjTimer)
        x28c_sjTimer -= dt;

    x300_fallingTime += dt;
    if (x258_movementState == EPlayerMovementState::FallingMorphed && x300_fallingTime > 0.4f)
        SetMoveState(EPlayerMovementState::StartingJump, mgr);

    if (x570_immuneTimer > 0.f)
        x570_immuneTimer -= dt;

    Update(dt, mgr);
    UpdateTransitionFilter(dt, mgr);
    CalculatePlayerMovementDirection(dt);
    UpdatePlayerControlDirection(dt, mgr);

#if 0
    if (g_factoryManager == 4)
        x2b0_ = 0;
    else
        x2ac_surfaceRestraint = ESurfaceRestraints::Normal;
#endif

    if (x2f8_morphBallState == EPlayerMorphBallState::Unmorphed && x9c5_27_camSubmerged &&
        mgr.GetCameraManager()->GetFluidCounter() == 0)
    {
        if (auto water = GetVisorRunoffEffect(mgr))
        {
            if (const auto& effect = water->GetVisorRunoffEffect())
            {
                CHUDBillboardEffect* sheets = new CHUDBillboardEffect(*effect, {}, mgr.AllocateUniqueId(),
                                                                      true, "WaterSheets",
                                                                      CHUDBillboardEffect::GetNearClipDistance(mgr),
                                                                      CHUDBillboardEffect::GetScaleForPOV(mgr),
                                                                      zeus::CColor::skWhite, zeus::CVector3f::skOne,
                                                                      zeus::CVector3f::skZero);
                mgr.AddObject(sheets);
            }
            CSfxHandle hnd = CSfxManager::SfxStart(water->GetVisorRunoffSfx(), 1.f, 0.f,
                                                   false, 0x7f, false, kInvalidAreaId);
            ApplySubmergedPitchBend(hnd);
        }
    }
    x9c5_27_camSubmerged = mgr.GetCameraManager()->GetFluidCounter() != 0;

    if (x2f8_morphBallState != EPlayerMorphBallState::Morphed)
    {
        if (std::fabs(x34_transform.basis[0].z) > FLT_EPSILON ||
            std::fabs(x34_transform.basis[1].z) > FLT_EPSILON)
        {
            zeus::CVector3f backupTranslation = GetTranslation();
            zeus::CVector3f lookDirFlat(x34_transform.basis[1].x, x34_transform.basis[1].y, 0.f);
            if (lookDirFlat.canBeNormalized())
            {
                SetTransform(zeus::lookAt(zeus::CVector3f::skZero, lookDirFlat.normalized()));
            }
            else
            {
                SetTransform(zeus::CTransform::Identity());
            }
            SetTranslation(backupTranslation);
        }
    }

    x794_lastVelocity = x138_velocity;
}

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
            x2f8_morphBallState != EPlayerMorphBallState::Morphed &&
            x300_fallingTime > 0.3f)
        {
            if (x258_movementState != EPlayerMovementState::Falling)
            {
                float hardThres = 30.f * 2.f * -g_tweakPlayer->GetNormalGravAccel();
                hardThres = (hardThres != 0.f) ? hardThres * std::sqrt(hardThres) : 0.f;
                float landVol = zeus::clamp(95.f, 1.6f * -x794_lastVelocity.z + 95.f, 127.f) / 127.f;
                u16 landSfx;
                if (-x794_lastVelocity.z < hardThres)
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

                float rumbleMag = -x794_lastVelocity.z / 110.f;
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
                 x2f8_morphBallState == EPlayerMorphBallState::Morphed)
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
        if (x2f8_morphBallState == EPlayerMorphBallState::Morphed)
            if (x768_morphball->GetX187c() == 1)
                break;
        if (x2f8_morphBallState != EPlayerMorphBallState::Morphed)
            SetMoveState(EPlayerMovementState::Falling, mgr);
        else if (x258_movementState == EPlayerMovementState::OnGround)
            SetMoveState(EPlayerMovementState::FallingMorphed, mgr);
        break;
    case EScriptObjectMessage::LandOnNotFloor:
        if (x2f8_morphBallState == EPlayerMorphBallState::Morphed &&
            x768_morphball->GetX187c() == 1 &&
            x258_movementState != EPlayerMovementState::StartingJump)
            SetMoveState(EPlayerMovementState::StartingJump, mgr);
        break;
    case EScriptObjectMessage::OnIceSurface:
        x2ac_surfaceRestraint = ESurfaceRestraints::Ice;
        break;
    case EScriptObjectMessage::OnMudSlowSurface:
        x2ac_surfaceRestraint = ESurfaceRestraints::MudSlow;
        break;
    case EScriptObjectMessage::OnNormalSurface:
        x2ac_surfaceRestraint = ESurfaceRestraints::Normal;
        break;
    case EScriptObjectMessage::InSnakeWeed:
        x2ac_surfaceRestraint = ESurfaceRestraints::SnakeWeed;
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
                case CFluidPlane::EFluidType::NormalWater:
                    x2b0_outOfWaterTicks = 0;
                    break;
                case CFluidPlane::EFluidType::Lava:
                case CFluidPlane::EFluidType::ThickLava:
                    x2ac_surfaceRestraint = ESurfaceRestraints::Lava;
                    break;
                case CFluidPlane::EFluidType::PoisonWater:
                    x2b0_outOfWaterTicks = 0;
                    break;
                case CFluidPlane::EFluidType::PhazonFluid:
                    x2ac_surfaceRestraint = ESurfaceRestraints::PhazonFluid;
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
        x378_orbitPreventionTimer = g_tweakPlayer->GetOrbitPreventionTime();
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
        x730_transitionModels.clear();
        break;
    default: break;
    }

    x490_gun->AcceptScriptMsg(msg, sender, mgr);
    x768_morphball->AcceptScriptMsg(msg, sender, mgr);
    CActor::AcceptScriptMsg(msg, sender, mgr);
}

void CPlayer::SetVisorSteam(float f1, float f2, float f3, CAssetId txtr, bool affectsThermal)
{
    x7a0_visorSteam.SetSteam(f1, f2, f3, txtr, affectsThermal);
}

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
    if (x2f8_morphBallState != EPlayerMorphBallState::Unmorphed ||
        x258_movementState != EPlayerMovementState::OnGround ||
        x3dc_inFreeLook || x3dd_lookButtonHeld)
        return;

    float sfxVol = 1.f;
    x78c_footstepSfxTimer += dt;
    float turn = TurnInput(input);
    float forward = std::fabs(ForwardInput(input, turn));
    turn = std::fabs(turn);
    float sfxDelay = 0.f;
    if (forward > 0.05f || x304_orbitState != EPlayerOrbitState::NoOrbit)
    {
        float vel = std::min(1.f, x138_velocity.magnitude() / GetActualFirstPersonMaxVelocity(dt));
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

void CPlayer::UpdateCrosshairsState(const CFinalInput& input)
{
    x9c4_25_showCrosshairs = ControlMapper::GetDigitalInput(ControlMapper::ECommands::ShowCrosshairs, input);
}

void CPlayer::UpdateVisorTransition(float dt, CStateManager& mgr)
{
    if (mgr.GetPlayerState()->GetIsVisorTransitioning())
        mgr.GetPlayerState()->UpdateVisorTransition(dt);
}

static const std::pair<CPlayerState::EItemType, ControlMapper::ECommands> skVisorToItemMapping[] =
{
    {CPlayerState::EItemType::CombatVisor, ControlMapper::ECommands::NoVisor},
    {CPlayerState::EItemType::XRayVisor, ControlMapper::ECommands::XrayVisor},
    {CPlayerState::EItemType::ScanVisor, ControlMapper::ECommands::InviroVisor},
    {CPlayerState::EItemType::ThermalVisor, ControlMapper::ECommands::ThermoVisor}
};

void CPlayer::UpdateVisorState(const CFinalInput& input, float dt, CStateManager& mgr)
{
    x7a0_visorSteam.Update(dt);
    if (x7a0_visorSteam.AffectsThermal())
        mgr.SetThermalColdScale2(mgr.GetThermalColdScale2() + x7a0_visorSteam.GetAlpha());

    if (x304_orbitState == EPlayerOrbitState::Grapple ||
        TCastToPtr<CScriptGrapplePoint>(mgr.ObjectById(x310_orbitTargetId)) ||
        x2f8_morphBallState != EPlayerMorphBallState::Unmorphed ||
        mgr.GetPlayerState()->GetIsVisorTransitioning() ||
        x3a8_scanState != EPlayerScanState::NotScanning)
        return;

    if (mgr.GetPlayerState()->GetTransitioningVisor() == CPlayerState::EPlayerVisor::Scan &&
        (ControlMapper::GetDigitalInput(ControlMapper::ECommands::FireOrBomb, input) ||
         ControlMapper::GetDigitalInput(ControlMapper::ECommands::MissileOrPowerBomb, input)) &&
        mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::CombatVisor))
    {
        mgr.GetPlayerState()->StartVisorTransition(CPlayerState::EPlayerVisor::Combat);
        DrawGun(mgr);
    }

    for (int i=0 ; i<4 ; ++i)
    {
        if (mgr.GetPlayerState()->HasPowerUp(skVisorToItemMapping[i].first) &&
            ControlMapper::GetPressInput(skVisorToItemMapping[i].second, input))
        {
            x9c4_24_visorChangeRequested = true;
            CPlayerState::EPlayerVisor visor = CPlayerState::EPlayerVisor(i);
            if (mgr.GetPlayerState()->GetTransitioningVisor() != visor)
            {
                mgr.GetPlayerState()->StartVisorTransition(visor);
                if (visor == CPlayerState::EPlayerVisor::Scan)
                    HolsterGun(mgr);
                else
                    DrawGun(mgr);
            }
        }
    }
}

void CPlayer::UpdateGunState(const CFinalInput& input, CStateManager& mgr)
{
    switch (x498_gunHolsterState)
    {
    case EGunHolsterState::Drawn:
    {
        bool needsHolster = false;
        if (g_tweakPlayer->GetGunButtonTogglesHolster())
        {
            if (ControlMapper::GetPressInput(ControlMapper::ECommands::ToggleHolster, input))
                needsHolster = true;
            if (!ControlMapper::GetDigitalInput(ControlMapper::ECommands::FireOrBomb, input) &&
                !ControlMapper::GetDigitalInput(ControlMapper::ECommands::MissileOrPowerBomb, input) &&
                g_tweakPlayer->GetGunNotFiringHolstersGun())
            {
                x49c_gunHolsterRemTime -= input.DeltaTime();
                if (x49c_gunHolsterRemTime <= 0.f)
                    needsHolster = true;
            }
        }
        else
        {
            if (!ControlMapper::GetDigitalInput(ControlMapper::ECommands::FireOrBomb, input) &&
                !ControlMapper::GetDigitalInput(ControlMapper::ECommands::MissileOrPowerBomb, input) &&
                x490_gun->IsFidgeting())
            {
                if (g_tweakPlayer->GetGunNotFiringHolstersGun())
                    x49c_gunHolsterRemTime -= input.DeltaTime();
            }
            else
            {
                x49c_gunHolsterRemTime = g_tweakPlayerGun->GetGunNotFiringTime();
            }
        }

        if (needsHolster)
            HolsterGun(mgr);
        break;
    }
    case EGunHolsterState::Drawing:
    {
        if (x49c_gunHolsterRemTime > 0.f)
        {
            x49c_gunHolsterRemTime -= input.DeltaTime();
        } else
        {
            x498_gunHolsterState = EGunHolsterState::Drawn;
            x49c_gunHolsterRemTime = g_tweakPlayerGun->GetGunNotFiringTime();
        }
        break;
    }
    case EGunHolsterState::Holstered:
    {
        bool needsDraw = false;
        if (ControlMapper::GetDigitalInput(ControlMapper::ECommands::FireOrBomb, input) ||
            ControlMapper::GetDigitalInput(ControlMapper::ECommands::MissileOrPowerBomb, input) ||
            x3b8_grappleState == EGrappleState::None ||
            (g_tweakPlayer->GetGunButtonTogglesHolster() &&
             ControlMapper::GetPressInput(ControlMapper::ECommands::ToggleHolster, input)))
            needsDraw = true;

        if (x3b8_grappleState == EGrappleState::None &&
            (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Scan ||
             mgr.GetPlayerState()->GetTransitioningVisor() == CPlayerState::EPlayerVisor::Scan ))
            needsDraw = false;

        if (needsDraw)
            DrawGun(mgr);
        break;
    }
    case EGunHolsterState::Holstering:
        if (x49c_gunHolsterRemTime > 0.f)
            x49c_gunHolsterRemTime -= input.DeltaTime();
        else
            x498_gunHolsterState = EGunHolsterState::Holstered;
        break;
    }
}

void CPlayer::ResetGun(CStateManager& mgr)
{
    x498_gunHolsterState = EGunHolsterState::Holstered;
    x49c_gunHolsterRemTime = 0.f;
    x490_gun->CancelFiring(mgr);
    ResetAimTargetPrediction(kInvalidUniqueId);
}

void CPlayer::UpdateArmAndGunTransforms(float dt, CStateManager& mgr)
{
    zeus::CVector3f grappleOffset;
    zeus::CVector3f gunOffset;
    if (x2f8_morphBallState == EPlayerMorphBallState::Morphed)
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
    x530_gunDir = xf.basis[1];
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
    if (x3dc_inFreeLook || x3dd_lookButtonHeld)
    {
        x294_jumpCameraTimer = 0.f;
        x29c_fallCameraTimer = 0.f;
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
        x294_jumpCameraTimer > 0.f && !x2a4_cancelCameraPitch && x298_jumpPresses <= 2)
        x294_jumpCameraTimer += dt;

    if (x29c_fallCameraTimer > 0.f && !x2a4_cancelCameraPitch)
        x29c_fallCameraTimer += dt;
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
    if (x2f8_morphBallState == EPlayerMorphBallState::Morphing)
        viewGunPos = camXf * (gunPos - zeus::CVector3f(0.f, 0.f, eyeHeight));
    else
        viewGunPos = camXf.rotate(gunPos - zeus::CVector3f(0.f, 0.f, eyeHeight)) + GetEyePosition();

    zeus::CUnitVector3f rightDir(gunXf.basis[0]);
    gunXf.origin = viewGunPos;

    switch (x498_gunHolsterState)
    {
    case EGunHolsterState::Drawing:
    {
        float liftAngle = zeus::clamp(-1.f, x49c_gunHolsterRemTime / 0.45f, 1.f);
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
            zeus::clamp(-1.f, x49c_gunHolsterRemTime / g_tweakPlayerGun->GetGunHolsterTime(), 1.f);
        if (x2f8_morphBallState == EPlayerMorphBallState::Morphing)
            liftAngle = 1.f - zeus::clamp(-1.f, x49c_gunHolsterRemTime / 0.1f, 1.f);
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
    zeus::CTransform assistXf = xf;
    if (TCastToConstPtr<CActor> target = mgr.GetObjectById(x3f4_aimTarget))
    {
        zeus::CVector3f gunToTarget = x480_assistedTargetAim - xf.origin;
        zeus::CVector3f gunToTargetFlat = gunToTarget;
        gunToTargetFlat.z = 0.f;
        float gunToTargetFlatMag = gunToTargetFlat.magnitude();
        zeus::CVector3f gunDirFlat = xf.basis[1];
        gunDirFlat.z = 0.f;
        float gunDirFlatMag = gunDirFlat.magnitude();
        if (gunToTargetFlat.canBeNormalized() && gunDirFlat.canBeNormalized())
        {
            gunToTargetFlat = gunToTargetFlat / gunToTargetFlatMag;
            gunDirFlat = gunDirFlat / gunDirFlatMag;
            float vAngleDelta = std::atan2(gunToTarget.z, gunToTargetFlatMag) -
                std::atan2(xf.basis[1].z, gunDirFlatMag);
            bool hasVAngleDelta = true;
            if (!x9c6_27_aimingAtProjectile && std::fabs(vAngleDelta) > g_tweakPlayer->GetAimAssistVerticalAngle())
            {
                if (g_tweakPlayer->GetAssistedAimingIgnoreVertical())
                {
                    vAngleDelta = 0.f;
                    hasVAngleDelta = false;
                }
                else if (vAngleDelta > 0.f)
                {
                    vAngleDelta = g_tweakPlayer->GetAimAssistVerticalAngle();
                }
                else
                {
                    vAngleDelta = -g_tweakPlayer->GetAimAssistVerticalAngle();
                }
            }

            bool targetToLeft = gunDirFlat.cross(gunToTargetFlat).z > 0.f;
            float hAngleDelta = std::acos(zeus::clamp(-1.f, gunDirFlat.dot(gunToTargetFlat), 1.f));
            bool hasHAngleDelta = true;
            if (!x9c6_27_aimingAtProjectile && std::fabs(hAngleDelta) > g_tweakPlayer->GetAimAssistHorizontalAngle())
            {
                hAngleDelta = g_tweakPlayer->GetAimAssistHorizontalAngle();
                if (g_tweakPlayer->GetAssistedAimingIgnoreHorizontal())
                {
                    hAngleDelta = 0.f;
                    hasHAngleDelta = false;
                }
            }

            if (targetToLeft)
                hAngleDelta = -hAngleDelta;

            if (!hasVAngleDelta || !hasHAngleDelta)
            {
                vAngleDelta = 0.f;
                hAngleDelta = 0.f;
            }

            gunToTarget.x = std::sin(hAngleDelta) * std::cos(vAngleDelta);
            gunToTarget.y = std::cos(hAngleDelta) * std::cos(vAngleDelta);
            gunToTarget.z = std::sin(vAngleDelta);
            gunToTarget = xf.rotate(gunToTarget);
            assistXf = zeus::lookAt(zeus::CVector3f::skZero, gunToTarget, zeus::CVector3f::skUp);
        }
    }

    x490_gun->SetAssistAimTransform(assistXf);
}

void CPlayer::UpdateAimTargetPrediction(const zeus::CTransform& xf, const CStateManager& mgr)
{
    if (x3f4_aimTarget != kInvalidUniqueId)
    {
        if (TCastToConstPtr<CActor> target = mgr.GetObjectById(x3f4_aimTarget))
        {
            x9c6_27_aimingAtProjectile = TCastToConstPtr<CGameProjectile>(target.GetPtr());
            zeus::CVector3f instantTarget = target->GetAimPosition(mgr, 0.f);
            zeus::CVector3f gunToTarget = instantTarget - xf.origin;
            float timeToTarget = gunToTarget.magnitude() / x490_gun->GetBeamVelocity();
            zeus::CVector3f predictTarget = target->GetAimPosition(mgr, timeToTarget);
            zeus::CVector3f predictOffset = predictTarget - instantTarget;
            x3f8_targetAimPosition = instantTarget;
            if (predictOffset.magnitude() < 0.1f)
                x404_aimTargetAverage.AddValue(zeus::CVector3f::skZero);
            else
                x404_aimTargetAverage.AddValue(predictOffset);
            if (auto avg = x404_aimTargetAverage.GetAverage())
                x480_assistedTargetAim = instantTarget + *avg;
            else
                x480_assistedTargetAim = predictTarget;
        }
    }
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
    x49c_gunHolsterRemTime = 0.45f;
    x490_gun->ResetIdle(mgr);
}

void CPlayer::HolsterGun(CStateManager& mgr)
{
    if (x498_gunHolsterState == EGunHolsterState::Holstered ||
        x498_gunHolsterState == EGunHolsterState::Holstering)
        return;
    float time = x2f8_morphBallState == EPlayerMorphBallState::Morphing ? 0.1f :
                 g_tweakPlayerGun->GetGunHolsterTime();
    if (x498_gunHolsterState == EGunHolsterState::Drawing)
        x49c_gunHolsterRemTime = time * (1.f - x49c_gunHolsterRemTime / 0.45f);
    else
        x49c_gunHolsterRemTime = time;
    x498_gunHolsterState = EGunHolsterState::Holstering;
    x490_gun->CancelFiring(mgr);
    ResetAimTargetPrediction(kInvalidUniqueId);
}

bool CPlayer::IsMorphBallTransitioning() const
{
    switch (x2f8_morphBallState)
    {
    case EPlayerMorphBallState::Morphing:
    case EPlayerMorphBallState::Unmorphing:
        return true;
    default:
        return false;
    }
}

void CPlayer::UpdateGrappleArmTransform(const zeus::CVector3f& offset, CStateManager& mgr, float dt)
{
    zeus::CTransform armXf = x34_transform;
    zeus::CVector3f armPosition = x34_transform.rotate(offset) + x34_transform.origin;
    armXf.origin = armPosition;
    if (x2f8_morphBallState != EPlayerMorphBallState::Unmorphed)
    {
        x490_gun->GetGrappleArm().SetTransform(armXf);
    }
    else if (!x490_gun->GetGrappleArm().IsArmMoving())
    {
        zeus::CVector3f lookDir = x34_transform.basis[1];
        zeus::CVector3f armToTarget = x490_gun->GetGrappleArm().GetTransform().basis[1];
        if (lookDir.canBeNormalized())
        {
            lookDir.normalize();
            if (x3b8_grappleState != EGrappleState::None)
            {
                if (TCastToPtr<CActor> target = mgr.ObjectById(x310_orbitTargetId))
                {
                    armToTarget = target->GetTranslation() - armPosition;
                    zeus::CVector3f armToTargetFlat = armToTarget;
                    armToTargetFlat.z = 0.f;
                    if (armToTarget.canBeNormalized())
                        armToTarget.normalize();
                    if (armToTargetFlat.canBeNormalized() && x3b8_grappleState != EGrappleState::Firing)
                    {
                        zeus::CQuaternion adjRot =
                            zeus::CQuaternion::lookAt(armToTargetFlat.normalized(), lookDir, 2.f * M_PIF);
                        armToTarget = adjRot.transform(armToTarget);
                        if (x3bc_grappleSwingTimer >= 0.25f * g_tweakPlayer->GetGrappleSwingPeriod() &&
                            x3bc_grappleSwingTimer < 0.75f * g_tweakPlayer->GetGrappleSwingPeriod())
                            armToTarget = x490_gun->GetGrappleArm().GetTransform().basis[1];
                    }
                }
            }
            armXf = zeus::lookAt(zeus::CVector3f::skZero, armToTarget, zeus::CVector3f::skUp);
            armXf.origin = armPosition;
            x490_gun->GetGrappleArm().SetTransform(armXf);
        }
    }
}

float CPlayer::GetGravity() const
{
    if (!g_GameState->GetPlayerState()->HasPowerUp(CPlayerState::EItemType::GravitySuit) &&
        CheckSubmerged())
        return g_tweakPlayer->GetFluidGravAccel();
    if (x37c_sidewaysDashing)
        return -100.f;
    return g_tweakPlayer->GetNormalGravAccel();
}

void CPlayer::ApplyGrappleForces(const CFinalInput& input, CStateManager& mgr, float dt)
{
    if (TCastToPtr<CScriptGrapplePoint> point = mgr.ObjectById(x310_orbitTargetId))
    {
        switch (x3b8_grappleState)
        {
        case EGrappleState::Pull:
        {
            zeus::CVector3f playerToPoint = point->GetTranslation() - GetTranslation();
            if (playerToPoint.canBeNormalized())
            {
                zeus::CVector3f playerToSwingLow = point->GetTranslation() +
                    zeus::CVector3f(0.f, 0.f, -g_tweakPlayer->GetGrappleSwingLength()) - GetTranslation();
                if (playerToSwingLow.canBeNormalized())
                {
                    float distToSwingLow = playerToSwingLow.magnitude();
                    playerToSwingLow.normalize();
                    float timeToLow =
                        zeus::clamp(-1.f, distToSwingLow / g_tweakPlayer->GetGrapplePullSpeedProportion(), 1.f);
                    float pullSpeed = timeToLow * (g_tweakPlayer->GetGrapplePullSpeedMax() -
                        g_tweakPlayer->GetGrapplePullSpeedMin()) + g_tweakPlayer->GetGrapplePullSpeedMin();
                    SetVelocityWR(playerToSwingLow * pullSpeed);

                    if (distToSwingLow < g_tweakPlayer->GetMaxGrappleLockedTurnAlignDistance())
                    {
                        x3b8_grappleState = EGrappleState::Swinging;
                        x3bc_grappleSwingTimer = 0.25f * g_tweakPlayer->GetGrappleSwingPeriod();
                        x3d8_grappleJumpTimeout = 0.f;
                        x9c6_28_aligningGrappleSwingTurn = point->GetGrappleParameters().GetLockSwingTurn();
                    }
                    else
                    {
                        CMotionState mState = PredictMotion(dt);
                        zeus::CVector3f lookDirFlat = x34_transform.basis[1];
                        lookDirFlat.z = 0.f;
                        zeus::CVector3f newPlayerToPointFlat =
                            point->GetTranslation() - (GetTranslation() + mState.x0_translation);
                        newPlayerToPointFlat.z = 0.f;
                        if (lookDirFlat.canBeNormalized())
                            lookDirFlat.normalize();
                        if (newPlayerToPointFlat.canBeNormalized())
                            newPlayerToPointFlat.normalize();
                        float lookToPointAngle =
                            std::acos(zeus::clamp(-1.f, lookDirFlat.dot(newPlayerToPointFlat), 1.f));
                        if (lookToPointAngle > 0.001f)
                        {
                            float deltaAngle = dt * g_tweakPlayer->GetGrappleLookCenterSpeed();
                            if (lookToPointAngle >= deltaAngle)
                            {
                                zeus::CVector3f leftDirFlat(lookDirFlat.y, -lookDirFlat.x, 0.f);
                                if (leftDirFlat.canBeNormalized())
                                    leftDirFlat.normalize();
                                if (newPlayerToPointFlat.dot(leftDirFlat) >= 0.f)
                                    deltaAngle = -deltaAngle;
                                RotateToOR(zeus::CQuaternion::fromAxisAngle(zeus::CVector3f::skUp, deltaAngle), dt);
                            }
                            else if (std::fabs(lookToPointAngle - M_PIF) > 0.001f)
                            {
                                RotateToOR(zeus::CQuaternion::shortestRotationArc(lookDirFlat,
                                                                                  newPlayerToPointFlat), dt);
                            }
                        }
                        else
                        {
                            SetAngularVelocityWR(zeus::CAxisAngle::sIdentity);
                            x174_torque = zeus::CAxisAngle::sIdentity;
                        }
                    }
                }
                else
                {
                    x3b8_grappleState = EGrappleState::Swinging;
                    x3bc_grappleSwingTimer = 0.25f * g_tweakPlayer->GetGrappleSwingPeriod();
                    x3d8_grappleJumpTimeout = 0.f;
                }
            }
            break;
        }
        case EGrappleState::Swinging:
        {
            float turnAngleSpeed = zeus::degToRad(g_tweakPlayer->GetMaxGrappleTurnSpeed());
            if (g_tweakPlayer->GetInvertGrappleTurn())
                turnAngleSpeed *= -1.f;
            zeus::CVector3f pointToPlayer = GetTranslation() - point->GetTranslation();
            float pointToPlayerZProj =
                zeus::clamp(-1.f, std::fabs(pointToPlayer.z / pointToPlayer.magnitude()), 1.f);

            bool enableTurn = false;
            if (!point->GetGrappleParameters().GetLockSwingTurn())
            {
                if (ControlMapper::GetAnalogInput(ControlMapper::ECommands::TurnLeft, input) > 0.05f)
                {
                    enableTurn = true;
                    turnAngleSpeed *= -ControlMapper::GetAnalogInput(ControlMapper::ECommands::TurnLeft, input);
                }
                if (ControlMapper::GetAnalogInput(ControlMapper::ECommands::TurnRight, input) > 0.05f)
                {
                    enableTurn = true;
                    turnAngleSpeed *= ControlMapper::GetAnalogInput(ControlMapper::ECommands::TurnRight, input);
                }
            }
            else if (x9c6_28_aligningGrappleSwingTurn)
            {
                enableTurn = true;
            }

            x3bc_grappleSwingTimer += dt;
            if (x3bc_grappleSwingTimer > g_tweakPlayer->GetGrappleSwingPeriod())
                x3bc_grappleSwingTimer -= g_tweakPlayer->GetGrappleSwingPeriod();

            zeus::CVector3f swingAxis = x3c0_grappleSwingAxis;
            if (x3bc_grappleSwingTimer < 0.5f * g_tweakPlayer->GetGrappleSwingPeriod())
                swingAxis *= zeus::CVector3f::skNegOne;

            float pullSpeed = std::fabs(zeus::clamp(-1.f,
                        std::cos(2.f * M_PIF * (x3bc_grappleSwingTimer / g_tweakPlayer->GetGrappleSwingPeriod()) +
                                     (M_PIF / 2.f)), 1.f)) * g_tweakPlayer->GetGrapplePullSpeedMin();
            zeus::CVector3f pullVec = pointToPlayer.normalized().cross(swingAxis) * pullSpeed;
            pullVec += pointToPlayer * zeus::clamp(-1.f, (pointToPlayer.magnitude() -
                g_tweakPlayer->GetGrappleSwingLength()) /
                g_tweakPlayer->GetGrappleSwingLength(), 1.f) * -32.f * pointToPlayerZProj;
            zeus::CVector3f backupVel = x138_velocity;
            SetVelocityWR(pullVec);

            zeus::CTransform backupXf = x34_transform;
            CMotionState predMotion = PredictMotion(dt);
            zeus::CVector3f newPos = x34_transform.origin + predMotion.x0_translation;
            if (ValidateFPPosition(newPos, mgr))
            {
                if (enableTurn)
                {
                    zeus::CQuaternion turnRot;
                    turnRot.rotateZ(turnAngleSpeed * dt);
                    if (point->GetGrappleParameters().GetLockSwingTurn() && x9c6_28_aligningGrappleSwingTurn)
                    {
                        zeus::CVector3f pointDir = point->GetTransform().basis[1].normalized();
                        zeus::CVector3f playerDir = x34_transform.basis[1].normalized();
                        float playerPointProj = zeus::clamp(-1.f, playerDir.dot(pointDir), 1.f);
                        if (std::fabs(playerPointProj) == 1.f)
                            x9c6_28_aligningGrappleSwingTurn = false;
                        if (playerPointProj < 0.f)
                        {
                            playerPointProj = -playerPointProj;
                            pointDir = -pointDir;
                        }
                        float turnAngleAdj = std::acos(playerPointProj) * dt;
                        turnRot = zeus::CQuaternion::lookAt(playerDir, pointDir, turnAngleAdj);
                    }
                    if (pointToPlayer.magSquared() > 0.04f)
                    {
                        zeus::CVector3f pointToPlayerFlat = pointToPlayer;
                        pointToPlayerFlat.z = 0.f;
                        zeus::CVector3f playerToGrapplePlane =
                            point->GetTranslation() + turnRot.transform(pointToPlayerFlat) - GetTranslation();
                        if (playerToGrapplePlane.canBeNormalized())
                            pullVec += playerToGrapplePlane / dt;
                    }
                    zeus::CVector3f swingAxisBackup = x3c0_grappleSwingAxis;
                    x3c0_grappleSwingAxis = turnRot.transform(x3c0_grappleSwingAxis);
                    x3c0_grappleSwingAxis.normalize();
                    zeus::CVector3f swingForward(-x3c0_grappleSwingAxis.y, x3c0_grappleSwingAxis.x, 0.f);
                    SetTransform(
                        zeus::CTransform(x3c0_grappleSwingAxis, swingForward, zeus::CVector3f::skUp, GetTranslation()));
                    SetVelocityWR(pullVec);

                    if (!ValidateFPPosition(GetTranslation(), mgr))
                    {
                        x3c0_grappleSwingAxis = swingAxisBackup;
                        SetTransform(backupXf);
                        SetVelocityWR(backupVel);
                    }
                }
            }
            else
            {
                BreakGrapple(EPlayerOrbitRequest::Six, mgr);
            }
            break;
        }
        case EGrappleState::JumpOff:
        {
            zeus::CVector3f gravForce = {0.f, 0.f, GetGravity() * xe8_mass};
            ApplyForceOR(gravForce, zeus::CAxisAngle::sIdentity);
            break;
        }
        default:
            break;
        }
    }

    zeus::CVector3f newAngVel = {0.f, 0.f, 0.9f * GetAngularVelocityOR().getVector().z};
    SetAngularVelocityOR(newAngVel);
}

bool CPlayer::ValidateFPPosition(const zeus::CVector3f& pos, CStateManager& mgr)
{
    CMaterialFilter solidFilter = CMaterialFilter::MakeInclude({EMaterialTypes::Solid});
    zeus::CAABox aabb(x2d8_fpBounds.min - 1.f + pos, x2d8_fpBounds.max + 1.f + pos);
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    mgr.BuildColliderList(nearList, *this, aabb);
    CCollidableAABox colAABB({GetBaseBoundingBox().min + pos, GetBaseBoundingBox().max + pos}, {});
    return !CGameCollision::DetectCollisionBoolean(mgr, colAABB, zeus::CTransform::Identity(), solidFilter, nearList);
}

void CPlayer::UpdateGrappleState(const CFinalInput& input, CStateManager& mgr)
{
    if (!mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::GrappleBeam) ||
        x2f8_morphBallState == EPlayerMorphBallState::Morphed ||
        mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Scan ||
        mgr.GetPlayerState()->GetTransitioningVisor() == CPlayerState::EPlayerVisor::Scan)
        return;

    if (x310_orbitTargetId == kInvalidUniqueId)
    {
        x3b8_grappleState = EGrappleState::None;
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
            case EPlayerOrbitState::Grapple:
                switch (g_tweakPlayer->GetGrappleJumpMode())
                {
                case 0:
                case 1:
                    if (ControlMapper::GetPressInput(ControlMapper::ECommands::FireOrBomb, input))
                    {
                        if (TCastToPtr<CScriptGrapplePoint> point2 = mgr.ObjectById(x33c_orbitNextTargetId))
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
                                SetOrbitTargetId(x33c_orbitNextTargetId, mgr);
                                x3b8_grappleState = EGrappleState::Pull;
                                x33c_orbitNextTargetId = kInvalidUniqueId;
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
                                              LineOfSightFilter);
                    if (result.IsInvalid())
                    {
                        HolsterGun(mgr);
                        switch (x3b8_grappleState)
                        {
                        case EGrappleState::Firing:
                        case EGrappleState::Swinging:
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
                                    break;
                                default:
                                    break;
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
                                        break;
                                    default:
                                        break;
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
                                    break;
                                default:
                                    break;
                                }
                                break;
                            }
                        case EGrappleState::None:
                            x3b8_grappleState = EGrappleState::Firing;
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

    if (x304_orbitState != EPlayerOrbitState::Grapple)
    {
        if (x304_orbitState >= EPlayerOrbitState::Grapple)
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
            if (x3b8_grappleState == EGrappleState::JumpOff)
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
            case EGrappleState::Swinging:
                if (!ControlMapper::GetDigitalInput(ControlMapper::ECommands::FireOrBomb, input) &&
                    x3d8_grappleJumpTimeout <= 0.f)
                {
                    x3d8_grappleJumpTimeout = g_tweakPlayer->GetGrappleReleaseTime();
                    x3b8_grappleState = EGrappleState::JumpOff;
                    ApplyGrappleJump(mgr);
                }
                break;
            case EGrappleState::JumpOff:
                x3d8_grappleJumpTimeout -= input.DeltaTime();
                if (x3d8_grappleJumpTimeout <= 0.f)
                {
                    SetMoveState(EPlayerMovementState::StartingJump, mgr);
                    ComputeMovement(input, mgr, input.DeltaTime());
                    BreakGrapple(EPlayerOrbitRequest::Zero, mgr);
                    PreventFallingCameraPitch();
                }
                break;
            case EGrappleState::Firing:
            case EGrappleState::Pull:
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
                                      LineOfSightFilter);
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
        SetOrbitState(EPlayerOrbitState::Grapple, mgr);
        x3b8_grappleState = EGrappleState::Pull;
        RemoveMaterial(EMaterialTypes::GroundCollider, mgr);
    }
}

void CPlayer::BreakGrapple(EPlayerOrbitRequest req, CStateManager& mgr)
{
    x294_jumpCameraTimer = 0.f;
    x29c_fallCameraTimer = 0.f;
    if (g_tweakPlayer->GetGrappleJumpMode() == 2 && x3b8_grappleState == EGrappleState::Swinging)
    {
        ApplyGrappleJump(mgr);
        PreventFallingCameraPitch();
    }

    SetOrbitRequest(req, mgr);
    x3b8_grappleState = EGrappleState::None;
    AddMaterial(EMaterialTypes::GroundCollider, mgr);
    x490_gun->GetGrappleArm().SetAnimState(CGrappleArm::EArmState::Eight);
    if (!InGrappleJumpCooldown() && x3b8_grappleState != EGrappleState::JumpOff)
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
        x314_orbitPoint = g_tweakPlayer->GetOrbitNormalDistance(int(x308_orbitType)) *
            x34_transform.basis[1] + x34_transform.origin;
        break;
    default:
        SetOrbitState(EPlayerOrbitState::NoOrbit, mgr);
        break;
    }
}

bool CPlayer::InGrappleJumpCooldown() const
{
    if (x258_movementState == EPlayerMovementState::OnGround)
        return false;
    return x3d8_grappleJumpTimeout > 0.f || x294_jumpCameraTimer == 0.f;
}

void CPlayer::PreventFallingCameraPitch()
{
    x294_jumpCameraTimer = 0.f;
    x29c_fallCameraTimer = 0.01f;
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
    SetOrbitPosition(g_tweakPlayer->GetOrbitNormalDistance(int(x308_orbitType)), mgr);
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
        cam->SetLockCamera(false);
        break;
    case EPlayerOrbitState::Three:
    {
        cam->SetLockCamera(true);
        zeus::CVector3f playerToPoint = x314_orbitPoint - GetTranslation();
        if (playerToPoint.canBeNormalized())
            x340_ = playerToPoint.magnitude();
        else
            x340_ = 0.f;
        SetOrbitTargetId(kInvalidUniqueId, mgr);
        x33c_orbitNextTargetId = kInvalidUniqueId;
        break;
    }
    case EPlayerOrbitState::NoOrbit:
        x32c_orbitModeTimer = g_tweakPlayer->GetOrbitModeTimer();
        x32c_orbitModeTimer = 0.28f;
        SetOrbitTargetId(kInvalidUniqueId, mgr);
        x33c_orbitNextTargetId = kInvalidUniqueId;
        break;
    case EPlayerOrbitState::Two:
        SetOrbitTargetId(kInvalidUniqueId, mgr);
        x33c_orbitNextTargetId = kInvalidUniqueId;
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
        x374_orbitLockEstablished = false;
}

void CPlayer::UpdateOrbitPosition(float dist, CStateManager& mgr)
{
    switch (x304_orbitState)
    {
    case EPlayerOrbitState::Two:
    case EPlayerOrbitState::Three:
        SetOrbitPosition(dist, mgr);
        break;
    case EPlayerOrbitState::One:
    case EPlayerOrbitState::Four:
    case EPlayerOrbitState::Grapple:
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

void CPlayer::SetOrbitPosition(float dist, CStateManager& mgr)
{
    zeus::CTransform camXf = GetFirstPersonCameraTransform(mgr);
    if (x304_orbitState == EPlayerOrbitState::Two && x30c_orbitRequest == EPlayerOrbitRequest::Seven)
        camXf = x34_transform;
    zeus::CVector3f fwd = camXf.basis[1];
    float dot = fwd.normalized().dot(fwd);
    if (std::fabs(dot) > 1.f)
        dot = (dot > 0.f) ? 1.f : -1.f;
    x314_orbitPoint = camXf.rotate(zeus::CVector3f(0.f, dist / dot, 0.f)) + camXf.origin;
    x320_orbitVector = zeus::CVector3f(0.f, dist, x314_orbitPoint.z - camXf.origin.z);
}

void CPlayer::UpdateAimTarget(CStateManager& mgr)
{
    if (!ValidateAimTargetId(x3f4_aimTarget, mgr))
        ResetAimTargetPrediction(kInvalidUniqueId);

    if (!GetCombatMode())
    {
        ResetAimTargetPrediction(kInvalidUniqueId);
        x48c_aimTargetTimer = 0.f;
        return;
    }

#if 0
    if (!0 && 0)
    {
        ResetAimTargetPrediction(kInvalidUniqueId);
        x48c_aimTargetTimer = 0.f;
        if (x304_orbitState == EPlayerOrbitState::One ||
            x304_orbitState == EPlayerOrbitState::Four)
            if (!ValidateOrbitTargetId(x310_orbitTargetId, mgr))
                ResetAimTargetPrediction(x310_orbitTargetId);
        return;
    }
#endif

    bool needsReset = false;
    TCastToPtr<CActor> act = mgr.ObjectById(x3f4_aimTarget);
    CActor* actp = act.GetPtr();
    if (act)
        if (!act->GetMaterialList().HasMaterial(EMaterialTypes::Target))
            actp = nullptr;

    if (g_tweakPlayer->GetAimWhenOrbitingPoint())
    {
        if (x304_orbitState == EPlayerOrbitState::One ||
            x304_orbitState == EPlayerOrbitState::Four)
        {
            if (ValidateOrbitTargetId(x310_orbitTargetId, mgr) == EOrbitValidationResult::OK)
                ResetAimTargetPrediction(x310_orbitTargetId);
            else
                needsReset = true;
        }
        else
        {
            needsReset = true;
        }
    }
    else
    {
        if (x304_orbitState == EPlayerOrbitState::NoOrbit)
            needsReset = true;
    }

    if (needsReset)
    {
        if (actp && ValidateObjectForMode(x3f4_aimTarget, mgr))
            ResetAimTargetPrediction(kInvalidUniqueId);
        else
            ResetAimTargetPrediction(FindAimTargetId(mgr));
    }
}

void CPlayer::UpdateAimTargetTimer(float dt)
{
    if (x3f4_aimTarget == kInvalidUniqueId)
        return;
    if (x48c_aimTargetTimer <= 0.f)
        return;
    x48c_aimTargetTimer -= dt;
}

bool CPlayer::ValidateAimTargetId(TUniqueId uid, CStateManager& mgr)
{
    if (uid == kInvalidUniqueId)
    {
        x404_aimTargetAverage.Clear();
        x48c_aimTargetTimer = 0.f;
        return false;
    }

    TCastToPtr<CActor> act = mgr.ObjectById(uid);
    if (!act || !act->GetMaterialList().HasMaterial(EMaterialTypes::Target) || !act->GetIsTargetable())
        return false;

    if (x304_orbitState == EPlayerOrbitState::One ||
        x304_orbitState == EPlayerOrbitState::Four)
    {
        if (ValidateOrbitTargetId(x310_orbitTargetId, mgr) != EOrbitValidationResult::OK)
        {
            ResetAimTargetPrediction(kInvalidUniqueId);
            x48c_aimTargetTimer = 0.f;
            return false;
        }
        return true;
    }

    if (act->GetMaterialList().HasMaterial(EMaterialTypes::Target) && uid != kInvalidUniqueId &&
        ValidateObjectForMode(uid, mgr))
    {
        float vpWHalf = g_Viewport.x8_width / 2;
        float vpHHalf = g_Viewport.xc_height / 2;
        zeus::CVector3f aimPos = act->GetAimPosition(mgr, 0.f);
        zeus::CVector3f eyePos = GetEyePosition();
        zeus::CVector3f eyeToAim = aimPos - eyePos;
        zeus::CVector3f screenPos = mgr.GetCameraManager()->GetFirstPersonCamera()->ConvertToScreenSpace(aimPos);
        zeus::CVector3f posInBox(vpWHalf + screenPos.x * vpWHalf,
                                 vpHHalf + screenPos.y * vpHHalf,
                                 screenPos.z);
        if (WithinOrbitScreenBox(posInBox, x330_orbitZoneMode, x334_orbitType) ||
            (x330_orbitZoneMode != EPlayerZoneInfo::Zero &&
             WithinOrbitScreenBox(posInBox, EPlayerZoneInfo::Zero, x334_orbitType)))
        {
            float eyeToAimMag = eyeToAim.magnitude();
            if (eyeToAimMag <= g_tweakPlayer->GetAimMaxDistance())
            {
                rstl::reserved_vector<TUniqueId, 1024> nearList;
                TUniqueId intersectId = kInvalidUniqueId;
                eyeToAim.normalize();
                mgr.BuildNearList(nearList, eyePos, eyeToAim, eyeToAimMag,
                                  OccluderFilter, act);
                eyeToAim.normalize();
                CRayCastResult result =
                    mgr.RayWorldIntersection(intersectId, eyePos, eyeToAim, eyeToAimMag,
                                             LineOfSightFilter, nearList);
                if (result.IsInvalid())
                {
                    x48c_aimTargetTimer = g_tweakPlayer->GetAimTargetTimer();
                    return true;
                }
            }
        }

        if (x48c_aimTargetTimer > 0.f)
            return true;
    }

    ResetAimTargetPrediction(kInvalidUniqueId);
    x48c_aimTargetTimer = 0.f;
    return false;
}

bool CPlayer::ValidateObjectForMode(TUniqueId uid, CStateManager& mgr) const
{
    TCastToPtr<CActor> act = mgr.ObjectById(uid);
    if (!act || uid == kInvalidUniqueId)
        return false;

    if (TCastToPtr<CScriptDoor>(mgr.ObjectById(uid)))
        return true;

    if (GetCombatMode())
    {
        if (CHealthInfo* hInfo = act->HealthInfo())
        {
            if (hInfo->GetHP() > 0.f)
                return true;
        }
        else
        {
            if (act->GetMaterialList().HasMaterial(EMaterialTypes::Projectile) ||
                act->GetMaterialList().HasMaterial(EMaterialTypes::Scannable))
                return true;

            if (TCastToPtr<CScriptGrapplePoint> point = mgr.ObjectById(uid))
            {
                zeus::CVector3f playerToPoint = point->GetTranslation() - GetTranslation();
                if (playerToPoint.canBeNormalized() && playerToPoint.magnitude() < g_tweakPlayer->GetOrbitDistanceMax())
                    return true;
            }
        }
    }

    if (GetExplorationMode())
    {
        if (!act->HealthInfo())
        {
            if (TCastToPtr<CScriptGrapplePoint> point = mgr.ObjectById(uid))
            {
                zeus::CVector3f playerToPoint = point->GetTranslation() - GetTranslation();
                if (playerToPoint.canBeNormalized() && playerToPoint.magnitude() < g_tweakPlayer->GetOrbitDistanceMax())
                    return true;
            }
            else
            {
                return true;
            }
        }
        else
        {
            return true;
        }
    }

    return false;
}

static zeus::CAABox BuildNearListBox(bool cropBottom, const zeus::CTransform& xf, float x, float z, float y)
{
    zeus::CAABox aabb(-x, cropBottom ? 0.f : -y, -z, x, y, z);
    return aabb.getTransformedAABox(xf);
}

TUniqueId CPlayer::FindAimTargetId(CStateManager& mgr)
{
    float dist = g_tweakPlayer->GetAimMaxDistance();
    if (x9c6_24_extendTargetDistance)
        dist *= 5.f;
    zeus::CAABox aabb =
    BuildNearListBox(true, GetFirstPersonCameraTransform(mgr), g_tweakPlayer->GetAimBoxWidth(),
                     g_tweakPlayer->GetAimBoxHeight(), dist);
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    mgr.BuildNearList(nearList, aabb, CMaterialFilter::MakeInclude({EMaterialTypes::Target}), this);
    return CheckEnemiesAgainstOrbitZone(nearList, EPlayerZoneInfo::Zero, EPlayerZoneType::Ellipse, mgr);
}

const zeus::CTransform& CPlayer::GetFirstPersonCameraTransform(const CStateManager& mgr) const
{
    return mgr.GetCameraManager()->GetFirstPersonCamera()->GetGunFollowTransform();
}

TUniqueId CPlayer::CheckEnemiesAgainstOrbitZone(const std::vector<TUniqueId>& list, EPlayerZoneInfo info,
                                                EPlayerZoneType zone, CStateManager& mgr) const
{
    zeus::CVector3f eyePos = GetEyePosition();
    zeus::CVector3f lookDir = x34_transform.basis[1].normalized();
    float minEyeToAimMag = 10000.f;
    float minPosInBoxMagSq = 10000.f;
    TUniqueId bestId = kInvalidUniqueId;
    float vpWHalf = g_Viewport.x8_width / 2;
    float vpHHalf = g_Viewport.xc_height / 2;
    float boxLeft = (g_tweakPlayer->GetOrbitZoneIdealX(int(info)) - vpWHalf) / vpWHalf;
    float boxTop = (g_tweakPlayer->GetOrbitZoneIdealY(int(info)) - vpHHalf) / vpHHalf;
    CFirstPersonCamera* fpCam = mgr.GetCameraManager()->GetFirstPersonCamera();

    for (TUniqueId id : list)
    {
        if (CActor* act = static_cast<CActor*>(mgr.ObjectById(id)))
        {
            if (act->GetUniqueId() != GetUniqueId() && ValidateObjectForMode(act->GetUniqueId(), mgr))
            {
                zeus::CVector3f aimPos = act->GetAimPosition(mgr, 0.f);
                zeus::CVector3f screenPos = fpCam->ConvertToScreenSpace(aimPos);
                zeus::CVector3f posInBox(vpWHalf + screenPos.x * vpWHalf,
                                         vpHHalf + screenPos.y * vpHHalf,
                                         screenPos.z);
                if (WithinOrbitScreenBox(posInBox, info, zone))
                {
                    zeus::CVector3f eyeToAim = aimPos - eyePos;
                    float eyeToAimMag = eyeToAim.magnitude();
                    if (eyeToAimMag <= g_tweakPlayer->GetAimMaxDistance())
                    {
                        if (minEyeToAimMag - eyeToAimMag > g_tweakPlayer->GetAimThresholdDistance())
                        {
                            rstl::reserved_vector<TUniqueId, 1024> nearList;
                            TUniqueId intersectId = kInvalidUniqueId;
                            eyeToAim.normalize();
                            mgr.BuildNearList(nearList, eyePos, eyeToAim, eyeToAimMag,
                                              OccluderFilter, act);
                            eyeToAim.normalize();
                            CRayCastResult result =
                            mgr.RayWorldIntersection(intersectId, eyePos, eyeToAim, eyeToAimMag,
                                                     LineOfSightFilter, nearList);
                            if (result.IsInvalid())
                            {
                                bestId = act->GetUniqueId();
                                float posInBoxLeft = posInBox.x - boxLeft;
                                float posInBoxTop = posInBox.y - boxTop;
                                minEyeToAimMag = eyeToAimMag;
                                minPosInBoxMagSq = posInBoxLeft * posInBoxLeft + posInBoxTop * posInBoxTop;
                            }
                        }
                        else if (std::fabs(eyeToAimMag - minEyeToAimMag) < g_tweakPlayer->GetAimThresholdDistance())
                        {
                            float posInBoxLeft = posInBox.x - boxLeft;
                            float posInBoxTop = posInBox.y - boxTop;
                            float posInBoxMagSq = posInBoxLeft * posInBoxLeft + posInBoxTop * posInBoxTop;
                            if (posInBoxMagSq < minPosInBoxMagSq)
                            {
                                rstl::reserved_vector<TUniqueId, 1024> nearList;
                                TUniqueId intersectId = kInvalidUniqueId;
                                eyeToAim.normalize();
                                mgr.BuildNearList(nearList, eyePos, eyeToAim, eyeToAimMag,
                                                  OccluderFilter, act);
                                eyeToAim.normalize();
                                CRayCastResult result =
                                    mgr.RayWorldIntersection(intersectId, eyePos, eyeToAim, eyeToAimMag,
                                                             LineOfSightFilter, nearList);
                                if (result.IsInvalid())
                                {
                                    bestId = act->GetUniqueId();
                                    minEyeToAimMag = eyeToAimMag;
                                    minPosInBoxMagSq = posInBoxMagSq;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return bestId;
}

TUniqueId CPlayer::FindOrbitTargetId(CStateManager& mgr)
{
    return FindBestOrbitableObject(x354_onScreenOrbitObjects, x330_orbitZoneMode, mgr);
}

void CPlayer::UpdateOrbitableObjects(CStateManager& mgr)
{
    x354_onScreenOrbitObjects.clear();
    x344_nearbyOrbitObjects.clear();
    x354_onScreenOrbitObjects.clear();

    if (CheckOrbitDisableSourceList(mgr))
        return;

    float dist = GetOrbitMaxTargetDistance(mgr);
    if (x9c6_24_extendTargetDistance)
        dist *= 5.f;
    zeus::CAABox nearAABB =
        BuildNearListBox(true, GetFirstPersonCameraTransform(mgr),
                         g_tweakPlayer->GetOrbitNearX(), g_tweakPlayer->GetOrbitNearZ(), dist);

    CMaterialFilter filter = mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Scan ?
    CMaterialFilter::MakeInclude({EMaterialTypes::Scannable}) : CMaterialFilter::MakeInclude({EMaterialTypes::Orbit});
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    mgr.BuildNearList(nearList, nearAABB, filter, nullptr);

    FindOrbitableObjects(nearList, x344_nearbyOrbitObjects, x330_orbitZoneMode, EPlayerZoneType::Always, mgr, true);
    FindOrbitableObjects(nearList, x354_onScreenOrbitObjects, x330_orbitZoneMode, x334_orbitType, mgr, true);
    FindOrbitableObjects(nearList, x364_offScreenOrbitObjects, x330_orbitZoneMode, x334_orbitType, mgr, false);
}

TUniqueId CPlayer::FindBestOrbitableObject(const std::vector<TUniqueId>& ids,
                                           EPlayerZoneInfo info, CStateManager& mgr) const
{
    zeus::CVector3f eyePos = GetEyePosition();
    zeus::CVector3f lookDir = x34_transform.basis[1].normalized();
    float minEyeToOrbitMag = 10000.f;
    float minPosInBoxMagSq = 10000.f;
    TUniqueId bestId = kInvalidUniqueId;
    float vpWidthHalf = g_Viewport.x8_width / 2;
    float vpHeightHalf = g_Viewport.xc_height / 2;
    float boxLeft = (g_tweakPlayer->GetOrbitZoneIdealX(int(info)) *
                         g_Viewport.x8_width / 640 - vpWidthHalf) / vpWidthHalf;
    float boxTop = (g_tweakPlayer->GetOrbitZoneIdealY(int(info)) *
                        g_Viewport.xc_height / 448 - vpHeightHalf) / vpHeightHalf;

    CFirstPersonCamera* fpCam = mgr.GetCameraManager()->GetFirstPersonCamera();

    for (TUniqueId id : ids)
    {
        if (TCastToPtr<CActor> act = mgr.ObjectById(id))
        {
            zeus::CVector3f orbitPos = act->GetOrbitPosition(mgr);
            zeus::CVector3f eyeToOrbit = orbitPos - eyePos;
            float eyeToOrbitMag = eyeToOrbit.magnitude();
            zeus::CVector3f orbitPosScreen = fpCam->ConvertToScreenSpace(orbitPos);
            if (orbitPosScreen.z >= 0.f)
            {
                if (x310_orbitTargetId != id)
                {
                    if (TCastToPtr<CScriptGrapplePoint> point = act.GetPtr())
                    {
                        if (x310_orbitTargetId != point->GetUniqueId())
                        {
                            if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::GrappleBeam) &&
                                eyeToOrbitMag < minEyeToOrbitMag &&
                                eyeToOrbitMag < g_tweakPlayer->GetOrbitDistanceMax())
                            {
                                rstl::reserved_vector<TUniqueId, 1024> nearList;
                                TUniqueId intersectId = kInvalidUniqueId;
                                eyeToOrbit.normalize();
                                mgr.BuildNearList(nearList, eyePos, eyeToOrbit, eyeToOrbitMag,
                                                  OccluderFilter, act.GetPtr());
                                eyeToOrbit.normalize();
                                CRayCastResult result =
                                mgr.RayWorldIntersection(intersectId, eyePos, eyeToOrbit, eyeToOrbitMag,
                                                         LineOfSightFilter, nearList);
                                if (result.IsInvalid())
                                {
                                    if (point->GetGrappleParameters().GetLockSwingTurn())
                                    {
                                        zeus::CVector3f pointToPlayer =
                                            GetTranslation() - point->GetTranslation();
                                        if (pointToPlayer.canBeNormalized())
                                        {
                                            pointToPlayer.z = 0.f;
                                            if (std::fabs(point->GetTransform().basis[1].normalized().
                                                dot(pointToPlayer.normalized())) <= M_SQRT1_2F)
                                                continue;
                                        }
                                    }

                                    bestId = act->GetUniqueId();
                                    float posInBoxLeft = orbitPosScreen.x - boxLeft;
                                    float posInBoxTop = orbitPosScreen.y - boxTop;
                                    minEyeToOrbitMag = eyeToOrbitMag;
                                    minPosInBoxMagSq = posInBoxLeft * posInBoxLeft + posInBoxTop * posInBoxTop;
                                }
                            }
                            continue;
                        }
                    }

                    if (minEyeToOrbitMag - eyeToOrbitMag > g_tweakPlayer->GetOrbitDistanceThreshold() &&
                        mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Scan)
                    {
                        rstl::reserved_vector<TUniqueId, 1024> nearList;
                        TUniqueId bestId = kInvalidUniqueId;
                        eyeToOrbit.normalize();
                        mgr.BuildNearList(nearList, eyePos, eyeToOrbit, eyeToOrbitMag,
                                          OccluderFilter, act.GetPtr());
                        for (auto it = nearList.begin() ; it != nearList.end() ;)
                        {
                            if (CEntity* obj = mgr.ObjectById(*it))
                            {
                                if (obj->GetAreaIdAlways() != kInvalidAreaId)
                                {
                                    if (mgr.GetNextAreaId() != obj->GetAreaIdAlways())
                                    {
                                        const CGameArea* area = mgr.GetWorld()->GetAreaAlways(obj->GetAreaIdAlways());
                                        CGameArea::EOcclusionState state =
                                            area->IsPostConstructed() ? area->GetOcclusionState() :
                                            CGameArea::EOcclusionState::Occluded;
                                        if (state == CGameArea::EOcclusionState::Occluded)
                                        {
                                            it = nearList.erase(it);
                                            continue;
                                        }
                                    }
                                }
                                else
                                {
                                    it = nearList.erase(it);
                                    continue;
                                }
                            }
                            ++it;
                        }

                        eyeToOrbit.normalize();
                        CRayCastResult result =
                        mgr.RayWorldIntersection(bestId, eyePos, eyeToOrbit, eyeToOrbitMag,
                                                 LineOfSightFilter, nearList);
                        if (result.IsInvalid())
                        {
                            bestId = act->GetUniqueId();
                            float posInBoxLeft = orbitPosScreen.x - boxLeft;
                            float posInBoxTop = orbitPosScreen.y - boxTop;
                            minEyeToOrbitMag = eyeToOrbitMag;
                            minPosInBoxMagSq = posInBoxLeft * posInBoxLeft + posInBoxTop * posInBoxTop;
                        }
                    }

                    if (std::fabs(eyeToOrbitMag - minEyeToOrbitMag) <
                            g_tweakPlayer->GetOrbitDistanceThreshold() ||
                        mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Scan)
                    {
                        float posInBoxLeft = orbitPosScreen.x - boxLeft;
                        float posInBoxTop = orbitPosScreen.y - boxTop;
                        float posInBoxMagSq = posInBoxLeft * posInBoxLeft + posInBoxTop * posInBoxTop;
                        if (posInBoxMagSq < minPosInBoxMagSq)
                        {
                            rstl::reserved_vector<TUniqueId, 1024> nearList;
                            TUniqueId bestId = kInvalidUniqueId;
                            eyeToOrbit.normalize();
                            mgr.BuildNearList(nearList, eyePos, eyeToOrbit, eyeToOrbitMag,
                                              OccluderFilter, act.GetPtr());
                            for (auto it = nearList.begin() ; it != nearList.end() ;)
                            {
                                if (CEntity* obj = mgr.ObjectById(*it))
                                {
                                    if (obj->GetAreaIdAlways() != kInvalidAreaId)
                                    {
                                        if (mgr.GetNextAreaId() != obj->GetAreaIdAlways())
                                        {
                                            const CGameArea* area =
                                                mgr.GetWorld()->GetAreaAlways(obj->GetAreaIdAlways());
                                            CGameArea::EOcclusionState state =
                                                area->IsPostConstructed() ? area->GetOcclusionState() :
                                                CGameArea::EOcclusionState::Occluded;
                                            if (state == CGameArea::EOcclusionState::Occluded)
                                            {
                                                it = nearList.erase(it);
                                                continue;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        it = nearList.erase(it);
                                        continue;
                                    }
                                }
                                ++it;
                            }

                            eyeToOrbit.normalize();
                            CRayCastResult result =
                                mgr.RayWorldIntersection(bestId, eyePos, eyeToOrbit, eyeToOrbitMag,
                                                         LineOfSightFilter, nearList);
                            if (result.IsInvalid())
                            {
                                bestId = act->GetUniqueId();
                                minPosInBoxMagSq = posInBoxMagSq;
                                minEyeToOrbitMag = eyeToOrbitMag;
                            }
                        }
                    }
                }
            }
        }
    }

    return bestId;
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
            if (ValidateOrbitTargetId(act->GetUniqueId(), mgr) != EOrbitValidationResult::OK)
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

            if (pass && (!act->GetDoTargetDistanceTest() ||
                (orbitPos - eyePos).magnitude() <= GetOrbitMaxTargetDistance(mgr)))
                listOut.push_back(id);
        }
    }
}

bool CPlayer::WithinOrbitScreenBox(const zeus::CVector3f& screenCoords, EPlayerZoneInfo zone,
                                   EPlayerZoneType type) const
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
        if (mgr.GetObjectById(*it) == nullptr)
        {
            it = x9e4_orbitDisableList.erase(it);
            continue;
        }
        ++it;
    }
    return !x9e4_orbitDisableList.empty();
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

void CPlayer::UpdateOrbitPreventionTimer(float dt)
{
    if (x378_orbitPreventionTimer > 0.f)
        x378_orbitPreventionTimer -= dt;
}

void CPlayer::UpdateOrbitModeTimer(float dt)
{
    if (x304_orbitState == EPlayerOrbitState::NoOrbit && x32c_orbitModeTimer > 0.f)
        x32c_orbitModeTimer -= dt;
    x32c_orbitModeTimer = 0.f;
}

void CPlayer::UpdateOrbitZone(CStateManager& mgr)
{
    if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Scan)
    {
        x334_orbitType = EPlayerZoneType::Ellipse;
        x338_ = 1;
        x330_orbitZoneMode = EPlayerZoneInfo::Zero;
    }
    else
    {
        x334_orbitType = EPlayerZoneType::Box;
        x338_ = 2;
        x330_orbitZoneMode = EPlayerZoneInfo::One;
    }
}

void CPlayer::UpdateOrbitInput(const CFinalInput& input, CStateManager& mgr)
{
    if (x2f8_morphBallState != EPlayerMorphBallState::Unmorphed || x378_orbitPreventionTimer > 0.f)
        return;

    UpdateOrbitableObjects(mgr);
    if (x304_orbitState == EPlayerOrbitState::NoOrbit)
        x33c_orbitNextTargetId = FindOrbitTargetId(mgr);

    if (ControlMapper::GetDigitalInput(ControlMapper::ECommands::OrbitClose, input) ||
        ControlMapper::GetDigitalInput(ControlMapper::ECommands::OrbitFar, input) ||
        ControlMapper::GetDigitalInput(ControlMapper::ECommands::OrbitObject, input))
    {
        switch (x304_orbitState)
        {
        case EPlayerOrbitState::NoOrbit:
            if (ControlMapper::GetPressInput(ControlMapper::ECommands::OrbitObject, input))
            {
                SetOrbitTargetId(x33c_orbitNextTargetId, mgr);
                if (x310_orbitTargetId != kInvalidUniqueId)
                {
                    if (ValidateAimTargetId(x310_orbitTargetId, mgr))
                        ResetAimTargetPrediction(x310_orbitTargetId);
                    SetOrbitState(EPlayerOrbitState::One, mgr);
                    UpdateOrbitPosition(g_tweakPlayer->GetOrbitNormalDistance(int(x308_orbitType)), mgr);
                }
            }
            else
            {
                if (ControlMapper::GetPressInput(ControlMapper::ECommands::OrbitFar, input))
                    OrbitPoint(EPlayerOrbitType::Far, mgr);
                if (ControlMapper::GetPressInput(ControlMapper::ECommands::OrbitClose, input))
                    OrbitPoint(EPlayerOrbitType::Close, mgr);
            }
            break;
        case EPlayerOrbitState::Grapple:
            if (x310_orbitTargetId == kInvalidUniqueId)
                BreakGrapple(EPlayerOrbitRequest::Zero, mgr);
            break;
        case EPlayerOrbitState::One:
            if (TCastToConstPtr<CScriptGrapplePoint> point = mgr.GetObjectById(x310_orbitTargetId))
            {
                if (ValidateCurrentOrbitTargetId(mgr) == EOrbitValidationResult::OK)
                    UpdateOrbitPosition(g_tweakPlayer->GetOrbitNormalDistance(int(x308_orbitType)), mgr);
                else
                    BreakGrapple(EPlayerOrbitRequest::Six, mgr);
            }
            else
            {
                EOrbitValidationResult result = ValidateCurrentOrbitTargetId(mgr);
                if (result == EOrbitValidationResult::OK)
                    UpdateOrbitPosition(g_tweakPlayer->GetOrbitNormalDistance(int(x308_orbitType)), mgr);
                else if (result == EOrbitValidationResult::BrokenLookAngle)
                    OrbitPoint(EPlayerOrbitType::Far, mgr);
                else if (result == EOrbitValidationResult::ExtremeHorizonAngle)
                    SetOrbitRequest(EPlayerOrbitRequest::Seven, mgr);
                else
                    ActivateOrbitSource(mgr);
            }
            UpdateOrbitSelection(input, mgr);
            break;
        case EPlayerOrbitState::Two:
            if (ControlMapper::GetPressInput(ControlMapper::ECommands::OrbitObject, input))
            {
                SetOrbitTargetId(FindOrbitTargetId(mgr), mgr);
                if (x310_orbitTargetId != kInvalidUniqueId)
                {
                    if (ValidateAimTargetId(x310_orbitTargetId, mgr))
                        ResetAimTargetPrediction(x310_orbitTargetId);
                    SetOrbitState(EPlayerOrbitState::One, mgr);
                    UpdateOrbitPosition(g_tweakPlayer->GetOrbitNormalDistance(int(x308_orbitType)), mgr);
                }
            }
            else
            {
                switch (x308_orbitType)
                {
                case EPlayerOrbitType::Far:
                    if (ControlMapper::GetDigitalInput(ControlMapper::ECommands::OrbitClose, input))
                    {
                        x308_orbitType = EPlayerOrbitType::Close;
                        UpdateOrbitPosition(g_tweakPlayer->GetOrbitNormalDistance(int(x308_orbitType)), mgr);
                    }
                    break;
                case EPlayerOrbitType::Close:
                    if (ControlMapper::GetDigitalInput(ControlMapper::ECommands::OrbitFar, input) &&
                        !ControlMapper::GetDigitalInput(ControlMapper::ECommands::OrbitClose, input))
                    {
                        x308_orbitType = EPlayerOrbitType::Far;
                        UpdateOrbitPosition(g_tweakPlayer->GetOrbitNormalDistance(int(x308_orbitType)), mgr);
                    }
                    break;
                    break;
                default:
                    break;
                }
            }
            UpdateOrbitPosition(g_tweakPlayer->GetOrbitNormalDistance(int(x308_orbitType)), mgr);
            break;
        case EPlayerOrbitState::Three:
            if (ControlMapper::GetPressInput(ControlMapper::ECommands::OrbitObject, input))
            {
                SetOrbitTargetId(FindOrbitTargetId(mgr), mgr);
                if (x310_orbitTargetId != kInvalidUniqueId)
                {
                    if (ValidateAimTargetId(x310_orbitTargetId, mgr))
                        ResetAimTargetPrediction(x310_orbitTargetId);
                    SetOrbitState(EPlayerOrbitState::One, mgr);
                    UpdateOrbitPosition(g_tweakPlayer->GetOrbitNormalDistance(int(x308_orbitType)), mgr);
                }
            }
            UpdateOrbitSelection(input, mgr);
            break;
        case EPlayerOrbitState::Four:
            UpdateOrbitPosition(g_tweakPlayer->GetOrbitNormalDistance(int(x308_orbitType)), mgr);
            UpdateOrbitSelection(input, mgr);
            break;
        }

        if (x304_orbitState == EPlayerOrbitState::Grapple)
        {
            x33c_orbitNextTargetId = FindOrbitTargetId(mgr);
            if (x33c_orbitNextTargetId == x310_orbitTargetId)
                x33c_orbitNextTargetId = kInvalidUniqueId;
        }
    }
    else
    {
        switch (x304_orbitState)
        {
        case EPlayerOrbitState::One:
            if (TCastToConstPtr<CScriptGrapplePoint> point = mgr.GetObjectById(x310_orbitTargetId))
                BreakGrapple(EPlayerOrbitRequest::Three, mgr);
            else
                SetOrbitRequest(EPlayerOrbitRequest::Zero, mgr);
            break;
        case EPlayerOrbitState::Grapple:
            if (!g_tweakPlayer->GetOrbitReleaseBreaksGrapple())
            {
                x33c_orbitNextTargetId = FindOrbitTargetId(mgr);
                if (x33c_orbitNextTargetId == x310_orbitTargetId)
                    x33c_orbitNextTargetId = kInvalidUniqueId;
            }
            else
            {
                BreakGrapple(EPlayerOrbitRequest::Zero, mgr);
            }
            break;
        case EPlayerOrbitState::Four:
            UpdateOrbitPosition(g_tweakPlayer->GetOrbitNormalDistance(int(x308_orbitType)), mgr);
            UpdateOrbitSelection(input, mgr);
            break;
        default:
            SetOrbitRequest(EPlayerOrbitRequest::Zero, mgr);
            break;
        }
    }
}

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
            OrbitPoint(EPlayerOrbitType::Far, mgr);
        else
            OrbitCarcass(mgr);
        break;
    }
}

void CPlayer::UpdateOrbitSelection(const CFinalInput& input, CStateManager& mgr)
{
    x33c_orbitNextTargetId = FindOrbitTargetId(mgr);
    TCastToConstPtr<CScriptGrapplePoint> curPoint = mgr.GetObjectById(x310_orbitTargetId);
    TCastToConstPtr<CScriptGrapplePoint> nextPoint = mgr.GetObjectById(x33c_orbitNextTargetId);
    if (curPoint || (x304_orbitState == EPlayerOrbitState::Grapple && !nextPoint))
    {
        x33c_orbitNextTargetId = kInvalidUniqueId;
        return;
    }

    if (ControlMapper::GetPressInput(ControlMapper::ECommands::OrbitObject, input) &&
        x33c_orbitNextTargetId != kInvalidUniqueId)
    {
        SetOrbitTargetId(x33c_orbitNextTargetId, mgr);
        if (ValidateAimTargetId(x310_orbitTargetId, mgr))
            ResetAimTargetPrediction(x310_orbitTargetId);
        SetOrbitState(EPlayerOrbitState::One, mgr);
        UpdateOrbitPosition(g_tweakPlayer->GetOrbitNormalDistance(int(x308_orbitType)), mgr);
    }
}

void CPlayer::UpdateOrbitOrientation(CStateManager& mgr)
{
    if (x2f8_morphBallState != EPlayerMorphBallState::Unmorphed)
        return;

    switch (x304_orbitState)
    {
    case EPlayerOrbitState::Two:
        if (x3dc_inFreeLook)
            return;
    case EPlayerOrbitState::One:
    case EPlayerOrbitState::Three:
    case EPlayerOrbitState::Four:
    {
        zeus::CVector3f playerToPoint = x314_orbitPoint - GetTranslation();
        if (!x374_orbitLockEstablished)
            playerToPoint = mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform().basis[1];
        playerToPoint.z = 0.f;
        if (playerToPoint.canBeNormalized())
        {
            zeus::CTransform xf = zeus::lookAt(zeus::CVector3f::skZero, playerToPoint);
            xf.origin = GetTranslation();
            SetTransform(xf);
        }
        break;
    }
    default:
        break;
    }
}

void CPlayer::UpdateOrbitTarget(CStateManager& mgr)
{
    if (!ValidateOrbitTargetIdAndPointer(x310_orbitTargetId, mgr))
        SetOrbitTargetId(kInvalidUniqueId, mgr);
    if (!ValidateOrbitTargetIdAndPointer(x33c_orbitNextTargetId, mgr))
        x33c_orbitNextTargetId = kInvalidUniqueId;
    zeus::CVector3f playerToPoint = x314_orbitPoint - GetTranslation();
    playerToPoint.z = 0.f;
    float playerToPointMag = playerToPoint.magnitude();

    switch (x304_orbitState)
    {
    case EPlayerOrbitState::One:
        if (auto* ent = static_cast<const CActor*>(mgr.GetObjectById(x310_orbitTargetId)))
        {
            if (ent->GetDoTargetDistanceTest() &&
                (playerToPointMag >= GetOrbitMaxLockDistance(mgr) || playerToPointMag < 0.5f))
            {
                if (playerToPointMag < 0.5f)
                    SetOrbitRequest(EPlayerOrbitRequest::Seven, mgr);
                else
                    ActivateOrbitSource(mgr);
            }
        }
        UpdateOrbitPosition(g_tweakPlayer->GetOrbitNormalDistance(int(x308_orbitType)), mgr);
        break;
    case EPlayerOrbitState::Two:
    {
        if (g_tweakPlayer->GetOrbitFixedOffset() &&
            std::fabs(x320_orbitVector.z) > g_tweakPlayer->GetOrbitFixedOffsetZDiff())
        {
            UpdateOrbitFixedPosition();
            return;
        }
        if (playerToPointMag < CalculateOrbitZBasedDistance(x308_orbitType))
            UpdateOrbitPosition(CalculateOrbitZBasedDistance(x308_orbitType), mgr);
        float maxDist = g_tweakPlayer->GetOrbitMaxDistance(int(x308_orbitType));
        if (playerToPointMag > maxDist)
            UpdateOrbitPosition(maxDist, mgr);
        if (x3dd_lookButtonHeld)
            SetOrbitPosition(g_tweakPlayer->GetOrbitNormalDistance(int(x308_orbitType)), mgr);
        zeus::CVector3f eyeToPoint = x314_orbitPoint - GetEyePosition();
        float angleToPoint = std::asin(zeus::clamp(-1.f, std::fabs(eyeToPoint.z) / eyeToPoint.magnitude(), 1.f));
        if ((eyeToPoint.z >= 0.f && angleToPoint >= g_tweakPlayer->GetOrbitUpperAngle()) ||
            (eyeToPoint.z < 0.f && angleToPoint >= g_tweakPlayer->GetOrbitLowerAngle()))
            SetOrbitRequest(EPlayerOrbitRequest::Seven, mgr);
        break;
    }
    case EPlayerOrbitState::Three:
    {
        if (x3dd_lookButtonHeld)
            SetOrbitPosition(x340_, mgr);
        if (playerToPointMag < CalculateOrbitZBasedDistance(x308_orbitType))
        {
            UpdateOrbitPosition(CalculateOrbitZBasedDistance(x308_orbitType), mgr);
            x340_ = CalculateOrbitZBasedDistance(x308_orbitType);
        }
        float maxDist = g_tweakPlayer->GetOrbitMaxDistance(int(x308_orbitType));
        if (playerToPointMag > maxDist)
        {
            UpdateOrbitPosition(maxDist, mgr);
            x340_ = g_tweakPlayer->GetOrbitMaxDistance(int(x308_orbitType));
        }
        break;
    }
    case EPlayerOrbitState::NoOrbit:
        SetOrbitTargetId(kInvalidUniqueId, mgr);
        break;
    default: break;
    }
    UpdateOrbitZPosition();
}

float CPlayer::GetOrbitMaxLockDistance(CStateManager& mgr) const
{
    return mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Scan ?
           g_tweakPlayer->GetScanMaxLockDistance() : g_tweakPlayer->GetOrbitMaxLockDistance();
}

float CPlayer::GetOrbitMaxTargetDistance(CStateManager& mgr) const
{
    return mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Scan ?
           g_tweakPlayer->GetScanMaxTargetDistance() : g_tweakPlayer->GetOrbitMaxTargetDistance();
}

CPlayer::EOrbitValidationResult CPlayer::ValidateOrbitTargetId(TUniqueId uid, CStateManager& mgr) const
{
    if (uid == kInvalidUniqueId)
        return EOrbitValidationResult::InvalidTarget;

    TCastToConstPtr<CActor> act = mgr.GetObjectById(x310_orbitTargetId);
    if (!act || !act->GetIsTargetable() || !act->GetActive())
        return EOrbitValidationResult::InvalidTarget;

    if (x740_staticTimer != 0.f)
        return EOrbitValidationResult::PlayerNotReadyToTarget;

    zeus::CVector3f eyePos = GetEyePosition();
    zeus::CVector3f eyeToOrbit = act->GetOrbitPosition(mgr) - eyePos;
    zeus::CVector3f eyeToOrbitFlat = eyeToOrbit;
    eyeToOrbitFlat.z = 0.f;

    if (eyeToOrbitFlat.canBeNormalized() && eyeToOrbitFlat.magnitude() > 1.f)
    {
        float angleFromHorizon =
            std::asin(zeus::clamp(-1.f, std::fabs(eyeToOrbit.z) / eyeToOrbit.magnitude(), 1.f));
        if ((eyeToOrbit.z >= 0.f && angleFromHorizon >= g_tweakPlayer->GetOrbitUpperAngle()) ||
            (eyeToOrbit.z < 0.f && angleFromHorizon >= g_tweakPlayer->GetOrbitLowerAngle()))
            return EOrbitValidationResult::ExtremeHorizonAngle;
    }
    else
    {
        return EOrbitValidationResult::ExtremeHorizonAngle;
    }

    CPlayerState::EPlayerVisor visor = mgr.GetPlayerState()->GetCurrentVisor();
    u8 flags = act->GetTargetableVisorFlags();
    if (visor == CPlayerState::EPlayerVisor::Combat && (flags & 1) == 0)
        return EOrbitValidationResult::PlayerNotReadyToTarget;
    if (visor == CPlayerState::EPlayerVisor::Scan && (flags & 2) == 0)
        return EOrbitValidationResult::PlayerNotReadyToTarget;
    if (visor == CPlayerState::EPlayerVisor::Thermal && (flags & 4) == 0)
        return EOrbitValidationResult::PlayerNotReadyToTarget;
    if (visor == CPlayerState::EPlayerVisor::XRay && (flags & 8) == 0)
        return EOrbitValidationResult::PlayerNotReadyToTarget;

    if (visor == CPlayerState::EPlayerVisor::Scan && act->GetAreaIdAlways() != GetAreaIdAlways())
        return EOrbitValidationResult::TargetingThroughDoor;

    return EOrbitValidationResult::OK;
}

CPlayer::EOrbitValidationResult CPlayer::ValidateCurrentOrbitTargetId(CStateManager& mgr)
{
    TCastToConstPtr<CActor> act = mgr.GetObjectById(x310_orbitTargetId);
    if (!act || !act->GetIsTargetable() || !act->GetActive())
        return EOrbitValidationResult::InvalidTarget;

    if (!act->GetMaterialList().HasMaterial(EMaterialTypes::Orbit))
    {
        if (!act->GetMaterialList().HasMaterial(EMaterialTypes::Scannable))
            return EOrbitValidationResult::NonTargetableTarget;
        if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Scan)
            return EOrbitValidationResult::NonTargetableTarget;
    }

    EOrbitValidationResult type = ValidateOrbitTargetId(x310_orbitTargetId, mgr);
    if (type != EOrbitValidationResult::OK)
        return type;

    if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Scan &&
        act->GetAreaIdAlways() != GetAreaIdAlways())
        return EOrbitValidationResult::TargetingThroughDoor;

    TCastToConstPtr<CScriptGrapplePoint> point = mgr.GetObjectById(x310_orbitTargetId);
    if ((mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Scan &&
        g_tweakPlayer->GetOrbitWhileScanning()) || point || act->GetAreaIdAlways() != GetAreaIdAlways())
    {
        zeus::CVector3f eyePos = GetEyePosition();
        TUniqueId bestId = kInvalidUniqueId;
        zeus::CVector3f eyeToOrbit = act->GetOrbitPosition(mgr) - eyePos;
        if (eyeToOrbit.canBeNormalized())
        {
            rstl::reserved_vector<TUniqueId, 1024> nearList;
            mgr.BuildNearList(nearList, eyePos, eyeToOrbit.normalized(), eyeToOrbit.magnitude(), OccluderFilter,
                              act.GetPtr());
            for (auto it = nearList.begin() ; it != nearList.end() ;)
            {
                if (const CEntity* ent = mgr.GetObjectById(*it))
                {
                    if (ent->GetAreaIdAlways() != mgr.GetNextAreaId())
                    {
                        const CGameArea* area = mgr.GetWorld()->GetAreaAlways(ent->GetAreaIdAlways());
                        CGameArea::EOcclusionState occState = CGameArea::EOcclusionState::Occluded;
                        if (area->IsPostConstructed())
                            occState = area->GetOcclusionState();
                        if (occState == CGameArea::EOcclusionState::Occluded)
                        {
                            it = nearList.erase(it);
                            continue;
                        }
                    }
                }
                ++it;
            }

            CRayCastResult result =
            mgr.RayWorldIntersection(bestId, eyePos, eyeToOrbit.normalized(), eyeToOrbit.magnitude(),
                                     LineOfSightFilter, nearList);
            if (result.IsValid())
                if (TCastToPtr<CScriptDoor>(mgr.ObjectById(bestId)) || point)
                    return EOrbitValidationResult::TargetingThroughDoor;
        }

        zeus::CVector3f eyeToOrbitFlat = eyeToOrbit;
        eyeToOrbitFlat.z = 0.f;
        if (eyeToOrbitFlat.canBeNormalized())
        {
            float lookToOrbitAngle =
                std::acos(zeus::clamp(-1.f, eyeToOrbitFlat.normalized().dot(GetTransform().basis[1]), 1.f));
            if (x374_orbitLockEstablished)
            {
                if (lookToOrbitAngle >= g_tweakPlayer->GetOrbitHorizAngle())
                    return EOrbitValidationResult::BrokenLookAngle;
            }
            else
            {
                if (lookToOrbitAngle <= M_PIF / 180.f)
                    x374_orbitLockEstablished = true;
            }
        }
        else
        {
            return EOrbitValidationResult::BrokenLookAngle;
        }
    }

    return EOrbitValidationResult::OK;
}

bool CPlayer::ValidateOrbitTargetIdAndPointer(TUniqueId uid, CStateManager& mgr) const
{
    if (uid == kInvalidUniqueId)
        return false;
    return TCastToConstPtr<CActor>(mgr.GetObjectById(uid));
}

zeus::CVector3f CPlayer::GetBallPosition() const
{
    return GetTranslation() + zeus::CVector3f(0.f, 0.f, g_tweakPlayer->GetPlayerBallHalfExtent());
}

zeus::CVector3f CPlayer::GetEyePosition() const
{
    return GetTranslation() + zeus::CVector3f(0.f, 0.f, GetEyeHeight());
}

float CPlayer::GetEyeHeight() const
{
    return x9c8_eyeZBias + (x2d8_fpBounds.max.z - g_tweakPlayer->GetEyeOffset());
}

float CPlayer::GetUnbiasedEyeHeight() const
{
    return x2d8_fpBounds.max.z - g_tweakPlayer->GetEyeOffset();
}

float CPlayer::GetStepUpHeight() const
{
    if (x258_movementState == EPlayerMovementState::Jump ||
        x258_movementState == EPlayerMovementState::StartingJump)
        return 0.3f;
    return CPhysicsActor::GetStepUpHeight();
}

float CPlayer::GetStepDownHeight() const
{
    if (x258_movementState == EPlayerMovementState::Jump)
        return -1.f;
    if (x258_movementState == EPlayerMovementState::StartingJump)
        return 0.1f;
    return CPhysicsActor::GetStepDownHeight();
}

void CPlayer::Teleport(const zeus::CTransform& xf, CStateManager& mgr, bool resetBallCam)
{
    CPhysicsActor::Stop();
    zeus::CVector3f lookDir = xf.basis[1];
    if (lookDir.canBeNormalized())
    {
        lookDir.normalize();
        SetTransform(zeus::lookAt(zeus::CVector3f::skZero, lookDir));
        SetTranslation(xf.origin);
        x500_lookDir = lookDir;
        x50c_moveDir = lookDir;
        x530_gunDir = lookDir;
        x524_lastPosForDirCalc = xf.origin;
        x4f8_moveSpeed = 0.f;
        x4fc_flatMoveSpeed = 0.f;
        x53c_timeMoving = 0.f;
        x4a4_moveSpeedAvg.Clear();
        x540_controlDir = lookDir;
        x54c_controlDirFlat = lookDir;
    }
    else
    {
        SetTranslation(xf.origin);
    }

    x9c5_31_stepCameraZBiasDirty = true;
    x9c8_eyeZBias = 0.f;
    x1f4_lastNonCollidingState = GetMotionState();
    SetMoveState(EPlayerMovementState::OnGround, mgr);
    zeus::CTransform eyeXf = x34_transform;
    eyeXf.origin = GetEyePosition();
    mgr.GetCameraManager()->GetFirstPersonCamera()->Reset(eyeXf, mgr);
    if (resetBallCam)
        mgr.GetCameraManager()->GetBallCamera()->Reset(eyeXf, mgr);
    ForceGunOrientation(x34_transform, mgr);
    SetOrbitRequest(EPlayerOrbitRequest::One, mgr);
}

void CPlayer::BombJump(const zeus::CVector3f& pos, CStateManager& mgr)
{
    if (x2f8_morphBallState == EPlayerMorphBallState::Morphed &&
        x768_morphball->GetBombJumpState() != CMorphBall::EBombJumpState::One)
    {
        zeus::CVector3f posToBall =
            GetTranslation() + zeus::CVector3f(0.f, 0.f, g_tweakPlayer->GetPlayerBallHalfExtent()) - pos;
        float maxJump = g_tweakPlayer->GetBombJumpHeight();
        if (posToBall.magSquared() < maxJump * maxJump &&
            posToBall.dot(zeus::CVector3f::skUp) >= -g_tweakPlayer->GetPlayerBallHalfExtent())
        {
            float upVel = std::sqrt(2.f * std::fabs(g_tweakPlayer->GetNormalGravAccel()) *
                                      g_tweakPlayer->GetBombJumpRadius());
            mgr.GetRumbleManager().Rumble(mgr, ERumbleFxId::Eleven, ERumblePriority::One);
            x2a0_ = 0.01f;
            switch (GetSurfaceRestraint())
            {
            case ESurfaceRestraints::Water:
                upVel *= g_tweakPlayer->GetWaterBallJumpFactor();
                break;
            case ESurfaceRestraints::Lava:
                upVel *= g_tweakPlayer->GetLavaBallJumpFactor();
                break;
            case ESurfaceRestraints::PhazonFluid:
                upVel *= g_tweakPlayer->GetPhazonBallJumpFactor();
                break;
            default:
                break;
            }
            SetVelocityWR(zeus::CVector3f(0.f, 0.f, upVel));
            x768_morphball->SetDamageTimer(0.1f);
            x768_morphball->CancelBoosting();
            if (x9d0_bombJumpCount > 0)
            {
                if (x9d0_bombJumpCount > 2)
                {
                    x9d0_bombJumpCount = 0;
                    x9d4_bombJumpCheckDelayFrames = 0;
                }
                else
                {
                    ++x9d0_bombJumpCount;
                }
            }
            else
            {
                CBallCamera* ballCam = mgr.GetCameraManager()->GetBallCamera();
                if (ballCam->GetTooCloseActorId() != kInvalidUniqueId && ballCam->GetX3E0() < 5.f)
                {
                    x9d0_bombJumpCount = 1;
                    x9d4_bombJumpCheckDelayFrames = 2;
                }
            }
            CSfxHandle hnd = CSfxManager::AddEmitter(1468, GetTranslation(), zeus::CVector3f::skZero,
                                                     false, false, 0x7f, kInvalidAreaId);
            ApplySubmergedPitchBend(hnd);
        }
    }
}

zeus::CTransform CPlayer::CreateTransformFromMovementDirection() const
{
    zeus::CVector3f moveDir = x50c_moveDir;
    if (moveDir.canBeNormalized())
        moveDir.normalize();
    else
        moveDir = zeus::CVector3f::skForward;

    return {zeus::CVector3f(moveDir.y, -moveDir.x, 0.f), moveDir, zeus::CVector3f::skUp, GetTranslation()};
}

const CCollisionPrimitive* CPlayer::GetCollisionPrimitive() const
{
    switch (x2f8_morphBallState)
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

zeus::CTransform CPlayer::GetPrimitiveTransform() const
{
    return CPhysicsActor::GetPrimitiveTransform();
}

void CPlayer::CollidedWith(TUniqueId id, const CCollisionInfoList& list, CStateManager& mgr)
{
    if (x2f8_morphBallState != EPlayerMorphBallState::Unmorphed)
        x768_morphball->CollidedWith(id, list, mgr);
}

float CPlayer::GetActualBallMaxVelocity(float dt) const
{
    ESurfaceRestraints surf = GetSurfaceRestraint();
    float friction = g_tweakBall->GetBallTranslationFriction(int(surf));
    float maxSpeed = g_tweakBall->GetBallTranslationMaxSpeed(int(surf));
    float acceleration = g_tweakBall->GetMaxBallTranslationAcceleration(int(surf));
    return -(friction * xe8_mass * maxSpeed / (acceleration * dt) - maxSpeed - friction);
}

float CPlayer::GetActualFirstPersonMaxVelocity(float dt) const
{
    ESurfaceRestraints surf = GetSurfaceRestraint();
    float friction = g_tweakPlayer->GetPlayerTranslationFriction(int(surf));
    float maxSpeed = g_tweakPlayer->GetPlayerTranslationMaxSpeed(int(surf));
    float acceleration = g_tweakPlayer->GetMaxTranslationalAcceleration(int(surf));
    return -(friction * xe8_mass * maxSpeed / (acceleration * dt) - maxSpeed - friction);
}

void CPlayer::SetMoveState(EPlayerMovementState newState, CStateManager& mgr)
{
    switch (newState)
    {
    case EPlayerMovementState::Jump:
        if (x258_movementState == EPlayerMovementState::StartingJump)
        {
            CSfxHandle hnd = CSfxManager::SfxStart(1470, 1.f, 0.f, true, 0x7f, false, kInvalidAreaId);
            ApplySubmergedPitchBend(hnd);
            mgr.GetRumbleManager().Rumble(mgr, ERumbleFxId::Eleven, ERumblePriority::One);
            x288_startingJumpTimeout = g_tweakPlayer->GetAllowedDoubleJumpTime();
            x290_minJumpTimeout = g_tweakPlayer->GetAllowedDoubleJumpTime() - g_tweakPlayer->GetMinDoubleJumpTime();
            x28c_sjTimer = 0.f;
        }
        else if (x258_movementState != EPlayerMovementState::Jump)
        {
            CSfxHandle hnd = CSfxManager::SfxStart(1471, 1.f, 0.f, true, 0x7f, false, kInvalidAreaId);
            ApplySubmergedPitchBend(hnd);
            x2a0_ = 0.01f;
            x288_startingJumpTimeout = g_tweakPlayer->GetAllowedJumpTime();
            x290_minJumpTimeout = g_tweakPlayer->GetAllowedJumpTime() - g_tweakPlayer->GetMinJumpTime();
            if (mgr.GetPlayerState()->GetItemAmount(CPlayerState::EItemType::SpaceJumpBoots) != 0)
                x28c_sjTimer = g_tweakPlayer->GetMaxDoubleJumpWindow();
            else
                x28c_sjTimer = 0.f;
            if (x294_jumpCameraTimer <= 0.f && x29c_fallCameraTimer <= 0.f &&
                !x3dc_inFreeLook && !x3dd_lookButtonHeld)
            {
                x294_jumpCameraTimer = 0.01f;
                x2a4_cancelCameraPitch = false;
            }
            x258_movementState = EPlayerMovementState::Jump;
            x2ac_surfaceRestraint = ESurfaceRestraints::InAir;
            x2a8_timeSinceJump = 0.f;
        }
        break;
    case EPlayerMovementState::Falling:
        if (x258_movementState == EPlayerMovementState::OnGround)
        {
            x288_startingJumpTimeout = g_tweakPlayer->GetAllowedLedgeTime();
            x258_movementState = EPlayerMovementState::Falling;
            x2a0_ = 0.01f;
            if (g_tweakPlayer->GetFallingDoubleJump())
                x28c_sjTimer = g_tweakPlayer->GetMaxDoubleJumpWindow();
            else
                x28c_sjTimer = 0.f;
        }
        break;
    case EPlayerMovementState::FallingMorphed:
        x258_movementState = EPlayerMovementState::FallingMorphed;
        x2ac_surfaceRestraint = ESurfaceRestraints::Normal;
        break;
    case EPlayerMovementState::OnGround:
        x300_fallingTime = 0.f;
        x258_movementState = EPlayerMovementState::OnGround;
        x288_startingJumpTimeout = 0.f;
        x28c_sjTimer = 0.f;
        x2ac_surfaceRestraint = ESurfaceRestraints::Normal;
        if (x2f8_morphBallState != EPlayerMorphBallState::Morphed)
            AddMaterial(EMaterialTypes::GroundCollider);
        x294_jumpCameraTimer = 0.f;
        x29c_fallCameraTimer = 0.f;
        x2a4_cancelCameraPitch = false;
        x298_jumpPresses = 0;
        break;
    case EPlayerMovementState::StartingJump:
        x288_startingJumpTimeout = 0.f;
        if (x258_movementState != EPlayerMovementState::StartingJump)
        {
            x258_movementState = EPlayerMovementState::StartingJump;
            if (x294_jumpCameraTimer <= x288_startingJumpTimeout &&
                x29c_fallCameraTimer <= x288_startingJumpTimeout &&
                !x3dc_inFreeLook && !x3dd_lookButtonHeld)
            {
                x29c_fallCameraTimer = 0.01f;
                x2a4_cancelCameraPitch = false;
            }
        }
        x2ac_surfaceRestraint = ESurfaceRestraints::InAir;
        break;
    }
}

float CPlayer::JumpInput(const CFinalInput& input, CStateManager& mgr)
{
    if (IsMorphBallTransitioning())
        return GetGravity() * xe8_mass;
    float jumpFactor = 1.f;
    if (!mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::GravitySuit))
    {
        ESurfaceRestraints restraints = GetSurfaceRestraint();
        switch (restraints)
        {
        case ESurfaceRestraints::Water:
            jumpFactor = g_tweakPlayer->GetWaterJumpFactor();
            break;
        case ESurfaceRestraints::Lava:
            jumpFactor = g_tweakPlayer->GetLavaJumpFactor();
            break;
        case ESurfaceRestraints::PhazonFluid:
            jumpFactor = g_tweakPlayer->GetPhazonJumpFactor();
            break;
        default:
            break;
        }
    }

    float vJumpAccel = g_tweakPlayer->GetVerticalJumpAccel();
    float hJumpAccel = g_tweakPlayer->GetHorizontalJumpAccel();
    float doubleJumpImpulse = g_tweakPlayer->GetDoubleJumpImpulse();
    float vDoubleJumpAccel = g_tweakPlayer->GetVerticalDoubleJumpAccel();
    float hDoubleJumpAccel = g_tweakPlayer->GetHorizontalDoubleJumpAccel();

    if (x37c_sidewaysDashing)
    {
        doubleJumpImpulse = g_tweakPlayer->GetSidewaysDoubleJumpImpulse();
        vDoubleJumpAccel = g_tweakPlayer->GetSidewaysVerticalDoubleJumpAccel();
        hDoubleJumpAccel = g_tweakPlayer->GetSidewaysHorizontalDoubleJumpAccel();
    }

    if (x828_waterLevelOnPlayer >= 0.8f * GetEyeHeight())
        doubleJumpImpulse *= jumpFactor;

    if (x258_movementState == EPlayerMovementState::StartingJump)
    {
        if (g_tweakPlayer->GetMaxDoubleJumpWindow() - g_tweakPlayer->GetMinDoubleJumpWindow() >= x28c_sjTimer &&
            0.f < x28c_sjTimer && ControlMapper::GetPressInput(ControlMapper::ECommands::JumpOrBoost, input))
        {
            SetMoveState(EPlayerMovementState::Jump, mgr);
            x384_dashTimer = 0.f;
            x380_strafeInputAtDash = StrafeInput(input);
            if (g_tweakPlayer->GetImpulseDoubleJump())
            {
                zeus::CVector3f impulse(0.f, 0.f, (doubleJumpImpulse - x138_velocity.z) * xe8_mass);
                ApplyImpulseWR(impulse, zeus::CAxisAngle::sIdentity);
            }

            float forwards = ControlMapper::GetAnalogInput(ControlMapper::ECommands::Forward, input);
            float backwards = ControlMapper::GetAnalogInput(ControlMapper::ECommands::Backward, input);
            if (forwards < backwards)
                forwards = ControlMapper::GetAnalogInput(ControlMapper::ECommands::Backward, input);
            return (vDoubleJumpAccel - (vDoubleJumpAccel - hDoubleJumpAccel) * forwards) * xe8_mass * jumpFactor;
        }

        return GetGravity() / xe8_mass;
    }

    if (ControlMapper::GetDigitalInput(ControlMapper::ECommands::JumpOrBoost, input) ||
        (x258_movementState == EPlayerMovementState::Jump && x290_minJumpTimeout <= x288_startingJumpTimeout))
    {
        if (x258_movementState != EPlayerMovementState::Jump)
        {
            if (ControlMapper::GetPressInput(ControlMapper::ECommands::JumpOrBoost, input))
            {
                SetMoveState(EPlayerMovementState::Jump, mgr);
                return vJumpAccel * xe8_mass * jumpFactor;
            }
            return 0.f;
        }
        float forwards = ControlMapper::GetAnalogInput(ControlMapper::ECommands::Forward, input);
        float backwards = ControlMapper::GetAnalogInput(ControlMapper::ECommands::Backward, input);
        if (forwards < backwards)
            forwards = ControlMapper::GetAnalogInput(ControlMapper::ECommands::Backward, input);
        return (vJumpAccel - (vJumpAccel - hJumpAccel) * forwards) * xe8_mass * jumpFactor;
    }

    if (x258_movementState == EPlayerMovementState::Jump)
        SetMoveState(EPlayerMovementState::StartingJump, mgr);

    return 0.f;
}

float CPlayer::TurnInput(const CFinalInput& input) const
{
    if (x304_orbitState == EPlayerOrbitState::One || x304_orbitState == EPlayerOrbitState::Grapple)
        return 0.f;
    if (IsMorphBallTransitioning())
        return 0.f;

    float left = ControlMapper::GetAnalogInput(ControlMapper::ECommands::TurnLeft, input);
    float right = ControlMapper::GetAnalogInput(ControlMapper::ECommands::TurnRight, input);

    if (g_tweakPlayer->GetFreeLookTurnsPlayer())
    {
        if (!g_tweakPlayer->GetHoldButtonsForFreeLook() || x3dd_lookButtonHeld)
        {
            if (left < 0.01f && right < 0.01f)
            {
                left = ControlMapper::GetAnalogInput(ControlMapper::ECommands::LookLeft, input);
                right = ControlMapper::GetAnalogInput(ControlMapper::ECommands::LookRight, input);
            }
        }
    }
    else
    {
        if (!g_tweakPlayer->GetHoldButtonsForFreeLook() || x3dd_lookButtonHeld)
        {
            if (left < 0.01f && right < 0.01f)
            {
                float f31 = ControlMapper::GetAnalogInput(ControlMapper::ECommands::LookLeft, input);
                float f1 = ControlMapper::GetAnalogInput(ControlMapper::ECommands::LookRight, input);
                if (f31 > 0.01f || f1 > 0.01f)
                    return 0.f;
            }
        }
    }

    right = left - right;
    if (x32c_orbitModeTimer > 0.f)
        right *= (1.f - zeus::clamp(0.f, x32c_orbitModeTimer / g_tweakPlayer->GetOrbitModeTimer(), 1.f) * 0.5f);
    return zeus::clamp(-1.f, right, 1.f);
}

float CPlayer::StrafeInput(const CFinalInput& input) const
{
    if (IsMorphBallTransitioning() || x304_orbitState == EPlayerOrbitState::NoOrbit)
        return 0.f;
    return ControlMapper::GetAnalogInput(ControlMapper::ECommands::StrafeRight, input) -
           ControlMapper::GetAnalogInput(ControlMapper::ECommands::StrafeLeft, input);
}

float CPlayer::ForwardInput(const CFinalInput& input, float turnInput) const
{
    float forwards = ControlMapper::GetAnalogInput(ControlMapper::ECommands::Forward, input);
    float backwards = ControlMapper::GetAnalogInput(ControlMapper::ECommands::Backward, input);
    if (x2f8_morphBallState != EPlayerMorphBallState::Unmorphed || InGrappleJumpCooldown())
        backwards = 0.f;
    if (x2f8_morphBallState == EPlayerMorphBallState::Morphing && x584_ballTransitionAnim == 2)
        forwards = 0.f;
    if (x2f8_morphBallState == EPlayerMorphBallState::Unmorphing && x584_ballTransitionAnim == 5)
        forwards = 0.f;
    if (forwards >= 0.001f)
    {
        forwards = zeus::clamp(-1.f, forwards / 0.8f, 1.f);
        if (std::fabs(std::atan2(std::fabs(turnInput), forwards)) < M_PIF / 3.6f)
        {
            zeus::CVector3f x50(std::fabs(turnInput), forwards, 0.f);
            if (x50.canBeNormalized())
                forwards = x50.magnitude();
        }
    }
    if (backwards >= 0.001f)
    {
        backwards = zeus::clamp(-1.f, backwards / 0.8f, 1.f);
        if (std::fabs(std::atan2(std::fabs(turnInput), backwards)) < M_PIF / 3.6f)
        {
            zeus::CVector3f x5c(std::fabs(turnInput), backwards, 0.f);
            if (x5c.canBeNormalized())
                backwards = x5c.magnitude();
        }
    }

    if (!g_tweakPlayer->GetMoveDuringFreeLook())
    {
        zeus::CVector3f velFlat = x138_velocity;
        velFlat.z = 0.f;
        if (x3dc_inFreeLook || x3dd_lookButtonHeld)
            if (x258_movementState != EPlayerMovementState::OnGround)
                if (std::fabs(velFlat.magnitude()) < 0.00001f)
                    return 0.f;
    }

    return zeus::clamp(-1.f, forwards - backwards * g_tweakPlayer->GetBackwardsForceMultiplier(), 1.f);
}

zeus::CVector3f CPlayer::CalculateLeftStickEdgePosition(float strafeInput, float forwardInput) const
{
    float f31 = -1.f;
    float f30 = -0.555f;
    float f29 = 0.555f;

    if (strafeInput >= 0.f)
    {
        f31 = -f31;
        f30 = -f30;
    }

    if (forwardInput < 0.f)
        f29 = -0.555f;

    float f4 = zeus::clamp(-1.f, std::atan(std::fabs(forwardInput) / std::fabs(strafeInput)) / (M_PIF / 4.f), 1.f);
    return zeus::CVector3f(f30 - f31, f29, 0.f) * f4 + zeus::CVector3f(f31, 0.f, 0.f);
}

bool CPlayer::SidewaysDashAllowed(float strafeInput, float forwardInput,
                                  const CFinalInput& input, CStateManager& mgr) const
{
    if (x9c5_28_slidingOnWall || x9c5_29_hitWall || x304_orbitState != EPlayerOrbitState::One)
        return false;

    if (g_tweakPlayer->GetDashOnButtonRelease())
    {
        if (x304_orbitState != EPlayerOrbitState::NoOrbit && g_tweakPlayer->GetDashEnabled() &&
            x288_startingJumpTimeout > 0.f &&
            !ControlMapper::GetDigitalInput(ControlMapper::ECommands::JumpOrBoost, input) &&
            x388_dashButtonHoldTime < g_tweakPlayer->GetDashButtonHoldCancelTime() &&
            std::fabs(strafeInput) >= std::fabs(forwardInput) &&
            std::fabs(strafeInput) > g_tweakPlayer->GetDashStrafeInputThreshold())
            return true;
    }
    else
    {
        if (x304_orbitState != EPlayerOrbitState::NoOrbit && g_tweakPlayer->GetDashEnabled() &&
            ControlMapper::GetDigitalInput(ControlMapper::ECommands::JumpOrBoost, input) &&
            x288_startingJumpTimeout > 0.f &&
            std::fabs(strafeInput) >= std::fabs(forwardInput) &&
            std::fabs(strafeInput) > 0.01f)
        {
            float threshold = std::sqrt(strafeInput * strafeInput + forwardInput * forwardInput) /
            CalculateLeftStickEdgePosition(strafeInput, forwardInput).magnitude();
            if (threshold >= g_tweakPlayer->GetDashStrafeInputThreshold())
                return true;
        }
    }

    return false;
}

void CPlayer::FinishSidewaysDash()
{
    if (x37c_sidewaysDashing)
        x38c_doneSidewaysDashing = true;
    x37c_sidewaysDashing = false;
    x380_strafeInputAtDash = 0.f;
    x384_dashTimer = 0.f;
}

void CPlayer::ComputeDash(const CFinalInput& input, float dt, CStateManager& mgr)
{
    float strafeInput = StrafeInput(input);
    float forwardInput = ForwardInput(input, TurnInput(input));
    zeus::CVector3f orbitPointFlattened(x314_orbitPoint.x, x314_orbitPoint.y, GetTranslation().z);
    zeus::CVector3f orbitToPlayer = GetTranslation() - orbitPointFlattened;
    if (!orbitToPlayer.canBeNormalized())
        return;
    zeus::CVector3f useOrbitToPlayer = orbitToPlayer;
    ESurfaceRestraints restraints = GetSurfaceRestraint();
    float strafeVel = dt * skStrafeDistances[int(restraints)];
    if (ControlMapper::GetDigitalInput(ControlMapper::ECommands::JumpOrBoost, input))
        x388_dashButtonHoldTime += dt;
    if (!x37c_sidewaysDashing)
    {
        if (SidewaysDashAllowed(strafeInput, forwardInput, input, mgr))
        {
            x37c_sidewaysDashing = true;
            x380_strafeInputAtDash = strafeInput;
            x38c_doneSidewaysDashing = true;
            x384_dashTimer = 0.f;
            zeus::CVector3f vel = x138_velocity;
            if (vel.z > 0.f)
            {
                vel.z *= 0.1f;
                if (!x9c5_28_slidingOnWall)
                {
                    SetVelocityWR(vel);
                    x778_dashSfx = CSfxManager::SfxStart(1560, 1.f, 0.f, true, 0x7f, false, kInvalidAreaId);
                    ApplySubmergedPitchBend(x778_dashSfx);
                    mgr.GetRumbleManager().Rumble(mgr, ERumbleFxId::Eleven, ERumblePriority::One);
                }
            }
        }
        strafeVel *= strafeInput;
    }
    else
    {
        x384_dashTimer += dt;
        if (x258_movementState == EPlayerMovementState::OnGround || x384_dashTimer >= x3a0_dashDuration ||
            x9c5_28_slidingOnWall || x9c5_29_hitWall || x304_orbitState != EPlayerOrbitState::One)
        {
            FinishSidewaysDash();
            strafeVel *= strafeInput;
            CSfxManager::RemoveEmitter(x778_dashSfx);
        }
        else
        {
            if (x39c_noStrafeDashBlend)
            {
                strafeVel = dt * skDashStrafeDistances[int(restraints)] * x398_dashSpeedMultiplier;
            }
            else
            {
                strafeVel = ((skDashStrafeDistances[int(restraints)] - skStrafeDistances[int(restraints)]) *
                    (1.f - zeus::clamp(-1.f, x384_dashTimer / x3a4_strafeDashBlendDuration, 1.f)) +
                    skStrafeDistances[int(restraints)]) * x398_dashSpeedMultiplier * dt;
            }
            if (x380_strafeInputAtDash < 0.f)
                strafeVel = -strafeVel;
        }
    }

    float f3 = strafeVel / orbitToPlayer.magnitude();
    float f2 = dt * (x37c_sidewaysDashing ? M_PIF : (M_PIF * 2.f / 3.f));
    useOrbitToPlayer = zeus::CQuaternion::fromAxisAngle(zeus::CVector3f::skUp,
                                                        zeus::clamp(-f2, f3, f2)).transform(orbitToPlayer);
    orbitPointFlattened += useOrbitToPlayer;
    if (!ControlMapper::GetDigitalInput(ControlMapper::ECommands::JumpOrBoost, input))
        x388_dashButtonHoldTime = 0.f;
    strafeVel = skOrbitForwardDistances[int(restraints)] * forwardInput * dt;
    orbitPointFlattened += -useOrbitToPlayer.normalized() * strafeVel;
    zeus::CVector2f velFlat(x138_velocity.x, x138_velocity.y);
    zeus::CVector3f newVelocity = (orbitPointFlattened - GetTranslation()) / dt;
    zeus::CVector3f velDelta(newVelocity.x - x138_velocity.x, newVelocity.y - x138_velocity.y, 0.f);
    strafeVel = velDelta.magnitude();
    if (strafeVel > FLT_EPSILON)
    {
        float accel = dt * GetAcceleration();
        newVelocity = velDelta / strafeVel * accel * zeus::clamp(-1.f, strafeVel / accel, 1.f) + x138_velocity;
        if (!x9c5_28_slidingOnWall)
            SetVelocityWR(newVelocity);
    }
}

void CPlayer::ComputeMovement(const CFinalInput& input, CStateManager& mgr, float dt)
{
    ESurfaceRestraints restraints = GetSurfaceRestraint();

    float jumpInput = JumpInput(input, mgr);
    float zRotateInput = TurnInput(input);
    float forwardInput = ForwardInput(input, zRotateInput);
    SetVelocityWR(GetDampedClampedVelocityWR());
    float turnSpeedMul = g_tweakPlayer->GetTurnSpeedMultiplier();
    if (g_tweakPlayer->GetFreeLookTurnsPlayer())
        if (!g_tweakPlayer->GetHoldButtonsForFreeLook() || x3dd_lookButtonHeld)
            turnSpeedMul = g_tweakPlayer->GetFreeLookTurnSpeedMultiplier();

    if (x304_orbitState == EPlayerOrbitState::NoOrbit ||
        (x3dd_lookButtonHeld && x304_orbitState != EPlayerOrbitState::One &&
            x304_orbitState != EPlayerOrbitState::Grapple))
    {
        if (std::fabs(zRotateInput) < 0.00001f)
        {
            float frictionZAngVel = g_tweakPlayer->GetPlayerRotationFriction(int(restraints)) *
                GetAngularVelocityOR().getVector().z;
            SetAngularVelocityOR({0.f, 0.f, frictionZAngVel});
        }

        float curZAngVel = GetAngularVelocityOR().getVector().z;
        float maxZAngVel = g_tweakPlayer->GetPlayerRotationMaxSpeed(int(restraints)) * turnSpeedMul;
        if (curZAngVel > maxZAngVel)
            SetAngularVelocityOR({0.f, 0.f, maxZAngVel});
        else if (-curZAngVel > maxZAngVel)
            SetAngularVelocityOR({0.f, 0.f, -maxZAngVel});
    }

    float f26 = g_tweakPlayer->GetPlayerRotationMaxSpeed(int(restraints)) * zRotateInput * turnSpeedMul;
    f26 -= GetAngularVelocityOR().getVector().z;
    float remainVel = zeus::clamp(0.f, std::fabs(f26) /
        (turnSpeedMul * g_tweakPlayer->GetPlayerRotationMaxSpeed(int(restraints))), 1.f);
    if (f26 < 0.f)
        zRotateInput = remainVel * -g_tweakPlayer->GetMaxRotationalAcceleration(int(restraints));
    else
        zRotateInput = remainVel * g_tweakPlayer->GetMaxRotationalAcceleration(int(restraints));

    float forwardForce;
    if (std::fabs(forwardInput) >= 0.00001f)
    {
        float maxSpeed = g_tweakPlayer->GetPlayerTranslationMaxSpeed(int(restraints));
        float calcSpeed = g_tweakPlayer->GetPlayerTranslationFriction(int(restraints)) * xe8_mass /
                          (dt * g_tweakPlayer->GetMaxTranslationalAcceleration(int(restraints))) * maxSpeed;
        float f28 = (forwardInput > 0.f ? 1.f : -1.f) * calcSpeed + (maxSpeed - calcSpeed) * forwardInput;
        forwardForce = zeus::clamp(-1.f, (f28 - x34_transform.transposeRotate(x138_velocity).y) / maxSpeed, 1.f) *
            g_tweakPlayer->GetMaxTranslationalAcceleration(int(restraints));
    }
    else
    {
        forwardForce = 0.f;
    }

    if (x304_orbitState != EPlayerOrbitState::NoOrbit && x3dd_lookButtonHeld)
        forwardForce = 0.f;

    if (x304_orbitState == EPlayerOrbitState::NoOrbit || x3dd_lookButtonHeld)
    {
        ApplyForceOR({0.f, forwardForce, jumpInput}, zeus::CAxisAngle::sIdentity);
        if (zRotateInput != 0.f)
            ApplyForceOR(zeus::CVector3f::skZero, zeus::CAxisAngle({0.f, 0.f, 1.f}, zRotateInput));
        if (x37c_sidewaysDashing)
            x38c_doneSidewaysDashing = true;
        x37c_sidewaysDashing = false;
        x380_strafeInputAtDash = 0.f;
        x384_dashTimer = 0.f;
    }
    else
    {
        switch (x304_orbitState)
        {
        case EPlayerOrbitState::One:
        case EPlayerOrbitState::Two:
        case EPlayerOrbitState::Three:
        case EPlayerOrbitState::Four:
            if (!InGrappleJumpCooldown())
                ComputeDash(input, dt, mgr);
            break;
        default:
            break;
        }
        ApplyForceOR({0.f, 0.f, jumpInput}, zeus::CAxisAngle::sIdentity);
    }

    if (x3dc_inFreeLook || x3dd_lookButtonHeld)
    {
        if (!x9c5_28_slidingOnWall && x258_movementState == EPlayerMovementState::OnGround)
        {
            zeus::CVector3f revVelFlat(-x138_velocity.x, -x138_velocity.y, 0.f);
            float revVelFlatMag = revVelFlat.magnitude();
            if (revVelFlatMag > FLT_EPSILON)
            {
                float accel = 0.2f * dt * GetAcceleration();
                float damp = zeus::clamp(-1.f, revVelFlatMag / accel, 1.f);
                SetVelocityWR(revVelFlat / revVelFlatMag * accel * damp + x138_velocity);
            }
        }
    }

    x9c5_29_hitWall = false;
    if (x2d4_accelerationChangeTimer > 0.f)
        x2d0_curAcceleration = 0;
    else
        x2d0_curAcceleration += 1;
    x2d4_accelerationChangeTimer -= dt;
    x2d4_accelerationChangeTimer = std::max(0.f, x2d4_accelerationChangeTimer);
}

float CPlayer::GetWeight() const { return xe8_mass * -GetGravity(); }

zeus::CVector3f CPlayer::GetDampedClampedVelocityWR() const
{
    zeus::CVector3f localVel = x34_transform.transposeRotate(x138_velocity);
    if ((x258_movementState != EPlayerMovementState::StartingJump ||
        GetSurfaceRestraint() != ESurfaceRestraints::InAir) &&
        x304_orbitState == EPlayerOrbitState::NoOrbit)
    {
        float friction = g_tweakPlayer->GetPlayerTranslationFriction(int(GetSurfaceRestraint()));
        if (localVel.y > 0.f)
            localVel.y = std::max(0.f, localVel.y - friction);
        else
            localVel.y = std::min(0.f, localVel.y + friction);
        if (localVel.x > 0.f)
            localVel.x = std::max(0.f, localVel.x - friction);
        else
            localVel.x = std::min(0.f, localVel.x + friction);
    }

    float maxSpeed = g_tweakPlayer->GetPlayerTranslationMaxSpeed(int(GetSurfaceRestraint()));
    localVel.y = zeus::clamp(-maxSpeed, localVel.y, maxSpeed);
    if (x258_movementState == EPlayerMovementState::OnGround)
        localVel.z = 0.f;
    return x34_transform.rotate(localVel);
}

const CScriptWater* CPlayer::GetVisorRunoffEffect(const CStateManager& mgr) const
{
    if (xc4_fluidId == kInvalidUniqueId)
        return nullptr;
    return TCastToConstPtr<CScriptWater>(mgr.GetObjectById(xc4_fluidId)).GetPtr();
}

void CPlayer::SetMorphBallState(EPlayerMorphBallState state, CStateManager& mgr)
{
    if (x2f8_morphBallState == EPlayerMorphBallState::Morphed &&
        state != EPlayerMorphBallState::Morphed)
        x9c5_26_ = x9c4_31_dampUnderwaterMotion;

    x2f8_morphBallState = state;
    xf9_standardCollider = state == EPlayerMorphBallState::Morphed;

    switch (x2f8_morphBallState)
    {
    case EPlayerMorphBallState::Unmorphed:
        if (x9c5_26_ && mgr.GetCameraManager()->GetFluidCounter() == 0)
        {
            if (auto water = GetVisorRunoffEffect(mgr))
            {
                if (const auto& effect = water->GetUnmorphVisorRunoffEffect())
                {
                    CHUDBillboardEffect* sheets = new CHUDBillboardEffect(*effect, {}, mgr.AllocateUniqueId(),
                        true, "WaterSheets", CHUDBillboardEffect::GetNearClipDistance(mgr),
                        CHUDBillboardEffect::GetScaleForPOV(mgr), zeus::CColor::skWhite,
                        zeus::CVector3f::skOne, zeus::CVector3f::skZero);
                    mgr.AddObject(sheets);
                }
                CSfxHandle hnd = CSfxManager::SfxStart(water->GetUnmorphVisorRunoffSfx(), 1.f, 0.f,
                                                       false, 0x7f, false, kInvalidAreaId);
                ApplySubmergedPitchBend(hnd);
            }
        }
        break;
    case EPlayerMorphBallState::Morphed:
    case EPlayerMorphBallState::Morphing:
        x768_morphball->LoadMorphBallModel(mgr);
        break;
    default:
        break;
    }
}

bool CPlayer::CanLeaveMorphBallState(CStateManager& mgr, zeus::CVector3f& pos) const
{
    if (x768_morphball->IsProjectile() || !x590_ ||
        (IsUnderBetaMetroidAttack(mgr) && x2f8_morphBallState == EPlayerMorphBallState::Morphed))
        return false;
    if (!x9c4_28_canLeaveMorphBall)
        return false;
    CMaterialFilter filter = CMaterialFilter::MakeInclude({EMaterialTypes::Solid});
    zeus::CAABox aabb(x2d8_fpBounds.min - zeus::CVector3f(1.f) + GetTranslation(),
                      x2d8_fpBounds.max + zeus::CVector3f(1.f) + GetTranslation());
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    mgr.BuildColliderList(nearList, *this, aabb);
    const zeus::CAABox& baseAABB = GetBaseBoundingBox();
    pos = zeus::CVector3f::skZero;
    for (int i=0 ; i<8 ; ++i)
    {
        zeus::CAABox aabb(baseAABB.min + pos + GetTranslation(),
                          baseAABB.max + pos + GetTranslation());
        CCollidableAABox cAABB(aabb, CMaterialList());
        if (!CGameCollision::DetectCollisionBoolean(mgr, cAABB, zeus::CTransform::Identity(), filter, nearList))
            return true;
        pos.z += 0.1f;
    }
    return false;
}

void CPlayer::SetHudDisable(float staticTimer, float outSpeed, float inSpeed)
{
    x740_staticTimer = staticTimer;
    x744_staticOutSpeed = outSpeed;
    x748_staticInSpeed = inSpeed;
    if (x744_staticOutSpeed != 0.f)
        return;
    if (x740_staticTimer == 0.f)
        x74c_visorStaticAlpha = 1.f;
    else
        x74c_visorStaticAlpha = 0.f;
}

void CPlayer::SetIntoBallReadyAnimation(CStateManager& mgr)
{
    CAnimPlaybackParms parms(2, -1, 1.f, true);
    x64_modelData->AnimationData()->SetAnimation(parms, false);
    x64_modelData->AnimationData()->EnableLooping(false);
    x64_modelData->AdvanceAnimation(0.f, mgr, kInvalidAreaId, true);
    x64_modelData->AnimationData()->EnableAnimation(false);
}

void CPlayer::LeaveMorphBallState(CStateManager& mgr)
{
    x730_transitionModels.clear();
    AddMaterial(EMaterialTypes::GroundCollider, mgr);
    x150_momentum = zeus::CVector3f::skZero;
    SetMorphBallState(EPlayerMorphBallState::Unmorphed, mgr);
    SetHudDisable(FLT_EPSILON, 0.f, 2.f);
    SetHudDisable(FLT_EPSILON, 0.f, 2.f);
    SetIntoBallReadyAnimation(mgr);
    CPhysicsActor::Stop();
    x3e4_freeLookYawAngle = 0.f;
    x3e8_horizFreeLookAngleVel = 0.f;
    x3ec_freeLookPitchAngle = 0.f;
    x3f0_vertFreeLookAngleVel = 0.f;
    x768_morphball->LeaveMorphBallState(mgr);
    mgr.GetCameraManager()->SetPlayerCamera(mgr, mgr.GetCameraManager()->GetFirstPersonCamera()->GetUniqueId());
    mgr.GetCameraManager()->GetBallCamera()->SetState(CBallCamera::EBallCameraState::Zero, mgr);
    SetCameraState(EPlayerCameraState::FirstPerson, mgr);
    mgr.GetCameraManager()->GetFirstPersonCamera()->SetX1C6_24(true);
    mgr.GetCameraManager()->GetFirstPersonCamera()->Think(0.f, mgr);
    ForceGunOrientation(x34_transform, mgr);
    DrawGun(mgr);
}

bool CPlayer::CanEnterMorphBallState(CStateManager& mgr, float f1) const
{
    if (x3b8_grappleState != EGrappleState::None ||
        (IsUnderBetaMetroidAttack(mgr) && x2f8_morphBallState == EPlayerMorphBallState::Unmorphed))
        return false;
    return x9c4_27_canEnterMorphBall;
}

void CPlayer::EnterMorphBallState(CStateManager& mgr)
{
    SetMorphBallState(EPlayerMorphBallState::Morphed, mgr);
    RemoveMaterial(EMaterialTypes::GroundCollider, mgr);
    x730_transitionModels.clear();
    SetAngularVelocityOR(zeus::CAxisAngle(
    zeus::CVector3f(x138_velocity.magnitude() / g_tweakPlayer->GetPlayerBallHalfExtent(), 0.f, 0.f)));
    x768_morphball->EnterMorphBallState(mgr);
    x768_morphball->TakeDamage(-1.f);
    x768_morphball->SetDamageTimer(0.f);
    mgr.GetPlayerState()->StartVisorTransition(CPlayerState::EPlayerVisor::Combat);
}

void CPlayer::ActivateMorphBallCamera(CStateManager& mgr)
{
    SetCameraState(EPlayerCameraState::Ball, mgr);
    mgr.GetCameraManager()->GetBallCamera()->SetState(CBallCamera::EBallCameraState::Zero, mgr);
}

void CPlayer::UpdateCinematicState(CStateManager& mgr)
{
    if (mgr.GetCameraManager()->IsInCinematicCamera())
    {
        if (x2f4_cameraState != EPlayerCameraState::Spawned)
        {
            x2fc_spawnedMorphBallState = x2f8_morphBallState;
            if (x2fc_spawnedMorphBallState == EPlayerMorphBallState::Unmorphing)
                x2fc_spawnedMorphBallState = EPlayerMorphBallState::Unmorphed;
            if (x2fc_spawnedMorphBallState == EPlayerMorphBallState::Morphing)
                x2fc_spawnedMorphBallState = EPlayerMorphBallState::Morphed;
            SetCameraState(EPlayerCameraState::Spawned, mgr);
        }
    }
    else
    {
        if (x2f4_cameraState == EPlayerCameraState::Spawned)
        {
            if (x2fc_spawnedMorphBallState == x2f8_morphBallState)
            {
                switch (x2fc_spawnedMorphBallState)
                {
                case EPlayerMorphBallState::Morphed:
                    SetCameraState(EPlayerCameraState::Ball, mgr);
                    break;
                case EPlayerMorphBallState::Unmorphed:
                    SetCameraState(EPlayerCameraState::FirstPerson, mgr);
                    if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Scan)
                    {
                        ForceGunOrientation(x34_transform, mgr);
                        DrawGun(mgr);
                    }
                default:
                    break;
                }
            }
            else
            {
                CPhysicsActor::Stop();
                SetOrbitRequest(EPlayerOrbitRequest::One, mgr);
                switch (x2fc_spawnedMorphBallState)
                {
                case EPlayerMorphBallState::Unmorphed:
                {
                    zeus::CVector3f vec;
                    if (CanLeaveMorphBallState(mgr, vec))
                    {
                        SetTranslation(GetTranslation() + vec);
                        LeaveMorphBallState(mgr);
                        SetCameraState(EPlayerCameraState::FirstPerson, mgr);
                        ForceGunOrientation(x34_transform, mgr);
                        DrawGun(mgr);
                    }
                    break;
                }
                case EPlayerMorphBallState::Morphed:
                    EnterMorphBallState(mgr);
                    ActivateMorphBallCamera(mgr);
                    mgr.GetCameraManager()->SetupBallCamera(mgr);
                    mgr.GetCameraManager()->GetBallCamera()->Reset(
                    CreateTransformFromMovementDirection(), mgr);
                default:
                    break;
                }
            }
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
    case EPlayerCameraState::FirstPerson:
        camMgr->SetCurrentCameraId(camMgr->GetFirstPersonCamera()->GetUniqueId(), stateMgr);
        x768_morphball->SetBallLightActive(stateMgr, false);
        break;
    case EPlayerCameraState::Ball:
    case EPlayerCameraState::Transitioning:
        camMgr->SetCurrentCameraId(camMgr->GetBallCamera()->GetUniqueId(), stateMgr);
        x768_morphball->SetBallLightActive(stateMgr, true);
        break;
    case EPlayerCameraState::Two:
        break;
    case EPlayerCameraState::Spawned:
    {
        bool ballLight = false;
        if (TCastToPtr<CCinematicCamera> cineCam = camMgr->GetCurrentCamera(stateMgr))
            ballLight = x2f8_morphBallState == EPlayerMorphBallState::Morphed && cineCam->GetW1() & 0x40;
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
    x2d0_curAcceleration = 1;
}

void CPlayer::Touch(CActor& actor, CStateManager& mgr)
{
    if (x2f8_morphBallState == EPlayerMorphBallState::Morphed)
        x768_morphball->Touch(actor, mgr);
}

void CPlayer::CVisorSteam::SetSteam(float targetAlpha, float alphaInDur, float alphaOutDur,
                                    CAssetId txtr, bool affectsThermal)
{
    if (x1c_txtr.IsValid() || targetAlpha > x10_nextTargetAlpha)
    {
        x10_nextTargetAlpha = targetAlpha;
        x14_nextAlphaInDur = alphaInDur;
        x18_nextAlphaOutDur = alphaOutDur;
        x1c_txtr = txtr;
    }
    x28_affectsThermal = affectsThermal;
}

CAssetId CPlayer::CVisorSteam::GetTextureId() const { return xc_tex; }

void CPlayer::CVisorSteam::Update(float dt)
{
    if (!x1c_txtr.IsValid())
        x0_curTargetAlpha = 0.f;
    else
    {
        x0_curTargetAlpha = x10_nextTargetAlpha;
        x4_curAlphaInDur = x14_nextAlphaInDur;
        x8_curAlphaOutDur = x18_nextAlphaOutDur;
        xc_tex = x1c_txtr;
    }

    x1c_txtr.Reset();
    if ((x20_alpha - x0_curTargetAlpha) < 0.000009999f || std::fabs(x20_alpha) > 0.000009999f)
        return;

    if (x20_alpha > x0_curTargetAlpha)
    {
        if (x24_delayTimer <= 0.f)
        {
            x20_alpha -= dt / x8_curAlphaOutDur;
            x20_alpha = std::min(x20_alpha, x0_curTargetAlpha);
        }
        else
        {
            x24_delayTimer = x0_curTargetAlpha - dt;
            x24_delayTimer = zeus::max(0.f, x24_delayTimer);
        }
        return;
    }

    CToken tmpTex = g_SimplePool->GetObj({SBIG('TXTR'), xc_tex});
    if (!tmpTex)
        return;

    x20_alpha += dt / x4_curAlphaInDur;
    if (x20_alpha > x0_curTargetAlpha)
        x20_alpha = x0_curTargetAlpha;

    x24_delayTimer = 0.1f;
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

    if (notEqualStates == 0 && x0_stateSamples[0] == EInputState::StartingJump)
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

void CPlayer::SetSpawnedMorphBallState(EPlayerMorphBallState state, CStateManager& mgr)
{
    x2fc_spawnedMorphBallState = state;
    SetCameraState(EPlayerCameraState::Spawned, mgr);
    if (x2fc_spawnedMorphBallState != x2f8_morphBallState)
    {
        CPhysicsActor::Stop();
        SetOrbitRequest(EPlayerOrbitRequest::One, mgr);
        switch (x2fc_spawnedMorphBallState)
        {
        case EPlayerMorphBallState::Unmorphed:
        {
            zeus::CVector3f pos;
            if (CanLeaveMorphBallState(mgr, pos))
            {
                SetTranslation(GetTranslation() + pos);
                LeaveMorphBallState(mgr);
                ForceGunOrientation(x34_transform, mgr);
                DrawGun(mgr);
            }
            break;
        }
        case EPlayerMorphBallState::Morphed:
            EnterMorphBallState(mgr);
            ActivateMorphBallCamera(mgr);
            mgr.GetCameraManager()->SetupBallCamera(mgr);
            mgr.GetCameraManager()->GetBallCamera()->Reset(CreateTransformFromMovementDirection(), mgr);
            break;
        default:
            break;
        }
    }
}

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
        xa14_phazonCameraShakeTimer = 0.f;
}

bool CPlayer::CheckSubmerged() const
{
    if (xe6_24_fluidCounter == 0)
        return false;

    return x828_waterLevelOnPlayer >= (x2f8_morphBallState == EPlayerMorphBallState::Morphed ?
        2.f * g_tweakPlayer->GetPlayerBallHalfExtent() : 0.5f * GetEyeHeight());
}

void CPlayer::UpdateSubmerged(CStateManager& mgr)
{
    x82c_inLava = false;
    x828_waterLevelOnPlayer = 0.f;
    if (xe6_24_fluidCounter != 0)
    {
        if (TCastToPtr<CScriptWater> water = mgr.ObjectById(xc4_fluidId))
        {
            x828_waterLevelOnPlayer =
                -(zeus::CVector3f::skUp.dot(x34_transform.origin) - water->GetTriggerBoundsWR().max.z);
            CFluidPlane::EFluidType fluidType = water->GetFluidPlane().GetFluidType();
            x82c_inLava = (fluidType == CFluidPlane::EFluidType::Lava || fluidType == CFluidPlane::EFluidType::ThickLava);
            CheckSubmerged();
        }
    }
}

void CPlayer::ApplySubmergedPitchBend(CSfxHandle& sfx)
{
    if (CheckSubmerged())
        CSfxManager::PitchBend(sfx, -1.f);
}

void CPlayer::DetachActorFromPlayer()
{
    x26c_attachedActor = kInvalidUniqueId;
    x270_attachedActorTime = 0.f;
    xa28_attachedActorStruggle = 0.f;
    x490_gun->SetActorAttached(false);
}

bool CPlayer::AttachActorToPlayer(TUniqueId id, bool disableGun)
{
    if (x26c_attachedActor == kInvalidUniqueId)
    {
        if (disableGun)
            x490_gun->SetActorAttached(true);
        x26c_attachedActor = id;
        x270_attachedActorTime = 0.f;
        xa28_attachedActorStruggle = 0.f;
        x768_morphball->ActorAttached();
        return true;
    }

    return false;
}

}
