#pragma once

#include <algorithm>
#include <cctype>
#include <string>

namespace urde {

class CStringExtras {
public:
  // Checks if the provided views into string data can be considered equal or not based on
  // whether or not all their characters are lexicographically equal to one another in
  // a character insensitive manner.
  //
  // NOTE: This differs slightly from the actual version of this function within the game executable
  //       in order to better accomodate string views and potentially non-null-terminated string data.
  //
  //       In the game executable, the function essentially behaves like strcasecmp in that it returns
  //       an int indicating whether or not the first argument is lexicographically less than, equal to,
  //       or greater than the second argument. Given no usages in the code depend on the less than or
  //       greater than cases, but rather just care about whether or not the strings are equal to one
  //       another, this is a safe change to make.
  //
  static bool CompareCaseInsensitive(std::string_view a, std::string_view b) {
    return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [](char lhs, char rhs) {
      return std::tolower(static_cast<unsigned char>(lhs)) < std::tolower(static_cast<unsigned char>(rhs));
    });
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
