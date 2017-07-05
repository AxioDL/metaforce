#ifndef __URDE_CPLAYER_HPP__
#define __URDE_CPLAYER_HPP__

#include "CPhysicsActor.hpp"
#include "zeus/CAABox.hpp"
#include "zeus/CTransform.hpp"
#include "Weapon/CWeaponMgr.hpp"
#include "CPlayerEnergyDrain.hpp"
#include "Weapon/CPlayerGun.hpp"
#include "CMorphBall.hpp"

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
    friend class CMorphBall;
    friend class CGroundMovement;

public:
    enum class EPlayerScanState
    {
        NotScanning,
        Scanning,
        ScanComplete
    };

    enum class EPlayerOrbitType
    {
        Zero,
        One
    };

    enum class EPlayerOrbitState
    {
        Zero,
        One,
        Two,
        Three,
        Four,
        Five
    };

    enum class EPlayerOrbitRequest
    {
        Zero,
        One,
        Two,
        Three,
        Four,
        Five,
        Six,
        Seven,
        Eight,
        Nine,
        Ten,
        Eleven
    };

    enum class EPlayerZoneInfo
    {
        Zero,
        One
    };

    enum class EPlayerZoneType
    {
        Always = -1,
        Box = 0,
        Ellipse
    };

    enum class EPlayerMovementState
    {
        OnGround,
        Jump,
        StartingJump,
        Falling,
        FallingMorphed
    };

    enum class EPlayerMorphBallState
    {
        Unmorphed,
        Morphed,
        Morphing,
        Unmorphing
    };

    enum class EPlayerCameraState
    {
        Zero,
        One,
        Two,
        Three,
        Four
    };

    enum class EPlayerMovementSurface
    {
        Normal,
        One,
        Ice,
        MudSlow,
        Four,
        Fluid2Or5,
        Fluid3,
        SnakeWeed
    };

private:
    struct CVisorSteam
    {
        float x0_;
        float x4_;
        float x8_;
        ResId xc_tex;
        float x10_ = 0.f;
        float x14_ = 0.f;
        float x18_ = 0.f;
        ResId x1c_ = -1;
        float x20_alpha = 0.f;
        float x24_ = 0.f;
        bool x28_ = false;

    public:
        CVisorSteam(float a, float b, float c, ResId tex) : x0_(a), x4_(b), x8_(c), xc_tex(tex) {}
        ResId GetTextureId() const;
        void SetSteam(float a, float b, float c, ResId d, bool e);
        void Update(float dt);
        float GetAlpha() const { return x20_alpha; }
    };

    class CInputFilter
    {
    public:
        enum class EInputState
        {
            Jump,
            StartingJump,
            Moving
        };
    private:
        rstl::reserved_vector<EInputState, 20> x0_stateSamples;
        rstl::reserved_vector<zeus::CVector3f, 20> x54_posSamples;
        rstl::reserved_vector<zeus::CVector3f, 20> x148_velSamples;
        rstl::reserved_vector<zeus::CVector2f, 20> x23c_inputSamples;
    public:
        void Reset();
        void AddSample(EInputState state, const zeus::CVector3f& pos,
                       const zeus::CVector3f& vel, const zeus::CVector2f& input);
        bool Passes() const;
    };

    EPlayerMovementState x258_movementState = EPlayerMovementState::OnGround;
    std::vector<CToken> x25c_ballTransitionsRes;
    TUniqueId x26c_ = kInvalidUniqueId;
    float x270_ = 0.f;
    CPlayerEnergyDrain x274_energyDrain = CPlayerEnergyDrain(4);
    float x288_ = 0.f;
    float x28c_sjTimer = 0.f;
    float x290_ = 0.f;
    float x294_ = 0.f;
    u32 x298_ = 0;
    float x29c_ = 0.f;
    float x2a0_ = 0.f;
    u8 x2a4_ = 0;
    float x2a8_ = 1000.f;
    EPlayerMovementSurface x2ac_movementSurface = EPlayerMovementSurface::Normal;
    u32 x2b0_ = 2;
    rstl::reserved_vector<float, 6> x2b4_;
    u32 x2d0_ = 3;
    float x2d4_ = 0.f;
    zeus::CAABox x2d8_;
    float x2f0_ballTransHeight = 0.f;
    EPlayerCameraState x2f4_cameraState = EPlayerCameraState::Zero;
    EPlayerMorphBallState x2f8_morphTransState = EPlayerMorphBallState::Unmorphed;
    u32 x2fc_ = 0;
    float x300_fallingTime = 0.f;
    EPlayerOrbitState x304_orbitState = EPlayerOrbitState::Zero;
    EPlayerOrbitType x308_orbitType = EPlayerOrbitType::Zero;
    EPlayerOrbitRequest x30c_orbitRequest = EPlayerOrbitRequest::Three;
    TUniqueId x310_orbitTargetId = kInvalidUniqueId;
    zeus::CVector3f x314_orbitPoint;
    zeus::CVector3f x320_orbitVector;
    float x32c_ = 0.f;
    EPlayerZoneInfo x330_orbitZone = EPlayerZoneInfo::Zero;
    EPlayerZoneType x334_orbitType = EPlayerZoneType::Ellipse;
    u32 x338_ = 1;
    TUniqueId x33c_ = kInvalidUniqueId;
    float x340_ = 0.f;
    std::vector<TUniqueId> x344_nearbyOrbitObjects;
    std::vector<TUniqueId> x354_onScreenOrbitObjects;
    std::vector<TUniqueId> x364_offScreenOrbitObjects;
    bool x374_ = false;
    float x378_ = 0.f;
    u8 x37c_ = 0;
    float x380_ = 0.f;
    float x384_ = 0.f;
    float x388_ = 0.f;
    bool x38c_ = false;
    u32 x390_orbitSource = 2;
    u8 x394_ = 0;
    float x398_ = 1.5f;
    u8 x39c_ = 0;
    float x3a0_ = 0.5f;
    float x3a4_ = 0.449f;
    EPlayerScanState x3a8_scanState = EPlayerScanState::NotScanning;
    float x3ac_scanningTime = 0.f;
    float x3b0_ = 0.f;
    TUniqueId x3b4_scanningObject = kInvalidUniqueId;
    u32 x3b8_ = 0;
    float x3bc_ = 0.f;
    float x3c0_ = 1.0f;
    float x3c4_ = 0.f;
    float x3c8_ = 0.f;
    float x3cc_ = 0.f;
    float x3d0_ = 0.f;
    float x3d4_ = 0.f;
    float x3d8_ = 0.f;
    bool x3dc_inFreeLook = 0;
    bool x3dd_ = 0;
    bool x3de_lookControlHeld = 0;
    float x3e4_ = 0.f;
    float x3e8_ = 0.f;
    float x3ec_ = 0.f;
    float x3f0_ = 0.f;
    TUniqueId x3f4_aimTarget = kInvalidUniqueId;
    zeus::CVector3f x3f8_targetAimPosition = zeus::CVector3f::skZero;
    TReservedAverage<zeus::CVector3f, 20> x404_aimTargetAverage;
    zeus::CVector3f x480_assistedTargetAim = zeus::CVector3f::skZero;
    float x48c_ = 0.f;
    std::unique_ptr<CPlayerGun> x490_gun;
    float x494_mapAlpha = 1.f;
    float x49c_gunNotFiringTimeout;
    std::unique_ptr<CInputFilter> x4a0_inputFilter;
    u32 x4a4_ = 0;
    float x4f8_ = 0.f;
    float x4fc_ = 0.f;
    zeus::CVector3f x500_ = x34_transform.basis[1];
    zeus::CVector3f x50c_ = x34_transform.basis[1];
    zeus::CVector3f x518_ = x34_transform.basis[1];
    zeus::CVector3f x524_ = x34_transform.basis[1];
    zeus::CVector3f x530_ = x34_transform.basis[1];
    zeus::CVector3f x53c_ = x34_transform.basis[1];
    zeus::CVector3f x548_ = x34_transform.basis[1];
    float x554_ = x34_transform.basis[1].x;
    bool x558_wasDamaged = false;
    float x55c_damageAmt = 0.f;
    float x560_prevDamageAmt = 0.f;
    zeus::CVector3f x564_damageLocation;
    float x570_ = 0.f;
    float x574_morphTime = 0.f;
    float x578_morphDuration = 0.f;
    u32 x57c_ = 0;
    u32 x580_ = 0;
    float x588_alpha = 1.f;
    u32 x594_ = 0;
    u32 x658_ = 0;
    u32 x71c_ = 0;
    std::vector<std::unique_ptr<CModelData>> x730_;
    float x740_ = 0.f;
    float x744_ = 0.f;
    float x748_ = 0.f;
    float x74c_visorStaticAlpha = 1.f;
    float x750_ = 0.f;
    u32 x754_ = 0;
    float x758_ = 0.f;
    u32 x75c_ = 0;
    bool x760_controlsFrozen = false;
    float x764_controlsFrozenTimeout = 0.f;
    std::unique_ptr<CMorphBall> x768_morphball;
    std::unique_ptr<CPlayerCameraBob> x76c_cameraBob;
    CSfxHandle x770_damageLoopSfx;
    float x774_samusVoiceTimeout = 0.f;
    u32 x778_ = 0;
    CSfxHandle x77c_samusVoiceSfx;
    int x780_samusVoicePriority = 0;
    float x784_ = 0.f;
    u16 x788_damageLoopSfxId = 0;
    float x78c_ = 0.f;
    u32 x790_ = 0;
    zeus::CVector3f x794_;
    CVisorSteam x7a0_visorSteam = CVisorSteam(0.f, 0.f, 0.f, -1);
    ResId x7cc_ = -1;
    CAnimRes x7d0_animRes;
    CPlayerState::EBeamId x7ec_beam = CPlayerState::EBeamId::Power;
    std::unique_ptr<CModelData> x7f0_ballTransitionBeamModel;
    zeus::CTransform x7f4_;
    float x824_ = 0.f;
    float x828_waterLevelOnPlayer = 0.f;
    bool x82c_ = false;
    TUniqueId x82e_ridingPlatform = kInvalidUniqueId;
    TUniqueId x830_ = kInvalidUniqueId;
    u32 x834_ = 1000;
    u32 x838_ = 0;
    u32 x93c_ = 0;
    u32 x980_ = 0;

    union
    {
        struct
        {
            bool x9c4_24_ : 1;
            bool x9c4_25_ : 1;
            bool x9c4_26_ : 1;
            bool x9c4_27_ : 1;
            bool x9c4_28_ : 1;
            bool x9c4_29_ : 1;
            bool x9c4_30_ : 1;
            bool x9c4_31_dampUnderwaterMotion : 1;
            bool x9c5_24_ : 1;
            bool x9c5_25_splashUpdated : 1;
            bool x9c5_26_ : 1;
            bool x9c5_27_ : 1;
            bool x9c5_28_slidingOnWall : 1;
            bool x9c5_29_hitWall : 1;
            bool x9c5_30_ : 1;
            bool x9c5_31_ : 1;
            bool x9c6_24_ : 1;
            bool x9c6_25_ : 1;
            bool x9c6_26_ : 1;
            bool x9c6_27_aimingAtProjectile : 1;
            bool x9c6_28_ : 1;
            bool x9c6_29_disableInput : 1;
            bool x9c6_30_newScanScanning : 1;
            bool x9c6_31_overrideRadarRadius : 1;
            bool x9c7_24_ : 1;
            bool x9c7_25_ : 1;
        };
        u32 _dummy = 0;
    };

    float x9c8_ = 0.f;
    float x9cc_ = 0.f;
    u32 x9d0_ = 0;
    u32 x9d4_ = 0;
    float x9d8_ = 0.f;
    float x9dc_ = 1.f;
    float x9e0_ = 0.f;
    rstl::reserved_vector<TUniqueId, 5> x9e4_orbitDisableList;

    float x9f4_deathTime = 0.f;
    float x9f8_ = 0.f;
    float x9fc_ = 0.f;
    TUniqueId xa00_ = kInvalidUniqueId;
    float xa04_ = 0.f;
    ResId xa08_steamTextureId = -1;
    ResId xa0c_;
    u32 xa10_phazonCounter = 0;
    float xa14_ = 0.f;
    float xa18_ = 0.f;
    float xa1c_threatOverride = 0.f;
    float xa20_radarXYRadiusOverride = 1.f;
    float xa24_radarZRadiusOverride = 1.f;
    float xa28_ = 0.f;
    u32 xa2c_ = 2;
    float xa30_ = 4.f;

    void StartLandingControlFreeze();
    void EndLandingControlFreeze();
    void ProcessFrozenInput(float dt, CStateManager& mgr);
    bool CheckSubmerged() const;
    void UpdateSubmerged(CStateManager& mgr);
    void InitializeBallTransition();

public:
    CPlayer(TUniqueId, const zeus::CTransform&, const zeus::CAABox&, ResId w1, const zeus::CVector3f&, float, float,
            float, float, const CMaterialList&);

    bool IsTransparent() const;
    void Update(float, CStateManager& mgr);
    bool StartSamusVoiceSfx(u16 sfx, float vol, int prio);
    bool IsPlayerDeadEnough() const;
    void AsyncLoadSuit(CStateManager& mgr);
    void LoadAnimationTokens();
    bool HasTransitionBeamModel() const;
    virtual bool CanRenderUnsorted(CStateManager& mgr) const;
    virtual const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f& v1, const zeus::CVector3f& v2,
                                                               const CDamageInfo& info) const;
    virtual const CDamageVulnerability* GetDamageVulnerability() const;
    virtual zeus::CVector3f GetHomingPosition(CStateManager& mgr, float) const;
    zeus::CVector3f GetAimPosition(CStateManager& mgr, float) const;
    virtual void FluidFXThink(CActor::EFluidState, CScriptWater& water, CStateManager& mgr);
    zeus::CVector3f GetDamageLocationWR() const { return x564_damageLocation; }
    float GetPrevDamageAmount() const { return x560_prevDamageAmt; }
    float GetDamageAmount() const { return x55c_damageAmt; }
    bool WasDamaged() const { return x558_wasDamaged; }
    void TakeDamage(bool, const zeus::CVector3f&, float, EWeaponType, CStateManager& mgr);
    void Accept(IVisitor& visitor);
    static CHealthInfo* HealthInfo(const CStateManager& mgr);
    bool IsUnderBetaMetroidAttack(CStateManager& mgr) const;
    rstl::optional_object<zeus::CAABox> GetTouchBounds() const;
    void Touch(CActor&, CStateManager& mgr);
    void DoPreThink(float dt, CStateManager& mgr);
    void DoThink(float dt, CStateManager& mgr);
    void UpdateScanningState(const CFinalInput& input, CStateManager& mgr, float);
    void ValidateScanning(const CFinalInput& input, CStateManager& mgr);
    void SetScanningState(EPlayerScanState, CStateManager& mgr);
    void SetSpawnedMorphBallState(EPlayerMorphBallState, CStateManager&);
    bool GetExplorationMode() const;
    bool GetCombatMode() const;
    void RenderGun(const CStateManager& mgr, const zeus::CVector3f&) const;
    void Render(const CStateManager& mgr) const;
    void RenderReflectedPlayer(CStateManager& mgr) const;
    void PreRender(CStateManager& mgr, const zeus::CFrustum&);
    void CalculateRenderBounds();
    void AddToRenderer(const zeus::CFrustum&, const CStateManager&);
    void ComputeFreeLook(const CFinalInput& input);
    void UpdateFreeLook(float dt);
    float GetMaximumPlayerPositiveVerticalVelocity(CStateManager&) const;
    void ProcessInput(const CFinalInput&, CStateManager&);
    void Stop(CStateManager& stateMgr);
    bool GetFrozenState() const;
    void Think(float, CStateManager&);
    void PreThink(float, CStateManager&);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void SetVisorSteam(float, float, float, u32, bool);
    void UpdateFootstepBounds(const CFinalInput& input, CStateManager&, float);
    u16 GetMaterialSoundUnderPlayer(CStateManager& mgr, const u16*, u32, u16);
    u16 SfxIdFromMaterial(const CMaterialList&, const u16*, u32, u16);
    void UpdateCrosshairsState(const CFinalInput&);
    void UpdateVisorTransition(float, CStateManager& mgr);
    void UpdateVisorState(const CFinalInput&, float, CStateManager& mgr);
    void ForceGunOrientation(const zeus::CTransform&, CStateManager& mgr);
    void UpdateDebugCamera(CStateManager& mgr);
    CFirstPersonCamera& GetFirstPersonCamera(CStateManager& mgr);
    void UpdateGunTransform(const zeus::CVector3f&, float, CStateManager& mgr, bool);
    void UpdateAssistedAiming(const zeus::CTransform& xf, const CStateManager& mgr);
    void UpdateAimTargetPrediction(const zeus::CTransform& xf, const CStateManager& mgr);
    void ResetAimTargetPrediction(TUniqueId target);
    void DrawGun(CStateManager& mgr);
    void HolsterGun(CStateManager& mgr);
    EPlayerCameraState GetCameraState() const { return x2f4_cameraState; }
    EPlayerMorphBallState GetMorphballTransitionState() const { return x2f8_morphTransState; }
    void UpdateGrappleArmTransform(const zeus::CVector3f&, CStateManager& mgr, float);
    void ApplyGrappleForces(const CFinalInput& input, CStateManager& mgr, float);
    bool ValidateFPPosition(const zeus::CVector3f& pos, CStateManager& mgr);
    void UpdateGrappleState(const CFinalInput& input, CStateManager& mgr);
    void ApplyGrappleJump(CStateManager& mgr);
    void BeginGrapple(zeus::CVector3f&, CStateManager& mgr);
    void BreakGrapple(EPlayerOrbitRequest, CStateManager& mgr);
    void SetOrbitRequest(EPlayerOrbitRequest req, CStateManager& mgr);
    void PreventFallingCameraPitch();
    void OrbitCarcass(CStateManager&);
    void OrbitPoint(EPlayerOrbitType, CStateManager& mgr);
    zeus::CVector3f GetHUDOrbitTargetPosition() const;
    void SetOrbitState(EPlayerOrbitState, CStateManager& mgr);
    void SetOrbitTargetId(TUniqueId, CStateManager& mgr);
    void UpdateOrbitPosition(float, CStateManager& mgr);
    void UpdateOrbitZPosition();
    void UpdateOrbitFixedPosition();
    void SetOrbitPosition(float, CStateManager& mgr);
    void UpdateAimTarget(CStateManager& mgr);
    void UpdateAimTargetTimer(float);
    bool ValidateAimTargetId(TUniqueId, CStateManager& mgr);
    bool ValidateObjectForMode(TUniqueId, CStateManager& mgr) const;
    TUniqueId FindAimTargetId(CStateManager& mgr);
    TUniqueId CheckEnemiesAgainstOrbitZone(const std::vector<TUniqueId>&, EPlayerZoneInfo, EPlayerZoneType,
                                           CStateManager& mgr) const;
    TUniqueId FindOrbitTargetId(CStateManager& mgr);
    void UpdateOrbitableObjects(CStateManager& mgr);
    TUniqueId FindBestOrbitableObject(const std::vector<TUniqueId>&, EPlayerZoneInfo, CStateManager& mgr) const;
    void FindOrbitableObjects(const rstl::reserved_vector<TUniqueId, 1024>&, std::vector<TUniqueId>&,
                              EPlayerZoneInfo, EPlayerZoneType, CStateManager& mgr, bool) const;
    bool WithinOrbitScreenBox(const zeus::CVector3f&, EPlayerZoneInfo, EPlayerZoneType) const;
    bool WithinOrbitScreenEllipse(const zeus::CVector3f&, EPlayerZoneInfo) const;
    bool CheckOrbitDisableSourceList(CStateManager& mgr);
    bool CheckOrbitDisableSourceList() const { return x9e4_orbitDisableList.size() != 0; }
    void RemoveOrbitDisableSource(TUniqueId);
    void AddOrbitDisableSource(CStateManager& mgr, TUniqueId);
    void UpdateOrbitPreventionTimer(float);
    void UpdateOrbitModeTimer(float);
    void UpdateOrbitZone(CStateManager& mgr);
    void UpdateOrbitInput(const CFinalInput& input, CStateManager& mgr);
    void ActivateOrbitSource(CStateManager& mgr);
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
    void BombJump(const zeus::CVector3f& pos, CStateManager& mgr);
    zeus::CTransform CreateTransformFromMovementDirection() const;
    const CCollisionPrimitive* GetCollisionPrimitive() const;
    zeus::CTransform GetPrimitiveTransform() const;
    void CollidedWith(TUniqueId, const CCollisionInfoList&, CStateManager& mgr);
    float GetActualFirstPersonMaxVelocity() const;
    void SetMoveState(EPlayerMovementState, CStateManager& mgr);
    float JumpInput(const CFinalInput& input, CStateManager& mgr);
    float TurnInput(const CFinalInput& input) const;
    float StrafeInput(const CFinalInput& input) const;
    float ForwardInput(const CFinalInput& input, float) const;
    void ComputeMovement(const CFinalInput& input, CStateManager& mgr, float);
    float GetWeight() const;
    zeus::CVector3f GetDampedClampedVelocityWR() const;
    const CVisorSteam& GetVisorSteam() const { return x7a0_visorSteam; }
    float GetVisorStaticAlpha() const { return x74c_visorStaticAlpha; }
    float GetMapAlpha() const { return x494_mapAlpha; }
    void UpdateCinematicState(CStateManager& mgr);
    void SetCameraState(EPlayerCameraState camState, CStateManager& stateMgr);
    bool IsEnergyLow(const CStateManager& mgr) const;
    EPlayerOrbitState GetOrbitState() const { return x304_orbitState; }
    EPlayerScanState GetScanningState() const { return x3a8_scanState; }
    float GetScanningTime() const { return x3ac_scanningTime; }
    TUniqueId GetOrbitTargetId() const { return x310_orbitTargetId; }
    TUniqueId GetScanningObjectId() const { return x3b4_scanningObject; }
    bool IsNewScanScanning() const { return x9c6_30_newScanScanning; }
    float GetThreatOverride() const { return xa1c_threatOverride; }
    bool IsOverrideRadarRadius() const { return x9c6_31_overrideRadarRadius; }
    float GetRadarXYRadiusOverride() const { return xa20_radarXYRadiusOverride; }
    float GetRadarZRadiusOverride() const { return xa24_radarZRadiusOverride; }
    bool ObjectInScanningRange(TUniqueId id, const CStateManager& mgr) const;
    float GetMorphTime() const { return x574_morphTime; }
    float GetMorphDuration() const { return x578_morphDuration; }
    bool IsInFreeLook() const { return x3dc_inFreeLook; }
    bool IsLookControlHeld() const { return x3de_lookControlHeld; }
    CPlayerGun* GetPlayerGun() const { return x490_gun.get(); }
    CMorphBall* GetMorphBall() const { return x768_morphball.get(); }
    CPlayerCameraBob* GetCameraBob() const { return x76c_cameraBob.get(); }
    float GetDeathTime() const { return x9f4_deathTime; }
    const CPlayerEnergyDrain& GetEnergyDrain() const { return x274_energyDrain; }
    EPlayerZoneInfo GetOrbitZone() const { return x330_orbitZone; }
    EPlayerZoneType GetOrbitType() const { return x334_orbitType; }
    const zeus::CTransform& GetFirstPersonCameraTransform(const CStateManager& mgr) const;
    const std::vector<TUniqueId>& GetNearbyOrbitObjects() const { return x344_nearbyOrbitObjects; }
    const std::vector<TUniqueId>& GetOnScreenOrbitObjects() const { return x354_onScreenOrbitObjects; }
    const std::vector<TUniqueId>& GetOffScreenOrbitObjects() const { return x364_offScreenOrbitObjects; }
    void SetPlayerHitWallDuringMove();

    void Touch();

    void DecrementPhazon();
    void IncrementPhazon();
    void ApplySubmergedPitchBend(CSfxHandle& sfx);
};
}

#endif // __URDE_CPLAYER_HPP__
