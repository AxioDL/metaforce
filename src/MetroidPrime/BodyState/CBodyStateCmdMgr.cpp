#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"

#include "Kyoto/Math/CloseEnough.hpp"
#include <float.h>

CBodyStateCmdMgr::CBodyStateCmdMgr()
: mMove(CVector3f::Zero())
, mFace(CVector3f::Zero())
, mTarget(CVector3f::Zero())
, mAdditiveTarget(CVector3f::Zero())
, mSteeringMode(kSBM_Normal)
, mSteeringSpeedMin(0.f)
, mSteeringSpeedMax(1.f)
, mSteeringSpeed(0.f)
, mCommandTable(nullptr)
, mDeliveredCmdMask(0)
, mGetup(pas::kGetup_Invalid)
, mStep(pas::kSD_Invalid, pas::kStep_Normal)
, mDie(kBSC_Die)
, mKnockDown(CVector3f::Zero(), pas::kS_Invalid)
, mKnockBack(CVector3f::Zero(), pas::kS_Invalid)
, mMeleeAttack(pas::kS_Invalid)
, mProjectileAttack(pas::kS_Invalid, CVector3f::Zero(), false)
, mLoopAttack(pas::kLAT_Invalid)
, mLoopReaction(pas::kRT_Invalid)
, mLoopHitReaction(pas::kRT_Invalid)
, mExitState(kBSC_ExitState)
, mLeanFromCover(kBSC_LeanFromCover)
, mNextState(kBSC_NextState)
, mMaintainVelocity(kBSC_MaintainVelocity)
, mGenerate(pas::kGType_Invalid, -1)
, mHurled(CVector3f::Zero(), CVector3f::Zero(), false)
, mJump(CVector3f::Zero(), pas::kJT_Normal)
, mSlide(pas::kSlide_Invalid, CVector3f::Zero())
, mTaunt(pas::kTT_Invalid)
, mScripted(-1, false, false, 0.f)
, mCover(pas::kCD_Invalid, CVector3f::Zero(), CVector3f::Forward())
, mWallHang(kInvalidUniqueId)
, mLocomotion(kBSC_Locomotion)
, mAdditiveIdle(kBSC_AdditiveIdle)
, mAdditiveFlinch(1.f)
, mAdditiveReaction(pas::kART_Invalid, 0.f, false)
, mStopReaction(kBSC_StopReaction) {
  mCommandTable[kBSC_Getup] = &mGetup;
  mCommandTable[kBSC_Step] = &mStep;
  mCommandTable[kBSC_Die] = &mDie;
  mCommandTable[kBSC_KnockDown] = &mKnockDown;
  mCommandTable[kBSC_KnockBack] = &mKnockBack;
  mCommandTable[kBSC_MeleeAttack] = &mMeleeAttack;
  mCommandTable[kBSC_ProjectileAttack] = &mProjectileAttack;
  mCommandTable[kBSC_LoopAttack] = &mLoopAttack;
  mCommandTable[kBSC_LoopReaction] = &mLoopReaction;
  mCommandTable[kBSC_LoopHitReaction] = &mLoopHitReaction;
  mCommandTable[kBSC_ExitState] = &mExitState;
  mCommandTable[kBSC_LeanFromCover] = &mLeanFromCover;
  mCommandTable[kBSC_NextState] = &mNextState;
  mCommandTable[kBSC_MaintainVelocity] = &mMaintainVelocity;
  mCommandTable[kBSC_Generate] = &mGenerate;
  mCommandTable[kBSC_Hurled] = &mHurled;
  mCommandTable[kBSC_Jump] = &mJump;
  mCommandTable[kBSC_Slide] = &mSlide;
  mCommandTable[kBSC_Taunt] = &mTaunt;
  mCommandTable[kBSC_Scripted] = &mScripted;
  mCommandTable[kBSC_Cover] = &mCover;
  mCommandTable[kBSC_WallHang] = &mWallHang;
  mCommandTable[kBSC_Locomotion] = &mLocomotion;
  mCommandTable[kBSC_AdditiveIdle] = &mAdditiveIdle;
  mCommandTable[kBSC_AdditiveAim] = &mAdditiveAim;
  mCommandTable[kBSC_AdditiveFlinch] = &mAdditiveFlinch;
  mCommandTable[kBSC_AdditiveReaction] = &mAdditiveReaction;
  mCommandTable[kBSC_StopReaction] = &mStopReaction;
}

CBodyStateCmdMgr::~CBodyStateCmdMgr() {
  mMove = CVector3f::Zero();
  mFace = CVector3f::Zero();
  mTarget = CVector3f::Zero();
  mSteeringSpeed = 0.f;
  mDeliveredCmdMask = 0;
}

void CBodyStateCmdMgr::SetSteeringSpeedRange(float rmin, float rmax) {
  mSteeringSpeedMin = rmin;
  mSteeringSpeedMax = rmax;
}

void CBodyStateCmdMgr::DeliverCmd(const CBodyStateCmd& cmd) {
  DeliverCmd(cmd.GetCommandId());
  *mCommandTable[(uint)cmd.GetCommandId()] = cmd;
}

void CBodyStateCmdMgr::DeliverCmd(const CBCLocomotionCmd& cmd) {
  if (!(cmd.GetWeight() > FLT_EPSILON)) {
    return;
  }
  mSteeringSpeed += cmd.GetWeight();
  mMove += cmd.GetMoveVector() * cmd.GetWeight();
  mFace += cmd.GetFaceVector() * cmd.GetWeight();
}

void CBodyStateCmdMgr::BlendSteeringCmds() {
  if (mSteeringSpeed > FLT_EPSILON) {
    const float weightScale = 1.f / mSteeringSpeed;
    mFace *= weightScale;
    switch (mSteeringMode) {
    case kSBM_Normal:
      mMove *= weightScale;
      break;
    case kSBM_FullSpeed:
      if (!close_enough(mMove, CVector3f::Zero(), 0.0001f)) {
        mMove.Normalize();
        mMove *= mSteeringSpeedMax;
      }
      break;
    case kSBM_Clamped:
      mMove *= weightScale;
      if (!close_enough(mMove, CVector3f::Zero(), 0.0001f)) {
        const float speed = mMove.Magnitude();
        if (speed < mSteeringSpeedMin) {
          mMove = mMove.AsNormalized() * mSteeringSpeedMin;
        } else if (speed > mSteeringSpeedMax) {
          mMove = mMove.AsNormalized() * mSteeringSpeedMax;
        }
      }
      break;
    }
  }
}

void CBodyStateCmdMgr::Reset() {
  mMove = CVector3f::Zero();
  mFace = CVector3f::Zero();
  mTarget = CVector3f::Zero();
  mSteeringSpeed = 0.f;
  mDeliveredCmdMask = 0;
}

void CBodyStateCmdMgr::ClearLocomotionCmds() {
  mMove = CVector3f::Zero();
  mFace = CVector3f::Zero();
  mSteeringSpeed = 0.f;
}

const CBodyStateCmd* CBodyStateCmdMgr::GetCmd(EBodyStateCmd cmd) const {
  if (mDeliveredCmdMask & (1 << cmd)) {
    return mCommandTable[cmd];
  }
  return nullptr;
}

CBodyStateCmd* CBodyStateCmdMgr::GetCmd(EBodyStateCmd cmd) {
  if (mDeliveredCmdMask & (1 << cmd)) {
    return mCommandTable[cmd];
  }
  return nullptr;
}

void CBodyStateCmdMgr::DeliverCmd(const EBodyStateCmd cmd) {
  mDeliveredCmdMask |= (1 << cmd);
}
