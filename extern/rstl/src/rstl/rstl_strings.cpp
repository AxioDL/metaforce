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
#if RSTL_VERSION >= RSTL_R3IJ
: rmemory_allocator(alloc), mPtr(&mNull), mCow(nullptr), mSize(0) {
#else
: mPtr(&mNull), mCow(nullptr), mSize(0), mAllocator(alloc) {
#endif
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
#if RSTL_VERSION >= RSTL_R3IJ
: rmemory_allocator(alloc) {
#else
: mAllocator(alloc) {
#endif
  if (count <= 0 && !*data) {
    mPtr = &mNull;
    mSize = 0;
    mCow = nullptr;
    return;
  }

  const pair< const char*, int > range = compute_length(data, count);
  const int len = range.second;
  internal_allocate(len + 1);
  mSize = len;
  char_traits< char >::copy(const_cast< char* >(mPtr), data, len);
  char_traits< char >::assign(const_cast< char& >(mPtr[len]), char_traits< char >::eos());
}

template <>
basic_string< char >::basic_string(const basic_string& other)
#if RSTL_VERSION >= RSTL_R3IJ
: rmemory_allocator(other)
, mPtr(other.mPtr)
#else
: mPtr(other.mPtr)
#endif
, mCow(other.mCow)
, mSize(other.mSize)
#if RSTL_VERSION < RSTL_R3IJ
, mAllocator(other.mAllocator)
#endif
{
  internal_reference();
}

template <>
basic_string< char >& basic_string< char >::append(const basic_string& other) {
  internal_prepare_to_write(length() + other.length(), true);
  char_traits< char >::copy(const_cast< char* >(mPtr) + length(), other.data(), other.length());
  mSize += other.length();
  char_traits< char >::assign(const_cast< char& >(mPtr[length()]), char_traits< char >::eos());
  return *this;
}

template <>
basic_string< char >& basic_string< char >::append(const char* data, int count) {
  const pair< const char*, int > range = compute_length(data, count);
  const int len = range.second;
  internal_prepare_to_write(mSize + len, true);
  char_traits< char >::copy(const_cast< char* >(mPtr) + length(), data, len);
  mSize += len;
  char_traits< char >::assign(const_cast< char& >(mPtr[length()]), char_traits< char >::eos());
  return *this;
}

template <>
basic_string< char >& basic_string< char >::append(int count, char value) {
  internal_prepare_to_write(mSize + count, true);
  char_traits< char >::assign(const_cast< char* >(mPtr) + length(), count, value);
  mSize += count;
  char_traits< char >::assign(const_cast< char& >(mPtr[length()]), char_traits< char >::eos());
  return *this;
}

template <>
basic_string< char >& basic_string< char >::assign(const basic_string& other) {
  if (mCow && mCow == other.mCow) {
    return *this;
  }

  internal_dereference();
  mCow = other.mCow;
  mPtr = other.mPtr;
  mSize = other.mSize;
  internal_reference();
  return *this;
}

template <>
void basic_string< char >::PutTo(COutputStream& out) const {
  for (int i = 0; i < length() + 1; ++i) {
    out.WriteChar(mPtr[i]);
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
  rmemory_allocator::allocate(reinterpret_cast< uchar*& >(mCow),
                              sizeof(control) + sizeof(char) * size);
  mPtr = reinterpret_cast< char* >(mCow + 1);
  mCow->mCapacity = size;
  mCow->mRefCount = 1;
}

template <>
void basic_string< char >::internal_dereference() {
  if (mCow && --mCow->mRefCount == 0) {
    rmemory_allocator::deallocate(mCow);
  }
}

template <>
void basic_string< char >::internal_prepare_to_write(int len, bool preserve) {
  const int required = len + 1;
  if (mCow == nullptr || mCow->mRefCount != 1 || mCow->mCapacity < required) {
    int capacity;
    if (mCow) {
      capacity = mCow->mCapacity < 4 ? 4 : mCow->mCapacity;
      while (capacity < required) {
        capacity *= 2;
      }
    } else {
      capacity = required;
    }

    uchar* allocation;
    rmemory_allocator::allocate(allocation, sizeof(control) + sizeof(char) * capacity);
    control* newControl = reinterpret_cast< control* >(allocation);
    newControl->mCapacity = capacity;
    newControl->mRefCount = 1;
    char* const newData = reinterpret_cast< char* >(newControl + 1);
    if (preserve) {
      char_traits< char >::copy(newData, mPtr, length());
      char_traits< char >::assign(newData[length()], char_traits< char >::eos());
    }
    internal_dereference();
    mCow = newControl;
    mPtr = newData;
  }
}

template <>
basic_string< wchar_t >::basic_string(const wchar_t* data, int count,
                                      const rmemory_allocator& alloc)
#if RSTL_VERSION >= RSTL_R3IJ
: rmemory_allocator(alloc) {
#else
: mAllocator(alloc) {
#endif
  if (count <= 0 && !*data) {
    mPtr = &mNull;
    mSize = 0;
    mCow = nullptr;
    return;
  }

  const pair< const wchar_t*, int > range = compute_length(data, count);
  const int len = range.second;
  internal_allocate(len + 1);
  mSize = len;
  char_traits< wchar_t >::copy(const_cast< wchar_t* >(mPtr), data, len);
  char_traits< wchar_t >::assign(const_cast< wchar_t& >(mPtr[len]),
                                 char_traits< wchar_t >::eos());
}

template <>
basic_string< wchar_t >::basic_string(const basic_string& other)
#if RSTL_VERSION >= RSTL_R3IJ
: rmemory_allocator(other)
, mPtr(other.mPtr)
#else
: mPtr(other.mPtr)
#endif
, mCow(other.mCow)
, mSize(other.mSize)
#if RSTL_VERSION < RSTL_R3IJ
, mAllocator(other.mAllocator)
#endif
{
  internal_reference();
}

template <>
basic_string< wchar_t >& basic_string< wchar_t >::append(const basic_string& other) {
  internal_prepare_to_write(length() + other.length(), true);
  char_traits< wchar_t >::copy(const_cast< wchar_t* >(mPtr) + length(), other.data(),
                               other.length());
  mSize += other.length();
  char_traits< wchar_t >::assign(const_cast< wchar_t& >(mPtr[length()]),
                                 char_traits< wchar_t >::eos());
  return *this;
}

template <>
basic_string< wchar_t >& basic_string< wchar_t >::append(const wchar_t* data, int count) {
  const pair< const wchar_t*, int > range = compute_length(data, count);
  const int len = range.second;
  internal_prepare_to_write(mSize + len, true);
  char_traits< wchar_t >::copy(const_cast< wchar_t* >(mPtr) + length(), data, len);
  mSize += len;
  char_traits< wchar_t >::assign(const_cast< wchar_t& >(mPtr[length()]),
                                 char_traits< wchar_t >::eos());
  return *this;
}

template <>
basic_string< wchar_t >& basic_string< wchar_t >::append(int count, wchar_t value) {
  internal_prepare_to_write(mSize + count, true);
  char_traits< wchar_t >::assign(const_cast< wchar_t* >(mPtr) + length(), count, value);
  mSize += count;
  char_traits< wchar_t >::assign(const_cast< wchar_t& >(mPtr[length()]),
                                 char_traits< wchar_t >::eos());
  return *this;
}

template <>
basic_string< wchar_t >& basic_string< wchar_t >::assign(const basic_string& other) {
  if (mCow && mCow == other.mCow) {
    return *this;
  }

  internal_dereference();
  mCow = other.mCow;
  mPtr = other.mPtr;
  mSize = other.mSize;
  internal_reference();
  return *this;
}

template <>
basic_string< wchar_t >& basic_string< wchar_t >::assign(const wchar_t* data, int count) {
  const pair< const wchar_t*, int > range = compute_length(data, count);
  const int len = range.second;
  internal_prepare_to_write(len, false);
  char_traits< wchar_t >::copy(const_cast< wchar_t* >(mPtr), data, len);
  mSize = len;
  char_traits< wchar_t >::assign(const_cast< wchar_t& >(mPtr[length()]),
                                 char_traits< wchar_t >::eos());
  return *this;
}

template <>
void basic_string< wchar_t >::internal_allocate(int size) {
  rmemory_allocator::allocate(reinterpret_cast< uchar*& >(mCow),
                              sizeof(control) + sizeof(wchar_t) * size);
  mPtr = reinterpret_cast< wchar_t* >(mCow + 1);
  mCow->mCapacity = size;
  mCow->mRefCount = 1;
}

template <>
void basic_string< wchar_t >::internal_dereference() {
  if (mCow && --mCow->mRefCount == 0) {
    rmemory_allocator::deallocate(mCow);
  }
}

template <>
void basic_string< wchar_t >::internal_prepare_to_write(int len, bool preserve) {
  const int required = len + 1;
  if (mCow == nullptr || mCow->mRefCount != 1 || mCow->mCapacity < required) {
    int capacity;
    if (mCow) {
      capacity = mCow->mCapacity < 4 ? 4 : mCow->mCapacity;
      while (capacity < required) {
        capacity *= 2;
      }
    } else {
      capacity = required;
    }

    uchar* allocation;
    rmemory_allocator::allocate(allocation, sizeof(control) + sizeof(wchar_t) * capacity);
    control* newControl = reinterpret_cast< control* >(allocation);
    newControl->mCapacity = capacity;
    newControl->mRefCount = 1;
    wchar_t* const newData = reinterpret_cast< wchar_t* >(newControl + 1);
    if (preserve) {
      char_traits< wchar_t >::copy(newData, mPtr, length());
      char_traits< wchar_t >::assign(newData[length()], char_traits< wchar_t >::eos());
    }
    internal_dereference();
    mCow = newControl;
    mPtr = newData;
  }
}

template <>
basic_string< char, case_insensitive_char_traits< char > >::basic_string(
    const char* data, int count, const rmemory_allocator& alloc)
#if RSTL_VERSION >= RSTL_R3IJ
: rmemory_allocator(alloc) {
#else
: mAllocator(alloc) {
#endif
  if (count <= 0 && !*data) {
    mPtr = &mNull;
    mSize = 0;
    mCow = nullptr;
    return;
  }

  const pair< const char*, int > range = compute_length(data, count);
  const int len = range.second;
  internal_allocate(len + 1);
  mSize = len;
  case_insensitive_char_traits< char >::copy(const_cast< char* >(mPtr), data, len);
  case_insensitive_char_traits< char >::assign(const_cast< char& >(mPtr[len]),
                                               case_insensitive_char_traits< char >::eos());
}

template <>
void basic_string< char, case_insensitive_char_traits< char > >::internal_allocate(int size) {
  rmemory_allocator::allocate(reinterpret_cast< uchar*& >(mCow),
                              sizeof(control) + sizeof(char) * size);
  mPtr = reinterpret_cast< char* >(mCow + 1);
  mCow->mCapacity = size;
  mCow->mRefCount = 1;
}

template <>
void basic_string< char, case_insensitive_char_traits< char > >::internal_dereference() {
  if (mCow && --mCow->mRefCount == 0) {
    rmemory_allocator::deallocate(mCow);
  }
}

} // namespace rstl
