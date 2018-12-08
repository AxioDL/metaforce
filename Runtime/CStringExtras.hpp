#pragma once

#include <string>
#include <cstring>

namespace urde {

class CStringExtras {
public:
  static int CompareCaseInsensitive(const char* a, const char* b) {
#if _WIN32
    return _stricmp(a, b);
#else
    return strcasecmp(a, b);
#endif
  }
  static int CompareCaseInsensitive(std::string_view a, std::string_view b) {
    return CompareCaseInsensitive(a.data(), b.data());
  }

  static int IndexOfSubstring(std::string_view haystack, std::string_view needle) {
    std::string str(haystack);
    std::transform(str.begin(), str.end(), str.begin(), tolower);
    std::string::size_type s = str.find(needle);
    if (s == std::string::npos)
      return -1;
    return s;
  }
};

} // namespace urde
