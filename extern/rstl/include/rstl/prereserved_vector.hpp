#ifndef _RSTL_PRERESERVED_VECTOR
#define _RSTL_PRERESERVED_VECTOR

#include <types.h>

template < typename T >
class prereserved_vector {
public:
  prereserved_vector() : mSize(0), mData(nullptr) {}

  int size() const { return mSize; }
  void set_size(int size) { mSize = size; }
  void set_data(T* data) { mData = data; }

  T& operator[](int idx) { return mData[idx]; }
  const T& operator[](int idx) const { return mData[idx]; }
  T& back() { return mData[mSize - 1]; }
  const T& back() const { return mData[mSize - 1]; }

private:
  int mSize;
  T* mData;
};

#endif // _RSTL_PRERESERVED_VECTOR
