#ifndef _RSTL_MAP
#define _RSTL_MAP

#include "types.h"

#include "rstl/pair.hpp"
#include "rstl/red_black_tree.hpp"
#include "rstl/allocator.hpp"
namespace rstl {
template < typename K, typename V, typename Cmp = less< K >, typename Alloc = rmemory_allocator >
class map : public red_black_tree< K, pair< K, V >, false, select1st< pair< K, V > >, Cmp, Alloc > {
public:
  typedef pair< K, V > value_type;

private:
  typedef red_black_tree< K, value_type, false, select1st< value_type >, Cmp, Alloc >
      rep_type;

public:
  typedef typename rep_type::iterator iterator;
  typedef typename rep_type::const_iterator const_iterator;

  explicit map(const Cmp& cmp = Cmp(), const Alloc& alloc = Alloc())
  : rep_type(select1st< value_type >(), cmp, alloc) {}
  map(CInputStream& in, const Cmp& cmp = Cmp(), const Alloc& alloc = Alloc())
  : rep_type(in, select1st< value_type >(), cmp, alloc) {}
  ~map() {}
};

typedef map< char, char > unk_map;
CHECK_SIZEOF(unk_map, 0x14)
} // namespace rstl

#endif // _RSTL_MAP
