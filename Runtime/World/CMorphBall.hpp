#ifndef __URDE_CMORPHBALL_HPP__
#define __URDE_CMORPHBALL_HPP__

#include "World/CActor.hpp"
#include "World/ScriptObjectSupport.hpp"
#include "zeus/CVector3f.hpp"
#include "Collision/CCollidableSphere.hpp"
#include "RetroTypes.hpp"

namespace urde
{
class CActorLights;
class CPlayer;
class CDamageInfo;
class CFinalInput;
class CScriptWater;
class CStateManager;
class CMorphBallShadow;

class CMorphBall
{
public:
    enum class EBallBoostState
    {
        Zero
    };

    enum class ESpiderBallState
    {
        Zero
    };
private:
    CPlayer& x0_player;
    u32 x187c_ = 0;
    float x1DE8_boostTime = 0.f;
    CMorphBallShadow* x1e50_shadow = nullptr;

public:
    CMorphBall(CPlayer& player, float);
    void AcceptScriptMessage(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr);
    ESpiderBallState GetSpiderBallState() const { return ESpiderBallState::Zero; }
    bool InSpiderBallMode() const { return false; }
    zeus::CVector3f GetBallContactSurfaceNormal() const { return {}; }
    void GetModel() const {}
    CCollidableSphere* GetCollidableSphere() const { return nullptr; }
    bool IsProjectile() const { return false; }
    void GetBallContactMeterials() const {}
    void GetWallBumpCounter() const {}
    void GetBoostChargeTimer() const {}
    bool IsBoosting() const { return false; }
    float GetBallRadius() const { return 0.f; }
    float GetBallTouchRadius() const { return 0.f; }
    void ForwardInput(const CFinalInput&) const {}
    void ComputeBallMovement(const CFinalInput&, CStateManager&, float) {}
    bool IsMovementAllowed() const { return false; }
    void UpdateSpiderBall(const CFinalInput&, CStateManager&, float) {}
    void ApplySpiderBallSwingingForces(const CFinalInput&, CStateManager&, float) {}
    void ApplySpiderBallRollForces(const CFinalInput&, CStateManager&, float) {}
    void CalculateSpiderBallAttractionSurfaceForces(const CFinalInput&, CStateManager&,
                                                    const zeus::CTransform&) {}
    void CheckForSwitchToSpiderBallSwinging(CStateManager&) {}
    void FindClosestSpiderBallWaypoint(CStateManager&, const zeus::CVector3f&, zeus::CVector3f&,
                                       zeus::CVector3f&, zeus::CVector3f&, float&, zeus::CVector3f&, bool&,
                                       zeus::CTransform&) const {}
    void SetSpiderBallSwingingState(bool) {}
    void GetSpiderBallControllerMovement(const CFinalInput&, bool, bool) {}
    void ResetSpiderBallSwingControllerMovementTimer() {}
    void UpdateSpiderBallSwingControllerMovementTimer(float, float) {}
    float GetSpiderBallSwingControllerMovementScalar() const { return 0.f; }
    void CreateSpiderBallParticles(const zeus::CVector3f&, const zeus::CVector3f&) {}
    void ComputeMarioMovement(const CFinalInput&, CStateManager&, float) {}
    void SetSpiderBallState(ESpiderBallState) {}
    void GetSwooshToWorld() const {}
    void GetBallToWorld() const {}
    void CalculateSurfaceToWorld(const zeus::CVector3f&, const zeus::CVector3f&, const zeus::CVector3f&) const {}
    void CalculateBallContactInfo(zeus::CVector3f&, zeus::CVector3f&) const {}
    void BallTurnInput(const CFinalInput&) const {}
    void UpdateBallDynamics(CStateManager&, float) {}
    void SwitchToMarble() {}
    void SwitchToTire() {}
    void Update(float, CStateManager&) {}
    void UpdateScriptMessage(EScriptObjectMessage, TUniqueId, CStateManager&) {}
    void DeleteLight(CStateManager&) {}
    void SetBallLightActive(CStateManager&, bool) {}
    void EnterMorphBallState(CStateManager&) {}
    void LeaveMorphBallState(CStateManager&) {}
    void UpdateEffects(float, CStateManager&) {}
    void ComputeBoostBallMovement(const CFinalInput&, const CStateManager&, float) {}
    void EnterBoosting() {}
    void LeaveBoosting() {}
    void CancelBoosting() {}
    void UpdateMarbleDynamics(CStateManager&, float, const zeus::CVector3f&) {}
    void ApplyFriction(float) {}
    void DampLinearAndAngularVelocities(float, float) {}
    zeus::CTransform GetPrimitiveTransform() const { return {}; }
    void DrawCollisionPrimitive() const {}
    void GetMinimumAlignmentSpeed() const {}
    void PreRender(CStateManager&, const zeus::CFrustum&);
    void Render(const CStateManager&, const CActorLights*) const {}
    void ResetMorphBallTransitionFlash() {}
    void UpdateMorphBallTransitionFlash(float) {}
    void RenderMorphBallTransitionFlash(const CStateManager&) const {}
    void IsMorphBallTransitionFlashValid() const {}
    void RenderDamageEffects(const CStateManager&, const zeus::CTransform&) const {}
    void UpdateHalfPipeStatus(CStateManager&, float) {}
    bool GetIsInHalfPipeMode() const { return false; }
    void SetIsInHalfPipeMode(bool) {}
    void GetIsInHalfPipeModeInAir() const {}
    void SetIsInHalfPipeModeInAir(bool) {}
    void GetTouchedHalfPipeRecently() const {}
    void SetTouchedHalfPipeRecently(bool) {}
    void DisableHalfPipeStatus() {}
    void BallCloseToCollision(const CStateManager&, float) const {}
    void CollidedWith(const TUniqueId&, const CCollisionInfoList&, CStateManager&) {}
    void IsInFrustum(const zeus::CFrustum&) const {}
    void ComputeLiftForces(const zeus::CVector3f&, const zeus::CVector3f&, const CStateManager&) {}
    void CalculateSurfaceFriction() const {}
    void ApplyGravity(CStateManager&) {}
    void SpinToSpeed(float, zeus::CVector3f, float) {}
    void ComputeMaxSpeed() const {}
    void Touch(CActor&, CStateManager&) {}
    bool IsClimable(const CCollisionInfo&) const { return false; }
    void FluidFXThink(CActor::EFluidState, CScriptWater&, CStateManager&) {}
    void GetMorphBallModel(const std::string&, float) {}
    void AddSpiderBallElectricalEffect() {}
    void UpdateSpiderBallElectricalEffect() {}
    void RenderSpiderBallElectricalEffect() const {}
    void RenderEnergyDrainEffects(const CStateManager&) const {}
    void TouchModel(const CStateManager&) const {}
    void SetAsProjectile(const CDamageInfo&, const CDamageInfo&) {}
    EBallBoostState GetBallBoostState() const { return EBallBoostState::Zero; }
    void SetBallBoostState(EBallBoostState) {}
    void GetBombJumpState() const {}
    void LoadAnimationTokens(const std::string&) {}
    void TakeDamage(float) {}
    void DrawBallShadow(const CStateManager& mgr);
    void StartLandingSfx() {}
    bool GetX187c() const { return x187c_; }
};

}

#endif // __URDE_CMORPHBALL_HPP__
