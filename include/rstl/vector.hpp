#ifndef _RSTL_VECTOR
#define _RSTL_VECTOR

#include "types.h"

#include "rstl/allocator_auto_ptr.hpp"
#include "rstl/iterator.hpp"
#include "rstl/pointer_iterator.hpp"
#include "rstl/allocator.hpp"
class CInputStream;
class COutputStream;

namespace rstl {

template < typename T, typename Alloc = rmemory_allocator >
class vector {
public:
  Alloc x0_allocator;
  int x4_count;
  int x8_capacity;
  T* xc_items;

public:
  typedef Alloc allocator_type;
  typedef pointer_iterator< T, vector< T, Alloc >, Alloc > iterator;
  typedef const_pointer_iterator< T, vector< T, Alloc >, Alloc > const_iterator;
  typedef int size_type;
  typedef T value_type;

  iterator begin() { return iterator(this, data()); }
  const_iterator begin() const { return const_iterator(this, data()); }
  iterator end() {
    T* const end = data() + x4_count;
    return iterator(end);
  }
  const_iterator end() const { return const_iterator(this, data() + size()); }
  vector(const Alloc& alloc = Alloc())
  : x0_allocator(alloc), x4_count(0), x8_capacity(0), xc_items(nullptr) {}
  vector(int count) : x4_count(0), x8_capacity(0), xc_items(0) { reserve(count); }
  vector(int count, const T& v) : x4_count(count), x8_capacity(count) {
    x0_allocator.allocate(xc_items, x4_count);
    uninitialized_fill_n(xc_items, count, v);
  }
  vector(int count, const T& v, const Alloc& alloc)
  : x0_allocator(alloc), x4_count(count), x8_capacity(count) {
    x0_allocator.allocate(xc_items, x4_count);
    uninitialized_fill_n(xc_items, count, v);
  }

  vector(const vector& other)
  : x0_allocator(other.x0_allocator)
  , x4_count(other.x4_count)
  , x8_capacity(other.x8_capacity) {
    if (other.x4_count == 0 && other.x8_capacity == 0) {
      xc_items = nullptr;
    } else {
      x0_allocator.allocate(xc_items, x8_capacity);
      uninitialized_copy_n(other.xc_items, x4_count, xc_items);
    }
  }
  vector(CInputStream& in, const Alloc& alloc = Alloc());
  template < typename It >
  vector(It first, It last, const Alloc& alloc = Alloc())
  : x0_allocator(alloc), x4_count(0), x8_capacity(0) {
    x4_count = x8_capacity = rstl::distance(first, last);
    x0_allocator.allocate(xc_items, x4_count);
    rstl::uninitialized_copy(first, last, xc_items);
  }
  ~vector() {
    destroy(begin(), end());
    x0_allocator.deallocate(xc_items);
  }

  inline void resize(int size, const T& in = T());
  inline void assign(int size, const T& in = T());
  void reserve(int size);
  inline iterator insert(iterator it, const T& value);

  template < typename from_iterator >
  inline void insert(iterator it, from_iterator begin, from_iterator end);

  // iterator erase(iterator it);
  // iterator erase(iterator first, iterator last);

  iterator erase(iterator it);
  iterator erase(iterator first, iterator last);

  void push_back(const T& in) {
    if (x4_count >= x8_capacity) {
      reserve(x8_capacity != 0 ? x8_capacity * 2 : 4);
    }
    rstl::construct(xc_items + x4_count, in);
    ++x4_count;
  }

  void pop_back() {
    destroy(xc_items + x4_count - 1);
    --x4_count;
  }

  inline vector& operator=(const vector& other);

  void clear() {
    destroy(begin(), end());
    x4_count = 0;
  }

  T* data() { return xc_items; }
  const T* data() const { return xc_items; }
  int size() const { return x4_count; }
  bool empty() const { return x4_count == 0; }
  int capacity() const { return x8_capacity; }
  T& at(int idx) { return xc_items[idx]; }
  const T& at(int idx) const { return xc_items[idx]; }
  T& front() { return at(0); }
  const T& front() const { return at(0); }
  T& back() { return at(x4_count - 1); }
  const T& back() const { return at(x4_count - 1); }
  T& operator[](int idx) { return xc_items[idx]; }
  const T& operator[](int idx) const { return xc_items[idx]; }

  void PutTo(COutputStream& out) const;

protected:
  template < typename In >
  inline void insert_into(iterator at, int n, In in);
};

template < typename T, typename Alloc >
void vector< T, Alloc >::assign(int size, const T& in) {
  clear();
  reserve(size);
  for (int i = 0; i < size; ++i) {
    push_back(in);
  }
}

template < typename T, typename Alloc >
inline void vector< T, Alloc >::resize(int size, const T& in) {
  if (x4_count != size) {
    if (size > x4_count) {
      reserve(size);
      uninitialized_fill_n(xc_items + x4_count, size - x4_count, in);
    } else {
      destroy(begin() + size, end());
    }
    x4_count = size;
  }
}

template < typename T, typename Alloc >
void vector< T, Alloc >::reserve(int newSize) {
  if (newSize <= x8_capacity) {
    return;
  }

  T* newData;
  x0_allocator.allocate(newData, newSize);
  uninitialized_copy(begin(), end(), newData);
  destroy(xc_items, xc_items + x4_count);
  x0_allocator.deallocate(xc_items);
  xc_items = newData;
  x8_capacity = newSize;
}

template < typename T, typename Alloc >
typename vector< T, Alloc >::iterator vector< T, Alloc >::insert(iterator it, const T& value) {
  typename iterator::difference_type diff = it.operator->() - xc_items;
  const_counting_iterator< T > in(&value, 0);
  insert_into(it, 1, in);
  return iterator(xc_items) + diff;
}

template < typename T, typename Alloc >
template < typename from_iterator >
void vector< T, Alloc >::insert(iterator it, from_iterator begin, from_iterator end) {
  insert_into(it, rstl::distance(begin, end), begin);
}

template < typename T, typename Alloc >
template < typename In >
void vector< T, Alloc >::insert_into(iterator at, int n, In in) {
  T* oldData = xc_items;
  In input = in;

  if (x4_count + n <= x8_capacity) {
    long atIdx = at - begin();
    int moveCount = x4_count - atIdx;
    int i = moveCount - 1;
    for (; i >= 0; --i) {
      construct(oldData + atIdx + n + i, data()[atIdx + i]);
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
      T* const item = oldData + atIdx + i;
      destroy(item);
#else
      destroy(oldData + atIdx + i);
#endif
    }
    for (i = 0; i < n; ++input, ++i) {
      construct(oldData + atIdx + i, *input);
    }
    x4_count += n;
  } else {
    int newCapacity = x8_capacity ? x8_capacity * 2 : 4;
    while (newCapacity < x4_count + n) {
      newCapacity *= 2;
    }

    T* newData;
    x0_allocator.allocate(newData, newCapacity);
    long atIdx = at - begin();
    int newIdx = 0;
    for (int i = 0; i < atIdx; ++newIdx, ++i) {
      construct(newData + newIdx, data()[i]);
    }
    for (int i = 0; i < n; ++input, ++newIdx, ++i) {
      construct(newData + newIdx, *input);
    }
    for (int i = atIdx; i < size(); ++newIdx, ++i) {
      construct(newData + newIdx, data()[i]);
    }

    destroy(oldData, oldData + size());
    x0_allocator.deallocate(xc_items);
    xc_items = newData;
    x8_capacity = newCapacity;
    x4_count += n;
  }
}

template < typename T, typename Alloc >
inline vector< T, Alloc >& vector< T, Alloc >::operator=(const vector< T, Alloc >& other) {
  if (this == &other)
    return *this;
  clear();
  if (other.size() == 0) {
    x0_allocator.deallocate(xc_items);
    x4_count = 0;
    x8_capacity = 0;
    xc_items = nullptr;
  } else {
    reserve(other.size());
    uninitialized_copy(other.xc_items, other.xc_items + other.x4_count, data());
    x4_count = other.x4_count;
  }
  return *this;
}

template < typename T, typename Alloc >
inline typename vector< T, Alloc >::iterator vector< T, Alloc >::erase(iterator it) {
  return erase(it, it + 1);
}

template < typename T, typename Alloc >
inline typename vector< T, Alloc >::iterator vector< T, Alloc >::erase(iterator first, iterator last) {
  destroy(first, last);

  const typename iterator::difference_type tmp = first - begin();

  int newCount = tmp;

  for (iterator it = last, moved = iterator(xc_items + tmp); it != end(); ++moved, ++newCount, ++it) {
    construct(&*moved, *it);
    destroy(&*it);
  }
  x4_count = newCount;

  return first;
}
} // namespace rstl

#endif // _RSTL_VECTOR
