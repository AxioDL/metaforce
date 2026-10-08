#ifndef _TAVERAGE
#define _TAVERAGE

#include "types.h"

#include "rstl/optional_object.hpp"
#include "rstl/vector.hpp"

template < typename T >
T GetAverageValue(const T* ptr, int count) {
  T ret = *ptr;
  for (const T* cur = ptr + 1; cur < ptr + count; ++cur) {
    ret += *cur;
  }

  const float scale = 1.f / count;
  return ret * scale;
}

template < typename T >
class TAverage : rstl::vector< T > {
public:
  TAverage() {}
  TAverage(int capacity, const T& value = T());

  void AddValue(const T& value);
  rstl::optional_object< T > GetAverage() const {
    if (this->empty()) {
      return rstl::optional_object_null();
    } else {
      return GetAverageValue(this->data(), this->size());
    }
  }
};

template < typename T >
TAverage< T >::TAverage(int capacity, const T& value) {
  this->resize(capacity, value);
}

template < typename T >
void TAverage< T >::AddValue(const T& value) {
  if (this->size() == this->capacity()) {
    // TODO ?
    this->mCount -= 1;
  }
  this->insert(this->begin(), value);
}

#endif // _TAVERAGE
