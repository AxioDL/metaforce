#ifndef _RSTL_VECTOR
#define _RSTL_VECTOR

#include "types.h"

#include "rstl/RstlVersions.h"
#include "rstl/allocator_auto_ptr.hpp"
#include "rstl/iterator.hpp"
#include "rstl/pointer_iterator.hpp"
#include "rstl/allocator.hpp"

class CInputStream;
class COutputStream;

namespace rstl {

template < typename T, typename Alloc = rmemory_allocator >
class vector
#if RSTL_VERSION >= RSTL_R3IJ
: private Alloc
#endif
{
public:
#if RSTL_VERSION < RSTL_R3IJ
  Alloc mAllocator;
#endif
  int mCount;
  int mCapacity;
  T* mItems;

public:
  typedef Alloc allocator_type;
  typedef pointer_iterator< T, vector< T, Alloc >, Alloc > iterator;
  typedef const_pointer_iterator< T, vector< T, Alloc >, Alloc > const_iterator;
  typedef int size_type;
  typedef T value_type;

  iterator begin() { return iterator(this, data()); }
  const_iterator begin() const { return const_iterator(this, data()); }
  iterator end() {
    T* const end = data() + mCount;
    return iterator(end);
  }
  const_iterator end() const { return const_iterator(this, data() + size()); }
  vector(const Alloc& alloc = Alloc())
  :
#if RSTL_VERSION >= RSTL_R3IJ
    Alloc(alloc),
#else
    mAllocator(alloc),
#endif
    mCount(0), mCapacity(0), mItems(nullptr) {}
  vector(int count) : mCount(0), mCapacity(0), mItems(0) { reserve(count); }
  vector(int count, const T& v) : mCount(count), mCapacity(count) {
    Alloc::allocate(mItems, mCount);
    uninitialized_fill_n(mItems, count, v);
  }
  vector(int count, const T& v, const Alloc& alloc)
  :
#if RSTL_VERSION >= RSTL_R3IJ
    Alloc(alloc),
#else
    mAllocator(alloc),
#endif
    mCount(count), mCapacity(count) {
    Alloc::allocate(mItems, mCount);
    uninitialized_fill_n(mItems, count, v);
  }

  vector(const vector& other)
  :
#if RSTL_VERSION >= RSTL_R3IJ
    Alloc(other),
#else
    mAllocator(other.mAllocator),
#endif
    mCount(other.mCount)
  , mCapacity(other.mCapacity) {
    if (other.mCount == 0 && other.mCapacity == 0) {
      mItems = nullptr;
    } else {
      Alloc::allocate(mItems, mCapacity);
      uninitialized_copy_n(other.mItems, mCount, mItems);
    }
  }
  vector(CInputStream& in, const Alloc& alloc = Alloc());
  template < typename It >
  vector(It first, It last, const Alloc& alloc = Alloc())
  :
#if RSTL_VERSION >= RSTL_R3IJ
    Alloc(alloc),
#else
    mAllocator(alloc),
#endif
    mCount(0), mCapacity(0) {
    mCount = mCapacity = rstl::distance(first, last);
    Alloc::allocate(mItems, mCount);
    rstl::uninitialized_copy(first, last, mItems);
  }
  ~vector() {
    destroy(begin(), end());
    Alloc::deallocate(mItems);
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
    if (mCount >= mCapacity) {
      reserve(mCapacity != 0 ? mCapacity * 2 : 4);
    }
    rstl::construct(mItems + mCount, in);
    ++mCount;
  }

  void pop_back() {
    destroy(mItems + mCount - 1);
    --mCount;
  }

  inline vector& operator=(const vector& other);

  void clear() {
    destroy(begin(), end());
    mCount = 0;
  }

  T* data() { return mItems; }
  const T* data() const { return mItems; }
  int size() const { return mCount; }
  bool empty() const { return mCount == 0; }
  int capacity() const { return mCapacity; }
  T& at(int idx) { return mItems[idx]; }
  const T& at(int idx) const { return mItems[idx]; }
  T& front() { return at(0); }
  const T& front() const { return at(0); }
  T& back() { return at(mCount - 1); }
  const T& back() const { return at(mCount - 1); }
  T& operator[](int idx) { return mItems[idx]; }
  const T& operator[](int idx) const { return mItems[idx]; }

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
  if (mCount != size) {
    if (size > mCount) {
      reserve(size);
      uninitialized_fill_n(mItems + mCount, size - mCount, in);
    } else {
      destroy(begin() + size, end());
    }
    mCount = size;
  }
}

template < typename T, typename Alloc >
void vector< T, Alloc >::reserve(int newSize) {
  if (newSize <= mCapacity) {
    return;
  }

  T* newData;
  Alloc::allocate(newData, newSize);
  uninitialized_copy(begin(), end(), newData);
  destroy(mItems, mItems + mCount);
  Alloc::deallocate(mItems);
  mItems = newData;
  mCapacity = newSize;
}

template < typename T, typename Alloc >
typename vector< T, Alloc >::iterator vector< T, Alloc >::insert(iterator it, const T& value) {
  typename iterator::difference_type diff = it.operator->() - mItems;
  const_counting_iterator< T > in(&value, 0);
  insert_into(it, 1, in);
  return iterator(mItems) + diff;
}

template < typename T, typename Alloc >
template < typename from_iterator >
void vector< T, Alloc >::insert(iterator it, from_iterator begin, from_iterator end) {
  insert_into(it, rstl::distance(begin, end), begin);
}

template < typename T, typename Alloc >
template < typename In >
void vector< T, Alloc >::insert_into(iterator at, int n, In in) {
  T* oldData = mItems;
  In input = in;

  if (mCount + n <= mCapacity) {
    long atIdx = at - begin();
    int moveCount = mCount - atIdx;
    int i = moveCount - 1;
    for (; i >= 0; --i) {
      construct(oldData + atIdx + n + i, data()[atIdx + i]);
#if RSTL_VERSION >= RSTL_GM8P_00
      T* const item = oldData + atIdx + i;
      destroy(item);
#else
      destroy(oldData + atIdx + i);
#endif
    }
    for (i = 0; i < n; ++input, ++i) {
      construct(oldData + atIdx + i, *input);
    }
    mCount += n;
  } else {
    int newCapacity = mCapacity ? mCapacity * 2 : 4;
    while (newCapacity < mCount + n) {
      newCapacity *= 2;
    }

    T* newData;
    Alloc::allocate(newData, newCapacity);
    long atIdx = at - begin();
    // The const alias makes MWCC retain a separate allocation-base register on PAL.
#if RSTL_VERSION >= RSTL_GM8P_00
    T* const newItems = newData;
#else
    T* newItems = newData;
#endif
    int newIdx = 0;
    for (int i = 0; i < atIdx; ++newIdx, ++i) {
      construct(newItems + newIdx, data()[i]);
    }
    for (int i = 0; i < n; ++input, ++newIdx, ++i) {
      construct(newItems + newIdx, *input);
    }
    for (int i = atIdx; i < size(); ++newIdx, ++i) {
      construct(newItems + newIdx, data()[i]);
    }

    destroy(oldData, oldData + size());
    Alloc::deallocate(mItems);
    mItems = newData;
    mCapacity = newCapacity;
    mCount += n;
  }
}

template < typename T, typename Alloc >
inline vector< T, Alloc >& vector< T, Alloc >::operator=(const vector< T, Alloc >& other) {
  if (this == &other)
    return *this;
  clear();
  if (other.size() == 0) {
    Alloc::deallocate(mItems);
    mCount = 0;
    mCapacity = 0;
    mItems = nullptr;
  } else {
    reserve(other.size());
    uninitialized_copy(other.mItems, other.mItems + other.mCount, data());
    mCount = other.mCount;
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

  for (iterator it = last, moved = iterator(mItems + tmp); it != end(); ++moved, ++newCount, ++it) {
    construct(&*moved, *it);
    destroy(&*it);
  }
  mCount = newCount;

  return first;
}
} // namespace rstl

#endif // _RSTL_VECTOR
