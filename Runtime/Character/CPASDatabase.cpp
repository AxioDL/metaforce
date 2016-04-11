#include "CPASDatabase.hpp"

namespace urde
{

void CPASDatabase::AddAnimState(CPASAnimState&& state)
{
    auto it = std::lower_bound(x0_states.begin(), x0_states.end(), state,
    [](const CPASAnimState& item, const CPASAnimState& test) -> bool {return item.GetId() < test.GetId();});
    x0_states.insert(it, std::move(state));
}

CPASDatabase::CPASDatabase(CInputStream& in)
{
    in.readUint32Big();
    u32 animStateCount = in.readUint32Big();
    u32 defaultState = in.readUint32Big();

    x0_states.reserve(animStateCount);
    for (u32 i=0 ; i<animStateCount ; ++i)
    {
        CPASAnimState state(in);
        AddAnimState(std::move(state));
    }

    if (animStateCount)
        SetDefaultState(defaultState);
}

}
