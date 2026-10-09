#ifndef _TRESERVEDAVERAGE
#define _TRESERVEDAVERAGE

#include "types.h"

#include "Kyoto/TAverage.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"

template < typename T, int N >
class TReservedAverage : public rstl::reserved_vector< T, N > {
public:
  TReservedAverage() {}
  TReservedAverage(const T& value) { this->resize(N, value); }
  void AddValue(const T& value);
  rstl::optional_object< T > GetAverage() const;
  rstl::optional_object< T > GetEntry(int idx) const;
};

template < typename T, int N >
/* inline */ void TReservedAverage< T, N >::AddValue(const T& value) {
  if (this->size() < this->capacity()) {
    this->push_back(value);
  }
  for (int i = this->size() - 1; i > 0; --i) {
    this->operator[](i) = this->operator[](i - 1u);
  }
  this->operator[](0) = value;
}

template < typename T, int N >
inline rstl::optional_object< T > TReservedAverage< T, N >::GetAverage() const {
  if (this->empty()) {
    return rstl::optional_object_null();
  } else {
    return GetAverageValue(this->data(), this->size());
  }
}

template < typename T, int N >
inline rstl::optional_object< T > TReservedAverage< T, N >::GetEntry(int idx) const {
  if (idx >= this->size()) {
    return rstl::optional_object_null();
  } else {
    return rstl::optional_object< T >(this->operator[](idx));
  }
}

#endif // _TRESERVEDAVERAGE
