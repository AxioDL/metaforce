#ifndef __URDE_CBODYSTATECMDMGR_HPP__
#define __URDE_CBODYSTATECMDMGR_HPP__

#include "RetroTypes.hpp"
#include "CharacterCommon.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{

class CBodyStateCmd
{
    EBodyStateCmd x4_cmd;
public:
    virtual ~CBodyStateCmd() = default;
    CBodyStateCmd(EBodyStateCmd cmd) : x4_cmd(cmd) {}
    EBodyStateCmd GetCommandId() const { return x4_cmd; }
};

class CBCMeleeAttackCmd : public CBodyStateCmd
{
    pas::ESeverity x8_severity = pas::ESeverity::Invalid;
    zeus::CVector3f xc_;
    bool x18_ = false;
public:
    CBCMeleeAttackCmd() : CBodyStateCmd(EBodyStateCmd::MeleeAttack) {}
    CBCMeleeAttackCmd(pas::ESeverity severity)
    : CBodyStateCmd(EBodyStateCmd::MeleeAttack), x8_severity(severity) {}
};

class CBCProjectileAttackCmd : public CBodyStateCmd
{
    pas::ESeverity x8_severity = pas::ESeverity::Invalid;
    zeus::CVector3f xc_;
    bool x18_ = false;
public:
    CBCProjectileAttackCmd() : CBodyStateCmd(EBodyStateCmd::ProjectileAttack) {}
    CBCProjectileAttackCmd(pas::ESeverity severity, const zeus::CVector3f& vec, bool b)
    : CBodyStateCmd(EBodyStateCmd::ProjectileAttack), x8_severity(severity), xc_(vec), x18_(b) {}
};

class CBCStepCmd : public CBodyStateCmd
{
    pas::EStepDirection x8_dir = pas::EStepDirection::Invalid;
    pas::EStepType xc_type = pas::EStepType::Normal;
public:
    CBCStepCmd() : CBodyStateCmd(EBodyStateCmd::Step) {}
    CBCStepCmd(pas::EStepDirection dir, pas::EStepType type)
    : CBodyStateCmd(EBodyStateCmd::Step), x8_dir(dir), xc_type(type) {}
};

class CBCJumpCmd : public CBodyStateCmd
{
    pas::EJumpType x8_type = pas::EJumpType::Zero;
    zeus::CVector3f xc_;
    zeus::CVector3f x18_;
    bool x24_24_ : 1;
    bool x24_25_ : 1;
public:
    CBCJumpCmd()
    : CBodyStateCmd(EBodyStateCmd::Jump) { x24_24_ = false; x24_25_ = false; }
    CBCJumpCmd(const zeus::CVector3f& vec, pas::EJumpType type)
    : CBodyStateCmd(EBodyStateCmd::Jump), x8_type(type), xc_(vec) { x24_24_ = false; x24_25_ = false; }
};

class CBCGenerateCmd : public CBodyStateCmd
{
    pas::EGenerateType x8_type = pas::EGenerateType::Invalid;
public:
    CBCGenerateCmd() : CBodyStateCmd(EBodyStateCmd::Generate) {}
    CBCGenerateCmd(pas::EGenerateType type, int i)
    : CBodyStateCmd(EBodyStateCmd::Generate), x8_type(type) {}
    CBCGenerateCmd(pas::EGenerateType type, const zeus::CVector3f& vec)
    : CBodyStateCmd(EBodyStateCmd::Generate), x8_type(type) {}
};

class CBCKnockBackCmd : public CBodyStateCmd
{
    zeus::CVector3f x8_;
    pas::ESeverity x14_severity = pas::ESeverity::Invalid;
public:
    CBCKnockBackCmd() : CBodyStateCmd(EBodyStateCmd::KnockBack) {}
    CBCKnockBackCmd(const zeus::CVector3f& vec, pas::ESeverity severity)
    : CBodyStateCmd(EBodyStateCmd::KnockBack), x8_(vec), x14_severity(severity) {}
};

class CBCHurledCmd : public CBodyStateCmd
{
    zeus::CVector3f x8_v0;
    zeus::CVector3f x14_v1;
    bool x20_ = false;
public:
    CBCHurledCmd() : CBodyStateCmd(EBodyStateCmd::Hurled) {}
    CBCHurledCmd(const zeus::CVector3f& v0, const zeus::CVector3f& v1)
    : CBodyStateCmd(EBodyStateCmd::Hurled), x8_v0(v0), x14_v1(v1) {}
};

class CBCGetupCmd : public CBodyStateCmd
{
    pas::EGetupType x8_type = pas::EGetupType::Invalid;
public:
    CBCGetupCmd() : CBodyStateCmd(EBodyStateCmd::Getup) {}
    CBCGetupCmd(pas::EGetupType type)
    : CBodyStateCmd(EBodyStateCmd::Getup), x8_type(type) {}
};

class CBCLoopReactionCmd : public CBodyStateCmd
{
    pas::EReactionType x8_type = pas::EReactionType::Invalid;
public:
    CBCLoopReactionCmd() : CBodyStateCmd(EBodyStateCmd::LoopReaction) {}
    CBCLoopReactionCmd(pas::EReactionType type)
    : CBodyStateCmd(EBodyStateCmd::LoopReaction), x8_type(type) {}
};

class CBCLoopHitReactionCmd : public CBodyStateCmd
{
    pas::EReactionType x8_type = pas::EReactionType::Invalid;
public:
    CBCLoopHitReactionCmd() : CBodyStateCmd(EBodyStateCmd::LoopHitReaction) {}
    CBCLoopHitReactionCmd(pas::EReactionType type)
    : CBodyStateCmd(EBodyStateCmd::LoopHitReaction), x8_type(type) {}
};

class CBCKnockDownCmd : public CBodyStateCmd
{
    zeus::CVector3f x8_;
    pas::ESeverity x14_severity = pas::ESeverity::Invalid;
public:
    CBCKnockDownCmd() : CBodyStateCmd(EBodyStateCmd::KnockDown) {}
    CBCKnockDownCmd(const zeus::CVector3f& vec, pas::ESeverity severity)
    : CBodyStateCmd(EBodyStateCmd::KnockDown), x8_(vec), x14_severity(severity) {}
};

class CBCSlideCmd : public CBodyStateCmd
{
    pas::ESlideType x8_type = pas::ESlideType::Invalid;
    zeus::CVector3f xc_;
public:
    CBCSlideCmd() : CBodyStateCmd(EBodyStateCmd::Slide) {}
    CBCSlideCmd(pas::ESlideType type, const zeus::CVector3f& vec)
    : CBodyStateCmd(EBodyStateCmd::Slide), x8_type(type), xc_(vec) {}
};

class CBCScriptedCmd : public CBodyStateCmd
{
    int x8_ = -1;
    bool xc_24_ : 1;
    bool xc_25_ : 1;
    float x10_ = 0.f;
public:
    CBCScriptedCmd() : CBodyStateCmd(EBodyStateCmd::Scripted) { xc_24_ = false; xc_25_ = false; }
    CBCScriptedCmd(int i, bool b1, bool b2, float f) : CBodyStateCmd(EBodyStateCmd::Scripted),
    x8_(i), x10_(f) { xc_24_ = b1; xc_25_ = b2; }
};

class CBCCoverCmd : public CBodyStateCmd
{
    pas::ECoverDirection x8_dir = pas::ECoverDirection::Invalid;
    zeus::CVector3f xc_;
    zeus::CVector3f x18_;
public:
    CBCCoverCmd() : CBodyStateCmd(EBodyStateCmd::Cover) {}
    CBCCoverCmd(pas::ECoverDirection dir, const zeus::CVector3f& v1, const zeus::CVector3f& v2) :
    CBodyStateCmd(EBodyStateCmd::Cover), x8_dir(dir), xc_(v1), x18_(v2) {}
};

class CBCWallHangCmd : public CBodyStateCmd
{
    TUniqueId x8_uid = kInvalidUniqueId;
public:
    CBCWallHangCmd() : CBodyStateCmd(EBodyStateCmd::WallHang) {}
    CBCWallHangCmd(TUniqueId uid) :
    CBodyStateCmd(EBodyStateCmd::WallHang), x8_uid(uid) {}
};

class CBCAdditiveAimCmd : public CBodyStateCmd
{
public:
    CBCAdditiveAimCmd() : CBodyStateCmd(EBodyStateCmd::AdditiveAim) {}
};

class CBCAdditiveFlinchCmd : public CBodyStateCmd
{
    float x8_ = 1.f;
public:
    CBCAdditiveFlinchCmd() : CBodyStateCmd(EBodyStateCmd::AdditiveFlinch) {}
    CBCAdditiveFlinchCmd(float f) : CBodyStateCmd(EBodyStateCmd::AdditiveFlinch), x8_(f) {}
};

class CBCAdditiveReactionCmd : public CBodyStateCmd
{
    float x8_ = 1.f;
    pas::EReactionType xc_type = pas::EReactionType::Invalid;
    bool x10_ = false;
public:
    CBCAdditiveReactionCmd() : CBodyStateCmd(EBodyStateCmd::AdditiveReaction) {}
    CBCAdditiveReactionCmd(pas::EReactionType type, float f)
    : CBodyStateCmd(EBodyStateCmd::AdditiveReaction), x8_(f), xc_type(type) {}
};

class CBCLoopAttackCmd : public CBodyStateCmd
{
    pas::ELoopAttackType x8_type = pas::ELoopAttackType::Invalid;
    u32 xc_ = 0;
public:
    CBCLoopAttackCmd() : CBodyStateCmd(EBodyStateCmd::LoopAttack) {}
    CBCLoopAttackCmd(pas::ELoopAttackType type)
    : CBodyStateCmd(EBodyStateCmd::LoopAttack), x8_type(type) {}
};

class CBCTauntCmd : public CBodyStateCmd
{
    pas::ETauntType x8_type = pas::ETauntType::Invalid;
public:
    CBCTauntCmd() : CBodyStateCmd(EBodyStateCmd::Taunt) {}
    CBCTauntCmd(pas::ETauntType type)
    : CBodyStateCmd(EBodyStateCmd::Taunt), x8_type(type) {}
};

class CBCLocomotionCmd
{
public:
};

class CBodyStateCmdMgr
{
public:
    enum class ESteeringBlendMode
    {
    };
private:
    zeus::CVector3f x0_;
    zeus::CVector3f xc_;
    zeus::CVector3f x18_;
    zeus::CVector3f x24_;
    u32 x30_ = 0;
    float x34_steeringSpeedMin;
    float x38_steeringSpeedMax;
    rstl::reserved_vector<CBodyStateCmd*, 28> x40_commandTable;
    u32 xb4_deliveredCmdMask = 0;
    CBCGetupCmd xb8_getup;
    CBCStepCmd xc4_step;
    CBodyStateCmd xd4_ = {EBodyStateCmd::Two};
    CBCKnockDownCmd xdc_knockDown;
    CBCKnockBackCmd xf4_knockBack;
    CBCMeleeAttackCmd x10c_meleeAttack;
    CBCProjectileAttackCmd x128_projectileAttack;
    CBCLoopAttackCmd x144_loopAttack;
    CBCLoopReactionCmd x154_loopReaction;
    CBCLoopHitReactionCmd x160_loopHitReaction;
    CBodyStateCmd x16c_ = {EBodyStateCmd::Ten};
    CBodyStateCmd x174_ = {EBodyStateCmd::Eleven};
    CBodyStateCmd x17c_ = {EBodyStateCmd::Twelve};
    CBodyStateCmd x184_ = {EBodyStateCmd::Thirteen};
    CBCGenerateCmd x18c_generate;
    CBCHurledCmd x1ac_hurled;
    CBCJumpCmd x1d0_jump;
    CBCSlideCmd x1f8_slide;
    CBCTauntCmd x210_taunt;
    CBCScriptedCmd x21c_scripted;
    CBCCoverCmd x230_cover;
    CBCWallHangCmd x254_wallHang;
    CBodyStateCmd x260_ = {EBodyStateCmd::TwentyTwo};
    CBodyStateCmd x268_ = {EBodyStateCmd::TwentyThree};
    CBCAdditiveAimCmd x270_additiveAim;
    CBCAdditiveFlinchCmd x278_additiveFlinch;
    CBCAdditiveReactionCmd x284_additiveReaction;
    CBodyStateCmd x298_ = {EBodyStateCmd::TwentySeven};
    void DeliverCmd(EBodyStateCmd cmd) { xb4_deliveredCmdMask |= (1 << int(cmd)); }
public:
    CBodyStateCmdMgr();
    void DeliverCmd(const CBodyStateCmd& cmd)
    {
        *x40_commandTable[int(cmd.GetCommandId())] = cmd;
        DeliverCmd(cmd.GetCommandId());
    }
    void DeliverCmd(const CBCGetupCmd& cmd)
    {
        xb8_getup = cmd;
        DeliverCmd(EBodyStateCmd::Getup);
    }
    void DeliverCmd(const CBCStepCmd& cmd)
    {
        xc4_step = cmd;
        DeliverCmd(EBodyStateCmd::Step);
    }
    void DeliverCmd(const CBCKnockDownCmd& cmd)
    {
        xdc_knockDown = cmd;
        DeliverCmd(EBodyStateCmd::KnockDown);
    }
    void DeliverCmd(const CBCKnockBackCmd& cmd)
    {
        xf4_knockBack = cmd;
        DeliverCmd(EBodyStateCmd::KnockBack);
    }
    void DeliverCmd(const CBCMeleeAttackCmd& cmd)
    {
        x10c_meleeAttack = cmd;
        DeliverCmd(EBodyStateCmd::MeleeAttack);
    }
    void DeliverCmd(const CBCProjectileAttackCmd& cmd)
    {
        x128_projectileAttack = cmd;
        DeliverCmd(EBodyStateCmd::ProjectileAttack);
    }
    void DeliverCmd(const CBCLoopAttackCmd& cmd)
    {
        x144_loopAttack = cmd;
        DeliverCmd(EBodyStateCmd::LoopAttack);
    }
    void DeliverCmd(const CBCLoopReactionCmd& cmd)
    {
        x154_loopReaction = cmd;
        DeliverCmd(EBodyStateCmd::LoopReaction);
    }
    void DeliverCmd(const CBCLoopHitReactionCmd& cmd)
    {
        x160_loopHitReaction = cmd;
        DeliverCmd(EBodyStateCmd::LoopHitReaction);
    }
    void DeliverCmd(const CBCGenerateCmd& cmd)
    {
        x18c_generate = cmd;
        DeliverCmd(EBodyStateCmd::Generate);
    }
    void DeliverCmd(const CBCHurledCmd& cmd)
    {
        x1ac_hurled = cmd;
        DeliverCmd(EBodyStateCmd::Hurled);
    }
    void DeliverCmd(const CBCJumpCmd& cmd)
    {
        x1d0_jump = cmd;
        DeliverCmd(EBodyStateCmd::Jump);
    }
    void DeliverCmd(const CBCSlideCmd& cmd)
    {
        x1f8_slide = cmd;
        DeliverCmd(EBodyStateCmd::Slide);
    }
    void DeliverCmd(const CBCTauntCmd& cmd)
    {
        x210_taunt = cmd;
        DeliverCmd(EBodyStateCmd::Taunt);
    }
    void DeliverCmd(const CBCScriptedCmd& cmd)
    {
        x21c_scripted = cmd;
        DeliverCmd(EBodyStateCmd::Scripted);
    }
    void DeliverCmd(const CBCCoverCmd& cmd)
    {
        x230_cover = cmd;
        DeliverCmd(EBodyStateCmd::Cover);
    }
    void DeliverCmd(const CBCWallHangCmd& cmd)
    {
        x254_wallHang = cmd;
        DeliverCmd(EBodyStateCmd::WallHang);
    }
    void DeliverCmd(const CBCAdditiveAimCmd& cmd)
    {
        x270_additiveAim = cmd;
        DeliverCmd(EBodyStateCmd::AdditiveAim);
    }
    void DeliverCmd(const CBCAdditiveFlinchCmd& cmd)
    {
        x278_additiveFlinch = cmd;
        DeliverCmd(EBodyStateCmd::AdditiveFlinch);
    }
    void DeliverCmd(const CBCAdditiveReactionCmd& cmd)
    {
        x284_additiveReaction = cmd;
        DeliverCmd(EBodyStateCmd::AdditiveReaction);
    }
    void DeliverCmd(const CBCLocomotionCmd& cmd);
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
