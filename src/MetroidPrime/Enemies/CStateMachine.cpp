#include "MetroidPrime/Enemies/CStateMachine.hpp"

#include "MetroidPrime/Enemies/CAi.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

void CStateMachineState::Update(CStateManager& mgr, CAi& ai, float delta) {
  if (mState) {
    mTime += delta;
    mState->CallFunc(mgr, ai, kStateMsg_Update, delta);
    for (int i = 0; i < mState->GetNumTriggers(); ++i) {
      CAiTrigger* trig = mState->GetTrig(i);
      CAiState* state = nullptr;
      bool andPassed = true;
      while (andPassed && trig) {
        andPassed = false;
        if (trig->CallFunc(mgr, ai)) {
          andPassed = true;
          state = trig->GetState();
          trig = trig->GetAnd();
        }
      }

      if (andPassed && state != nullptr) {
        mState->CallFunc(mgr, ai, kStateMsg_Deactivate, 0.f);
        mState = state;
        mTime = 0.f;
        mCodeTrigger = false;
        mRandom = mgr.Random()->Float();
        mState->CallFunc(mgr, ai, kStateMsg_Activate, 0.f);
        return;
      }
    }
  }
}

void CStateMachineState::SetState(CStateManager& mgr, CAi& ai, int idx) {
  const CAiState* state = &mMachine->GetStateVector()[idx];
  if (mState == state) {
    return;
  }
  if (mState) {
    mState->CallFunc(mgr, ai, kStateMsg_Deactivate, 0.f);
  }

  mState = const_cast< CAiState* >(state);
  mTime = 0.f;
  mRandom = mgr.Random()->Float();
  mCodeTrigger = false;
  mState->CallFunc(mgr, ai, kStateMsg_Activate, 0.f);
}

void CStateMachineState::SetState(CStateManager& mgr, CAi& ai, const CStateMachine* machine,
                                  const rstl::string& state) {
  if (!machine)
    return;

  if (!mMachine)
    Setup(machine);

  s32 idx = machine->GetStateIndex(state);
  SetState(mgr, ai, idx);
}

CStateMachineState::CStateMachineState()
: mMachine(nullptr)
, mState(nullptr)
, mTime(0.f)
, mRandom(0.f)
, mDelay(0.f)
, mCodeTrigger(false) {}

void CStateMachineState::Setup(const CStateMachine* machine) {
  mMachine = machine;
  mState = nullptr;
  mTime = 0.f;
  mRandom = 0.f;
  mDelay = 0.f;
}

const char* CStateMachineState::GetName() const {
  if (mState != nullptr) {
    return mState->GetName();
  }

  return nullptr;
}

CStateMachine::CStateMachine(CInputStream& in) {
  CAiTrigger* lastTrig = nullptr;
  const int stateCount = in.Get< int >();
  char name[32];
  int nameLen;
  int i;

  mStates.reserve(stateCount);

  for (i = 0; i < stateCount; ++i) {
    nameLen = 0;
    for (; nameLen < 31; ++nameLen) {
      name[nameLen] = in.Get< char >();
      if (name[nameLen] == '\0') {
        break;
      }
    }
    name[nameLen] = '\0';
    CAiStateFunc func = CAi::GetStateFunc(name);
    mStates.push_back(CAiState(func, name));
  }

  mTriggers.reserve(in.Get< int >());

  for (i = 0; i < stateCount; ++i) {
    int j;
    CAiState& state = mStates[i];
    const int firstTriggerIdx = mTriggers.size();
    state.SetNumTriggers(in.Get< int >());

    if (state.GetNumTriggers() == 0) {
      continue;
    }

    for (j = 0; j < state.GetNumTriggers(); ++j) {
      mTriggers.push_back(CAiTrigger());
    }

    state.SetTriggers(&mTriggers[firstTriggerIdx]);

    for (j = 0; j < state.GetNumTriggers(); ++j) {
      const int triggerCount = in.Get< int >();
      const int lastTriggerIdx = triggerCount - 1;

      for (int k = 0; k < triggerCount; ++k) {
        nameLen = 0;
        for (; nameLen < 31; ++nameLen) {
          name[nameLen] = in.Get< char >();

          if (name[nameLen] == '\0') {
            break;
          }
        }

        name[nameLen] = '\0';

        const CAiTriggerFunc func = CAi::GetTriggerFunc(name[0] == '!' ? name + 1 : name);
        const float arg = in.Get< float >();
        const bool isNot = name[0] == '!';
        CAiTrigger* newTrig;
        if (k < lastTriggerIdx) {
          mTriggers.push_back(CAiTrigger());
          newTrig = &mTriggers.back();
        } else {
          newTrig = state.GetTrig(j);
        }
        if (k == 0) {
          newTrig->Setup(func, isNot, arg, &mStates[in.Get< int >()]);
        } else {
          newTrig->Setup(func, isNot, arg, lastTrig);
        }
        lastTrig = newTrig;
      }
    }
  }
}

int CStateMachine::GetStateIndex(const rstl::string& state) const {
  for (int i = 0; i < mStates.size(); ++i) {
    if (strncmp(mStates[i].GetName(), state.data(), 31) == 0) {
      return i;
    }
  }

  return 0;
}
