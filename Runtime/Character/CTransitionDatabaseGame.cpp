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

  x24_halfTransitions.reserve(halfTransitions.size());
  for (const CHalfTransition& trans : halfTransitions)
    x24_halfTransitions.emplace_back(trans.GetId(), trans.GetMetaTrans());
}

const std::shared_ptr<IMetaTrans>& CTransitionDatabaseGame::GetMetaTrans(u32 a, u32 b) const {
  auto it = std::find_if(x14_transitions.cbegin(), x14_transitions.cend(),
                         [&](const std::pair<std::pair<u32, u32>, std::shared_ptr<IMetaTrans>>& elem) -> bool {
                           return elem.first.first == a && elem.first.second == b;
                         });
  if (it != x14_transitions.cend())
    return it->second;
  return x10_defaultTrans;
}

} // namespace urde
