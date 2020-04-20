#include "Runtime/World/CStateMachine.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CAi.hpp"

namespace urde {
static logvisor::Module Log("urde::CStateMachine");

CStateMachine::CStateMachine(CInputStream& in) {
  CAiTrigger* lastTrig = nullptr;
  u32 stateCount = in.readUint32Big();

  x0_states.reserve(stateCount);

  for (u32 i = 0; i < stateCount; ++i) {
    std::string name = in.readString(31, false);
    CAiStateFunc func = CAi::GetStateFunc(name.c_str());
    x0_states.emplace_back(func, name.c_str());
  }

  x10_triggers.reserve(in.readUint32Big());

  for (u32 i = 0; i < stateCount; ++i) {
    x0_states[i].SetNumTriggers(in.readUint32Big());
    if (x0_states[i].GetNumTriggers() == 0)
      continue;
    CAiTrigger* firstTrig = x10_triggers.data() + x10_triggers.size();
    x0_states[i].SetTriggers(firstTrig);
    x10_triggers.resize(x10_triggers.size() + x0_states[i].GetNumTriggers());

    for (u32 j = 0; j < x0_states[i].GetNumTriggers(); ++j) {
      u32 triggerCount = in.readUint32Big();
      u32 lastTriggerIdx = triggerCount - 1;
      for (u32 k = 0; k < triggerCount; ++k) {
        std::string name = in.readString(31, false);
        bool isNot = name.front() == '!';
        CAiTriggerFunc func = CAi::GetTrigerFunc(isNot ? name.c_str() + 1 : name.c_str());
        float arg = in.readFloatBig();
        CAiTrigger* newTrig;
        if (k < lastTriggerIdx) {
          newTrig = &x10_triggers.emplace_back();
        } else {
          newTrig = &firstTrig[j];
        }
        if (k == 0)
          newTrig->Setup(func, isNot, arg, &x0_states[in.readUint32Big()]);
        else
          newTrig->Setup(func, isNot, arg, lastTrig);
        lastTrig = newTrig;
      }
    }
  }
}

s32 CStateMachine::GetStateIndex(std::string_view state) const {
  auto it = std::find_if(x0_states.begin(), x0_states.end(),
                         [state](const CAiState& st) { return (strncmp(st.GetName(), state.data(), 31) == 0); });
  if (it == x0_states.end())
    return 0;

  return it - x0_states.begin();
}

void CStateMachineState::Update(CStateManager& mgr, CAi& ai, float delta) {
  if (x4_state) {
    x8_time += delta;
    x4_state->CallFunc(mgr, ai, EStateMsg::Update, delta);
    for (int i = 0; i < x4_state->GetNumTriggers(); ++i) {
      CAiTrigger* trig = x4_state->GetTrig(i);
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
      if (andPassed && state) {
        x4_state->CallFunc(mgr, ai, EStateMsg::Deactivate, 0.f);
        x4_state = state;
        Log.report(logvisor::Info, FMT_STRING("{} {} {} - {} {}"), ai.GetUniqueId(), ai.GetEditorId(), ai.GetName(),
                   state->xc_name, int(state - x0_machine->GetStateVector().data()));
        x8_time = 0.f;
        x18_24_codeTrigger = false;
        xc_random = mgr.GetActiveRandom()->Float();
        x4_state->CallFunc(mgr, ai, EStateMsg::Activate, delta);
        return;
      }
    }
  }
}

void CStateMachineState::SetState(CStateManager& mgr, CAi& ai, s32 idx) {
  CAiState* state = const_cast<CAiState*>(&x0_machine->GetStateVector()[idx]);
  if (x4_state != state) {
    if (x4_state)
      x4_state->CallFunc(mgr, ai, EStateMsg::Deactivate, 0.f);
    x4_state = state;
    x8_time = 0.f;
    xc_random = mgr.GetActiveRandom()->Float();
    x18_24_codeTrigger = false;
    x4_state->CallFunc(mgr, ai, EStateMsg::Activate, 0.f);
  }
}

void CStateMachineState::SetState(CStateManager& mgr, CAi& ai, const CStateMachine* machine, std::string_view state) {
  if (!machine)
    return;

  if (!x0_machine)
    Setup(machine);

  s32 idx = machine->GetStateIndex(state);
  SetState(mgr, ai, idx);
}

const std::vector<CAiState>* CStateMachineState::GetStateVector() const {
  if (!x0_machine)
    return nullptr;

  return &x0_machine->GetStateVector();
}

void CStateMachineState::Setup(const CStateMachine* machine) {
  x0_machine = machine;
  x4_state = nullptr;
  x8_time = 0.f;
  xc_random = 0.f;
  x10_delay = 0.f;
}

CFactoryFnReturn FAiFiniteStateMachineFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                              CObjectReference*) {
  return TToken<CStateMachine>::GetIObjObjectFor(std::make_unique<CStateMachine>(in));
}

} // namespace urde
