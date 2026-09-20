#ifndef _RSTL_PAIR
#define _RSTL_PAIR

#include "types.h"

#include "rstl/functional.hpp"

class CInputStream;
namespace rstl {
template < typename L, typename R >
class pair {
public:
  pair() {}
  pair(const L& first, const R& second) : first(first), second(second) {}
  pair(CInputStream& in);

  bool operator==(const pair& other) const {
    return first == other.first && second == other.second;
  }

  bool operator!=(const pair& other) const {
    return first != other.first || second != other.second;
  }

  bool operator<(const pair& other) const {
    return first < other.first || (first == other.first && second < other.second);
  }

  L first;
  R second;
};

template < typename P >
struct select1st : unary_function< P, P > {
  const P& operator()(const P& it) const { return it; }
};

template < typename K, typename V >
struct select1st< pair< K, V > > : unary_function< pair< K, V >, K > {
  typedef K value_type;

  const K& operator()(const pair< K, V >& it) const { return it.first; }
};

} // namespace rstl

#endif // _RSTL_PAIR
