#ifndef _RSTL_STACK
#define _RSTL_STACK

#include "rstl/list.hpp"

namespace rstl {
template < typename T, typename Container = list< T > >
class stack {
public:
  bool empty() const { return mContainer.empty(); }
  int size() const { return mContainer.size(); }
  T& top() { return mContainer.front(); }
  const T& top() const { return mContainer.front(); }
  void push(const T& value) { mContainer.push_front(value); }
  void pop() { mContainer.pop_front(); }
  void clear() { mContainer.clear(); }

private:
  Container mContainer;
};
} // namespace rstl

#endif // _RSTL_STACK
