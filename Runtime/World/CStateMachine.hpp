#ifndef CSTATEMACHINE_HPP
#define CSTATEMACHINE_HPP

#include "RetroTypes.hpp"
#include "CAiFuncMap.hpp"

namespace urde
{
class CAiState;
class CStateManager;
class CAiTrigger
{
    u32 x0_ = 0;
    u32 x4_ = 0;
    u32 x8_ = 0;
    float xc_ = 0.f;
    u32 x10_ = 0;
    u32 x14_ = 0;
    bool x18_ = false;
public:
    CAiTrigger() = default;
    bool GetAnd();
    void GetState();
    bool CallFunc(CStateManager&, CAi&)
    {
        return false;
    }

    void Setup(CAiTriggerFunc func, bool, float, CAiTrigger*);
    void Setup(CAiTriggerFunc func, bool, float, CAiState*);
};

class CAiState
{
    CAiStateFunc x0_func;
    const char* x4_name;
    u32 x8_;
    u32 xc_;
    u32 x10_;
    u32 x14_;
    u32 x18_;
    u32 x1c_;
    u32 x20_;
    u32 x24_;
    u32 x28_;
    u32 x2c_numTriggers;
    u32 x30_;
public:
    CAiState(CAiStateFunc func, const char* name)
    {}

    s32 GetNumTriggers() const;
    CAiTrigger& GetTrig(s32) const;
    const char* GetName() const { return x4_name; }
    void SetTriggers(CAiTrigger* triggers);
    void SetNumTriggers(s32 numTriggers) { x2c_numTriggers = numTriggers; }
    void CallFunc(CStateManager& mgr, CAi& ai, EStateMsg msg, float delta) const
    {
        if (x0_func)
            (ai.*x0_func)(mgr, msg, delta);
    }
};

class CStateMachine
{
    std::vector<CAiState> x0_states;
    std::vector<CAiTrigger> x10_triggers;
public:
    CStateMachine(CInputStream& in);

    s32 GetStateIndex(std::string_view state) const;
    const std::vector<CAiState>& GetStateVector() const;
};

class CStateMachineState
{
    const CStateMachine* x0_machine = nullptr;
    CAiState* x4_state = nullptr;
    float x8_time = 0.f;
    float xc_ = 0.f;
    float x10_ = 0.f;
    union
    {
        struct
        {
            bool x18_24_ : 1;
        };
        u32 dummy = 0;
    };
public:
    CStateMachineState()=default;

    void GetActorState() const;
    float GetTime() const;

    void Update(CStateManager& mgr, CAi& ai, float delta)
    {
        x8_time += delta;
        if (x4_state)
            x4_state->CallFunc(mgr, ai, EStateMsg::One, delta);
    }
    void SetState(CStateManager&, CAi&, s32);
    void SetState(CStateManager&, CAi&, const CStateMachine*, std::string_view);
    const std::vector<CAiState>* GetStateVector() const;
    void Setup(const CStateMachine* machine);
    std::string GetName() const;
    void SetDelay(float);
    void GetRandom() const;
    float GetDelay() const;
};

}

#endif // CSTATEMACHINE_HPP
