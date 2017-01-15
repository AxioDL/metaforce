#include "CPlayer.hpp"
#include "CActorParameters.hpp"
#include "CMorphBall.hpp"
#include "Weapon/CPlayerGun.hpp"
#include "CStateManager.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"
#include "Particle/CGenDescription.hpp"
#include "TCastTo.hpp"

namespace urde
{

static CModelData MakePlayerAnimRes(ResId resId, const zeus::CVector3f& scale)
{
    return {CAnimRes(resId, 0, scale, 0, true), 1};
}

CPlayer::CPlayer(TUniqueId uid, const zeus::CTransform& xf, const zeus::CAABox& aabb, unsigned int resId,
                 const zeus::CVector3f& playerScale, float mass, float stepUp, float stepDown, float f4,
                 const CMaterialList& ml)
: CPhysicsActor(uid, true, "CPlayer", CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList), xf,
                MakePlayerAnimRes(resId, playerScale), ml, aabb, SMoverData(mass), CActorParameters::None(), stepUp,
                stepDown)
{
    x768_morphball.reset(new CMorphBall(*this, f4));
}

bool CPlayer::IsTransparent() const { return x588_alpha < 1.f; }

void CPlayer::Update(float, CStateManager& mgr) {}

bool CPlayer::IsPlayerDeadEnough() const
{
    if (x2f8_morphTransState == 0)
        return x9f4_ < 2.5f;
    else if (x2f8_morphTransState == 1)
        return x9f4_ < 6.f;

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

CHealthInfo* CPlayer::HealthInfo(CStateManager& mgr) { return nullptr; }

bool CPlayer::IsUnderBetaMetroidAttack(CStateManager& mgr) const { return false; }

rstl::optional_object<zeus::CAABox> CPlayer::GetTouchBounds() const { return {}; }

void CPlayer::Touch(CActor&, CStateManager& mgr) {}

void CPlayer::UpdateScanningState(const CFinalInput& input, CStateManager& mgr, float) {}

void CPlayer::ValidateScanning(const CFinalInput& input, CStateManager& mgr) {}

void CPlayer::SetScanningState(EPlayerScanState, CStateManager& mgr) {}

bool CPlayer::GetExplorationMode() const { return false; }

bool CPlayer::GetCombatMode() const { return false; }

void CPlayer::RenderGun(CStateManager& mgr, const zeus::CVector3f&) const {}

void CPlayer::Render(CStateManager& mgr) const {}

void CPlayer::RenderReflectedPlayer(CStateManager& mgr) const {}

void CPlayer::PreRender(CStateManager& mgr, const zeus::CFrustum&) {}

void CPlayer::CalculateRenderBounds() {}

void CPlayer::AddToRenderer(const zeus::CFrustum&, CStateManager&) {}

void CPlayer::ComputeFreeLook(const CFinalInput& input) {}

void CPlayer::UpdateFreeLook(float dt) {}

float CPlayer::GetMaximumPlayerPositiveVerticalVelocity(CStateManager&) const { return 0.f; }

void CPlayer::ProcessInput(const CFinalInput&, CStateManager&) {}

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

void CPlayer::AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) {}

void CPlayer::SetVisorSteam(float, float, float, u32, bool) {}

void CPlayer::UpdateFootstepBounds(const CFinalInput& input, CStateManager&, float) {}

u16 CPlayer::GetMaterialSoundUnderPlayer(CStateManager& mgr, const u16*, int, u16) { return 0; }

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

void CPlayer::DrawGun(CStateManager& mgr) {}

void CPlayer::HolsterGun(CStateManager& mgr) {}

bool CPlayer::GetMorphballTransitionState() const { return false; }

void CPlayer::UpdateGrappleArmTransform(const zeus::CVector3f&, CStateManager& mgr, float) {}

void CPlayer::ApplyGrappleForces(const CFinalInput& input, CStateManager& mgr, float) {}

bool CPlayer::ValidateFPPosition(const zeus::CVector3f& pos, CStateManager& mgr) { return false; }

void CPlayer::UpdateGrappleState(const CFinalInput& input, CStateManager& mgr) {}

void CPlayer::ApplyGrappleJump(CStateManager& mgr) {}

void CPlayer::BeginGrapple(zeus::CVector3f&, CStateManager& mgr) {}

void CPlayer::BreakGrapple(CStateManager& mgr) {}

void CPlayer::PreventFallingCameraPitch() {}

void CPlayer::OrbitCarcass(CStateManager&) {}

void CPlayer::OrbitPoint(EPlayerOrbitType, CStateManager& mgr) {}

zeus::CVector3f CPlayer::GetHUDOrbitTargetPosition() const { return {}; }

void CPlayer::SetOrbitState(EPlayerOrbitState, CStateManager& mgr) {}

void CPlayer::SetOrbitTargetId(TUniqueId) {}

void CPlayer::UpdateOrbitPosition(float, CStateManager& mgr) {}

void CPlayer::UpdateOrbitZPosition() {}

void CPlayer::UpdateOrbitFixedPosition() {}

void CPlayer::SetOrbitPosition(float, CStateManager& mgr) {}

void CPlayer::UpdateAimTarget(CStateManager& mgr) {}

void CPlayer::UpdateAimTargetTimer(float) {}

bool CPlayer::ValidateAimTargetId(TUniqueId, CStateManager& mgr) { return false; }

bool CPlayer::ValidateObjectForMode(TUniqueId, CStateManager& mgr) const { return false; }

TUniqueId CPlayer::FindAimTargetId(CStateManager& mgr) { return {}; }

TUniqueId CPlayer::CheckEnemiesAgainstOrbitZone(const std::vector<TUniqueId>&, EPlayerZoneInfo, EPlayerZoneType,
                                                CStateManager& mgr) const
{
    return {};
}

TUniqueId CPlayer::FindOrbitTargetId(CStateManager& mgr) { return {}; }

void CPlayer::UpdateOrbitableObjects(CStateManager& mgr) {}

TUniqueId CPlayer::FindBestOrbitableObject(const std::vector<TUniqueId>&, EPlayerZoneInfo, CStateManager& mgr) const
{
    return {};
}

void CPlayer::FindOrbitableObjects(const std::vector<TUniqueId>&, std::vector<TUniqueId>&, EPlayerZoneInfo,
                                   EPlayerZoneType, CStateManager& mgr, bool) const
{
}

bool CPlayer::WithinOrbitScreenBox(const zeus::CVector3f&, EPlayerZoneInfo, EPlayerZoneType) const { return false; }

bool CPlayer::WithinOrbitScreenEllipse(const zeus::CVector3f&, EPlayerZoneInfo) const { return false; }

void CPlayer::CheckOrbitDisableSourceList(CStateManager& mgr) {}

void CPlayer::CheckOrbitDisableSourceList() const {}

void CPlayer::RemoveOrbitDisableSource(TUniqueId) {}

void CPlayer::AddOrbitDisableSource(CStateManager& mgr, TUniqueId) {}

void CPlayer::UpdateOrbitPreventionTimer(float) {}

void CPlayer::UpdateOrbitModeTimer(float) {}

void CPlayer::UpdateOrbitZone(CStateManager& mgr) {}

void CPlayer::UpdateOrbitInput(const CFinalInput& input, CStateManager& mgr) {}

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

zeus::CTransform CPlayer::CreateTransformFromMovementDirection() const { return {}; }

const CCollisionPrimitive& CPlayer::GetCollisionPrimitive() const { return CPhysicsActor::GetCollisionPrimitive(); }

zeus::CTransform CPlayer::GetPrimitiveTransform() const { return {}; }

bool CPlayer::CollidedWith(TUniqueId, const CCollisionInfoList&, CStateManager& mgr) { return false; }

float CPlayer::GetActualFirstPersonMaxVelocity() const { return 0.f; }

void CPlayer::SetMoveState(EPlayerMovementState, CStateManager& mgr) {}

float CPlayer::JumpInput(const CFinalInput& input, CStateManager& mgr) { return 0.f; }

float CPlayer::TurnInput(const CFinalInput& input) const { return 0.f; }

float CPlayer::StrafeInput(const CFinalInput& input) const { return 0.f; }

float CPlayer::ForwardInput(const CFinalInput& input, float) const { return 0.f; }

void CPlayer::ComputeMovement(const CFinalInput& input, CStateManager& mgr, float) {}

float CPlayer::GetWeight() const { return 0.f; }

float CPlayer::GetDampedClampedVelocityWR() const { return 0.f; }

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

ResId CPlayer::CVisorSteam::GetTextureId() const { return xc_; }

void CPlayer::CVisorSteam::Update(float dt)
{
    if (x1c_ == -1)
        x0_ = 0.f;
    else
    {
        x0_ = x10_;
        x4_ = x14_;
        x8_ = x18_;
        xc_ = x1c_;
    }

    x1c_ = -1;
    if ((x20_ - x0_) < 0.000009999f || std::fabs(x20_) > 0.000009999f)
        return;

    if (x20_ > x0_)
    {
        if (x24_ <= 0.f)
        {
            x20_ -= (dt / x8_);
            x20_ = std::min(x20_, x0_);
        }
        else
        {
            x24_ = x0_ - dt;
            x24_ = zeus::max(0.f, x24_);
        }
        return;
    }

    CToken tmpTex = g_SimplePool->GetObj({SBIG('TXTR'), xc_});
    if (!tmpTex)
        return;

    x20_ += (x20_ + (dt / x4_));
    if (x20_ > x0_)
        x20_ = x0_;

    x24_ = 0.1f;
}

void CPlayer::SetSpawnedMorphBallState(CPlayer::EPlayerMorphBallState, CStateManager&) {}
}
