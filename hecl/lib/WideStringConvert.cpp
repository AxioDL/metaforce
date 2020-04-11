#include <logvisor/logvisor.hpp>
#include <utf8proc.h>

namespace hecl {
static logvisor::Module Log("hecl-wsconv");

std::string WideToUTF8(std::wstring_view src) {
  std::string retval;
  retval.reserve(src.length());
  for (wchar_t ch : src) {
    utf8proc_uint8_t mb[4];
    utf8proc_ssize_t c = utf8proc_encode_char(utf8proc_int32_t(ch), mb);
    if (c < 0) {
      Log.report(logvisor::Warning, FMT_STRING("invalid UTF-8 character while encoding"));
      return retval;
    }
    retval.append(reinterpret_cast<char*>(mb), c);
  }
  return retval;
}

std::string Char16ToUTF8(std::u16string_view src) {
  std::string retval;
  retval.reserve(src.length());
  for (char16_t ch : src) {
    utf8proc_uint8_t mb[4];
    utf8proc_ssize_t c = utf8proc_encode_char(utf8proc_int32_t(ch), mb);
    if (c < 0) {
      Log.report(logvisor::Warning, FMT_STRING("invalid UTF-8 character while encoding"));
      return retval;
    }
    retval.append(reinterpret_cast<char*>(mb), c);
  }
  return retval;
}

std::wstring UTF8ToWide(std::string_view src) {
  std::wstring retval;
  retval.reserve(src.length());
  const utf8proc_uint8_t* buf = reinterpret_cast<const utf8proc_uint8_t*>(src.data());
  while (*buf) {
    utf8proc_int32_t wc;
    utf8proc_ssize_t len = utf8proc_iterate(buf, -1, &wc);
    if (len < 0) {
      Log.report(logvisor::Warning, FMT_STRING("invalid UTF-8 character while decoding"));
      return retval;
    }
    buf += len;
    retval += wchar_t(wc);
  }
  return retval;
}

std::u16string UTF8ToChar16(std::string_view src) {
  std::u16string retval;
  retval.reserve(src.length());
  const utf8proc_uint8_t* buf = reinterpret_cast<const utf8proc_uint8_t*>(src.data());
  while (*buf) {
    utf8proc_int32_t wc;
    utf8proc_ssize_t len = utf8proc_iterate(buf, -1, &wc);
    if (len < 0) {
      Log.report(logvisor::Warning, FMT_STRING("invalid UTF-8 character while decoding"));
      return retval;
    }
    buf += len;
    retval += char16_t(wc);
  }
  return retval;
}

} // namespace hecl
