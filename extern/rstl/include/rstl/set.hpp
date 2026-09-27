#ifndef _RSTL_SET
#define _RSTL_SET

#include "types.h"

#include "rstl/RstlVersions.h"
#include "rstl/red_black_tree.hpp"
#include "rstl/allocator.hpp"

namespace rstl {
template < typename T, typename Cmp = less< T >, typename Alloc = rmemory_allocator >
class set : public red_black_tree< T, T, false, identity< T >, Cmp, Alloc > {
public:
  typedef T value_type;

private:
  typedef red_black_tree< T, value_type, false, identity< T >, Cmp, Alloc > rep_type;

public:
  typedef typename rep_type::iterator iterator;
  typedef typename rep_type::const_iterator const_iterator;

  ~set() {}

  explicit set(const Cmp& cmp = Cmp(), const Alloc& alloc = Alloc())
  : rep_type(identity< T >(), cmp, alloc) {}

  set(CInputStream& in, const Cmp& cmp = Cmp(), const Alloc& alloc = Alloc());

  set(const set& other);
};

template < typename T, typename Cmp, typename Alloc >
#if RSTL_VERSION < RSTL_GM8E_02
inline
#endif
    set< T, Cmp, Alloc >::set(const set& other)
: rep_type(other) {
}

typedef set< char, char > unk_set;
CHECK_SIZEOF(unk_set, 0x14)

}

#endif // _RSTL_SET
