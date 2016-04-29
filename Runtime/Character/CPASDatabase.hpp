#ifndef __URDE_CPASDATABASE_HPP__
#define __URDE_CPASDATABASE_HPP__

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

    void FindBestAnimation(const CPASAnimParmData&, int) const;
    void FindBestAnimation(const CPASAnimParmData&, CRandom16&, int) const;
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

#endif // __URDE_CPASDATABASE_HPP__
