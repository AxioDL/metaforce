#include "MetroidPrime/ScriptObjects/CScriptTimer.hpp"

#include "MetroidPrime/CStateManager.hpp"

CScriptTimer::CScriptTimer(const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const float startTime, const float maxRandDelay, const bool loop,
                           const bool autoStart, const bool active)
: CEntity(uid, info, active, name)
#if VERSION >= VERSION_GM8P_00
, mStartFrame(0)
#endif
, mTime(startTime)
, mStartTime(startTime)
, mMaxRandDelay(maxRandDelay)
, mLoop(loop)
, mAutoStart(autoStart)
, mIsTiming(autoStart) {}

CScriptTimer::~CScriptTimer() {}

void CScriptTimer::Reset(CStateManager& mgr) {
  const float rDt = mgr.Random()->Float();
  mTime = (mMaxRandDelay * rDt) + mStartTime;
}

void CScriptTimer::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId,
                                   CStateManager& stateMgr) {
#if VERSION >= VERSION_GM8P_00
  if (GetActive()) {
    switch (msg) {
    case kSM_Start:
      StartTiming(true);
      mStartFrame = stateMgr.GetInputFrameIdx();
      break;

    case kSM_Stop:
      StartTiming(false);
      break;

    case kSM_Reset:
      Reset(stateMgr);
      if (mAutoStart) {
        StartTiming(true);
        mStartFrame = stateMgr.GetInputFrameIdx();
      }
      break;

    case kSM_StopAndReset:
      Reset(stateMgr);
      StartTiming(false);
      break;

    case kSM_ResetAndStart:
      Reset(stateMgr);
      StartTiming(true);
      mStartFrame = stateMgr.GetInputFrameIdx();
      break;
    }
  }
#else
  switch (msg) {
  case kSM_Start:
    if (GetActive()) {
      StartTiming(true);
    }
    break;

  case kSM_Stop:
    if (GetActive()) {
      StartTiming(false);
    }
    break;

  case kSM_Reset:
    if (GetActive()) {
      Reset(stateMgr);
      if (mAutoStart) {
        StartTiming(true);
      }
    }
    break;

  case kSM_StopAndReset:
    if (GetActive()) {
      Reset(stateMgr);
      StartTiming(false);
    }
    break;

  case kSM_ResetAndStart:
    if (GetActive()) {
      Reset(stateMgr);
      StartTiming(true);
    }
    break;
  }
#endif
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
}

void CScriptTimer::ApplyTime(float dt, CStateManager& mgr) {
  if (mTime > 0.f && GetActive()) {
#if VERSION >= VERSION_GM8P_00
    if (mStartFrame == mgr.GetInputFrameIdx()) {
      return;
    }
#endif
    mTime -= dt;
    if (mTime <= 0.f) {
      SendScriptMsgs(kSS_Zero, mgr, kSM_None);

      mIsTiming = false;
      if (!mLoop) {
        return;
      }

      Reset(mgr);
      if (!mAutoStart) {
        return;
      }

      mIsTiming = true;
    }
  }
}

void CScriptTimer::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    bool should = false;
    if (IsTiming() && GetActive()) {
      should = true;
    }
    if (should) {
      ApplyTime(dt, mgr);
    }
  }
}

ENTITY_ACCEPT_IMPL(CScriptTimer)
