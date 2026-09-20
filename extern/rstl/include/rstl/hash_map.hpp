#ifndef _RSTL_HASH_MAP
#define _RSTL_HASH_MAP

#include "types.h"

#include "rstl/list.hpp"
#include "rstl/pair.hpp"
#include "rstl/red_black_tree.hpp"
#include "rstl/allocator.hpp"
#include "rstl/vector.hpp"

namespace rstl {
template < typename K, typename P, int unk, typename Select, typename Hash, typename Equal,
           typename Alloc = rmemory_allocator >
class hash_table {
public:
  int size() const { return x.size(); }
private:
  rstl::vector< rstl::list< P, Alloc > /*::iterator*/, Alloc > x;
};

template < typename K, typename V, typename Hash, typename Equal,
           typename Alloc = rmemory_allocator >
class hash_map {
  typedef rstl::pair< K, V > Pair;
public:
  int size() const { return table.size(); }

private:
  hash_table< K, Pair, 0, select1st< Pair >, Hash, Equal, Alloc > table;
};
} // namespace rstl

#endif // _RSTL_HASH_MAP
