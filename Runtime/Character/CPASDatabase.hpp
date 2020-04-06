#pragma once

#include <algorithm>
#include <utility>
#include <vector>

#include "Runtime/IOStreams.hpp"
#include "Runtime/Character/CPASAnimState.hpp"

namespace urde {

class CRandom16;
class CPASAnimParmData;
class CPASDatabase {
  std::vector<CPASAnimState> x0_states;
  s32 x10_defaultState;
  void AddAnimState(CPASAnimState&& state);
  void SetDefaultState(s32 state) { x10_defaultState = state; }

public:
  explicit CPASDatabase(CInputStream& in);

  std::pair<float, s32> FindBestAnimation(const CPASAnimParmData& data, s32 ignoreAnim) const;
  std::pair<float, s32> FindBestAnimation(const CPASAnimParmData& data, CRandom16& rand, s32 ignoreAnim) const;
  s32 GetDefaultState() const { return x10_defaultState; }
  size_t GetNumAnimStates() const { return x0_states.size(); }
  const CPASAnimState* GetAnimState(s32 id) const {
    for (const CPASAnimState& state : x0_states)
      if (id == state.GetStateId())
        return &state;

    return nullptr;
  }
  const CPASAnimState* GetAnimStateByIndex(size_t index) const {
    if (index >= x0_states.size()) {
      return nullptr;
    }

    return &x0_states[index];
  }

  bool HasState(s32 id) const {
    const auto& st = std::find_if(x0_states.begin(), x0_states.end(),
                                  [&id](const CPASAnimState& other) -> bool { return other.GetStateId() == id; });
    return st != x0_states.end();
  }
};

} // namespace urde
