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
#include "Weapon/CEnergyProjectile.hpp"
#include "MP1/World/CThardusRockProjectile.hpp"

namespace urde
{

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
    x4a0_inputFilter.reset(new CInputFilter());
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
    x490_gun->SetX3e8(x34_transform);
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

void CPlayer::LoadAnimationTokens() {}

bool CPlayer::CanRenderUnsorted(CStateManager& mgr) const { return false; }

const CDamageVulnerability* CPlayer::GetDamageVulnerability(const zeus::CVector3f& v1, const zeus::CVector3f& v2,
                                                            const CDamageInfo& info) const
{
    return nullptr;
}

const CDamageVulnerability* CPlayer::GetDamageVulnerability() const { return nullptr; }

zeus::CVector3f CPlayer::GetHomingPosition(CStateManager& mgr, float) const { return {}; }

zeus::CVector3f CPlayer::GetAimPosition(CStateManager& mgr, float) const { return {}; }

void CPlayer::FluidFXThink(CActor::EFluidState, CScriptWater& water, CStateManager& mgr) {}

zeus::CVector3f CPlayer::GetDamageLocationWR() const { return {}; }

float CPlayer::GetPrevDamageAmount() const { return 0.f; }

float CPlayer::GetDamageAmount() const { return 0.f; }

bool CPlayer::WasDamaged() const { return false; }

void CPlayer::TakeDamage(bool, const zeus::CVector3f&, float, EWeaponType, CStateManager& mgr) {}

void CPlayer::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

CHealthInfo* CPlayer::HealthInfo(const CStateManager& mgr) { return &mgr.GetPlayerState()->HealthInfo(); }

bool CPlayer::IsUnderBetaMetroidAttack(CStateManager& mgr) const { return false; }

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

void CPlayer::ProcessInput(const CFinalInput&, CStateManager&) {}

void CPlayer::Stop(CStateManager& stateMgr)
{

}

bool CPlayer::GetFrozenState() const { return false; }

void CPlayer::Think(float, CStateManager&) {}

void CPlayer::PreThink(float dt, CStateManager& mgr)
{
    x558_ = false;
    x55c_ = 0.f;
    x560_ = 0.f;
    x564_ = zeus::CVector3f::skZero;
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

static const CMaterialFilter SolidMaterialFilter =
    CMaterialFilter::MakeInclude(CMaterialList(EMaterialTypes::Solid));

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
                    x55c_ = 0.f;
                    x560_ = 10.f;
                    x564_ = x34_transform.origin;
                    x558_ = true;
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

void CPlayer::SetVisorSteam(float, float, float, u32, bool) {}

void CPlayer::UpdateFootstepBounds(const CFinalInput& input, CStateManager&, float) {}

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

void CPlayer::ForceGunOrientation(const zeus::CTransform&, CStateManager& mgr) {}

void CPlayer::UpdateDebugCamera(CStateManager& mgr) {}

CFirstPersonCamera& CPlayer::GetFirstPersonCamera(CStateManager& mgr)
{
    return *mgr.GetCameraManager()->GetFirstPersonCamera();
}

void CPlayer::UpdateGunTransform(const zeus::CVector3f&, float, CStateManager& mgr, bool) {}

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

void CPlayer::DrawGun(CStateManager& mgr) {}

void CPlayer::HolsterGun(CStateManager& mgr) {}

void CPlayer::UpdateGrappleArmTransform(const zeus::CVector3f&, CStateManager& mgr, float) {}

void CPlayer::ApplyGrappleForces(const CFinalInput& input, CStateManager& mgr, float) {}

bool CPlayer::ValidateFPPosition(const zeus::CVector3f& pos, CStateManager& mgr) { return false; }

void CPlayer::UpdateGrappleState(const CFinalInput& input, CStateManager& mgr) {}

void CPlayer::ApplyGrappleJump(CStateManager& mgr) {}

void CPlayer::BeginGrapple(zeus::CVector3f&, CStateManager& mgr) {}

void CPlayer::BreakGrapple(CStateManager& mgr) {}

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

void CPlayer::PreventFallingCameraPitch() {}

void CPlayer::OrbitCarcass(CStateManager&) {}

void CPlayer::OrbitPoint(EPlayerOrbitType, CStateManager& mgr) {}

zeus::CVector3f CPlayer::GetHUDOrbitTargetPosition() const { return {}; }

void CPlayer::SetOrbitState(EPlayerOrbitState, CStateManager& mgr) {}

void CPlayer::SetOrbitTargetId(TUniqueId id, CStateManager& mgr)
{
    if (id != kInvalidUniqueId)
    {
        x394_ = (TCastToPtr<CPatterned>(mgr.ObjectById(id)) ||
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
            if (ValidateOrbitTargetId(act->GetUniqueId(), mgr))
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
        if (x394_)
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

bool CPlayer::ValidateOrbitTargetId(TUniqueId, CStateManager& mgr) const { return false; }

bool CPlayer::ValidateCurrentOrbitTargetId(CStateManager& mgr) { return false; }

bool CPlayer::ValidateOrbitTargetIdAndPointer(TUniqueId, CStateManager& mgr) const { return false; }

zeus::CVector3f CPlayer::GetBallPosition() const { return {}; }

zeus::CVector3f CPlayer::GetEyePosition() const { return {}; }

float CPlayer::GetEyeHeight() const { return 0.f; }

float CPlayer::GetStepUpHeight() const { return 0.f; }

float CPlayer::GetStepDownHeight() const { return 0.f; }

void CPlayer::Teleport(const zeus::CTransform& xf, CStateManager& mgr, bool) {}

void CPlayer::BombJump(const zeus::CVector3f& pos, CStateManager& mgr) {}

zeus::CTransform CPlayer::CreateTransformFromMovementDirection() const { return {}; }

const CCollisionPrimitive* CPlayer::GetCollisionPrimitive() const { return CPhysicsActor::GetCollisionPrimitive(); }

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

float CPlayer::GetDampedClampedVelocityWR() const { return 0.f; }

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
    if (x1c_ == -1 || a > x10_)
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
    if (x1c_ == -1)
        x0_ = 0.f;
    else
    {
        x0_ = x10_;
        x4_ = x14_;
        x8_ = x18_;
        xc_tex = x1c_;
    }

    x1c_ = -1;
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

void CPlayer::CInputFilter::Reset()
{
    x0_stateSamples.clear();
    x54_posSamples.clear();
    x148_velSamples.clear();
    x23c_inputSamples.clear();
}

void CPlayer::CInputFilter::AddSample(EInputState state, const zeus::CVector3f& pos,
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

bool CPlayer::CInputFilter::Passes() const
{
    // TODO: Do
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
    x82c_ = false;
    x828_waterLevelOnPlayer = 0.f;
    if (xe6_24_fluidCounter)
    {
        if (TCastToPtr<CScriptWater> water = mgr.ObjectById(xc4_fluidId))
        {
            x828_waterLevelOnPlayer =
                -(zeus::CVector3f::skUp.dot(x34_transform.origin) - water->GetTriggerBoundsWR().max.z);
            CFluidPlane::EFluidType fluidType = water->GetFluidPlane().GetFluidType();
            x82c_ = (fluidType == CFluidPlane::EFluidType::Two || fluidType == CFluidPlane::EFluidType::Five);
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
