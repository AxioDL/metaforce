#pragma once

#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Character/CharacterCommon.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {

class CBodyStateCmd {
  EBodyStateCmd x4_cmd;

public:
  virtual ~CBodyStateCmd() = default;
  explicit CBodyStateCmd(EBodyStateCmd cmd) : x4_cmd(cmd) {}
  EBodyStateCmd GetCommandId() const { return x4_cmd; }
};

class CBCMeleeAttackCmd : public CBodyStateCmd {
  pas::ESeverity x8_severity = pas::ESeverity::Invalid;
  zeus::CVector3f xc_targetPos;
  bool x18_hasTargetPos = false;

public:
  explicit CBCMeleeAttackCmd() : CBodyStateCmd(EBodyStateCmd::MeleeAttack) {}
  explicit CBCMeleeAttackCmd(pas::ESeverity severity)
  : CBodyStateCmd(EBodyStateCmd::MeleeAttack), x8_severity(severity) {}
  explicit CBCMeleeAttackCmd(pas::ESeverity severity, const zeus::CVector3f& target)
  : CBodyStateCmd(EBodyStateCmd::MeleeAttack), x8_severity(severity), xc_targetPos(target), x18_hasTargetPos(true) {}
  pas::ESeverity GetAttackSeverity() const { return x8_severity; }
  bool HasAttackTargetPos() const { return x18_hasTargetPos; }
  const zeus::CVector3f& GetAttackTargetPos() const { return xc_targetPos; }
};

class CBCProjectileAttackCmd : public CBodyStateCmd {
  pas::ESeverity x8_severity = pas::ESeverity::Invalid;
  zeus::CVector3f xc_target;
  bool x18_blendAnims = false;

public:
  explicit CBCProjectileAttackCmd() : CBodyStateCmd(EBodyStateCmd::ProjectileAttack) {}
  explicit CBCProjectileAttackCmd(pas::ESeverity severity, const zeus::CVector3f& vec, bool b)
  : CBodyStateCmd(EBodyStateCmd::ProjectileAttack), x8_severity(severity), xc_target(vec), x18_blendAnims(b) {}
  pas::ESeverity GetAttackSeverity() const { return x8_severity; }
  const zeus::CVector3f& GetTargetPosition() const { return xc_target; }
  bool BlendTwoClosest() const { return x18_blendAnims; }
};

class CBCStepCmd : public CBodyStateCmd {
  pas::EStepDirection x8_dir = pas::EStepDirection::Invalid;
  pas::EStepType xc_type = pas::EStepType::Normal;

public:
  explicit CBCStepCmd() : CBodyStateCmd(EBodyStateCmd::Step) {}
  explicit CBCStepCmd(pas::EStepDirection dir, pas::EStepType type)
  : CBodyStateCmd(EBodyStateCmd::Step), x8_dir(dir), xc_type(type) {}
  pas::EStepDirection GetStepDirection() const { return x8_dir; }
  pas::EStepType GetStepType() const { return xc_type; }
};

class CBCJumpCmd : public CBodyStateCmd {
  pas::EJumpType x8_type = pas::EJumpType::Normal;
  zeus::CVector3f xc_waypoint1;
  zeus::CVector3f x18_waypoint2;
  bool x24_24_wallJump : 1 = false;
  bool x24_25_startInJumpLoop : 1 = false;

public:
  explicit CBCJumpCmd() : CBodyStateCmd(EBodyStateCmd::Jump) {}
  explicit CBCJumpCmd(const zeus::CVector3f& wp1, pas::EJumpType type, bool startInLoop = false)
  : CBodyStateCmd(EBodyStateCmd::Jump), x8_type(type), xc_waypoint1(wp1), x24_25_startInJumpLoop{startInLoop} {}
  explicit CBCJumpCmd(const zeus::CVector3f& wp1, const zeus::CVector3f& wp2, pas::EJumpType type)
  : CBodyStateCmd(EBodyStateCmd::Jump), x8_type(type), xc_waypoint1(wp1), x18_waypoint2(wp2), x24_24_wallJump{true} {}
  pas::EJumpType GetJumpType() const { return x8_type; }
  const zeus::CVector3f& GetJumpTarget() const { return xc_waypoint1; }
  const zeus::CVector3f& GetSecondJumpTarget() const { return x18_waypoint2; }
  bool IsWallJump() const { return x24_24_wallJump; }
  bool StartInJumpLoop() const { return x24_25_startInJumpLoop; }
};

class CBCGenerateCmd : public CBodyStateCmd {
  pas::EGenerateType x8_type = pas::EGenerateType::Invalid;
  zeus::CVector3f xc_targetPos;
  s32 x18_animId = -1;
  bool x1c_24_targetTransform : 1 = false;
  bool x1c_25_overrideAnim : 1 = false;

public:
  explicit CBCGenerateCmd() : CBodyStateCmd(EBodyStateCmd::Generate) {}
  explicit CBCGenerateCmd(pas::EGenerateType type)
  : CBodyStateCmd(EBodyStateCmd::Generate), x8_type(type) {}
  explicit CBCGenerateCmd(pas::EGenerateType type, s32 animId)
  : CBodyStateCmd(EBodyStateCmd::Generate), x8_type(type), x18_animId(animId), x1c_25_overrideAnim{animId != -1} {}
  explicit CBCGenerateCmd(pas::EGenerateType type, const zeus::CVector3f& vec, bool targetTransform = false,
                          bool overrideAnim = false)
  : CBodyStateCmd(EBodyStateCmd::Generate)
  , x8_type(type)
  , xc_targetPos(vec)
  , x1c_24_targetTransform{targetTransform}
  , x1c_25_overrideAnim{overrideAnim} {}
  pas::EGenerateType GetGenerateType() const { return x8_type; }
  const zeus::CVector3f& GetExitTargetPos() const { return xc_targetPos; }
  bool HasExitTargetPos() const { return x1c_24_targetTransform; }
  s32 GetSpecialAnimId() const { return x18_animId; }
  bool UseSpecialAnimId() const { return x1c_25_overrideAnim; }
};

class CBCKnockBackCmd : public CBodyStateCmd {
  zeus::CVector3f x8_dir;
  pas::ESeverity x14_severity = pas::ESeverity::Invalid;

public:
  explicit CBCKnockBackCmd() : CBodyStateCmd(EBodyStateCmd::KnockBack) {}
  explicit CBCKnockBackCmd(const zeus::CVector3f& vec, pas::ESeverity severity)
  : CBodyStateCmd(EBodyStateCmd::KnockBack), x8_dir(vec), x14_severity(severity) {}
  const zeus::CVector3f& GetHitDirection() const { return x8_dir; }
  pas::ESeverity GetHitSeverity() const { return x14_severity; }
};

class CBCHurledCmd : public CBodyStateCmd {
  zeus::CVector3f x8_direction;
  zeus::CVector3f x14_launchVel;
  bool x20_startInKnockLoop = false;

public:
  explicit CBCHurledCmd() : CBodyStateCmd(EBodyStateCmd::Hurled) {}
  explicit CBCHurledCmd(const zeus::CVector3f& dir, const zeus::CVector3f& launchVel, bool startInLoop = false)
  : CBodyStateCmd(EBodyStateCmd::Hurled)
  , x8_direction(dir)
  , x14_launchVel(launchVel)
  , x20_startInKnockLoop(startInLoop) {}
  const zeus::CVector3f& GetHitDirection() const { return x8_direction; }
  const zeus::CVector3f& GetLaunchVelocity() const { return x14_launchVel; }
  bool GetSkipLaunchState() const { return x20_startInKnockLoop; }
  void SetSkipLaunchState(bool s) { x20_startInKnockLoop = s; }
};

class CBCGetupCmd : public CBodyStateCmd {
  pas::EGetupType x8_type = pas::EGetupType::Invalid;

public:
  explicit CBCGetupCmd() : CBodyStateCmd(EBodyStateCmd::Getup) {}
  explicit CBCGetupCmd(pas::EGetupType type) : CBodyStateCmd(EBodyStateCmd::Getup), x8_type(type) {}
  pas::EGetupType GetGetupType() const { return x8_type; }
};

class CBCLoopReactionCmd : public CBodyStateCmd {
  pas::EReactionType x8_type = pas::EReactionType::Invalid;

public:
  explicit CBCLoopReactionCmd() : CBodyStateCmd(EBodyStateCmd::LoopReaction) {}
  explicit CBCLoopReactionCmd(pas::EReactionType type) : CBodyStateCmd(EBodyStateCmd::LoopReaction), x8_type(type) {}
  pas::EReactionType GetReactionType() const { return x8_type; }
};

class CBCLoopHitReactionCmd : public CBodyStateCmd {
  pas::EReactionType x8_type = pas::EReactionType::Invalid;

public:
  explicit CBCLoopHitReactionCmd() : CBodyStateCmd(EBodyStateCmd::LoopHitReaction) {}
  explicit CBCLoopHitReactionCmd(pas::EReactionType type) : CBodyStateCmd(EBodyStateCmd::LoopHitReaction), x8_type(type) {}
  pas::EReactionType GetReactionType() const { return x8_type; }
};

class CBCKnockDownCmd : public CBodyStateCmd {
  zeus::CVector3f x8_dir;
  pas::ESeverity x14_severity = pas::ESeverity::Invalid;

public:
  explicit CBCKnockDownCmd() : CBodyStateCmd(EBodyStateCmd::KnockDown) {}
  explicit CBCKnockDownCmd(const zeus::CVector3f& vec, pas::ESeverity severity)
  : CBodyStateCmd(EBodyStateCmd::KnockDown), x8_dir(vec), x14_severity(severity) {}
  const zeus::CVector3f& GetHitDirection() const { return x8_dir; }
  pas::ESeverity GetHitSeverity() const { return x14_severity; }
};

class CBCSlideCmd : public CBodyStateCmd {
  pas::ESlideType x8_type = pas::ESlideType::Invalid;
  zeus::CVector3f xc_dir;

public:
  explicit CBCSlideCmd() : CBodyStateCmd(EBodyStateCmd::Slide) {}
  explicit CBCSlideCmd(pas::ESlideType type, const zeus::CVector3f& dir)
  : CBodyStateCmd(EBodyStateCmd::Slide), x8_type(type), xc_dir(dir) {}
  pas::ESlideType GetSlideType() const { return x8_type; }
  const zeus::CVector3f& GetSlideDirection() const { return xc_dir; }
};

class CBCScriptedCmd : public CBodyStateCmd {
  s32 x8_anim = -1;
  bool xc_24_loopAnim : 1 = false;
  bool xc_25_timedLoop : 1 = false;
  float x10_loopDur = 0.f;

public:
  explicit CBCScriptedCmd() : CBodyStateCmd(EBodyStateCmd::Scripted) {}
  explicit CBCScriptedCmd(int i, bool b1, bool b2, float f)
  : CBodyStateCmd(EBodyStateCmd::Scripted), x8_anim(i), xc_24_loopAnim{b1}, xc_25_timedLoop{b2}, x10_loopDur(f) {}
  s32 GetAnimId() const { return x8_anim; }
  bool IsLooped() const { return xc_24_loopAnim; }
  bool GetUseLoopDuration() const { return xc_25_timedLoop; }
  float GetLoopDuration() const { return x10_loopDur; }
};

class CBCCoverCmd : public CBodyStateCmd {
  pas::ECoverDirection x8_dir = pas::ECoverDirection::Invalid;
  zeus::CVector3f xc_targetPos;
  zeus::CVector3f x18_alignDir;

public:
  explicit CBCCoverCmd() : CBodyStateCmd(EBodyStateCmd::Cover) {}
  explicit CBCCoverCmd(pas::ECoverDirection dir, const zeus::CVector3f& v1, const zeus::CVector3f& v2)
  : CBodyStateCmd(EBodyStateCmd::Cover), x8_dir(dir), xc_targetPos(v1), x18_alignDir(v2) {}
  pas::ECoverDirection GetDirection() const { return x8_dir; }
  const zeus::CVector3f& GetTarget() const { return xc_targetPos; }
  const zeus::CVector3f& GetAlignDirection() const { return x18_alignDir; }
};

class CBCWallHangCmd : public CBodyStateCmd {
  TUniqueId x8_wpId = kInvalidUniqueId;

public:
  explicit CBCWallHangCmd() : CBodyStateCmd(EBodyStateCmd::WallHang) {}
  explicit CBCWallHangCmd(TUniqueId uid) : CBodyStateCmd(EBodyStateCmd::WallHang), x8_wpId(uid) {}
  TUniqueId GetTarget() const { return x8_wpId; }
};

class CBCAdditiveAimCmd : public CBodyStateCmd {
public:
  explicit CBCAdditiveAimCmd() : CBodyStateCmd(EBodyStateCmd::AdditiveAim) {}
};

class CBCAdditiveFlinchCmd : public CBodyStateCmd {
  float x8_weight = 1.f;

public:
  explicit CBCAdditiveFlinchCmd() : CBodyStateCmd(EBodyStateCmd::AdditiveFlinch) {}
  explicit CBCAdditiveFlinchCmd(float f) : CBodyStateCmd(EBodyStateCmd::AdditiveFlinch), x8_weight(f) {}
  float GetWeight() const { return x8_weight; }
};

class CBCAdditiveReactionCmd : public CBodyStateCmd {
  float x8_weight = 1.f;
  pas::EAdditiveReactionType xc_type = pas::EAdditiveReactionType::Invalid;
  bool x10_active = false;

public:
  explicit CBCAdditiveReactionCmd() : CBodyStateCmd(EBodyStateCmd::AdditiveReaction) {}
  explicit CBCAdditiveReactionCmd(pas::EAdditiveReactionType type, float weight, bool active)
  : CBodyStateCmd(EBodyStateCmd::AdditiveReaction), x8_weight(weight), xc_type(type), x10_active(active) {}
  pas::EAdditiveReactionType GetType() const { return xc_type; }
  float GetWeight() const { return x8_weight; }
  bool GetIsActive() const { return x10_active; }
};

class CBCLoopAttackCmd : public CBodyStateCmd {
  pas::ELoopAttackType x8_type = pas::ELoopAttackType::Invalid;
  u32 xc_waitForAnimOver = 0;

public:
  explicit CBCLoopAttackCmd() : CBodyStateCmd(EBodyStateCmd::LoopAttack) {}
  explicit CBCLoopAttackCmd(pas::ELoopAttackType type) : CBodyStateCmd(EBodyStateCmd::LoopAttack), x8_type(type) {}
  pas::ELoopAttackType GetAttackType() const { return x8_type; }
  bool WaitForAnimOver() const { return xc_waitForAnimOver == 1; }
};

class CBCTauntCmd : public CBodyStateCmd {
  pas::ETauntType x8_type = pas::ETauntType::Invalid;

public:
  explicit CBCTauntCmd() : CBodyStateCmd(EBodyStateCmd::Taunt) {}
  explicit CBCTauntCmd(pas::ETauntType type) : CBodyStateCmd(EBodyStateCmd::Taunt), x8_type(type) {}
  pas::ETauntType GetTauntType() const { return x8_type; }
};

class CBCLocomotionCmd {
  zeus::CVector3f x0_move;
  zeus::CVector3f xc_face;
  float x18_weight;

public:
  explicit CBCLocomotionCmd(const zeus::CVector3f& move, const zeus::CVector3f& face, float weight)
  : x0_move(move), xc_face(face), x18_weight(weight) {}
  const zeus::CVector3f& GetMoveVector() const { return x0_move; }
  const zeus::CVector3f& GetFaceVector() const { return xc_face; }
  float GetWeight() const { return x18_weight; }
};

enum class ESteeringBlendMode { Normal, FullSpeed, Clamped };

class CBodyStateCmdMgr {
  zeus::CVector3f x0_move;
  zeus::CVector3f xc_face;
  zeus::CVector3f x18_target;
  zeus::CVector3f x24_additiveTarget;
  ESteeringBlendMode x30_steeringMode = ESteeringBlendMode::Normal;
  float x34_steeringSpeedMin = 0.f;
  float x38_steeringSpeedMax = 1.f;
  float x3c_steeringSpeed = 0.f;
  rstl::reserved_vector<CBodyStateCmd*, 28> x40_commandTable;
  u32 xb4_deliveredCmdMask = 0;
  CBCGetupCmd xb8_getup;
  CBCStepCmd xc4_step;
  CBodyStateCmd xd4_die{EBodyStateCmd::Die};
  CBCKnockDownCmd xdc_knockDown;
  CBCKnockBackCmd xf4_knockBack;
  CBCMeleeAttackCmd x10c_meleeAttack;
  CBCProjectileAttackCmd x128_projectileAttack;
  CBCLoopAttackCmd x144_loopAttack;
  CBCLoopReactionCmd x154_loopReaction;
  CBCLoopHitReactionCmd x160_loopHitReaction;
  CBodyStateCmd x16c_exitState{EBodyStateCmd::ExitState};
  CBodyStateCmd x174_leanFromCover{EBodyStateCmd::LeanFromCover};
  CBodyStateCmd x17c_nextState{EBodyStateCmd::NextState};
  CBodyStateCmd x184_maintainVelocity{EBodyStateCmd::MaintainVelocity};
  CBCGenerateCmd x18c_generate;
  CBCHurledCmd x1ac_hurled;
  CBCJumpCmd x1d0_jump;
  CBCSlideCmd x1f8_slide;
  CBCTauntCmd x210_taunt;
  CBCScriptedCmd x21c_scripted;
  CBCCoverCmd x230_cover;
  CBCWallHangCmd x254_wallHang;
  CBodyStateCmd x260_locomotion{EBodyStateCmd::Locomotion};
  CBodyStateCmd x268_additiveIdle{EBodyStateCmd::AdditiveIdle};
  CBCAdditiveAimCmd x270_additiveAim;
  CBCAdditiveFlinchCmd x278_additiveFlinch;
  CBCAdditiveReactionCmd x284_additiveReaction;
  CBodyStateCmd x298_stopReaction{EBodyStateCmd::StopReaction};
  void DeliverCmd(EBodyStateCmd cmd) { xb4_deliveredCmdMask |= (1 << int(cmd)); }

public:
  CBodyStateCmdMgr();
  void DeliverCmd(const CBodyStateCmd& cmd) {
    *x40_commandTable[int(cmd.GetCommandId())] = cmd;
    DeliverCmd(cmd.GetCommandId());
  }
  void DeliverCmd(const CBCGetupCmd& cmd) {
    xb8_getup = cmd;
    DeliverCmd(EBodyStateCmd::Getup);
  }
  void DeliverCmd(const CBCStepCmd& cmd) {
    xc4_step = cmd;
    DeliverCmd(EBodyStateCmd::Step);
  }
  void DeliverCmd(const CBCKnockDownCmd& cmd) {
    xdc_knockDown = cmd;
    DeliverCmd(EBodyStateCmd::KnockDown);
  }
  void DeliverCmd(const CBCKnockBackCmd& cmd) {
    xf4_knockBack = cmd;
    DeliverCmd(EBodyStateCmd::KnockBack);
  }
  void DeliverCmd(const CBCMeleeAttackCmd& cmd) {
    x10c_meleeAttack = cmd;
    DeliverCmd(EBodyStateCmd::MeleeAttack);
  }
  void DeliverCmd(const CBCProjectileAttackCmd& cmd) {
    x128_projectileAttack = cmd;
    DeliverCmd(EBodyStateCmd::ProjectileAttack);
  }
  void DeliverCmd(const CBCLoopAttackCmd& cmd) {
    x144_loopAttack = cmd;
    DeliverCmd(EBodyStateCmd::LoopAttack);
  }
  void DeliverCmd(const CBCLoopReactionCmd& cmd) {
    x154_loopReaction = cmd;
    DeliverCmd(EBodyStateCmd::LoopReaction);
  }
  void DeliverCmd(const CBCLoopHitReactionCmd& cmd) {
    x160_loopHitReaction = cmd;
    DeliverCmd(EBodyStateCmd::LoopHitReaction);
  }
  void DeliverCmd(const CBCGenerateCmd& cmd) {
    x18c_generate = cmd;
    DeliverCmd(EBodyStateCmd::Generate);
  }
  void DeliverCmd(const CBCHurledCmd& cmd) {
    x1ac_hurled = cmd;
    DeliverCmd(EBodyStateCmd::Hurled);
  }
  void DeliverCmd(const CBCJumpCmd& cmd) {
    x1d0_jump = cmd;
    DeliverCmd(EBodyStateCmd::Jump);
  }
  void DeliverCmd(const CBCSlideCmd& cmd) {
    x1f8_slide = cmd;
    DeliverCmd(EBodyStateCmd::Slide);
  }
  void DeliverCmd(const CBCTauntCmd& cmd) {
    x210_taunt = cmd;
    DeliverCmd(EBodyStateCmd::Taunt);
  }
  void DeliverCmd(const CBCScriptedCmd& cmd) {
    x21c_scripted = cmd;
    DeliverCmd(EBodyStateCmd::Scripted);
  }
  void DeliverCmd(const CBCCoverCmd& cmd) {
    x230_cover = cmd;
    DeliverCmd(EBodyStateCmd::Cover);
  }
  void DeliverCmd(const CBCWallHangCmd& cmd) {
    x254_wallHang = cmd;
    DeliverCmd(EBodyStateCmd::WallHang);
  }
  void DeliverCmd(const CBCAdditiveAimCmd& cmd) {
    x270_additiveAim = cmd;
    DeliverCmd(EBodyStateCmd::AdditiveAim);
  }
  void DeliverCmd(const CBCAdditiveFlinchCmd& cmd) {
    x278_additiveFlinch = cmd;
    DeliverCmd(EBodyStateCmd::AdditiveFlinch);
  }
  void DeliverCmd(const CBCAdditiveReactionCmd& cmd) {
    x284_additiveReaction = cmd;
    DeliverCmd(EBodyStateCmd::AdditiveReaction);
  }
  void DeliverCmd(const CBCLocomotionCmd& cmd);
  void DeliverFaceVector(const zeus::CVector3f& f) { xc_face = f; }
  void DeliverTargetVector(const zeus::CVector3f& t) { x18_target = t; }
  void DeliverAdditiveTargetVector(const zeus::CVector3f& t) { x24_additiveTarget = t; }
  void SetSteeringBlendSpeed(float s) { x3c_steeringSpeed = s; }
  void SetSteeringBlendMode(ESteeringBlendMode m) { x30_steeringMode = m; }
  void SetSteeringSpeedRange(float rmin, float rmax) {
    x34_steeringSpeedMin = rmin;
    x38_steeringSpeedMax = rmax;
  }
  void BlendSteeringCmds();
  void Reset();
  void ClearLocomotionCmds();

  CBodyStateCmd* GetCmd(EBodyStateCmd cmd) {
    if ((xb4_deliveredCmdMask & (1U << u32(cmd))) != 0) {
      return x40_commandTable[size_t(cmd)];
    }
    return nullptr;
  }
  const CBodyStateCmd* GetCmd(EBodyStateCmd cmd) const {
    if ((xb4_deliveredCmdMask & (1U << u32(cmd))) != 0) {
      return x40_commandTable[size_t(cmd)];
    }
    return nullptr;
  }

  const zeus::CVector3f& GetMoveVector() const { return x0_move; }
  const zeus::CVector3f& GetFaceVector() const { return xc_face; }
  const zeus::CVector3f& GetTargetVector() const { return x18_target; }
  const zeus::CVector3f& GetAdditiveTargetVector() const { return x24_additiveTarget; }
};

} // namespace urde
