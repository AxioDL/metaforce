#pragma once

#include "IOStreams.hpp"
#include "CPASAnimState.hpp"

namespace urde
{

class CRandom16;
class CPASAnimParmData;
class CPASDatabase
{
    std::vector<CPASAnimState> x0_states;
    s32 x10_defaultState;
    void AddAnimState(CPASAnimState&& state);
    void SetDefaultState(s32 state) {x10_defaultState = state;}
public:
    CPASDatabase(CInputStream& in);

    std::pair<float,s32> FindBestAnimation(const CPASAnimParmData&, s32) const;
    std::pair<float,s32> FindBestAnimation(const CPASAnimParmData&, CRandom16&, s32) const;
    s32 GetDefaultState() const { return x10_defaultState; }
    s32 GetNumAnimStates() const { return x0_states.size(); }
    const CPASAnimState* GetAnimState(s32 id) const
    {
        for (const CPASAnimState& state : x0_states)
            if (id == state.GetStateId())
                return &state;

        return nullptr;
    }
    const CPASAnimState* GetAnimStateByIndex(s32 index) const
    {
        if (index < 0 || index >= x0_states.size())
            return nullptr;

        return &x0_states.at(index);
    }

    bool HasState(s32 id) const
    {
        const auto& st = std::find_if(x0_states.begin(), x0_states.end(),
                                      [&id](const CPASAnimState& other)->bool
                                      { return other.GetStateId() == id; });
        return st != x0_states.end();
    }
};

}

