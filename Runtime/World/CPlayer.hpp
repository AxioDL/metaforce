#ifndef __URDE_CPLAYER_HPP__
#define __URDE_CPLAYER_HPP__

#include "CPhysicsActor.hpp"
#include "zeus/CAABox.hpp"
#include "zeus/CTransform.hpp"
#include "Weapon/CWeaponMgr.hpp"
#include "CPlayerEnergyDrain.hpp"

namespace urde
{
class CMaterialList;
class CMorphBall;
class CPlayerGun;
class CDamageInfo;
class CScriptWater;
class IVisitor;
class CFinalInput;
class CPlayerCameraBob;
class CFirstPersonCamera;

class CPlayer : public CPhysicsActor
{
    friend class CStateManager;
    friend class CFirstPersonCamera;
    friend class CPlayerCameraBob;
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
        ResId xc_;
        float x10_ = 0.f;
        float x14_ = 0.f;
        float x18_ = 0.f;
        ResId x1c_ = -1;
        float x20_ = 0.f;
        float x24_ = 0.f;
        bool x28_ = false;
    public:
        CVisorSteam(float a, float b, float c, ResId tex) : x0_(a), x4_(b), x8_(c), xc_(tex){}
        ResId GetTextureId() const;
        void SetSteam(float a, float b, float c, ResId d, bool e);
        void Update(float dt);
        float GetAlpha() const;
    };
    zeus::CVector3f x1b4_;
    TUniqueId x1c4_ = kInvalidUniqueId;
    // std::vector<> x258_;
    TUniqueId x26c_ = kInvalidUniqueId;
    float x270_ = 0.f;
    CPlayerEnergyDrain x274_ = CPlayerEnergyDrain(4);
    float x288_ = 0.f;
    float x28c_ = 0.f;
    float x290_ = 0.f;
    float x294_ = 0.f;
    u32 x298_ = 0;
    float x29c_ = 0.f;
    float x2a0_ = 0.f;
    u8 x2a4_ = 0;
    float x2a8_ = 1000.f;
    u32 x2ac_ = 0;
    u32 x2b0_ = 2;
    u32 x2b4_ = 0;
    u32 x2d0_ = 3;
    float x2d4_ = 0.f;
    zeus::CAABox x2d8_;
    float x2f0_ = 0.f;
    u32 x2f4_cameraState = 0;
    u32 x2f8_morphTransState = 0;
    u32 x2fc_ = 0;
    float x300_ = 0.f;
    u32 x304_ = 0;
    u32 x308_ = 0;
    u32 x30c_ = 0;
    TUniqueId x310_grapplePointId = kInvalidUniqueId;
    float x314_ = 0.f;
    float x318_ = 0.f;
    float x31c_ = 0.f;
    float x320_ = 0.f;
    float x324_ = 0.f;
    float x328_ = 0.f;
    float x32c_ = 0.f;
    u32 x330_ = 0;
    u32 x334_ = 1;
    u32 x338_ = 1;
    TUniqueId x33c_ = kInvalidUniqueId;
    float x340_ = 0.f;
    // std::vector<> x344_;
    // std::vector<> x354_;
    // std::vector<> x364_;
    bool x374_ = false;
    float x378_ = 0.f;
    u8 x37c_ = 0;
    float x380_ = 0.f;
    float x384_ = 0.f;
    float x388_ = 0.f;
    bool x38c_;
    u32 x390_ = 2;
    u8 x394_ = 0;
    float x398_ = 1.5f;
    u8 x39c_ = 0;
    float x3a0_ = 0.5f;
    float x3a4_ = 0.449f;
    u32 x3a8_ = 0;
    float x3ac_ = 0.f;
    float x3b0_ = 0.f;
    TUniqueId x3b4_ = kInvalidUniqueId;
    u32 x3b8_ = 0;
    float x3bc_ = 0.f;
    float x3c0_ = 1.0f;
    float x3c4_ = 0.f;
    float x3c8_ = 0.f;
    float x3cc_ = 0.f;
    float x3d0_ = 0.f;
    float x3d4_ = 0.f;
    float x3d8_ = 0.f;
    bool x3dc_ = 0;
    bool x3dd_ = 0;
    bool x3de_ = 0;
    float x3e4_;
    float x3e8_;
    float x3ec_;
    float x3f0_ = 0.f;
    TUniqueId x3f4_ = kInvalidUniqueId;
    zeus::CVector3f x3f8_ = zeus::CVector3f::skZero;
    TReservedAverage<zeus::CVector3f, 20> x404_;
    zeus::CVector3f x480_ = zeus::CVector3f::skZero;
    float x48c_ = 0.f;
    std::unique_ptr<CPlayerGun> x490_gun;
    float x494_ = 1.f;
    float x49c_; /* Value retrieved from TweakPlayerGun */
    // std::unqiue_ptr<> x4a0_;
    u32 x4a4_ = 0;
    bool x558_;
    float x55c_;
    float x560_;
    zeus::CVector3f x564_;
    float x588_alpha;
    std::unique_ptr<CMorphBall> x768_morphball;
    std::unique_ptr<CPlayerCameraBob> x76c_cameraBob;
    CSfxHandle x770_;
    float x774_;
    u32 x778_;
    u32 x77c_;
    u32 x780_;
    float x784_;
    u16 x88_;
    u16 x88a_;
    float x78c_;
    u32 x790_;
    float x794_;
    float x798_;
    float x79c_;
    CVisorSteam x7a0_ = CVisorSteam(0.f, 0.f, 0.f, -1);
    float x9f4_;
    float xa04_;
    ResId xa08_steamTextureId;
    ResId xa0c_;
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
    void CalculateRenderBounds();
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

    void Touch();
    const std::unique_ptr<CPlayerCameraBob>& GetCameraBob() const { return x76c_cameraBob; }
};

}

#endif // __URDE_CPLAYER_HPP__
