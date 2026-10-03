#ifndef _RSTL_RESERVED_VECTOR
#define _RSTL_RESERVED_VECTOR

#include "types.h"

#include "rstl/construct.hpp"
#include "rstl/pointer_iterator.hpp"

class CInputStream;
class COutputStream;

namespace rstl {
#if RSTL_VERSION >= RSTL_R3IJ
// Scalar fill and destruction behavior recovered from Trilogy's input filters.
template < typename T >
struct reserved_vector_traits {
  typedef const T& fill_type;
  typedef const T& push_type;
  enum { trivial_destructor = false };
  static void fill(T* dest, int count, const T& value) { uninitialized_fill_n(dest, count, value); }
};

template <>
struct reserved_vector_traits< bool > {
  typedef bool fill_type;
  typedef bool push_type;
  enum { trivial_destructor = true };
  static void fill(bool* dest, int count, bool value) {
    for (int i = 0; i < count; ++i, ++dest) {
      *dest = value;
    }
  }
};

template <>
struct reserved_vector_traits< uchar > {
  typedef uchar fill_type;
  typedef const uchar& push_type;
  enum { trivial_destructor = true };
  static void fill(uchar* dest, int count, uchar value) {
    for (int i = 0; i < count; ++i, ++dest) {
      *dest = value;
    }
  }
};

template <>
struct reserved_vector_traits< float > {
  typedef float fill_type;
  typedef float push_type;
  enum { trivial_destructor = true };
  static void fill(float* dest, int count, float value) {
    for (int i = 0; i < count; ++i) {
      dest[i] = value;
    }
  }
};
#endif

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
#if RSTL_VERSION >= RSTL_R3IJ
  explicit reserved_vector(int count) : mCount(count) {
    int i;
    T* dest = data();
    for (i = 0; i < count; ++i, ++dest) {
      new (dest) T;
    }
  }
#endif
#if RSTL_VERSION >= RSTL_R3ME_00
  explicit reserved_vector(typename reserved_vector_traits< T >::fill_type value) : mCount(N) {
    reserved_vector_traits< T >::fill(data(), N, value);
  }
  explicit reserved_vector(int count, typename reserved_vector_traits< T >::fill_type value)
  : mCount(count) {
    reserved_vector_traits< T >::fill(data(), count, value);
  };
#else
  explicit reserved_vector(const T& value) : mCount(N) { uninitialized_fill_n(data(), N, value); }
  explicit reserved_vector(int count, const T& value) : mCount(count) {
    uninitialized_fill_n(data(), count, value);
  };
#endif
  reserved_vector(const reserved_vector& other) : mCount(other.mCount) {
    uninitialized_copy_n(other.data(), mCount, data());
  }
  reserved_vector(CInputStream& in);

  reserved_vector& operator=(const reserved_vector& other) {
    if (this == &other) {
      return *this;
    }
#if RSTL_VERSION >= RSTL_R3ME_00
    if (!reserved_vector_traits< T >::trivial_destructor) {
      T* ptr = data();
      for (int i = 0; i < mCount; ++i) {
        destroy(&ptr[i]);
      }
    }
#else
    clear();
#endif
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

  ~reserved_vector() {
#if RSTL_VERSION >= RSTL_R3IJ
    if (!reserved_vector_traits< T >::trivial_destructor) {
      T* ptr = data();
      for (int i = 0; i < mCount; ++i) {
        destroy(&ptr[i]);
      }
    }
#else
    clear();
#endif
  }

#if RSTL_VERSION >= RSTL_R3ME_00
  void push_back(typename reserved_vector_traits< T >::push_type in) {
#else
  void push_back(const T& in) {
#endif
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
#if RSTL_VERSION >= RSTL_R3ME_00
        destroy(j);
        construct(j, *(j + 1));
#else
        *j = *(j + 1);
#endif
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
