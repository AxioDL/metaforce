#ifndef _RSTL_OPTIONAL_OBJECT
#define _RSTL_OPTIONAL_OBJECT

#include "types.h"

#include "rstl/construct.hpp"

namespace rstl {
struct optional_object_null {};

template < typename T >
class optional_object {
public:
  optional_object() : m_valid(false) {}
  optional_object(optional_object_null) : m_valid(false) {}
  optional_object(const T& item) : m_valid(true) { rstl::construct< T >(m_data, item); }
  optional_object(const optional_object& other) : m_valid(other.m_valid) {
    if (other.m_valid) {
      construct< T >(m_data, other.data());
    }
  }
  ~optional_object() {
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
    if (m_valid) {
      rstl::destroy(&data());
    }
#else
    clear();
#endif
  }

  optional_object& operator=(const optional_object& other) {
    if (this == &other) {
      return *this;
    }
    if (other.m_valid) {
      assign(other.data());
    } else {
      clear();
    }
    return *this;
  }
  optional_object& operator=(const T& item) {
    assign(item);
    return *this;
  }

  T& data() { return *get_ptr(); }
  const T& data() const { return *get_ptr(); }
  T* get_ptr() { return reinterpret_cast< T* >(m_data); }
  const T* get_ptr() const { return reinterpret_cast< const T* >(m_data); }
  const bool valid() const { return m_valid; }
  operator bool() const { return m_valid; } // replace with valid()?
  void clear() {
    if (m_valid) {
      rstl::destroy(get_ptr());
    }
    m_valid = false;
  }

  T& operator*() { return data(); }
  T* operator->() { return &data(); }

  const T& operator*() const { return data(); }
  const T* operator->() const { return &data(); }

private:
  ALIGNAS(T) uchar m_data[sizeof(T)];
  bool m_valid ATTRIBUTE_ALIGN(4);

  void assign(const T& item) {
    if (!m_valid) {
      construct< T >(m_data, item);
      m_valid = true;
    } else {
      data() = item;
    }
  }
};
} // namespace rstl

#endif // _RSTL_OPTIONAL_OBJECT
