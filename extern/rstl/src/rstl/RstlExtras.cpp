#include "Kyoto/Alloc/CMemory.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/math.hpp"
#include "rstl/rc_ptr.hpp"
#include "stdio.h"

#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include <string.h>

namespace rstl {
#if RSTL_VERSION >= RSTL_R3IJ
int CRefData::sNull = 0x1000000 - 1;
#else
CRefData CRefData::sNull(nullptr, 0x1000000 - 1);
#endif
}

int CStringExtras::IndexOfSubstring(const rstl::string& left, const rstl::string& right) {
  int rightSize = right.length();
  if (rightSize == 0) {
    return 0;
  }
  int leftSize = left.length();
  if (leftSize == 0) {
    return -1;
  }

  for (int i = 0; i < (leftSize - rightSize) + 1; ++i) {
    int j = 0;
    for (; j < rightSize; ++j) {
      const int index = i + j;
      if (ConvertToUpperCase(right[j]) != ConvertToUpperCase(left[index])) {
        break;
      }
    }
    if (j == rightSize) {
      return i;
    }
  }
  return -1;
}

int CStringExtras::CompareCaseInsensitive(const rstl::string& left, const rstl::string& right) {
  int leftLength = left.length();
  int rightLength = right.length();
  int commonLength = rstl::min_val(leftLength, rightLength);

  for (int idx = 0; idx < commonLength; ++idx) {
    if (ConvertToUpperCase(left[idx]) < ConvertToUpperCase(right[idx])) {
      return -1;
    }

    if (ConvertToUpperCase(left[idx]) > ConvertToUpperCase(right[idx])) {
      return 1;
    }
  }
  if (leftLength < rightLength) {
    return -1;
  } else if (leftLength > rightLength) {
    return 1;
  } else {
    return 0;
  }
}

char CStringExtras::ConvertToUpperCase(char c) {
  if (c >= CCast::ToChar('a') && c <= CCast::ToChar('z')) {
    return (c - CCast::ToChar(' ')) & 0xFF;
  }

  return c;
}

rstl::string CStringExtras::CreateFromInteger(int v) {
  int magnitude = v < 0 ? -v : v;
  int count = 0;
  schar digits[24];
  if (v == 0) {
    return rstl::string_l("0");
  }

  while (magnitude > 0) {
    digits[count] = '0' + magnitude % 10;
    ++count;
    magnitude /= 10;
  }

  rstl::string ret;
  if (v < 0) {
    ret.append("-", -1);
  }

  for (int i = 0; i < count; ++i) {
    ret.append(1, digits[count - i - 1]);
  }

  return ret;
}

rstl::string CStringExtras::CreateFromReal(float v, int i) {
  char a[16];
  char b[136];
  
  sprintf(b, "%%.%df", (i > 12) ? 12 : i);
  sprintf(b, a, v);
  return rstl::string(b);
}

rstl::string CStringExtras::ConvertToANSI(const rstl::wstring& str) {
  rstl::string ret;
  ret.reserve(str.size() + 1);

  for (int i = 0; i < static_cast< int >(str.size()); ++i) {
    ret.assign(ret + static_cast< char >(str.at(i)));
  }
  return ret;
}

rstl::wstring CStringExtras::ConvertToUNICODE(const rstl::string& str) {
  rstl::wstring ret;
  ret.reserve(str.size() + 1);

  for (int i = 0; i < static_cast< int >(str.size()); ++i) {
    ret.append(1, static_cast< wchar_t >(str.at(i)));
  }
  return ret;
}

rstl::string CStringExtras::ReadString(CInputStream& in) {
  rstl::string ret;
  int strLen = in.ReadInt32();
  ret.reserve(strLen);
  while (strLen > 0) {
    int len = strLen > 512u ? 512 : strLen;
    char tmp[512];
    in.ReadBytes(tmp, len);
    ret.append(tmp, len);
    strLen -= len;
  }

  return ret;
}

rstl::vector< rstl::string > CStringExtras::TokenizeString(const rstl::string& string,
                                                           const char* delims, int expectedSize) {
  rstl::vector< rstl::string > ret;
  if (expectedSize > 0) {
    ret.reserve(expectedSize);
  }

  const int size = string.length();
  int pos = 0;
  while (pos < size) {
    while (pos < size && strchr(delims, string[pos]) != nullptr) {
      ++pos;
    }
    if (pos == size) {
      break;
    }

    int end = pos + 1;
    while (end < size && strchr(delims, string[end]) == nullptr) {
      ++end;
    }
    ret.push_back(string.substr(pos, end - pos));
    pos = end + 1;
  }

  return ret;
}
