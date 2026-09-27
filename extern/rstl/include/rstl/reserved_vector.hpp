#ifndef _RSTL_RESERVED_VECTOR
#define _RSTL_RESERVED_VECTOR

#include "types.h"

#include "rstl/construct.hpp"
#include "rstl/pointer_iterator.hpp"

class CInputStream;
class COutputStream;

namespace rstl {
template < typename T, int N >
class reserved_vector {
  int mCount;
  ALIGNAS(T) uchar mData[N * sizeof(T)];

public:
  // typedef pointer_iterator< T, reserved_vector< T, N >, void > iterator;
  // typedef const_pointer_iterator< T, reserved_vector< T, N >, void > const_iterator;
  typedef T* iterator;
  typedef const T* const_iterator;
  typedef T value_type;

  inline iterator begin() { return iterator(data()); }
  inline const_iterator begin() const { return const_iterator(data()); }
  inline iterator end() { return iterator(data() + mCount); }
  inline const_iterator end() const { return const_iterator(data() + mCount); }

  reserved_vector() : mCount(0) {}
  explicit reserved_vector(const T& value) : mCount(N) { uninitialized_fill_n(data(), N, value); }
  explicit reserved_vector(int count, const T& value) : mCount(count) {
    uninitialized_fill_n(data(), count, value);
  };
  reserved_vector(const reserved_vector& other) : mCount(other.mCount) {
    uninitialized_copy_n(other.data(), mCount, data());
  }
  reserved_vector(CInputStream& in);

  reserved_vector& operator=(const reserved_vector& other) {
    if (this == &other) {
      return *this;
    }
    clear();
    uninitialized_copy(other.data(), other.data() + other.size(), data());
    mCount = other.mCount;
    return *this;
  }

  void clear() {
    T* ptr = data();
    for (int i = 0; i < mCount; ++i) {
      destroy(&ptr[i]);
    }
    mCount = 0;
  }

  ~reserved_vector() { clear(); }

  void push_back(const T& in) {
    construct(data() + mCount, in);
    ++mCount;
  }

  void pop_back() {
    destroy(&data()[mCount - 1]);
    --mCount;
  }

  inline T* data() { return reinterpret_cast< T* >(mData); }
  inline const T* data() const { return reinterpret_cast< const T* >(mData); }
  inline bool empty() const { return size() == 0; }
  inline int size() const { return mCount; }
  inline int capacity() const { return N; }
  inline T& front() { return data()[0]; }
  inline const T& front() const { return data()[0]; }
  inline T& back() { return at(mCount - 1); }
  inline const T& back() const { return at(mCount - 1); }
  inline T& operator[](int idx) { return data()[idx]; }
  inline const T& operator[](int idx) const { return data()[idx]; }
  inline T& at(int idx) { return data()[idx]; }
  inline const T& at(int idx) const { return data()[idx]; }
  iterator erase(iterator it) {
    if (it >= begin() && it < end()) {
      for (iterator j = it; j < end() - 1; ++j) {
        *j = *(j + 1);
      }
      destroy(end() - 1);
      --mCount;
      return it;
    }
    return end();
  }

  void resize(int count, const T& item = T()) {
    if (mCount < count) {
      uninitialized_fill_n(data() + mCount, count - mCount, item);
      mCount = count;
    }
  }

  void PutTo(COutputStream& out) const;
};

// template < typename T, int N >
// reserved_vector< T, N >::reserved_vector(int count, const T& value) : x0_count(count) {
//   uninitialized_fill_n(data(), count, value);
// }

} // namespace rstl

#endif // _RSTL_RESERVED_VECTOR
