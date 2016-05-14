#include "CStateMachine.hpp"
#include "CAi.hpp"

namespace urde
{
CStateMachine::CStateMachine(CInputStream& in)
{
}

s32 CStateMachine::GetStateIndex(const std::string& state)
{
    auto it = std::find_if(x0_states.begin(), x0_states.end(), [&state](const CAiState& st) -> bool {
        return (strncmp(st.GetName(), state.c_str(), 31) == 0);
    });
    if (it == x0_states.end())
        return 0;

    return it - x0_states.begin();
}

const std::vector<CAiState>& CStateMachine::GetStateVector() const
{
    return x0_states;
}
}
