#ifndef __URDE_CMORPHBALL_HPP__
#define __URDE_CMORPHBALL_HPP__

#include "World/CActor.hpp"
#include "World/ScriptObjectSupport.hpp"
#include "zeus/CVector3f.hpp"
#include "Collision/CCollidableSphere.hpp"
#include "RetroTypes.hpp"
#include "Character/CAnimCharacterSet.hpp"
#include "Particle/CParticleSwoosh.hpp"
#include "Particle/CElementGen.hpp"
#include "CWorldShadow.hpp"
#include "Graphics/CRainSplashGenerator.hpp"
#include "CMorphBallShadow.hpp"
#include "Collision/CCollisionInfoList.hpp"

namespace urde
{
class CActorLights;
class CPlayer;
class CDamageInfo;
class CFinalInput;
class CScriptWater;
class CStateManager;

class CMorphBall
{
public:
    enum class EBallBoostState
    {
        Zero,
        One
    };

    enum class ESpiderBallState
    {
        Inactive,
        Active
    };

    enum class EBombJumpState
    {
        Zero,
        One
    };
private:
    CPlayer& x0_player;
    s32 x4_ = -1;
    u32 x8_ballGlowColorIdx = 0;
    float xc_radius;
    zeus::CVector3f x10_;
    zeus::CVector3f x1c_;
    bool x28_tireMode = false;
    float x2c_tireLeanAngle = 0.f;
    float x30_ballTiltAngle = 0.f;
    CCollidableSphere x38_collisionSphere;
    std::unique_ptr<CModelData> x58_ballModel;
    u32 x5c_ballModelShader = 0;
    std::unique_ptr<CModelData> x60_spiderBallGlassModel;
    u32 x64_spiderBallGlassModelShader = 0;
    std::unique_ptr<CModelData> x68_lowPolyBallModel;
    u32 x6c_lowPolyBallModelShader = 0;
    std::unique_ptr<CModelData> x70_frozenBallModel;
    CCollisionInfoList x74_collisionInfos;
    u32 xc78_ = 0;
    ESpiderBallState x187c_spiderBallState = ESpiderBallState::Inactive;
    zeus::CVector3f x1880_playerToSpiderNormal;
    float x188c_spiderPullMovement = 1.f;
    zeus::CVector3f x1890_spiderTrackPoint;
    zeus::CVector3f x189c_spiderInterpDistBetweenPoints;
    zeus::CVector3f x18a8_spiderDistBetweenPoints;
    float x18b4_ = 0.f;
    float x18b8_ = 0.f;
    bool x18bc_ = false;
    bool x18bd_ = false;
    bool x18be_spiderBallSwinging = false;
    bool x18bf_ = true;
    bool x18c0_isSpiderSurface = false;
    zeus::CTransform x18c4_spiderSurfaceTransform;
    float x18f4_ = 0.f;
    float x18f8_ = 0.f;
    float x18fc_refPullVel = 0.f;
    float x1900_playerToSpiderTrackDist = 0.f;
    float x1904_swingControlDir = 0.f;
    float x1908_swingControlTime = 0.f;
    zeus::CVector2f x190c_;
    float x1914_ = 0.f;
    float x1918_ = 0.f;
    float x191c_damageTimer = 0.f;
    bool x1920_ = false;
    zeus::CTransform x1924_surfaceToWorld;
    bool x1954_isProjectile = false;
    std::vector<CToken> x1958_animationTokens;
    TToken<CSwooshDescription> x1968_slowBlueTailSwoosh;
    TToken<CSwooshDescription> x1970_slowBlueTailSwoosh2;
    TToken<CSwooshDescription> x1978_jaggyTrail;
    TToken<CGenDescription> x1980_wallSpark;
    TToken<CGenDescription> x1988_ballInnerGlow;
    TToken<CGenDescription> x1990_spiderBallMagnetEffect;
    TToken<CGenDescription> x1998_boostBallGlow;
    TToken<CSwooshDescription> x19a0_spiderElectric;
    TToken<CGenDescription> x19a8_morphBallTransitionFlash;
    TToken<CGenDescription> x19b0_effect_morphBallIceBreak;
    std::unique_ptr<CParticleSwoosh> x19b8_slowBlueTailSwooshGen;
    std::unique_ptr<CParticleSwoosh> x19bc_slowBlueTailSwooshGen2;
    std::unique_ptr<CParticleSwoosh> x19c0_slowBlueTailSwoosh2Gen;
    std::unique_ptr<CParticleSwoosh> x19c4_slowBlueTailSwoosh2Gen2;
    std::unique_ptr<CParticleSwoosh> x19c8_jaggyTrailGen;
    std::unique_ptr<CElementGen> x19cc_wallSparkGen;
    std::unique_ptr<CElementGen> x19d0_ballInnerGlowGen;
    std::unique_ptr<CElementGen> x19d4_spiderBallMagnetEffectGen;
    std::unique_ptr<CElementGen> x19d8_boostBallGlowGen;
    std::unique_ptr<CElementGen> x19dc_morphBallTransitionFlashGen;
    std::unique_ptr<CElementGen> x19e0_effect_morphBallIceBreakGen;
    rstl::reserved_vector<std::pair<std::unique_ptr<CParticleSwoosh>, bool>, 32> x19e4_spiderElectricGens;
    std::list<u32> x1b6c_;
    CRandom16 x1b80_rand = {99};
    rstl::reserved_vector<TToken<CGenDescription>, 8> x1b84_wakeEffects;
    rstl::reserved_vector<std::unique_ptr<CElementGen>, 8> x1bc8_wakeEffectGens;
    std::unique_ptr<CElementGen> x1bcc_[8];
    s32 x1c0c_wakeEffectIdx = -1;
    TUniqueId x1c10_ballInnerGlowLight = kInvalidUniqueId;
    std::unique_ptr<CWorldShadow> x1c14_worldShadow;
    std::unique_ptr<CActorLights> x1c18_actorLights;
    std::unique_ptr<CRainSplashGenerator> x1c1c_rainSplashGen;
    float x1c20_ = 0.f;
    float x1c24_ = 0.5f;
    float x1c28_ = 1.f;
    bool x1c2c_ = false;
    float x1c30_ = 0.f;
    float x1c34_ = 0.f;
    float x1c38_ = 0.f;
    TReservedAverage<zeus::CQuaternion, 5> x1c3c_quats = {{}};
    TReservedAverage<zeus::CVector3f, 5> x1c90_vecs = {{}};
    TReservedAverage<float, 15> x1cd0_ = {{}};
    TReservedAverage<zeus::CVector3f, 15> x1d10_ = {{}};
    u32 x1dc8_ = 0;
    zeus::CVector3f x1dcc_;
    zeus::CVector3f x1dd8_;
    bool x1de4_24 : 1;
    bool x1de4_25 : 1;
    float x1de8_boostChargeTime = 0.f;
    float x1dec_ = 0.f;
    float x1df0_ = 0.f;
    float x1df4_boostDrainTime = 0.f;
    bool x1df8_24_inHalfPipeMode : 1;
    bool x1df8_25_inHalfPipeModeInAir : 1;
    bool x1df8_26_touchedHalfPipeRecently : 1;
    bool x1df8_27_ballCloseToCollision : 1;
    float x1dfc_ = 0.f;
    float x1e00_ = 0.f;
    float x1e04_ = 0.f;
    zeus::CVector3f x1e08_;
    zeus::CVector3f x1e14_;
    u32 x1e20_ = 0;
    CSfxHandle x1e24_boostSfxHandle;
    CSfxHandle x1e28_wallHitSfxHandle;
    CSfxHandle x1e2c_rollSfxHandle;
    CSfxHandle x1e30_spiderSfxHandle;
    u16 x1e34_rollSfx = 0xffff;
    u16 x1e36_landSfx = 0xffff;
    u32 x1e38_wallSparkFrameCountdown = 0;
    EBallBoostState x1e3c_boostState = EBallBoostState::Zero;
    EBombJumpState x1e40_bombJumpState = EBombJumpState::Zero;
    float x1e44_ = 0.f;
    float x1e48_ = 0.f;
    float x1e4c_ = 0.f;
    std::unique_ptr<CMorphBallShadow> x1e50_shadow;
    void LoadAnimationTokens(const std::string& ancsName);
    void InitializeWakeEffects();
    static std::unique_ptr<CModelData> GetMorphBallModel(const char* name, float radius);
    void SelectMorphBallSounds(const CMaterialList& mat);
    void UpdateMorphBallSounds(float dt);
    static zeus::CVector3f TransformSpiderBallForcesXY(const zeus::CVector2f& forces, CStateManager& mgr);
    static zeus::CVector3f TransformSpiderBallForcesXZ(const zeus::CVector2f& forces, CStateManager& mgr);
    void ResetSpiderBallForces();
public:
    CMorphBall(CPlayer& player, float radius);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr);
    const CCollidableSphere& GetCollidableSphere() const { return x38_collisionSphere; }
    bool IsProjectile() const { return x1954_isProjectile; }
    void GetBallContactMaterials() const {}
    void GetWallBumpCounter() const {}
    void GetBoostChargeTimer() const {}
    bool IsBoosting() const { return false; }
    float GetBallRadius() const;
    float GetBallTouchRadius() const;
    float ForwardInput(const CFinalInput& input) const;
    float BallTurnInput(const CFinalInput& input) const;
    void ComputeBallMovement(const CFinalInput& input, CStateManager& mgr, float dt);
    bool IsMovementAllowed() const;
    void UpdateSpiderBall(const CFinalInput& input, CStateManager& mgr, float dt);
    void ApplySpiderBallSwingingForces(const CFinalInput& input, CStateManager& mgr, float dt);
    void ApplySpiderBallRollForces(const CFinalInput& input, CStateManager& mgr, float dt);
    zeus::CVector2f CalculateSpiderBallAttractionSurfaceForces(const CFinalInput& input) const;
    bool CheckForSwitchToSpiderBallSwinging(CStateManager& mgr) const;
    bool FindClosestSpiderBallWaypoint(CStateManager& mgr, const zeus::CVector3f& ballCenter,
                                       zeus::CVector3f& closestPoint,
                                       zeus::CVector3f& interpDeltaBetweenPoints,
                                       zeus::CVector3f& deltaBetweenPoints, float& distance,
                                       zeus::CVector3f& normal, bool& isSurface,
                                       zeus::CTransform& surfaceTransform) const;
    void SetSpiderBallSwingingState(bool active);
    float GetSpiderBallControllerMovement(const CFinalInput& input) const;
    void ResetSpiderBallSwingControllerMovementTimer();
    void UpdateSpiderBallSwingControllerMovementTimer(float movement, float dt);
    float GetSpiderBallSwingControllerMovementScalar() const;
    void CreateSpiderBallParticles(const zeus::CVector3f&, const zeus::CVector3f&);
    void ComputeMarioMovement(const CFinalInput& input, CStateManager& mgr, float dt);
    void SetSpiderBallState(ESpiderBallState state) { x187c_spiderBallState = state; }
    zeus::CTransform GetSwooshToWorld() const;
    zeus::CTransform GetBallToWorld() const;
    zeus::CTransform CalculateSurfaceToWorld(const zeus::CVector3f& trackNormal,
                                             const zeus::CVector3f& trackPoint,
                                             const zeus::CVector3f& ballDir) const;
    bool CalculateBallContactInfo(zeus::CVector3f& normal, zeus::CVector3f& point) const;
    void UpdateBallDynamics(CStateManager& mgr, float dt);
    void SwitchToMarble();
    void SwitchToTire();
    void Update(float dt, CStateManager& mgr);
    void DeleteLight(CStateManager& mgr);
    void SetBallLightActive(CStateManager& mgr, bool active);
    void EnterMorphBallState(CStateManager& mgr);
    void LeaveMorphBallState(CStateManager& mgr);
    void UpdateEffects(float dt, CStateManager& mgr);
    void ComputeBoostBallMovement(const CFinalInput& input, CStateManager& mgr, float dt);
    void EnterBoosting(CStateManager& mgr);
    void LeaveBoosting();
    void CancelBoosting();
    bool UpdateMarbleDynamics(CStateManager& mgr, float dt, const zeus::CVector3f& point);
    void ApplyFriction(float);
    void DampLinearAndAngularVelocities(float, float);
    zeus::CTransform GetPrimitiveTransform() const;
    void DrawCollisionPrimitive() const;
    void GetMinimumAlignmentSpeed() const;
    void PreRender(CStateManager&, const zeus::CFrustum&);
    void Render(const CStateManager&, const CActorLights*) const;
    void ResetMorphBallTransitionFlash();
    void UpdateMorphBallTransitionFlash(float);
    void RenderMorphBallTransitionFlash(const CStateManager&) const;
    void UpdateIceBreakEffect(float dt);
    void RenderIceBreakEffect(const CStateManager& mgr) const;
    bool IsMorphBallTransitionFlashValid() const { return x19dc_morphBallTransitionFlashGen != 0; }
    void RenderDamageEffects(const CStateManager&, const zeus::CTransform&) const;
    void UpdateHalfPipeStatus(CStateManager&, float);
    bool GetIsInHalfPipeMode() const { return x1df8_24_inHalfPipeMode; }
    void SetIsInHalfPipeMode(bool b) { x1df8_24_inHalfPipeMode = b; }
    bool GetIsInHalfPipeModeInAir() const { return x1df8_25_inHalfPipeModeInAir; }
    void SetIsInHalfPipeModeInAir(bool b) { x1df8_25_inHalfPipeModeInAir = b; }
    bool GetTouchedHalfPipeRecently() const { return x1df8_26_touchedHalfPipeRecently; }
    void SetTouchedHalfPipeRecently(bool b) { x1df8_26_touchedHalfPipeRecently = b; }
    void DisableHalfPipeStatus();
    bool BallCloseToCollision(const CStateManager& mgr, float dist, const CMaterialFilter& filter) const;
    void CollidedWith(TUniqueId id, const CCollisionInfoList& list, CStateManager& mgr);
    bool IsInFrustum(const zeus::CFrustum& frustum) const;
    void ComputeLiftForces(const zeus::CVector3f&, const zeus::CVector3f&, const CStateManager&);
    float CalculateSurfaceFriction() const;
    void ApplyGravity(CStateManager& mgr);
    void SpinToSpeed(float holdMag, zeus::CVector3f torque, float mag);
    float ComputeMaxSpeed() const;
    void Touch(CActor& actor, CStateManager& mgr);
    bool IsClimbable(const CCollisionInfo& cinfo) const;
    void FluidFXThink(CActor::EFluidState, CScriptWater&, CStateManager&);
    void GetMorphBallModel(const std::string&, float);
    void LoadMorphBallModel(CStateManager& mgr);
    void AddSpiderBallElectricalEffect();
    void UpdateSpiderBallElectricalEffects();
    void RenderSpiderBallElectricalEffect() const;
    void RenderEnergyDrainEffects(const CStateManager&) const;
    void TouchModel(const CStateManager& mgr) const;
    void SetAsProjectile(const CDamageInfo&, const CDamageInfo&);
    EBallBoostState GetBallBoostState() const { return x1e3c_boostState; }
    void SetBallBoostState(EBallBoostState state) { x1e3c_boostState = state; }
    EBombJumpState GetBombJumpState() const { return x1e40_bombJumpState; }
    void TakeDamage(float);
    void DrawBallShadow(const CStateManager& mgr);
    void DeleteBallShadow();
    void CreateBallShadow();
    void RenderToShadowTex(CStateManager& mgr);
    void StartLandingSfx();
    ESpiderBallState GetSpiderBallState() const { return x187c_spiderBallState; }
    void SetDamageTimer(float t) { x191c_damageTimer = t; }
    void Stop();
    void StopSounds();
    void StopEffects();
    CModelData& GetMorphballModelData() const { return *x58_ballModel; }
    u32 GetMorphballModelShader() const { return x5c_ballModelShader; }
    bool GetX1DE4_25() const { return x1de4_25; }
    void SetX1DE4_25(bool b) { x1de4_25 = b; }

    static const u8 BallTransFlashColors[9][3];
};

}

#endif // __URDE_CMORPHBALL_HPP__
