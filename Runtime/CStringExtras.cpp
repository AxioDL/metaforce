#include "Runtime/CStringExtras.hpp"
#include "Runtime/Streams/CInputStream.hpp"

#include <nod/OSUTF.h>

namespace metaforce {
std::string CStringExtras::ReadString(CInputStream& in) {
  u32 strLen = in.ReadLong();
  std::string ret;
  u32 readLen = 512;
  char tmp[512] = {};
  for (; strLen > 0; strLen -= readLen) {
    readLen = 512;
    if (strLen <= 512) {
      readLen = strLen;
    }
    in.ReadBytes(tmp, readLen);
    ret.append(tmp, readLen);
  }

  return ret;
}

std::string CStringExtras::ConvertToANSI(std::u16string_view sv) {
  std::string out;
  out.reserve(sv.size());
  for (const char16_t c : sv) {
    out.push_back(static_cast<char>(c));
  }
  return out;
}

std::u16string CStringExtras::ConvertToUNICODE(std::string_view sv) {
  std::u16string out;
  out.reserve(sv.size());
  for (const char c : sv) {
    out.push_back(static_cast<char16_t>(c));
  }
  return out;
}

std::string CStringExtras::ConvertToUTF8(std::u16string_view sv) {
  std::string out;
  const auto* in = sv.data();
  const auto* end = in + sv.size();
  while (in < end) {
    char32_t utf32 = 0;
    const char16_t* next = OSUTF16To32(in, &utf32);
    // TODO: bug in OSUTF
    if (next == nullptr) {
      utf32 = *in;
      in++;
    } else {
      in = next;
    }
    std::array<char8_t, 4> chars8{};
    char8_t* end8 = OSUTF32To8(utf32, chars8.data());
    if (end8 == nullptr) {
      continue;
    }
    const auto* c = chars8.data();
    while (c < end8) {
      out.push_back(static_cast<char>(*c));
      c++;
    }
  }
  return out;
}

std::u16string CStringExtras::ConvertToUTF16(std::string_view sv) {
  std::u16string out;
  const auto* in = reinterpret_cast<const char8_t*>(sv.data());
  const auto* end = in + sv.size();
  while (in < end) {
    char32_t utf32 = 0;
    const char8_t* next = OSUTF8To32(in, &utf32);
    // TODO: bug in OSUTF
    if (next == nullptr) {
      utf32 = *in;
      in++;
    } else {
      in = next;
    }
    std::array<char16_t, 2> chars16{};
    char16_t* end16 = OSUTF32To16(utf32, chars16.data());
    if (end16 == nullptr) {
      continue;
    }
    const auto* c = chars16.data();
    while (c < end16) {
      out.push_back(*c);
      c++;
    }
  }
  return out;
}
} // namespace metaforce
