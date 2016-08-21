#include "CPASDatabase.hpp"
#include "CPASAnimParmData.hpp"
#include "CRandom16.hpp"

namespace urde
{

void CPASDatabase::AddAnimState(CPASAnimState&& state)
{
    auto it = std::lower_bound(x0_states.begin(), x0_states.end(), state,
    [](const CPASAnimState& item, const CPASAnimState& test) -> bool {return item.GetStateId() < test.GetStateId();});
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

std::pair<float, s32> CPASDatabase::FindBestAnimation(const CPASAnimParmData& data, s32 id) const
{
    CRandom16 rnd(4660);
    return FindBestAnimation(data, rnd, id);
}

std::pair<float, s32> CPASDatabase::FindBestAnimation(const CPASAnimParmData& data, CRandom16& rand, s32 id) const
{
    auto it = std::lower_bound(x0_states.cbegin(), x0_states.cend(), id,
    [](const CPASAnimState& item, const int& test) -> bool {return item.GetStateId() < test;});

    if (it == x0_states.cend())
        return {0.f, -1};

    return (*it).FindBestAnimation(data.GetAnimParmData(), rand, id);
}

}
