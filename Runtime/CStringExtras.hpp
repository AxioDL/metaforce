#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <sstream>
#include <vector>

namespace metaforce {
class CInputStream;
class CStringExtras {
public:
  static std::string ConvertToANSI(std::u16string_view sv);
  static std::u16string ConvertToUNICODE(std::string_view sv);
  // Metaforce addition: UTF-8/16 compatible versions of the above
  static std::string ConvertToUTF8(std::u16string_view sv);
  static std::u16string ConvertToUTF16(std::string_view sv);

  // Checks if the provided views into string data can be considered equal or not based on
  // whether or not all their characters are equal to one another in a character insensitive manner.
  //
  // NOTE: This differs slightly from the actual version of this function within the game executable
  //       in order to better accomodate string views and potentially non-null-terminated string data.
  //
  //       In the game executable, the function essentially behaves like strcasecmp in that it returns
  //       an int indicating whether or not the first argument is less than, equal to,
  //       or greater than the second argument. Given no usages in the code depend on the less than or
  //       greater than cases, but rather just care about whether or not the strings are equal to one
  //       another, this is a safe change to make.
  //
  static bool CompareCaseInsensitive(std::string_view a, std::string_view b) {
    return std::equal(a.begin(), a.end(), b.begin(), b.end(),
                      [](char lhs, char rhs) { return std::tolower(lhs) == std::tolower(rhs); });
  }

  static int IndexOfSubstring(std::string_view haystack, std::string_view needle) {
    std::string str(haystack);
    std::transform(str.begin(), str.end(), str.begin(),
                   [](char c) { return std::tolower(static_cast<unsigned char>(c)); });
    const std::string::size_type s = str.find(needle);
    if (s == std::string::npos) {
      return -1;
    }
    return s;
  }

  static inline void ToLower(std::string& str) { std::transform(str.begin(), str.end(), str.begin(), ::tolower); }
  static std::string ReadString(CInputStream& in);
  static inline bool ParseBool(std::string_view boolean, bool* valid) {
    std::string val(boolean);
    // compare must be case insensitive
    // This is the cleanest solution since I only need to do it once
    ToLower(val);

    // Check for true first
    if (val == "true" || val == "1" || val == "yes" || val == "on") {
      if (valid)
        *valid = true;

      return true;
    }

    // Now false
    if (val == "false" || val == "0" || val == "no" || val == "off") {
      if (valid)
        *valid = true;

      return false;
    }

    // Well that could've gone better

    if (valid)
      *valid = false;

    return false;
  }

  static inline std::vector<std::string>& Split(std::string_view s, char delim, std::vector<std::string>& elems) {
    std::string tmps(s);
    std::stringstream ss(tmps);
    std::string item;

    while (std::getline(ss, item, delim)) {
      elems.push_back(item);
    }

    return elems;
  }

  static inline std::vector<std::string> Split(std::string_view s, char delim) {
    std::vector<std::string> elems;
    Split(s, delim, elems);
    return elems;
  }

  static inline std::string LeftTrim(const std::string &s)
  {
    size_t start = s.find_first_not_of(" \n\r\t\f\v");
    return (start == std::string::npos) ? "" : s.substr(start);
  }

  static inline std::string RightTrim(const std::string &s)
  {
    size_t end = s.find_last_not_of(" \n\r\t\f\v");
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
  }

  static inline std::string Trim(const std::string &s) {
    return RightTrim(LeftTrim(s));
  }
};

} // namespace metaforce
