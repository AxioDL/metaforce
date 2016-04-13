#ifndef __URDE_CPASDATABASE_HPP__
#define __URDE_CPASDATABASE_HPP__

#include "IOStreams.hpp"
#include "CPASAnimState.hpp"

namespace urde
{

class CPASDatabase
{
    std::vector<CPASAnimState> x0_states;
    u32 x10_defaultState;
    void AddAnimState(CPASAnimState&& state);
    void SetDefaultState(u32 state) {x10_defaultState = state;}
public:
    CPASDatabase(CInputStream& in);
};

}

#endif // __URDE_CPASDATABASE_HPP__
