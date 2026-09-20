#ifndef _RSTL_STACK
#define _RSTL_STACK

#include "rstl/list.hpp"

namespace rstl {
template < typename T, typename Container = list< T > >
class stack {
public:
  bool empty() const { return x0_container.empty(); }
  int size() const { return x0_container.size(); }
  T& top() { return x0_container.front(); }
  const T& top() const { return x0_container.front(); }
  void push(const T& value) { x0_container.push_front(value); }
  void pop() { x0_container.pop_front(); }
  void clear() { x0_container.clear(); }

private:
  Container x0_container;
};
} // namespace rstl

#endif // _RSTL_STACK
