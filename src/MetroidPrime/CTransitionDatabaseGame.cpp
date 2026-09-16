#include "MetroidPrime/CTransitionDatabaseGame.hpp"

#include "Kyoto/Animation/CHalfTransition.hpp"
#include "Kyoto/Animation/CTransition.hpp"

#include "rstl/algorithm.hpp"

namespace {
struct uint_comparer {
  bool operator()(uint a, uint b) const { return a < b; }
  bool operator()(const rstl::pair< uint, uint >& a, const rstl::pair< uint, uint >& b) const {
    const uint& aTo = a.second;
    if (a.first == b.first) {
      return aTo < b.second;
    }
    return a.first < b.first;
  }
};
} // namespace

CTransitionDatabaseGame::CTransitionDatabaseGame(
    const rstl::vector< CTransition >& transitions,
    const rstl::vector< CHalfTransition >& halfTransitions, rstl::rc_ptr< IMetaTrans > defaultTrans)
: x10_defaultTrans(defaultTrans) {
  rstl::vector< CTransition >::const_iterator it = transitions.begin(), end = transitions.end();
  x14_transitions.reserve(transitions.size());
  for (; it != end;) {
    const rstl::pair< rstl::pair< uint, uint >, rstl::rc_ptr< IMetaTrans > > entry(
        rstl::pair< uint, uint >(it->GetFromAnimIndex(), it->GetToAnimIndex()), it->GetMetaTrans());
    x14_transitions.push_back(entry);
    ++it;
  }
  rstl::sort_by_key(x14_transitions, uint_comparer());

  rstl::vector< CHalfTransition >::const_iterator halfIt = halfTransitions.begin(),
                                                halfEnd = halfTransitions.end();
  x24_halfTransitions.reserve(halfTransitions.size());
  for (; halfIt != halfEnd;) {
    const rstl::pair< uint, rstl::rc_ptr< IMetaTrans > > entry(
        halfIt->GetPrimitiveIndex(), halfIt->GetMetaTrans());
    x24_halfTransitions.push_back(entry);
    ++halfIt;
  }
  rstl::sort_by_key(x24_halfTransitions, uint_comparer());
}

const rstl::rc_ptr< IMetaTrans >& CTransitionDatabaseGame::GetMetaTrans(uint from, uint to) const {
  AUTO(it, rstl::find_by_key(x14_transitions, rstl::pair< uint, uint >(from, to), uint_comparer()));
  if (it != x14_transitions.end()) {
    return it->second;
  }
  AUTO(halfIt, rstl::find_by_key(x24_halfTransitions, to, uint_comparer()));
  if (halfIt != x24_halfTransitions.end()) {
    return halfIt->second;
  }
  return x10_defaultTrans;
}
