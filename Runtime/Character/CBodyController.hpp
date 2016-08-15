#ifndef __URDE_CBODYCONTROLLER_HPP__
#define __URDE_CBODYCONTROLLER_HPP__

#include "RetroTypes.hpp"
#include "zeus/CQuaternion.hpp"
#include "CharacterCommon.hpp"

namespace urde
{

class CActor;
class CAnimPlaybackParms;
class CFinalInput;
class CPASAnimParmData;
class CRandom16;
class CStateManager;
class CBodyController
{
public:
    CBodyController(CActor&, float, EBodyType);
    void GetCurrentStateId() const;
    void GetComandMgr();
    void SetDoDeathAnims(bool);
    bool IsElectrocuting() const;
    bool IsOnFire() const;
    bool IsFrozen() const;
    void GetBodyStateInfo() const;
    bool GetIsActive() const;
    void BodyStateInfo();
    float GetTurnSpeed() const;
    pas::ELocomotionType GetLocomotionType() const;
    void GetOwner();
    bool IsAnimationOver() const;
    void EnableAnimation(bool);
    bool ShouldPlayDeathAnims() const;
    void GetCurrentAnimId() const;
    void Activate(CStateManager&);
    void GetCurrentAdditiveState();
    void SetState(pas::EAnimationState);
    void Update(float, CStateManager&);
    void Deactivate();
    bool ShouldBeHurled() const;
    bool HasBodyState(pas::EAnimationState);
    pas::EFallState GetFallState() const;
    void SetFallState(pas::EFallState);
    void UpdateBody(float, CStateManager&);
    void SetAdditiveState(pas::EAnimationState);
    void SetTurnSpeed(float);
    void SetCurrentAnimation(const CAnimPlaybackParms&, bool, bool);
    void GetAnimTimeRemaining() const;
    void SetPlaybackRate(float);
    void MultiplyPlaybackRate(float);
    void SetDeltaRotation(const zeus::CQuaternion&);
    void FaceDirection(const zeus::CVector3f&, float);
    void FaceDirection3D(const zeus::CVector3f&, float);
    void HasBodyInfo(CActor&);
    void ProcessInput(const CFinalInput&);
    void GetPASDatabase() const;
    void PlayBestAnimation(const CPASAnimParmData&, CRandom16&);
    void LoopBestAnimation(const CPASAnimParmData&, CRandom16&);
    void Freeze(float, float, float);
    void UnFreeze();
    void GetPercentageFrozen() const;
    void SetOnFire(float);
    void DouseFlames();
    void SetElectrocuting(float);
    void DouseElectrocuting();
    void UpdateFrozenInfo(float, CStateManager&);
    void GetCurrentAdditiveStateId() const;
    EBodyType GetBodyType() const;
    bool HasBeenFrozen() const;
    void GetOwner() const;
};
}

#endif // __URDE_CBODYCONTROLLER_HPP__
