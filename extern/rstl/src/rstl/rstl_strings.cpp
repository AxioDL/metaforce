#include "rstl/string.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

namespace rstl {

template <>
const char basic_string< char >::mNull = char_traits< char >::eos();

template <>
const wchar_t basic_string< wchar_t >::mNull = char_traits< wchar_t >::eos();

template <>
const char basic_string< char, case_insensitive_char_traits< char > >::mNull =
    case_insensitive_char_traits< char >::eos();

template <>
const wchar_t basic_string< wchar_t, case_insensitive_char_traits< wchar_t > >::mNull =
    case_insensitive_char_traits< wchar_t >::eos();

template <>
basic_string< char >::basic_string(CInputStream& in, const rmemory_allocator& alloc)
: x0_ptr(&mNull), x4_cow(nullptr), x8_size(0), xc_allocator(alloc) {
  char buffer[1025];
  int count = 0;
  char ch = in.Get< schar >();
  while (ch != '\0') {
    buffer[count++] = ch;
    if (count == 1024) {
      buffer[1024] = '\0';
      append(buffer, -1);
      count = 0;
    }
    ch = in.Get< schar >();
  }
  if (count != 0) {
    buffer[count] = '\0';
    append(buffer, -1);
  }
}

template <>
basic_string< char >::basic_string(const char* data, int count, const rmemory_allocator& alloc)
: xc_allocator(alloc) {
  if (count <= 0 && !*data) {
    x0_ptr = &mNull;
    x8_size = 0;
    x4_cow = nullptr;
    return;
  }

  const pair< const char*, int > range = compute_length(data, count);
  const int len = range.second;
  internal_allocate(len + 1);
  x8_size = len;
  char_traits< char >::copy(const_cast< char* >(x0_ptr), data, len);
  char_traits< char >::assign(const_cast< char& >(x0_ptr[len]), char_traits< char >::eos());
}

template <>
basic_string< char >::basic_string(const basic_string& other)
: x0_ptr(other.x0_ptr)
, x4_cow(other.x4_cow)
, x8_size(other.x8_size)
, xc_allocator(other.xc_allocator) {
  internal_reference();
}

template <>
basic_string< char >& basic_string< char >::append(const basic_string& other) {
  internal_prepare_to_write(length() + other.length(), true);
  char_traits< char >::copy(const_cast< char* >(x0_ptr) + length(), other.data(), other.length());
  x8_size += other.length();
  char_traits< char >::assign(const_cast< char& >(x0_ptr[length()]), char_traits< char >::eos());
  return *this;
}

template <>
basic_string< char >& basic_string< char >::append(const char* data, int count) {
  const pair< const char*, int > range = compute_length(data, count);
  const int len = range.second;
  internal_prepare_to_write(x8_size + len, true);
  char_traits< char >::copy(const_cast< char* >(x0_ptr) + length(), data, len);
  x8_size += len;
  char_traits< char >::assign(const_cast< char& >(x0_ptr[length()]), char_traits< char >::eos());
  return *this;
}

template <>
basic_string< char >& basic_string< char >::append(int count, char value) {
  internal_prepare_to_write(x8_size + count, true);
  char_traits< char >::assign(const_cast< char* >(x0_ptr) + length(), count, value);
  x8_size += count;
  char_traits< char >::assign(const_cast< char& >(x0_ptr[length()]), char_traits< char >::eos());
  return *this;
}

template <>
basic_string< char >& basic_string< char >::assign(const basic_string& other) {
  if (x4_cow && x4_cow == other.x4_cow) {
    return *this;
  }

  internal_dereference();
  x4_cow = other.x4_cow;
  x0_ptr = other.x0_ptr;
  x8_size = other.x8_size;
  internal_reference();
  return *this;
}

template <>
void basic_string< char >::PutTo(COutputStream& out) const {
  for (int i = 0; i < length() + 1; ++i) {
    out.WriteChar(x0_ptr[i]);
  }
}

template <>
basic_string< char >::const_iterator basic_string< char >::position_iterator(int pos) const {
  if (pos == -1 || pos >= length()) {
    return end();
  }

  return begin() + pos;
}

template <>
pair< basic_string< char >::const_iterator, basic_string< char >::const_iterator >
basic_string< char >::range_iterator(int pos, int count) const {
  const_iterator first = position_iterator(pos);
  const_iterator last = count != -1 && pos + count < length() ? first + count : end();
  return pair< const_iterator, const_iterator >(first, last);
}

template <>
void basic_string< char >::internal_allocate(int size) {
  rmemory_allocator::allocate(reinterpret_cast< uchar*& >(x4_cow),
                              sizeof(control) + sizeof(char) * size);
  x0_ptr = reinterpret_cast< char* >(x4_cow + 1);
  x4_cow->x0_capacity = size;
  x4_cow->x4_refCount = 1;
}

template <>
void basic_string< char >::internal_dereference() {
  if (x4_cow && --x4_cow->x4_refCount == 0) {
    rmemory_allocator::deallocate(x4_cow);
  }
}

template <>
void basic_string< char >::internal_prepare_to_write(int len, bool preserve) {
  const int required = len + 1;
  if (x4_cow == nullptr || x4_cow->x4_refCount != 1 || x4_cow->x0_capacity < required) {
    int capacity;
    if (x4_cow) {
      capacity = x4_cow->x0_capacity < 4 ? 4 : x4_cow->x0_capacity;
      while (capacity < required) {
        capacity *= 2;
      }
    } else {
      capacity = required;
    }

    uchar* allocation;
    rmemory_allocator::allocate(allocation, sizeof(control) + sizeof(char) * capacity);
    control* newControl = reinterpret_cast< control* >(allocation);
    newControl->x0_capacity = capacity;
    newControl->x4_refCount = 1;
    char* const newData = reinterpret_cast< char* >(newControl + 1);
    if (preserve) {
      char_traits< char >::copy(newData, x0_ptr, length());
      char_traits< char >::assign(newData[length()], char_traits< char >::eos());
    }
    internal_dereference();
    x4_cow = newControl;
    x0_ptr = newData;
  }
}

template <>
basic_string< wchar_t >::basic_string(const wchar_t* data, int count,
                                      const rmemory_allocator& alloc)
: xc_allocator(alloc) {
  if (count <= 0 && !*data) {
    x0_ptr = &mNull;
    x8_size = 0;
    x4_cow = nullptr;
    return;
  }

  const pair< const wchar_t*, int > range = compute_length(data, count);
  const int len = range.second;
  internal_allocate(len + 1);
  x8_size = len;
  char_traits< wchar_t >::copy(const_cast< wchar_t* >(x0_ptr), data, len);
  char_traits< wchar_t >::assign(const_cast< wchar_t& >(x0_ptr[len]),
                                 char_traits< wchar_t >::eos());
}

template <>
basic_string< wchar_t >::basic_string(const basic_string& other)
: x0_ptr(other.x0_ptr)
, x4_cow(other.x4_cow)
, x8_size(other.x8_size)
, xc_allocator(other.xc_allocator) {
  internal_reference();
}

template <>
basic_string< wchar_t >& basic_string< wchar_t >::append(const basic_string& other) {
  internal_prepare_to_write(length() + other.length(), true);
  char_traits< wchar_t >::copy(const_cast< wchar_t* >(x0_ptr) + length(), other.data(),
                               other.length());
  x8_size += other.length();
  char_traits< wchar_t >::assign(const_cast< wchar_t& >(x0_ptr[length()]),
                                 char_traits< wchar_t >::eos());
  return *this;
}

template <>
basic_string< wchar_t >& basic_string< wchar_t >::append(const wchar_t* data, int count) {
  const pair< const wchar_t*, int > range = compute_length(data, count);
  const int len = range.second;
  internal_prepare_to_write(x8_size + len, true);
  char_traits< wchar_t >::copy(const_cast< wchar_t* >(x0_ptr) + length(), data, len);
  x8_size += len;
  char_traits< wchar_t >::assign(const_cast< wchar_t& >(x0_ptr[length()]),
                                 char_traits< wchar_t >::eos());
  return *this;
}

template <>
basic_string< wchar_t >& basic_string< wchar_t >::append(int count, wchar_t value) {
  internal_prepare_to_write(x8_size + count, true);
  char_traits< wchar_t >::assign(const_cast< wchar_t* >(x0_ptr) + length(), count, value);
  x8_size += count;
  char_traits< wchar_t >::assign(const_cast< wchar_t& >(x0_ptr[length()]),
                                 char_traits< wchar_t >::eos());
  return *this;
}

template <>
basic_string< wchar_t >& basic_string< wchar_t >::assign(const basic_string& other) {
  if (x4_cow && x4_cow == other.x4_cow) {
    return *this;
  }

  internal_dereference();
  x4_cow = other.x4_cow;
  x0_ptr = other.x0_ptr;
  x8_size = other.x8_size;
  internal_reference();
  return *this;
}

template <>
basic_string< wchar_t >& basic_string< wchar_t >::assign(const wchar_t* data, int count) {
  const pair< const wchar_t*, int > range = compute_length(data, count);
  const int len = range.second;
  internal_prepare_to_write(len, false);
  char_traits< wchar_t >::copy(const_cast< wchar_t* >(x0_ptr), data, len);
  x8_size = len;
  char_traits< wchar_t >::assign(const_cast< wchar_t& >(x0_ptr[length()]),
                                 char_traits< wchar_t >::eos());
  return *this;
}

template <>
void basic_string< wchar_t >::internal_allocate(int size) {
  rmemory_allocator::allocate(reinterpret_cast< uchar*& >(x4_cow),
                              sizeof(control) + sizeof(wchar_t) * size);
  x0_ptr = reinterpret_cast< wchar_t* >(x4_cow + 1);
  x4_cow->x0_capacity = size;
  x4_cow->x4_refCount = 1;
}

template <>
void basic_string< wchar_t >::internal_dereference() {
  if (x4_cow && --x4_cow->x4_refCount == 0) {
    rmemory_allocator::deallocate(x4_cow);
  }
}

template <>
void basic_string< wchar_t >::internal_prepare_to_write(int len, bool preserve) {
  const int required = len + 1;
  if (x4_cow == nullptr || x4_cow->x4_refCount != 1 || x4_cow->x0_capacity < required) {
    int capacity;
    if (x4_cow) {
      capacity = x4_cow->x0_capacity < 4 ? 4 : x4_cow->x0_capacity;
      while (capacity < required) {
        capacity *= 2;
      }
    } else {
      capacity = required;
    }

    uchar* allocation;
    rmemory_allocator::allocate(allocation, sizeof(control) + sizeof(wchar_t) * capacity);
    control* newControl = reinterpret_cast< control* >(allocation);
    newControl->x0_capacity = capacity;
    newControl->x4_refCount = 1;
    wchar_t* const newData = reinterpret_cast< wchar_t* >(newControl + 1);
    if (preserve) {
      char_traits< wchar_t >::copy(newData, x0_ptr, length());
      char_traits< wchar_t >::assign(newData[length()], char_traits< wchar_t >::eos());
    }
    internal_dereference();
    x4_cow = newControl;
    x0_ptr = newData;
  }
}

template <>
basic_string< char, case_insensitive_char_traits< char > >::basic_string(
    const char* data, int count, const rmemory_allocator& alloc)
: xc_allocator(alloc) {
  if (count <= 0 && !*data) {
    x0_ptr = &mNull;
    x8_size = 0;
    x4_cow = nullptr;
    return;
  }

  const pair< const char*, int > range = compute_length(data, count);
  const int len = range.second;
  internal_allocate(len + 1);
  x8_size = len;
  case_insensitive_char_traits< char >::copy(const_cast< char* >(x0_ptr), data, len);
  case_insensitive_char_traits< char >::assign(const_cast< char& >(x0_ptr[len]),
                                               case_insensitive_char_traits< char >::eos());
}

template <>
void basic_string< char, case_insensitive_char_traits< char > >::internal_allocate(int size) {
  rmemory_allocator::allocate(reinterpret_cast< uchar*& >(x4_cow),
                              sizeof(control) + sizeof(char) * size);
  x0_ptr = reinterpret_cast< char* >(x4_cow + 1);
  x4_cow->x0_capacity = size;
  x4_cow->x4_refCount = 1;
}

template <>
void basic_string< char, case_insensitive_char_traits< char > >::internal_dereference() {
  if (x4_cow && --x4_cow->x4_refCount == 0) {
    rmemory_allocator::deallocate(x4_cow);
  }
}

} // namespace rstl
