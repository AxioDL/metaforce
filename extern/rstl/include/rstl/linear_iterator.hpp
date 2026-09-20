#ifndef _RSTL_LINEAR_ITERATOR
#define _RSTL_LINEAR_ITERATOR

#include "rstl/iterator.hpp"

namespace rstl {
template < typename T, typename Container, typename Alloc >
class const_linear_iterator {
public:
  typedef T value_type;
  typedef long difference_type;
  typedef random_access_iterator_tag iterator_category;

  const_linear_iterator(const Container* owner, int index) : x0_owner(owner), x4_index(index) {}

  const T& operator*() const { return (*x0_owner)[x4_index]; }
  const_linear_iterator& operator++() {
    ++x4_index;
    return *this;
  }
  const_linear_iterator& operator+=(int count) {
    x4_index += count;
    return *this;
  }
  const_linear_iterator& operator-=(int count) {
    x4_index -= count;
    return *this;
  }
  const_linear_iterator operator+(int count) const {
    const_linear_iterator result = *this;
    result += count;
    return result;
  }
  const_linear_iterator operator-(int count) const {
    const_linear_iterator result = *this;
    result -= count;
    return result;
  }
  difference_type operator-(const const_linear_iterator& other) const {
    return x4_index - other.x4_index;
  }
  bool operator==(const const_linear_iterator& other) const {
    return x0_owner == other.x0_owner && x4_index == other.x4_index;
  }
  bool operator!=(const const_linear_iterator& other) const {
    return x0_owner != other.x0_owner || x4_index != other.x4_index;
  }

private:
  const Container* x0_owner;
  int x4_index;
};
} // namespace rstl

#endif // _RSTL_LINEAR_ITERATOR
