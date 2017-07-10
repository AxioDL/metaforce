#ifndef __URDE_CBODYCONTROLLER_HPP__
#define __URDE_CBODYCONTROLLER_HPP__

#include "RetroTypes.hpp"
#include "zeus/CQuaternion.hpp"
#include "CharacterCommon.hpp"
#include "CBodyStateCmdMgr.hpp"
#include "CBodyStateInfo.hpp"

namespace urde
{

class CActor;
class CAnimPlaybackParms;
class CFinalInput;
class CPASAnimParmData;
class CRandom16;
class CStateManager;
class CPASDatabase;

class CBodyController
{
    CActor& x0_actor;
    CBodyStateCmdMgr x4_cmdMgr;
    CBodyStateInfo x2a4_bodyStateInfo;
    zeus::CQuaternion x2dc_rot;
    pas::ELocomotionType x2ec_locomotionType = pas::ELocomotionType::Relaxed;
    pas::EFallState x2f0_fallState = pas::EFallState::Zero;
    EBodyType x2f4_bodyType;
    s32 x2f8_curAnim = -1;
    float x2fc_rotRate;
    union
    {
        struct
        {
            bool x300_24_animationOver : 1;
            bool x300_25_ : 1;
            bool x300_26_ : 1;
            bool x300_27_ : 1;
            bool x300_28_playDeathAnims : 1;
        };
        u32 _dummy = 0;
    };
    float x304_ = 0.f;
    float x308_ = 0.f;
    float x30c_ = 0.f;
    float x310_ = 0.f;
    zeus::CVector3f x314_;
    float x320_ = 0.f;
    float x324_ = 0.f;
    float x328_ = 0.f;
    float x32c_ = 0.f;
    float x330_restrictedFlyerMoveSpeed = 0.f;
public:
    CBodyController(CActor& owner, float f1, EBodyType bodyType);
    void GetCurrentStateId() const;
    CBodyStateCmdMgr& GetCommandMgr() { return x4_cmdMgr; }
    const CBodyStateCmdMgr& GetCommandMgr() const { return x4_cmdMgr; }
    void SetDoDeathAnims(bool d) { x300_28_playDeathAnims = d; }
    bool IsElectrocuting() const;
    bool IsOnFire() const;
    bool IsFrozen() const;
    const CBodyStateInfo& GetBodyStateInfo() const { return x2a4_bodyStateInfo; }
    bool GetIsActive() const;
    void BodyStateInfo();
    float GetTurnSpeed() const;
    pas::ELocomotionType GetLocomotionType() const { return x2ec_locomotionType; }
    CActor& GetOwner() const { return x0_actor; }
    bool IsAnimationOver() const { return x300_24_animationOver; }
    void EnableAnimation(bool e);
    bool ShouldPlayDeathAnims() const { return x300_28_playDeathAnims; }
    s32 GetCurrentAnimId() const { return x2f8_curAnim; }
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
    float GetAnimTimeRemaining() const;
    void SetPlaybackRate(float);
    void MultiplyPlaybackRate(float);
    void SetDeltaRotation(const zeus::CQuaternion& q) { x2dc_rot *= q; }
    void FaceDirection(const zeus::CVector3f&, float);
    void FaceDirection3D(const zeus::CVector3f&, const zeus::CVector3f&, float);
    void HasBodyInfo(CActor&);
    void ProcessInput(const CFinalInput&);
    const CPASDatabase& GetPASDatabase() const;
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
    EBodyType GetBodyType() const { return x2f4_bodyType; }
    bool HasBeenFrozen() const;
    float GetRestrictedFlyerMoveSpeed() const { return x330_restrictedFlyerMoveSpeed; }
};
}

#endif // __URDE_CBODYCONTROLLER_HPP__
