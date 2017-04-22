#ifndef __URDE_CBODYSTATECMDMGR_HPP__
#define __URDE_CBODYSTATECMDMGR_HPP__

#include "RetroTypes.hpp"
#include "CharacterCommon.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{
class CBodyStateCmd;
class CBCMeleeAttackCmd;
class CBCProjectileAttackCmd;
class CBCStepCmd;
class CBCJumpCmd;
class CBCGenerateCmd;
class CBCKnockBackCmd;
class CBCHurledCmd;
class CBCGetupCmd;
class CBCLoopReactionCmd;
class CBCKnockDownCmd;
class CBCSlideCmd;
class CBCScriptedCmd;
class CBCCoverCmd;
class CBCWallHangCmd;
class CBCAdditiveAimCmd;
class CBCLocomotionCmd;
class CBCAdditiveAimCmd;
class CBCAdditiveReactionCmd;
class CBCLoopAttackCmd;
class CBodyStateCmdMgr
{
public:
    enum class ESteeringBlendMode
    {
    };
private:
    float x34_steeringSpeedMin;
    float x38_steeringSpeedMax;
    rstl::reserved_vector<CBodyStateCmd, 28> x44_;
public:
    CBodyStateCmdMgr() = default;
    void DeliverCmd(EBodyStateCmd);
    void DeliverCmd(const CBodyStateCmd&);
    void DeliverCmd(const CBCProjectileAttackCmd&);
    void DeliverCmd(const CBCMeleeAttackCmd&);
    void DeliverCmd(const CBCStepCmd&);
    void DeliverCmd(const CBCGenerateCmd&);
    void DeliverCmd(const CBCJumpCmd&);
    void DeliverCmd(const CBCSlideCmd&);
    void DeliverCmd(const CBCKnockBackCmd&);
    void DeliverCmd(const CBCHurledCmd&);
    void DeliverCmd(const CBCGetupCmd&);
    void DeliverCmd(const CBCLoopReactionCmd&);
    void DeliverCmd(const CBCKnockDownCmd&);
    void DeliverCmd(const CBCScriptedCmd&);
    void DeliverCmd(const CBCCoverCmd&);
    void DeliverCmd(const CBCWallHangCmd&);
    void DeliverCmd(const CBCAdditiveAimCmd&);
    void DeliverCmd(const CBCLocomotionCmd&);
    void DeliverCmd(const CBCAdditiveReactionCmd&);
    void DeliverCmd(const CBCLoopAttackCmd&);
    void DeliverTargetVector(const zeus::CVector3f&);
    void DeliverAdditiveTargetVector(const zeus::CVector3f&);
    void SetSteeringBlendSpeed(float);
    void SetSteeringBlendMode(ESteeringBlendMode);
    void SetSteeringSpeedRange(float, float);
    void BlendSteeringCmds();
    void Reset();
    void ClearLocomtionCmds();
    void GetCmd(EBodyStateCmd);
    zeus::CVector3f GetTargetVector() const;
    void GetFaceVector() const;
    void GetMoveVector() const;
    s32 GetNumSteerCmds() const;
    zeus::CVector3f GetAdditiveTargetVector() const;
};
}
#endif // __URDE_CBODYSTATECMDMGR_HPP__
