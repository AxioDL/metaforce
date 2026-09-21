#ifndef _CSTATEMACHINE
#define _CSTATEMACHINE

#include "rstl/string.hpp"
#include "rstl/vector.hpp"
#include "string.h"

#include "MetroidPrime/Enemies/CAi.hpp"

class CAi;
class CStateManager;
class CInputStream;

class CAiState;
class CAiTrigger {
public:
  CAiTrigger()
  : x0_func(nullptr), xc_arg(0.f), x10_andTrigger(nullptr), x14_state(nullptr), x18_lNot(false) {}
  CAiTrigger* GetAnd() const { return x10_andTrigger; }
  CAiState* GetState() const { return x14_state; }

  bool CallFunc(CStateManager& mgr, CAi& ai) {
    bool ret = true;
    if (x0_func) {
      ret = (ai.*x0_func)(mgr, xc_arg);
      if (x18_lNot) {
        ret = !ret;
      }
    }

    return ret;
  }

  void Setup(CAiTriggerFunc func, bool lnot, float arg, CAiTrigger* andTrig) {
    x0_func = func;
    xc_arg = arg;
    x10_andTrigger = andTrig;
    x18_lNot = lnot;
  }

  void Setup(CAiTriggerFunc func, bool lnot, float arg, CAiState* state) {
    x0_func = func;
    xc_arg = arg;
    x14_state = state;
    x18_lNot = lnot;
  }

private:
  CAiTriggerFunc x0_func;
  float xc_arg;
  CAiTrigger* x10_andTrigger;
  CAiState* x14_state;
  bool x18_lNot;
};

CHECK_SIZEOF(CAiTrigger, 0x1c)

class CAiState {
public:
  CAiState(CAiStateFunc func, const char* name)
  : x0_func(func), x2c_numTriggers(0), x30_firstTrigger(nullptr) {
    strncpy(xc_name, name, 31);
  }

  CAiTrigger* GetTrig(int idx) const { return &x30_firstTrigger[idx]; }
  const char* GetName() const { return xc_name; }
  void SetTriggers(CAiTrigger* triggers) { x30_firstTrigger = triggers; }
  void SetNumTriggers(int numTriggers) { x2c_numTriggers = numTriggers; }
  int GetNumTriggers() const { return x2c_numTriggers; }

  void CallFunc(CStateManager& mgr, CAi& ai, EStateMsg msg, float arg) const {
    if (x0_func) {
      (ai.*x0_func)(mgr, msg, arg);
    }
  }

private:
  CAiStateFunc x0_func;
  char xc_name[32];
  uint x2c_numTriggers;
  CAiTrigger* x30_firstTrigger;
};

CHECK_SIZEOF(CAiState, 0x34)

class CStateMachine {
public:
  explicit CStateMachine(CInputStream& in);

  int GetStateIndex(const rstl::string& state) const;
  const rstl::vector< CAiState >& GetStateVector() const { return x0_states; }

private:
  rstl::vector< CAiState > x0_states;
  rstl::vector< CAiTrigger > x10_triggers;
};

CHECK_SIZEOF(CStateMachine, 0x20)

class CStateMachineState {
public:
  CStateMachineState();

  void Update(CStateManager& mgr, CAi& ai, float delta);
  void SetState(CStateManager& mgr, CAi& ai, int state);
  void SetState(CStateManager& mgr, CAi&, const CStateMachine* machine, const rstl::string& state);
  const rstl::vector< CAiState >& GetStateVector() const { return x0_machine->GetStateVector(); }
  void Setup(const CStateMachine* machine);
  const char* GetName() const;
  CAiState* GetActorState() const { return x4_state; }
  void SetDelay(float delay) { x10_delay = delay; }
  float GetTime() const { return x8_time; }
  float GetRandom() const { return xc_random; }
  float GetDelay() const { return x10_delay; }
  float GetFixedRandom() const { return x14_fixedRandom; }
  bool GetCodeTrigger() const { return x18_24_codeTrigger; }
  void SetCodeTrigger() { x18_24_codeTrigger = true; }

private:
  const CStateMachine* x0_machine;
  CAiState* x4_state;
  float x8_time;
  float xc_random;
  float x10_delay;
  float x14_fixedRandom;
  bool x18_24_codeTrigger : 1;
};

CHECK_SIZEOF(CStateMachineState, 0x1c)

#endif // _CSTATEMACHINE
