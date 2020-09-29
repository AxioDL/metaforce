#pragma once

#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/GCNTypes.hpp"
#include "Runtime/IObj.hpp"
#include "Runtime/IObjFactory.hpp"
#include "Runtime/IOStreams.hpp"
#include "Runtime/World/CAiFuncMap.hpp"

namespace urde {
class CAiState;
class CStateManager;

class CAiTrigger {
  CAiTriggerFunc x0_func;
  float xc_arg = 0.f;
  CAiTrigger* x10_andTrig = nullptr;
  CAiState* x14_state = nullptr;
  bool x18_lNot = false;

public:
  CAiTrigger() = default;
  CAiTrigger* GetAnd() const { return x10_andTrig; }
  CAiState* GetState() const { return x14_state; }
  bool CallFunc(CStateManager& mgr, CAi& ai) const {
    if (x0_func) {
      bool ret = (ai.*x0_func)(mgr, xc_arg);
      return x18_lNot == !ret;
    }
    return true;
  }

  void Setup(CAiTriggerFunc func, bool lnot, float arg, CAiTrigger* andTrig) {
    x0_func = func;
    x18_lNot = lnot;
    xc_arg = arg;
    x10_andTrig = andTrig;
  }
  void Setup(CAiTriggerFunc func, bool lnot, float arg, CAiState* state) {
    x0_func = func;
    x18_lNot = lnot;
    xc_arg = arg;
    x14_state = state;
  }
};

class CAiState {
  friend class CStateMachineState;
  CAiStateFunc x0_func;
  char xc_name[32] = {};
  u32 x2c_numTriggers = 0;
  CAiTrigger* x30_firstTrigger = nullptr;

public:
  CAiState(CAiStateFunc func, const char* name) {
    x0_func = func;
    strncpy(xc_name, name, 31);
  }

  s32 GetNumTriggers() const { return x2c_numTriggers; }
  CAiTrigger* GetTrig(s32 i) const { return &x30_firstTrigger[i]; }
  const char* GetName() const { return xc_name; }
  void SetTriggers(CAiTrigger* triggers) { x30_firstTrigger = triggers; }
  void SetNumTriggers(s32 numTriggers) { x2c_numTriggers = numTriggers; }
  void CallFunc(CStateManager& mgr, CAi& ai, EStateMsg msg, float delta) const {
    if (x0_func)
      (ai.*x0_func)(mgr, msg, delta);
  }
};

class CStateMachine {
  std::vector<CAiState> x0_states;
  std::vector<CAiTrigger> x10_triggers;

public:
  explicit CStateMachine(CInputStream& in);

  s32 GetStateIndex(std::string_view state) const;
  const std::vector<CAiState>& GetStateVector() const { return x0_states; }
};

class CStateMachineState {
  friend class CPatterned;
  const CStateMachine* x0_machine = nullptr;
  CAiState* x4_state = nullptr;
  float x8_time = 0.f;
  float xc_random = 0.f;
  float x10_delay = 0.f;
  float x14_ = 0.f;
  bool x18_24_codeTrigger : 1 = false;

public:
  CStateMachineState() = default;

  CAiState* GetActorState() const { return x4_state; }

  void Update(CStateManager& mgr, CAi& ai, float delta);
  void SetState(CStateManager&, CAi&, s32);
  void SetState(CStateManager&, CAi&, const CStateMachine*, std::string_view);
  const std::vector<CAiState>* GetStateVector() const;
  void Setup(const CStateMachine* machine);
  void SetDelay(float delay) { x10_delay = delay; }
  float GetTime() const { return x8_time; }
  float GetRandom() const { return xc_random; }
  float GetDelay() const { return x10_delay; }
  void SetCodeTrigger() { x18_24_codeTrigger = true; }

  const char* GetName() const {
    if (x4_state)
      return x4_state->GetName();
    return nullptr;
  }
};

CFactoryFnReturn FAiFiniteStateMachineFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                              CObjectReference*);

} // namespace urde
