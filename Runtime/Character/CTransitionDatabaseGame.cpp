#include "CTransitionDatabaseGame.hpp"
#include "CTransition.hpp"
#include "CHalfTransition.hpp"

namespace urde {

CTransitionDatabaseGame::CTransitionDatabaseGame(const std::vector<CTransition>& transitions,
                                                 const std::vector<CHalfTransition>& halfTransitions,
                                                 const std::shared_ptr<IMetaTrans>& defaultTrans)
: x10_defaultTrans(defaultTrans) {
  x14_transitions.reserve(transitions.size());
  for (const CTransition& trans : transitions)
    x14_transitions.emplace_back(trans.GetAnimPair(), trans.GetMetaTrans());
  std::sort(x14_transitions.begin(), x14_transitions.end(),
    [](const auto& a, const auto& b) { return a.first < b.first; });

  x24_halfTransitions.reserve(halfTransitions.size());
  for (const CHalfTransition& trans : halfTransitions)
    x24_halfTransitions.emplace_back(trans.GetId(), trans.GetMetaTrans());
  std::sort(x24_halfTransitions.begin(), x24_halfTransitions.end(),
    [](const auto& a, const auto& b) { return a.first < b.first; });
}

const std::shared_ptr<IMetaTrans>& CTransitionDatabaseGame::GetMetaTrans(u32 a, u32 b) const {
  auto it = rstl::binary_find(x14_transitions.cbegin(), x14_transitions.cend(), std::make_pair(a, b),
    [](const std::pair<std::pair<u32, u32>, std::shared_ptr<IMetaTrans>>& p) { return p.first; });
  if (it != x14_transitions.cend())
    return it->second;
  auto it2 = rstl::binary_find(x24_halfTransitions.cbegin(), x24_halfTransitions.cend(), b,
    [](const std::pair<u32, std::shared_ptr<IMetaTrans>>& p) { return p.first; });
  if (it2 != x24_halfTransitions.cend())
    return it2->second;
  return x10_defaultTrans;
}

} // namespace urde
