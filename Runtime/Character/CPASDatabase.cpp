#include "Runtime/Character/CPASDatabase.hpp"

#include "Runtime/CRandom16.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"

#include <algorithm>

namespace metaforce {

void CPASDatabase::AddAnimState(CPASAnimState&& state) {
  auto it = std::lower_bound(x0_states.begin(), x0_states.end(), state,
                             [](const CPASAnimState& item, const CPASAnimState& test) -> bool {
                               return item.GetStateId() < test.GetStateId();
                             });
  x0_states.insert(it, std::move(state));
}

CPASDatabase::CPASDatabase(CInputStream& in) {
  in.ReadLong();
  u32 animStateCount = in.ReadLong();
  u32 defaultState = in.ReadLong();

  x0_states.reserve(animStateCount);
  for (u32 i = 0; i < animStateCount; ++i) {
    CPASAnimState state(in);
    AddAnimState(std::move(state));
  }

  if (animStateCount)
    SetDefaultState(defaultState);
}

std::pair<float, s32> CPASDatabase::FindBestAnimation(const CPASAnimParmData& data, s32 ignoreAnim) const {
  CRandom16 rnd(4660);
  return FindBestAnimation(data, rnd, ignoreAnim);
}

std::pair<float, s32> CPASDatabase::FindBestAnimation(const CPASAnimParmData& data, CRandom16& rand,
                                                      s32 ignoreAnim) const {
  auto it = rstl::binary_find(x0_states.cbegin(), x0_states.cend(), data.GetStateId(),
                              [](const CPASAnimState& item) { return item.GetStateId(); });

  if (it == x0_states.cend())
    return {0.f, -1};

  return (*it).FindBestAnimation(data.GetAnimParmData(), rand, ignoreAnim);
}

} // namespace metaforce
