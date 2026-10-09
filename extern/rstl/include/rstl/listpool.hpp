#ifndef _RSTL_LISTPOOL
#define _RSTL_LISTPOOL

#include "rstl/allocator.hpp"

#include <stddef.h>

namespace rstl {
template < typename T, int N, typename Alloc = rmemory_allocator >
class listpool : private Alloc {
public:
  listpool(const Alloc& alloc = Alloc());
  ~listpool();

private:
  struct node;
  node* mStart;
  node* mEnd;
  node* mEmpty_prev;
  node* mEmpty_next;
  size_t mEmptyCount;
  size_t mCount;
};
} // namespace rstl

#endif // _RSTL_LISTPOOL
