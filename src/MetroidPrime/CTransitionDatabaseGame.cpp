#include "MetroidPrime/CTransitionDatabaseGame.hpp"

#include "Kyoto/Animation/CHalfTransition.hpp"
#include "Kyoto/Animation/CTransition.hpp"
#if VERSION < VERSION_GM8P_00
#define RSTL_DONT_INLINE_ALGORITHM
#endif
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
: mDefaultTrans(defaultTrans) {
  rstl::vector< CTransition >::const_iterator it = transitions.begin(), end = transitions.end();
  mTransitions.reserve(transitions.size());
  for (; it != end;) {
    const rstl::pair< rstl::pair< uint, uint >, rstl::rc_ptr< IMetaTrans > > entry(
        rstl::pair< uint, uint >(it->GetFromAnimIndex(), it->GetToAnimIndex()), it->GetMetaTrans());
    mTransitions.push_back(entry);
    ++it;
  }
  rstl::sort_by_key(mTransitions, uint_comparer());

  rstl::vector< CHalfTransition >::const_iterator halfIt = halfTransitions.begin(),
                                                halfEnd = halfTransitions.end();
  mHalfTransitions.reserve(halfTransitions.size());
  for (; halfIt != halfEnd;) {
    const rstl::pair< uint, rstl::rc_ptr< IMetaTrans > > entry(
        halfIt->GetPrimitiveIndex(), halfIt->GetMetaTrans());
    mHalfTransitions.push_back(entry);
    ++halfIt;
  }
  rstl::sort_by_key(mHalfTransitions, uint_comparer());
}

const rstl::rc_ptr< IMetaTrans >& CTransitionDatabaseGame::GetMetaTrans(uint from, uint to) const {
  AUTO(it, rstl::find_by_key(mTransitions, rstl::pair< uint, uint >(from, to), uint_comparer()));
  if (it != mTransitions.end()) {
    return it->second;
  }
  AUTO(halfIt, rstl::find_by_key(mHalfTransitions, to, uint_comparer()));
  if (halfIt != mHalfTransitions.end()) {
    return halfIt->second;
  }
  return mDefaultTrans;
}
