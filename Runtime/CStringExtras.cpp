#include "Runtime/CStringExtras.hpp"
#include "Runtime/Streams/CInputStream.hpp"

#include <cstdint>

namespace {

constexpr char32_t kReplacementChar = 0xFFFD;

void AppendUTF8(char32_t codePoint, std::string& out) {
  if (codePoint <= 0x7F) {
    out.push_back(static_cast<char>(codePoint));
  } else if (codePoint <= 0x7FF) {
    out.push_back(static_cast<char>(0xC0 | ((codePoint >> 6) & 0x1F)));
    out.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
  } else if (codePoint <= 0xFFFF) {
    out.push_back(static_cast<char>(0xE0 | ((codePoint >> 12) & 0x0F)));
    out.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
    out.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
  } else {
    out.push_back(static_cast<char>(0xF0 | ((codePoint >> 18) & 0x07)));
    out.push_back(static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F)));
    out.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
    out.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
  }
}

void AppendUTF16(char32_t codePoint, std::u16string& out) {
  if (codePoint <= 0xFFFF) {
    out.push_back(static_cast<char16_t>(codePoint));
    return;
  }

  codePoint -= 0x10000;
  out.push_back(static_cast<char16_t>(0xD800 + (codePoint >> 10)));
  out.push_back(static_cast<char16_t>(0xDC00 + (codePoint & 0x3FF)));
}

} // namespace

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
  out.reserve(sv.size());

  for (size_t i = 0; i < sv.size(); ++i) {
    char32_t codePoint = sv[i];
    if (codePoint >= 0xD800 && codePoint <= 0xDBFF) {
      if (i + 1 < sv.size()) {
        const char32_t low = sv[i + 1];
        if (low >= 0xDC00 && low <= 0xDFFF) {
          codePoint = 0x10000 + ((codePoint - 0xD800) << 10) + (low - 0xDC00);
          ++i;
        } else {
          codePoint = kReplacementChar;
        }
      } else {
        codePoint = kReplacementChar;
      }
    } else if (codePoint >= 0xDC00 && codePoint <= 0xDFFF) {
      codePoint = kReplacementChar;
    }

    AppendUTF8(codePoint, out);
  }
  return out;
}

std::u16string CStringExtras::ConvertToUTF16(std::string_view sv) {
  std::u16string out;
  out.reserve(sv.size());

  const auto* bytes = reinterpret_cast<const uint8_t*>(sv.data());
  const size_t len = sv.size();
  size_t i = 0;
  while (i < len) {
    const uint8_t c0 = bytes[i++];
    char32_t codePoint = kReplacementChar;

    if (c0 < 0x80) {
      codePoint = c0;
    } else if ((c0 & 0xE0) == 0xC0) {
      if (i < len) {
        const uint8_t c1 = bytes[i];
        if ((c1 & 0xC0) == 0x80) {
          const char32_t cp = ((c0 & 0x1F) << 6) | (c1 & 0x3F);
          if (cp >= 0x80) {
            codePoint = cp;
            ++i;
          }
        }
      }
    } else if ((c0 & 0xF0) == 0xE0) {
      if (i + 1 < len) {
        const uint8_t c1 = bytes[i];
        const uint8_t c2 = bytes[i + 1];
        if ((c1 & 0xC0) == 0x80 && (c2 & 0xC0) == 0x80) {
          const char32_t cp = ((c0 & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
          if (cp >= 0x800 && !(cp >= 0xD800 && cp <= 0xDFFF)) {
            codePoint = cp;
            i += 2;
          }
        }
      }
    } else if ((c0 & 0xF8) == 0xF0) {
      if (i + 2 < len) {
        const uint8_t c1 = bytes[i];
        const uint8_t c2 = bytes[i + 1];
        const uint8_t c3 = bytes[i + 2];
        if ((c1 & 0xC0) == 0x80 && (c2 & 0xC0) == 0x80 && (c3 & 0xC0) == 0x80) {
          const char32_t cp =
              ((c0 & 0x07) << 18) | ((c1 & 0x3F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
          if (cp >= 0x10000 && cp <= 0x10FFFF) {
            codePoint = cp;
            i += 3;
          }
        }
      }
    }

    AppendUTF16(codePoint, out);
  }
  return out;
}
} // namespace metaforce
