#include "MetroidPrime/ScriptObjects/CScriptCounter.hpp"

CScriptCounter::CScriptCounter(const TUniqueId uid, const rstl::string& name,
                               const CEntityInfo& info, const int initial, const int max,
                               const bool autoReset, const bool active)
: CEntity(uid, info, active, name)
, mInitial(initial)
, mCurrent(initial)
, mMax(max)
, mAutoReset(autoReset) {}

CScriptCounter::~CScriptCounter() {}

void CScriptCounter::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId,
                                     CStateManager& stateMgr) {
  switch (msg) {
  case kSM_SetToZero:
    if (GetActive()) {
      mCurrent = 0;
      SendScriptMsgs(kSS_Zero, stateMgr, kSM_None);

      if (mAutoReset)
        mCurrent = mInitial;
    }
    break;
  case kSM_SetToMax:
    if (GetActive()) {
      mCurrent = mMax;
      SendScriptMsgs(kSS_MaxReached, stateMgr, kSM_None);

      if (mAutoReset)
        mCurrent = mInitial;
    }
    break;
  case kSM_Decrement:
    if (GetActive() && mCurrent != 0) {
      --mCurrent;
      if (mCurrent == 0) {
        SendScriptMsgs(kSS_Zero, stateMgr, kSM_None);
        if (mAutoReset)
          mCurrent = mInitial;
      }
    }
    break;
  case kSM_Increment:
    if (GetActive() && mCurrent < mMax) {
      ++mCurrent;
      if (mCurrent == mMax) {
        SendScriptMsgs(kSS_MaxReached, stateMgr, kSM_None);
        if (mAutoReset)
          mCurrent = mInitial;
      }
    }
    break;
  case kSM_Reset:
    if (GetActive())
      mCurrent = mInitial;
    break;
  default:
    break;
  }

  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
}

ENTITY_ACCEPT_IMPL(CScriptCounter)
