#ifndef _RSTL_POINTER_ITERATOR
#define _RSTL_POINTER_ITERATOR

#include "stddef.h"
#include "types.h"

#include "rstl/construct.hpp"
#include "rstl/iterator.hpp"

namespace rstl {

template < typename T, typename Vec, typename Alloc >
class const_pointer_iterator {
public:
  typedef long difference_type;
  typedef random_access_iterator_tag iterator_category;
  typedef T value_type;

  const_pointer_iterator() : current(nullptr) {}
  const_pointer_iterator(const T* begin) : current(const_cast< T* >(begin)) {}
  const_pointer_iterator(const Vec* owner, const T* begin) : current(const_cast< T* >(begin)) {}
  const_pointer_iterator& operator++() {
    return *this += 1;
  }
  const_pointer_iterator operator++(int) { return const_pointer_iterator(this->current++); }
  const_pointer_iterator& operator--() {
    --this->current;
    return *this;
  }
  const_pointer_iterator operator--(int) { return const_pointer_iterator(this->current--); }
  const_pointer_iterator& operator+=(int v) {
    this->current += v;
    return *this;
  }
  const_pointer_iterator& operator-=(int v) {
    this->current -= v;
    return *this;
  }
  const_pointer_iterator operator+(int v) const {
    return const_pointer_iterator(this->current + v);
  }
  const_pointer_iterator operator-(int v) const {
    return const_pointer_iterator(this->current - v);
  }
  difference_type operator-(const const_pointer_iterator& other) const {
    return this->current - other.current;
  }
  const T* get_pointer() const { return current; }
  const T& operator*() const { return *current; }
  const T* operator->() const { return current; }
  bool operator==(const const_pointer_iterator& other) const { return current == other.current; }
  bool operator!=(const const_pointer_iterator& other) const { return current != other.current; }
  bool operator<(const const_pointer_iterator& other) const { return current < other.current; }
  bool operator>(const const_pointer_iterator& other) const { return current > other.current; }
  bool operator<=(const const_pointer_iterator& other) const { return current <= other.current; }
  bool operator>=(const const_pointer_iterator& other) const { return current >= other.current; }

protected:
  T* current;
};

template < typename T, typename Vec, typename Alloc >
class pointer_iterator : public const_pointer_iterator< T, Vec, Alloc > {
  typedef const_pointer_iterator< T, Vec, Alloc > base;

public:
  typedef typename base::difference_type difference_type;
  typedef typename base::iterator_category iterator_category;
  typedef typename base::value_type value_type;

  pointer_iterator() : base(nullptr) {}
  pointer_iterator(T* begin) : base(begin) {}
  pointer_iterator(Vec* owner, T* begin) : base(owner, begin) {}
  T* get_pointer() const { return this->current; }
  T& operator*() const { return *get_pointer(); }
  // TODO map says const, but breaks CScriptMazeNode::GenerateObjects
  T* operator->() { return this->current; }
  pointer_iterator& operator++() {
    ++this->current;
    return *this;
  }
  pointer_iterator operator++(int) { return pointer_iterator(this->current++); }
  pointer_iterator& operator--() {
    --this->current;
    return *this;
  }
  pointer_iterator operator--(int) { return pointer_iterator(this->current--); }
  pointer_iterator& operator+=(int v) {
    this->current += v;
    return *this;
  }
  pointer_iterator& operator-=(int v) {
    this->current -= v;
    return *this;
  }
  pointer_iterator operator+(int v) const { return pointer_iterator(this->current) += v; }
  pointer_iterator operator-(int v) const { return pointer_iterator(this->current - v); }
  difference_type operator-(const base& other) const { return this->current - other.get_pointer(); }
};

template < typename T >
struct const_counting_iterator {
  const T* ptr;
  int count;

  const_counting_iterator(const T* ptr, int count) : ptr(ptr), count(count) {}

  const T& operator*() const { return *ptr; }

  const_counting_iterator& operator++() {
    ++this->count;
    return *this;
  }
};

} // namespace rstl

#endif // _RSTL_POINTER_ITERATOR
