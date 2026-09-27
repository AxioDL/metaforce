#ifndef _CBODYSTATECMDMGR
#define _CBODYSTATECMDMGR

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Animation/CharacterCommon.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/reserved_vector.hpp"

enum ESteeringBlendMode {
  kSBM_Normal,
  kSBM_FullSpeed,
  kSBM_Clamped,
};

class CBodyStateCmd {
public:
  CBodyStateCmd(EBodyStateCmd cmd) : mCmd(cmd) {}
  virtual ~CBodyStateCmd() {};

  EBodyStateCmd GetCommandId() const { return mCmd; }

private:
  EBodyStateCmd mCmd;
};

class CBCAdditiveFlinchCmd : public CBodyStateCmd {
public:
  CBCAdditiveFlinchCmd(float weight) : CBodyStateCmd(kBSC_AdditiveFlinch), mWeight(weight) {}

  float GetWeight() const { return mWeight; }

private:
  float mWeight;
};

class CBCGenerateCmd : public CBodyStateCmd {
public:
  CBCGenerateCmd(pas::EGenerateType type, int animId)
  : CBodyStateCmd(kBSC_Generate)
  , mType(type)
  , mTargetPos(CVector3f::Zero())
  , mAnimId(animId)
  , mTargetTransform(false)
  , mOverrideAnim(animId != -1) {}

  CBCGenerateCmd(const pas::EGenerateType type, const CVector3f& vec,
                 const bool targetTransform = false, const bool overrideAnim = false)
  : CBodyStateCmd(kBSC_Generate)
  , mType(type)
  , mTargetPos(vec)
  , mAnimId(-1)
  , mTargetTransform(targetTransform)
  , mOverrideAnim(overrideAnim) {}

  pas::EGenerateType GetGenerateType() const { return mType; }
  const CVector3f& GetExitTargetPos() const { return mTargetPos; }
  int GetSpecialAnimId() const { return mAnimId; }
  bool HasExitTargetPos() const { return mTargetTransform; }
  bool UseSpecialAnimId() const { return mOverrideAnim; }

private:
  pas::EGenerateType mType;
  CVector3f mTargetPos;
  int mAnimId;
  uint mTargetTransform : 1;
  uint mOverrideAnim : 1;
};

//

class CBCScriptedCmd : public CBodyStateCmd {
public:
  CBCScriptedCmd(int animId, bool isLooped, bool useLoopDuration, float loopDuration)
  : CBodyStateCmd(kBSC_Scripted)
  , mAnimId(animId)
  , mIsLooped(isLooped)
  , mUseLoopDuration(useLoopDuration)
  , mLoopDuration(loopDuration) {}

  int GetAnimId() const { return mAnimId; }
  bool IsLooped() const { return mIsLooped; }
  bool GetUseLoopDuration() const { return mUseLoopDuration; }
  float GetLoopDuration() const { return mLoopDuration; }

private:
  int mAnimId;
  bool mIsLooped : 1;
  bool mUseLoopDuration : 1;
  float mLoopDuration;
};

//

class CBCStepCmd : public CBodyStateCmd {
public:
  CBCStepCmd(pas::EStepDirection dir, pas::EStepType type)
  : CBodyStateCmd(kBSC_Step), mDir(dir), mType(type) {}

  pas::EStepDirection GetStepDirection() const { return mDir; }
  pas::EStepType GetStepType() const { return mType; }

private:
  pas::EStepDirection mDir;
  pas::EStepType mType;
};

//

class CBCTauntCmd : public CBodyStateCmd {
public:
  CBCTauntCmd(pas::ETauntType type) : CBodyStateCmd(kBSC_Taunt), mType(type) {}

  pas::ETauntType GetTauntType() const { return mType; }

private:
  pas::ETauntType mType;
};

//

class CBCGetupCmd : public CBodyStateCmd {
public:
  CBCGetupCmd(pas::EGetupType type) : CBodyStateCmd(kBSC_Getup), mType(type) {}

  pas::EGetupType GetGetupType() const { return mType; }

private:
  pas::EGetupType mType;
};

//

class CBCAdditiveReactionCmd : public CBodyStateCmd {
public:
  CBCAdditiveReactionCmd(pas::EAdditiveReactionType type, float weight, bool active)
  : CBodyStateCmd(kBSC_AdditiveReaction), mWeight(weight), mType(type), mActive(active) {}

  pas::EAdditiveReactionType GetType() const { return mType; }
  float GetWeight() const { return mWeight; }
  bool GetIsActive() const { return mActive; }

private:
  float mWeight;
  pas::EAdditiveReactionType mType;
  bool mActive;
};

//

class CBCSlideCmd : public CBodyStateCmd {
public:
  CBCSlideCmd(pas::ESlideType type, CVector3f dir)
  : CBodyStateCmd(kBSC_Slide), mType(type), mDir(dir) {}

  ~CBCSlideCmd() override {}

  pas::ESlideType GetSlideType() const { return mType; }
  const CVector3f& GetSlideDirection() const { return mDir; }

private:
  pas::ESlideType mType;
  CVector3f mDir;
};

//

class CBCKnockDownCmd : public CBodyStateCmd {
public:
  CBCKnockDownCmd(const CVector3f& vec, pas::ESeverity severity)
  : CBodyStateCmd(kBSC_KnockDown), mDir(vec), mSeverity(severity) {}

  const CVector3f& GetHitDirection() const { return mDir; }
  pas::ESeverity GetHitSeverity() const { return mSeverity; }

private:
  CVector3f mDir;
  pas::ESeverity mSeverity;
};

//

class CBCKnockBackCmd : public CBodyStateCmd {
public:
  CBCKnockBackCmd(const CVector3f& vec, const pas::ESeverity severity)
  : CBodyStateCmd(kBSC_KnockBack), mDir(vec), mSeverity(severity) {}

  const CVector3f& GetHitDirection() const { return mDir; }
  pas::ESeverity GetHitSeverity() const { return mSeverity; }

private:
  CVector3f mDir;
  pas::ESeverity mSeverity;
};

//

class CBCMeleeAttackCmd : public CBodyStateCmd {
public:
  CBCMeleeAttackCmd(pas::ESeverity severity)
  : CBodyStateCmd(kBSC_MeleeAttack)
  , mSeverity(severity)
  , mTargetPos(CVector3f::Zero())
  , mHasTargetPos(false) {}

  CBCMeleeAttackCmd(pas::ESeverity severity, const CVector3f& target)
  : CBodyStateCmd(kBSC_MeleeAttack)
  , mSeverity(severity)
  , mTargetPos(target)
  , mHasTargetPos(true) {}

  pas::ESeverity GetAttackSeverity() const { return mSeverity; }
  bool HasAttackTargetPos() const { return mHasTargetPos; }
  const CVector3f& GetAttackTargetPos() const { return mTargetPos; }

private:
  pas::ESeverity mSeverity;
  CVector3f mTargetPos;
  bool mHasTargetPos;
};

//

class CBCProjectileAttackCmd : public CBodyStateCmd {
public:
  CBCProjectileAttackCmd(pas::ESeverity severity, const CVector3f& vec, bool b)
  : CBodyStateCmd(kBSC_ProjectileAttack)
  , mSeverity(severity)
  , mTarget(vec)
  , mBlendAnims(b) {}

  pas::ESeverity GetAttackSeverity() const { return mSeverity; }
  const CVector3f& GetTargetPosition() const { return mTarget; }
  bool BlendTwoClosest() const { return mBlendAnims; }

private:
  pas::ESeverity mSeverity;
  CVector3f mTarget;
  bool mBlendAnims;
};

//

class CBCLoopAttackCmd : public CBodyStateCmd {
public:
  CBCLoopAttackCmd(pas::ELoopAttackType type, bool waitForAnimOver = false)
  : CBodyStateCmd(kBSC_LoopAttack), mType(type), mWaitForAnimOver(waitForAnimOver) {}

  pas::ELoopAttackType GetAttackType() const { return mType; }
  int WaitForAnimOver() const { return mWaitForAnimOver; }

private:
  pas::ELoopAttackType mType;
  int mWaitForAnimOver;
};

//

class CBCLoopReactionCmd : public CBodyStateCmd {
public:
  CBCLoopReactionCmd(pas::EReactionType type) : CBodyStateCmd(kBSC_LoopReaction), mType(type) {}

  pas::EReactionType GetReactionType() const { return mType; }

private:
  pas::EReactionType mType;
};

//

class CBCLoopHitReactionCmd : public CBodyStateCmd {
public:
  CBCLoopHitReactionCmd(pas::EReactionType type)
  : CBodyStateCmd(kBSC_LoopHitReaction), mType(type) {}

  pas::EReactionType GetReactionType() const { return mType; }

private:
  pas::EReactionType mType;
};

//

class CBCHurledCmd : public CBodyStateCmd {
public:
  CBCHurledCmd(const CVector3f& dir, const CVector3f& launchVel, bool startInLoop = false)
  : CBodyStateCmd(kBSC_Hurled)
  , mDirection(dir)
  , mLaunchVel(launchVel)
  , mStartInKnockLoop(startInLoop) {}

  const CVector3f& GetHitDirection() const { return mDirection; }
  const CVector3f& GetLaunchVelocity() const { return mLaunchVel; }
  bool GetSkipLaunchState() const { return mStartInKnockLoop; }
  void SetSkipLaunchState(bool s) { mStartInKnockLoop = s; }

private:
  CVector3f mDirection;
  CVector3f mLaunchVel;
  bool mStartInKnockLoop;
};

//

class CBCJumpCmd : public CBodyStateCmd {
public:
  CBCJumpCmd(const CVector3f& wp1, pas::EJumpType type, bool startInLoop = false)
  : CBodyStateCmd(kBSC_Jump)
  , mType(type)
  , mWaypoint1(wp1)
  , mWaypoint2(CVector3f::Zero())
  , mWallJump(false)
  , mStartInJumpLoop(startInLoop) {}

  CBCJumpCmd(const CVector3f& wp1, const CVector3f& wp2, pas::EJumpType type)
  : CBodyStateCmd(kBSC_Jump)
  , mType(type)
  , mWaypoint1(wp1)
  , mWaypoint2(wp2)
  , mWallJump(true)
  , mStartInJumpLoop(false) {}

  pas::EJumpType GetJumpType() const { return mType; }
  const CVector3f& GetJumpTarget() const { return mWaypoint1; }
  const CVector3f& GetSecondJumpTarget() const { return mWaypoint2; }
  bool IsWallJump() const { return mWallJump; }
  bool StartInJumpLoop() const { return mStartInJumpLoop; }

private:
  pas::EJumpType mType;
  CVector3f mWaypoint1;
  CVector3f mWaypoint2;
  bool mWallJump : 1;
  bool mStartInJumpLoop : 1;
};

//

class CBCCoverCmd : public CBodyStateCmd {
public:
  CBCCoverCmd(pas::ECoverDirection dir, const CVector3f& v1, const CVector3f& v2)
  : CBodyStateCmd(kBSC_Cover)
  , mDir(dir)
  , mTargetPos(v1)
  , mAlignDir(v2) {}

  pas::ECoverDirection GetDirection() const { return mDir; }
  const CVector3f& GetTarget() const { return mTargetPos; }
  const CUnitVector3f GetAlignDirection() const {
    return CUnitVector3f(mAlignDir, CUnitVector3f::kN_No);
  }

private:
  pas::ECoverDirection mDir;
  CVector3f mTargetPos;
  CVector3f mAlignDir;
};
CHECK_SIZEOF(CBCCoverCmd, 0x24)

//

class CBCWallHangCmd : public CBodyStateCmd {
public:
  CBCWallHangCmd(TUniqueId uid) : CBodyStateCmd(kBSC_WallHang), mWpId(uid) {}

  TUniqueId GetTarget() const { return mWpId; }

private:
  TUniqueId mWpId;
};

//

class CBCAdditiveAimCmd : public CBodyStateCmd {
public:
  CBCAdditiveAimCmd() : CBodyStateCmd(kBSC_AdditiveAim) {}
};

//

class CBCLocomotionCmd {
public:
  CBCLocomotionCmd(const CVector3f& move, const CVector3f& face, float weight)
  : mMove(move), mFace(face), mWeight(weight) {}

  const CVector3f& GetMoveVector() const { return mMove; }
  const CVector3f& GetFaceVector() const { return mFace; }
  float GetWeight() const { return mWeight; }

private:
  CVector3f mMove;
  CVector3f mFace;
  float mWeight;
};

//

class CBodyStateCmdMgr {
public:
  CBodyStateCmdMgr();
  ~CBodyStateCmdMgr();
  void ClearLocomotionCmds();
  void DeliverCmd(const CBCLocomotionCmd& cmd);
  void DeliverCmd(EBodyStateCmd cmd);

  void DeliverCmd(const CBodyStateCmd& cmd);

  void DeliverCmd(const CBCGetupCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mGetup = cmd;
  }

  void DeliverCmd(const CBCStepCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mStep = cmd;
  }

  void DeliverCmd(const CBCKnockDownCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mKnockDown = cmd;
  }

  void DeliverCmd(const CBCKnockBackCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mKnockBack = cmd;
  }

  void DeliverCmd(const CBCMeleeAttackCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mMeleeAttack = cmd;
  }

  void DeliverCmd(const CBCProjectileAttackCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mProjectileAttack = cmd;
  }

  void DeliverCmd(const CBCLoopAttackCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mLoopAttack = cmd;
  }

  void DeliverCmd(const CBCLoopReactionCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mLoopReaction = cmd;
  }

  void DeliverCmd(const CBCGenerateCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mGenerate = cmd;
  }

  void DeliverCmd(const CBCHurledCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mHurled = cmd;
  }

  void DeliverCmd(const CBCJumpCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mJump = cmd;
  }

  void DeliverCmd(const CBCSlideCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mSlide = cmd;
  }

  void DeliverCmd(const CBCTauntCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mTaunt = cmd;
  }

  void DeliverCmd(const CBCScriptedCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mScripted = cmd;
  }

  void DeliverCmd(const CBCWallHangCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mWallHang = cmd;
  }

  void DeliverCmd(const CBCCoverCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mCover = cmd;
  }

  void DeliverCmd(const CBCAdditiveReactionCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mAdditiveReaction = cmd;
  }
  void DeliverCmd(const CBCAdditiveAimCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mAdditiveAim = cmd;
  }

  void DeliverCmd(const CBCAdditiveFlinchCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mAdditiveFlinch = cmd;
  }

  void DeliverCmd(const CBCLoopHitReactionCmd& cmd) {
    DeliverCmd(cmd.GetCommandId());
    mLoopHitReaction = cmd;
  }

  void DeliverTargetVector(const CVector3f& t) { mTarget = t; }
  void DeliverAdditiveTargetVector(const CVector3f& t) { mAdditiveTarget = t; }
  void BlendSteeringCmds();
  void SetSteeringBlendMode(ESteeringBlendMode mode) { mSteeringMode = mode; }
  void SetSteeringSpeedRange(float rmin, float rmax);
  void Reset();
  CBodyStateCmd* GetCmd(EBodyStateCmd cmd);
  const CBodyStateCmd* GetCmd(EBodyStateCmd cmd) const;
  const CVector3f& GetMoveVector() const { return mMove; }
  CVector3f& MoveVector() { return mMove; }
  const CVector3f& GetFaceVector() const { return mFace; }
  const CVector3f& GetTargetVector() const { return mTarget; }
  const CVector3f& GetAdditiveTargetVector() const { return mAdditiveTarget; }

private:
  CVector3f mMove;
  CVector3f mFace;
  CVector3f mTarget;
  CVector3f mAdditiveTarget;
  ESteeringBlendMode mSteeringMode;
  float mSteeringSpeedMin;
  float mSteeringSpeedMax;
  float mSteeringSpeed;
  rstl::reserved_vector< CBodyStateCmd*, 28 > mCommandTable;
  uint mDeliveredCmdMask;
  CBCGetupCmd mGetup;
  CBCStepCmd mStep;
  CBodyStateCmd mDie;
  CBCKnockDownCmd mKnockDown;
  CBCKnockBackCmd mKnockBack;
  CBCMeleeAttackCmd mMeleeAttack;
  CBCProjectileAttackCmd mProjectileAttack;
  CBCLoopAttackCmd mLoopAttack;
  CBCLoopReactionCmd mLoopReaction;
  CBCLoopHitReactionCmd mLoopHitReaction;
  CBodyStateCmd mExitState;
  CBodyStateCmd mLeanFromCover;
  CBodyStateCmd mNextState;
  CBodyStateCmd mMaintainVelocity;
  CBCGenerateCmd mGenerate;
  CBCHurledCmd mHurled;
  CBCJumpCmd mJump;
  CBCSlideCmd mSlide;
  CBCTauntCmd mTaunt;
  CBCScriptedCmd mScripted;
  CBCCoverCmd mCover;
  CBCWallHangCmd mWallHang;
  CBodyStateCmd mLocomotion;
  CBodyStateCmd mAdditiveIdle;
  CBCAdditiveAimCmd mAdditiveAim;
  CBCAdditiveFlinchCmd mAdditiveFlinch;
  CBCAdditiveReactionCmd mAdditiveReaction;
  CBodyStateCmd mStopReaction;
};
CHECK_SIZEOF(CBodyStateCmdMgr, 0x2a0)

#endif // _CBODYSTATECMDMGR
