#ifndef _CSTATEMACHINE
#define _CSTATEMACHINE

#include "rstl/string.hpp"
#include "rstl/vector.hpp"
#include "string.h"

#include "MetroidPrime/Enemies/CAi.hpp"

class CStateManager;
class CInputStream;

class CAiState;
class CAiTrigger {
public:
  CAiTrigger()
  : mFunc(nullptr), mArg(0.f), mAndTrigger(nullptr), mState(nullptr), mLNot(false) {}
  CAiTrigger* GetAnd() const { return mAndTrigger; }
  CAiState* GetState() const { return mState; }

  bool CallFunc(CStateManager& mgr, CAi& ai) {
    bool ret = true;
    if (mFunc) {
      ret = (ai.*mFunc)(mgr, mArg);
      if (mLNot) {
        ret = !ret;
      }
    }

    return ret;
  }

  void Setup(CAiTriggerFunc func, bool lnot, float arg, CAiTrigger* andTrig) {
    mFunc = func;
    mArg = arg;
    mAndTrigger = andTrig;
    mLNot = lnot;
  }

  void Setup(CAiTriggerFunc func, bool lnot, float arg, CAiState* state) {
    mFunc = func;
    mArg = arg;
    mState = state;
    mLNot = lnot;
  }

private:
  CAiTriggerFunc mFunc;
  float mArg;
  CAiTrigger* mAndTrigger;
  CAiState* mState;
  bool mLNot;
};

CHECK_SIZEOF(CAiTrigger, 0x1c)

class CAiState {
public:
  CAiState(CAiStateFunc func, const char* name)
  : mFunc(func), mNumTriggers(0), mFirstTrigger(nullptr) {
    strncpy(mName, name, 31);
  }

  CAiTrigger* GetTrig(int idx) const { return &mFirstTrigger[idx]; }
  const char* GetName() const { return mName; }
  void SetTriggers(CAiTrigger* triggers) { mFirstTrigger = triggers; }
  void SetNumTriggers(int numTriggers) { mNumTriggers = numTriggers; }
  int GetNumTriggers() const { return mNumTriggers; }

  void CallFunc(CStateManager& mgr, CAi& ai, EStateMsg msg, float arg) const {
    if (mFunc) {
      (ai.*mFunc)(mgr, msg, arg);
    }
  }

private:
  CAiStateFunc mFunc;
  char mName[32];
  uint mNumTriggers;
  CAiTrigger* mFirstTrigger;
};

CHECK_SIZEOF(CAiState, 0x34)

class CStateMachine {
public:
  explicit CStateMachine(CInputStream& in);

  int GetStateIndex(const rstl::string& state) const;
  const rstl::vector< CAiState >& GetStateVector() const { return mStates; }

private:
  rstl::vector< CAiState > mStates;
  rstl::vector< CAiTrigger > mTriggers;
};

CHECK_SIZEOF(CStateMachine, VERSION >= VERSION_R3IJ_00 ? 0x18 : 0x20)

class CStateMachineState {
public:
  CStateMachineState();

  void Update(CStateManager& mgr, CAi& ai, float delta);
  void SetState(CStateManager& mgr, CAi& ai, int state);
  void SetState(CStateManager& mgr, CAi&, const CStateMachine* machine, const rstl::string& state);
  const rstl::vector< CAiState >& GetStateVector() const { return mMachine->GetStateVector(); }
  void Setup(const CStateMachine* machine);
  const char* GetName() const;
  CAiState* GetActorState() const { return mState; }
  void SetDelay(float delay) { mDelay = delay; }
  float GetTime() const { return mTime; }
  float GetRandom() const { return mRandom; }
  float GetDelay() const { return mDelay; }
  float GetFixedRandom() const { return mFixedRandom; }
  bool GetCodeTrigger() const { return mCodeTrigger; }
  void SetCodeTrigger() { mCodeTrigger = true; }

private:
  const CStateMachine* mMachine;
  CAiState* mState;
  float mTime;
  float mRandom;
  float mDelay;
  float mFixedRandom;
  bool mCodeTrigger : 1;
};

CHECK_SIZEOF(CStateMachineState, 0x1c)

#endif // _CSTATEMACHINE
