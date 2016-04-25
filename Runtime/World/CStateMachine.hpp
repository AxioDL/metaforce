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

    float xc_ = 0.f;
    u32  x10_;
    bool x18_;
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
    u32 x2c_;
    u32 x30_;
public:
    CAiState(CAiStateFunc func, const char* name)
    {}

    u32 GetNumTriggers() const;
    CAiTrigger& GetTrig(s32) const;
    const char* GetName() const;
    void SetTriggers(CAiTrigger* triggers);
    void SetNumTriggers(s32 numTriggers);
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

    const std::vector<CAiState>& GetStateVector() const { return x0_states; }
};

class CStateMachineState
{
    const CStateMachine* x0_ = nullptr;
    CAiState* x4_state = nullptr;
    float x8_ = 0;
    float xc_ = 0;
    float x10_ = 0;
public:
    CStateMachineState()=default;
    void Setup(const CStateMachine* machine)
    {
        x0_ = machine;
        x4_state = nullptr;
        x8_ = 0.f;
        xc_ = 0.f;
        x10_= 0.f;
    }

    void SetDelay(float);
    float GetDelay() const;

    void Update(CStateManager& mgr, CAi& ai, float delta)
    {
        x8_ += delta;
        if (x4_state)
            x4_state->CallFunc(mgr, ai, EStateMsg::One, delta);
    }

};

}

#endif // CSTATEMACHINE_HPP
