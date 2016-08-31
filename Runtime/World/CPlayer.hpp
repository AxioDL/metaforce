#ifndef __URDE_CPLAYER_HPP__
#define __URDE_CPLAYER_HPP__

#include "CPhysicsActor.hpp"
#include "zeus/CAABox.hpp"
#include "zeus/CTransform.hpp"
#include "CWeaponMgr.hpp"

namespace urde
{
class CMaterialList;
class CMorphBall;
class CPlayerGun;
class CDamageInfo;
class CScriptWater;
class IVisitor;
class CFinalInput;
class CFirstPersonCamera;

class CPlayer : public CPhysicsActor
{
    friend class CStateManager;
public:
    enum class EPlayerScanState
    {
    };
    enum class EPlayerOrbitType
    {
    };
    enum class EPlayerOrbitState
    {
    };
    enum class EPlayerZoneInfo
    {
    };
    enum class EPlayerZoneType
    {
    };
    enum class EPlayerMovementState
    {
    };

private:

    struct CVisorSteam
    {
        float x0_;
        float x4_;
        float x8_;
        u32 xc_;
        float x10_ = 0.f;
        float x14_ = 0.f;
        float x18_ = 0.f;
        u32 x1c_ = 0;
        float x20_ = 0.f;
        float x24_ = 0.f;
        bool x28_ = false;

        void SetSteam(float a, float b, float c, u32 d, bool e)
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
        void Update(float dt);
        CVisorSteam(float a, float b, float c, u32 d, bool e)
        : x0_(a), x4_(b), x8_(c), xc_(d), x1c_(e) {}
    };

    std::unique_ptr<CPlayerGun> x490_gun;
    std::unique_ptr<CMorphBall> x768_morphball;

public:
    CPlayer(TUniqueId, const zeus::CTransform&, const zeus::CAABox&, unsigned int,
            const zeus::CVector3f&, float, float, float, float, const CMaterialList&);

    bool IsTransparent() const;
    void Update(float, CStateManager& mgr);
    bool IsPlayerDeadEnough() const;
    void AsyncLoadSuit(CStateManager& mgr);
    void LoadAnimationTokens();
    virtual bool CanRenderUnsorted(CStateManager& mgr) const;
    virtual const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f& v1,
                                                       const zeus::CVector3f& v2,
                                                       const CDamageInfo& info) const;
    virtual const CDamageVulnerability* GetDamageVulnerability() const;
    virtual zeus::CVector3f GetHomingPosition(CStateManager& mgr, float) const;
    zeus::CVector3f GetAimPosition(CStateManager& mgr, float) const;
    virtual void FluidFXThink(CActor::EFluidState, CScriptWater& water, CStateManager& mgr);
    zeus::CVector3f GetDamageLocationWR() const;
    float GetPrevDamageAmount() const;
    float GetDamageAmount() const;
    bool WasDamaged() const;
    void TakeDamage(bool, const zeus::CVector3f&, float, EWeaponType, CStateManager& mgr);
    void Accept(IVisitor& visitor);
    CHealthInfo* HealthInfo(CStateManager& mgr);
    bool IsUnderBetaMetroidAttack(CStateManager& mgr) const;
    rstl::optional_object<zeus::CAABox> GetTouchBounds() const;
    void Touch(CActor&, CStateManager& mgr);
    void UpdateScanningState(const CFinalInput& input, CStateManager& mgr, float);
    void ValidateScanning(const CFinalInput& input, CStateManager& mgr);
    void SetScanningState(EPlayerScanState, CStateManager& mgr);
    bool GetExplorationMode() const;
    bool GetCombatMode() const;
    void RenderGun(CStateManager& mgr, const zeus::CVector3f&) const;
    void Render(CStateManager& mgr) const;
    void RenderReflectedPlayer(CStateManager& mgr) const;
    void PreRender(CStateManager& mgr, const zeus::CFrustum&);
    zeus::CAABox CalculateRenderBounds();
    void AddToRenderer(const zeus::CFrustum&, CStateManager&);
    void ComputeFreeLook(const CFinalInput& input);
    void UpdateFreeLook(float dt);
    float GetMaximumPlayerPositiveVerticalVelocity(CStateManager&) const;
    void ProcessInput(const CFinalInput&, CStateManager&);
    bool GetFrozenState() const;
    void Think(float, CStateManager&);
    void PreThink(float, CStateManager&);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void SetVisorSteam(float, float, float, u32, bool);
    void UpdateFootstepBounds(const CFinalInput& input, CStateManager&, float);
    u16 GetMaterialSoundUnderPlayer(CStateManager& mgr, const u16*, int, u16);
    u16 SfxIdFromMaterial(const CMaterialList&, const u16*, u16);
    void UpdateCrosshairsState(const CFinalInput&);
    void UpdateVisorTransition(float, CStateManager& mgr);
    void UpdateVisorState(const CFinalInput&, float, CStateManager& mgr);
    void ForceGunOrientation(const zeus::CTransform&, CStateManager& mgr);
    void UpdateDebugCamera(CStateManager& mgr);
    CFirstPersonCamera& GetFirstPersonCamera(CStateManager& mgr);
    void UpdateGunTransform(const zeus::CVector3f&, float, CStateManager& mgr, bool);
    void DrawGun(CStateManager& mgr);
    void HolsterGun(CStateManager& mgr);
    bool GetMorphballTransitionState() const;
    void UpdateGrappleArmTransform(const zeus::CVector3f&, CStateManager& mgr, float);
    void ApplyGrappleForces(const CFinalInput& input, CStateManager& mgr, float);
    bool ValidateFPPosition(const zeus::CVector3f& pos, CStateManager& mgr);
    void UpdateGrappleState(const CFinalInput& input, CStateManager& mgr);
    void ApplyGrappleJump(CStateManager& mgr);
    void BeginGrapple(zeus::CVector3f&, CStateManager& mgr);
    void BreakGrapple(CStateManager& mgr);
    void PreventFallingCameraPitch();
    void OrbitCarcass(CStateManager&);
    void OrbitPoint(EPlayerOrbitType, CStateManager& mgr);
    zeus::CVector3f GetHUDOrbitTargetPosition() const;
    void SetOrbitState(EPlayerOrbitState, CStateManager& mgr);
    void SetOrbitTargetId(TUniqueId);
    void UpdateOrbitPosition(float, CStateManager& mgr);
    void UpdateOrbitZPosition();
    void UpdateOrbitFixedPosition();
    void SetOrbitPosition(float, CStateManager& mgr);
    void UpdateAimTarget(CStateManager& mgr);
    void UpdateAimTargetTimer(float);
    bool ValidateAimTargetId(TUniqueId, CStateManager& mgr);
    bool ValidateObjectForMode(TUniqueId, CStateManager& mgr) const;
    TUniqueId FindAimTargetId(CStateManager& mgr);
    TUniqueId CheckEnemiesAgainstOrbitZone(const std::vector<TUniqueId>&, EPlayerZoneInfo,
                                           EPlayerZoneType, CStateManager& mgr) const;
    TUniqueId FindOrbitTargetId(CStateManager& mgr);
    void UpdateOrbitableObjects(CStateManager& mgr);
    TUniqueId FindBestOrbitableObject(const std::vector<TUniqueId>&, EPlayerZoneInfo, CStateManager& mgr) const;
    void FindOrbitableObjects(const std::vector<TUniqueId>&, std::vector<TUniqueId>&, EPlayerZoneInfo,
                              EPlayerZoneType, CStateManager& mgr, bool) const;
    bool WithinOrbitScreenBox(const zeus::CVector3f&, EPlayerZoneInfo, EPlayerZoneType) const;
    bool WithinOrbitScreenEllipse(const zeus::CVector3f&, EPlayerZoneInfo) const;
    void CheckOrbitDisableSourceList(CStateManager& mgr);
    void CheckOrbitDisableSourceList() const;
    void RemoveOrbitDisableSource(TUniqueId);
    void AddOrbitDisableSource(CStateManager& mgr, TUniqueId);
    void UpdateOrbitPreventionTimer(float);
    void UpdateOrbitModeTimer(float);
    void UpdateOrbitZone(CStateManager& mgr);
    void UpdateOrbitInput(const CFinalInput& input, CStateManager& mgr);
    void UpdateOrbitSelection(const CFinalInput& input, CStateManager& mgr);
    void UpdateOrbitOrientation(CStateManager& mgr);
    void UpdateOrbitTarget(CStateManager& mgr);
    float GetOrbitMaxLockDistance(CStateManager& mgr) const;
    float GetOrbitMaxTargetDistance(CStateManager& mgr) const;
    bool ValidateOrbitTargetId(TUniqueId, CStateManager& mgr) const;
    bool ValidateCurrentOrbitTargetId(CStateManager& mgr);
    bool ValidateOrbitTargetIdAndPointer(TUniqueId, CStateManager& mgr) const;
    zeus::CVector3f GetBallPosition() const;
    zeus::CVector3f GetEyePosition() const;
    float GetEyeHeight() const;
    float GetStepUpHeight() const;
    float GetStepDownHeight() const;
    void Teleport(const zeus::CTransform& xf, CStateManager& mgr, bool);
    zeus::CTransform CreateTransformFromMovementDirection() const;
    const CCollisionPrimitive& GetCollisionPrimitive() const;
    zeus::CTransform GetPrimitiveTransform() const;
    bool CollidedWith(TUniqueId, const CCollisionInfoList&, CStateManager& mgr);
    float GetActualFirstPersonMaxVelocity() const;
    void SetMoveState(EPlayerMovementState, CStateManager& mgr);
    float JumpInput(const CFinalInput& input, CStateManager& mgr);
    float TurnInput(const CFinalInput& input) const;
    float StrafeInput(const CFinalInput& input) const;
    float ForwardInput(const CFinalInput& input, float) const;
    void ComputeMovement(const CFinalInput& input, CStateManager& mgr, float);
    float GetWeight() const;
    float GetDampedClampedVelocityWR() const;
};

}

#endif // __URDE_CPLAYER_HPP__
