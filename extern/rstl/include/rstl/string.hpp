#ifndef _RSTL_STRING
#define _RSTL_STRING

#include "types.h"

#include "rstl/allocator.hpp"
#include "rstl/linear_iterator.hpp"
#include "rstl/pair.hpp"

class CInputStream;
class COutputStream;

namespace rstl {
template < typename _CharTp >
struct char_traits {
  static void copy(_CharTp* out, const _CharTp* in, int count) {
    for (int i = 0; i < count; ++i) {
      out[i] = in[i];
    }
  }

  static void assign(_CharTp& out, const _CharTp& value) { out = value; }

  static void assign(_CharTp* out, int count, const _CharTp& value) {
    for (int i = 0; i < count; ++i) {
      out[i] = value;
    }
  }

  static bool eq(const _CharTp& lhs, const _CharTp& rhs) { return lhs == rhs; }
  static _CharTp eos() { return 0; }
  static int compare(const _CharTp& lhs, const _CharTp& rhs) {
    return static_cast< int >(lhs) - static_cast< int >(rhs);
  }
};

template <>
struct char_traits< char > {
  static void copy(char* out, const char* in, int count) {
    for (int i = 0; i < count; ++i) {
      out[i] = in[i];
    }
  }

  static void assign(char& out, const char& value) { out = value; }

  static void assign(char* out, int count, const char& value) {
    for (int i = 0; i < count; ++i) {
      out[i] = value;
    }
  }

  static bool eq(const char& lhs, const char& rhs) { return lhs == rhs; }
  static char eos() { return 0; }
  static int compare(const char& lhs, const char& rhs) {
    return static_cast< int >(static_cast< signed char >(lhs)) -
           static_cast< int >(static_cast< signed char >(rhs));
  }
};

template < typename _CharTp >
struct case_insensitive_char_traits {
  static void copy(_CharTp* out, const _CharTp* in, int count) {
    for (int i = 0; i < count; ++i) {
      out[i] = in[i];
    }
  }

  static void assign(_CharTp& out, const _CharTp& value) { out = value; }

  static void assign(_CharTp* out, int count, const _CharTp& value) {
    for (int i = 0; i < count; ++i) {
      out[i] = value;
    }
  }

  static _CharTp eos() { return 0; }
  static _CharTp toupper(const _CharTp ch) {
    return (ch >= 'a' && ch <= 'z')         ? ch - 32
           : (ch >= 0xe0 && ch <= 0xfe)     ? ch - 32
           : (ch >= 0x30a0 && ch <= 0x30ff) ? ch - 96
                                            : ch;
  }

  static bool eq(const _CharTp& lhs, const _CharTp& rhs) { return toupper(lhs) == toupper(rhs); }

  static int compare(const _CharTp& lhs, const _CharTp& rhs) { return toupper(rhs) - toupper(lhs); }
};

template < typename _CharTp, typename Traits = char_traits< _CharTp >,
           typename Alloc = rmemory_allocator >
class basic_string {
  struct control {
    int x0_capacity;
    int x4_refCount;
  };

  const _CharTp* x0_ptr;
  control* x4_cow;
  uint x8_size;
  Alloc xc_allocator;

  void internal_prepare_to_write(int len, bool);
  void internal_allocate(int size);

  void internal_dereference();
  void internal_reference() {
    if (x4_cow) {
      ++x4_cow->x4_refCount;
    }
  }

  template < typename It >
  static pair< It, int > compute_length(It data, int count) {
    It end = data;
    int len = 0;
    while ((count == -1 || len < count) && !Traits::eq(*end, Traits::eos())) {
      ++end;
      ++len;
    }
    return pair< It, int >(end, len);
  }

  static const _CharTp mNull;

public:
  typedef const_linear_iterator< _CharTp, basic_string, Alloc > const_iterator;

  struct literal_t {};

  basic_string() : x0_ptr(&mNull), x4_cow(nullptr), x8_size(0) {}

  basic_string(literal_t, const _CharTp* data) {
    x0_ptr = data;
    x4_cow = nullptr;

    const _CharTp* it = data;
    while (*it)
      ++it;

    x8_size = static_cast< uint >(it - data);
  }

  basic_string(const basic_string& str);

  basic_string(CInputStream& in, const Alloc& = rmemory_allocator());

  template < typename It >
  basic_string(It first, It last, const Alloc& = rmemory_allocator()) {
    const int len = rstl::distance(first, last);
    internal_allocate(len + 1);
    int i = 0;
    for (It it = first; it != last; it = it + 1, ++i) {
      const_cast< _CharTp& >(x0_ptr[i]) = *it;
    }
    const_cast< _CharTp& >(x0_ptr[i]) = Traits::eos();
    x8_size = len;
  }

  basic_string(const _CharTp* data, int size = -1, const Alloc& = rmemory_allocator());

  ~basic_string() { internal_dereference(); }

  size_t size() const { return x8_size; }
  int length() const { return x8_size; }
  int refcount() { return x4_cow != nullptr ? x4_cow->x4_refCount : -1; }
  void reserve(int len) { internal_prepare_to_write(len, true); }

  basic_string& assign(const basic_string&);
  basic_string& assign(const _CharTp*, int);
  basic_string& operator=(const basic_string& other) {
    assign(other);
    return *this;
  }
  basic_string& append(const basic_string& other);
  basic_string& append(int, _CharTp);
  basic_string& append(const _CharTp*, int);

  int compare(const _CharTp* rhs, int count = -1) const;
  const _CharTp& operator[](int idx) const { return x0_ptr[idx]; }
  const_iterator begin() const { return const_iterator(this, 0); }
  const_iterator end() const { return const_iterator(this, size()); }

  template < typename It >
  static int internal_compare(const_iterator first, const_iterator last, It otherFirst,
                              It otherLast);
  template < typename It, typename OtherIt >
  static int internal_search(It first, It last, OtherIt otherFirst, OtherIt otherLast);
  template < typename It, typename OtherIt >
  static int internal_search_of(It first, It last, OtherIt otherFirst, OtherIt otherLast);
  int compare(const basic_string& other) const;
  bool operator==(const basic_string& other) const;
  bool operator!=(const basic_string& other) const;

  int find(const basic_string& other, int pos = 0) const;
  int find(_CharTp ch, int pos = 0) const;
  int find_first_of(const basic_string& other, int pos = 0) const;
  const_iterator position_iterator(int pos) const;
  pair< const_iterator, const_iterator > range_iterator(int pos, int count) const;
  basic_string substr(int pos = 0, int count = -1) const;
  int get_real_pos_for_begin(int pos) const {
    if (pos == -1 || pos >= static_cast< int >(size())) {
      return size();
    }
    return pos;
  }
  const _CharTp* c_str() const { return x0_ptr; }
  const _CharTp* data() const { return x0_ptr; }
  void PutTo(COutputStream& out) const;
  const _CharTp at(int idx) const { return data()[idx]; }
};

template < typename _CharTp, typename Traits, typename Alloc >
template < typename It, typename OtherIt >
inline int basic_string< _CharTp, Traits, Alloc >::internal_search_of(It first, It last,
                                                                      OtherIt otherFirst,
                                                                      OtherIt otherLast) {
  int index = 0;
  for (It it = first; it != last; ++it, ++index) {
    for (OtherIt other = otherFirst; other != otherLast; ++other) {
      if (Traits::eq(*it, *other)) {
        return index;
      }
    }
  }
  return -1;
}

template < typename _CharTp, typename Traits, typename Alloc >
inline int basic_string< _CharTp, Traits, Alloc >::find_first_of(const basic_string& other,
                                                                 int pos) const {
  pos = get_real_pos_for_begin(pos);
  const int found = internal_search_of(begin() + pos, end(), other.begin(), other.end());
  int result = found + pos;
  if (found == -1) {
    result = found;
  }
  return result;
}

template < typename _CharTp, typename Traits, typename Alloc >
template < typename It, typename OtherIt >
inline int basic_string< _CharTp, Traits, Alloc >::internal_search(It first, It last,
                                                                   OtherIt otherFirst,
                                                                   const OtherIt otherLast) {
  if (otherFirst == otherLast) {
    return 0;
  }
  It it = first;
  int matched = 0;
  OtherIt search = otherFirst;
  for (; it != last; ++it) {
    if (Traits::eq(*it, *search)) {
      ++search;
      ++matched;
      if (search == otherLast) {
        return (it - first) - matched + 1;
      }
    } else {
      search = otherFirst;
      matched = 0;
    }
  }
  return -1;
}

template < typename _CharTp, typename Traits, typename Alloc >
int basic_string< _CharTp, Traits, Alloc >::find(const basic_string& other, int pos) const {
  pos = get_real_pos_for_begin(pos);
  const int found = internal_search(begin() + pos, end(), other.begin(), other.end());
  int result = found + pos;
  if (found == -1) {
    result = found;
  }
  return result;
}

template < typename _CharTp, typename Traits, typename Alloc >
int basic_string< _CharTp, Traits, Alloc >::find(_CharTp ch, int pos) const {
  pos = get_real_pos_for_begin(pos);
  const int found = internal_search(begin() + pos, end(), static_cast< const _CharTp* >(&ch),
                                    static_cast< const _CharTp* >(&ch) + 1);
  int result = found + pos;
  if (found == -1) {
    result = found;
  }
  return result;
}

template < typename _CharTp, typename Traits, typename Alloc >
int basic_string< _CharTp, Traits, Alloc >::compare(const _CharTp* rhs, int count) const {
  int rhsCharCount = 0;
  const _CharTp* rhsStart = rhs;
  while ((count == -1 || rhsCharCount < count) && *rhs != '\0') {
    ++rhs;
    ++rhsCharCount;
  }
  return internal_compare(begin(), end(), rhsStart, rhs);
}

template < typename _CharTp, typename Traits, typename Alloc >
template < typename It >
inline int basic_string< _CharTp, Traits, Alloc >::internal_compare(const_iterator first,
                                                                    const_iterator last,
                                                                    It otherFirst, It otherLast) {
  const_iterator it = first;
  It other = otherFirst;
  for (; it != last && other != otherLast; ++it, ++other) {
    int cmp = Traits::compare(*it, *other);
    if (cmp != 0) {
      return cmp;
    }
  }
  if (it == last && other != otherLast) {
    return -1;
  }
  if (it == last) {
    return 0;
  }
  return 1;
}

template < typename _CharTp, typename Traits, typename Alloc >
inline int basic_string< _CharTp, Traits, Alloc >::compare(const basic_string& other) const {
  return internal_compare(begin(), end(), other.begin(), other.end());
}

template < typename _CharTp, typename Traits, typename Alloc >
bool basic_string< _CharTp, Traits, Alloc >::operator==(const basic_string& other) const {
  return compare(other) == 0;
}

template < typename _CharTp, typename Traits, typename Alloc >
inline bool basic_string< _CharTp, Traits, Alloc >::operator!=(const basic_string& other) const {
  return compare(other) != 0;
}

typedef basic_string< wchar_t > wstring;
typedef basic_string< char > string;
typedef basic_string< char, case_insensitive_char_traits< char > > istring;

inline bool operator<(const string& lhs, const string& rhs) { return lhs.compare(rhs) < 0; }

istring istring_l(const char* data);
inline bool operator==(const istring& a, const istring& b) { return a.compare(b) == 0; }

bool operator==(const string& lhs, const char* rhs);
bool operator==(const char* lhs, const string& rhs);
bool operator!=(const string& lhs, const char* rhs);

#ifdef __MWERKS__
__declspec(weak) // TODO
#else
static
#endif
wstring wstring_l(const wchar_t* data) {
  return wstring(wstring::literal_t(), data);
}

string string_l(const char* data);

string operator+(const string& a, const string& b);
inline wstring operator+(const wstring& a, const wstring& b) {
  wstring result(a);
  result.append(b);
  return result;
}

inline string operator+(const string& a, char c) {
  string result(a);
  result.append(1, c);
  return result;
}

inline string operator+(const string& a, const char* c) {
  string result(a);
  result.append(c, -1);
  return result;
}

static inline wstring operator+(const wstring& a, const wchar_t* c) {
  wstring result(a);
  result.append(c, -1);
  return result;
}

CHECK_SIZEOF(string, 0x10)
} // namespace rstl

#endif // _RSTL_STRING
