#include "locale.hpp"
#include <cstring>
#include <clocale>
#include <algorithm>

#undef min
#undef max

extern "C" const uint8_t L_en_US[];
extern "C" size_t L_en_US_SZ;

extern "C" const uint8_t L_en_GB[];
extern "C" size_t L_en_GB_SZ;

extern "C" const uint8_t L_ja_JP[];
extern "C" size_t L_ja_JP_SZ;

namespace urde {

using namespace std::literals;

static const specter::Locale Locales[] = {{"en_US"sv, "US English"sv, L_en_US, L_en_US_SZ},
                                          {"en_GB"sv, "British English"sv, L_en_GB, L_en_GB_SZ},
                                          {"ja_JP"sv, "Japanese"sv, L_ja_JP, L_ja_JP_SZ}};

std::vector<std::pair<std::string_view, std::string_view>> ListLocales() {
  constexpr size_t localeCount = std::extent<decltype(Locales)>::value;
  std::vector<std::pair<std::string_view, std::string_view>> ret;
  ret.reserve(localeCount);
  for (size_t i = 0; i < localeCount; ++i) {
    const specter::Locale& l = Locales[i];
    ret.emplace_back(l.name(), l.fullName());
  }
  return ret;
}

const specter::Locale* LookupLocale(std::string_view name) {
  constexpr size_t localeCount = std::extent<decltype(Locales)>::value;
  for (size_t i = 0; i < localeCount; ++i) {
    const specter::Locale& l = Locales[i];
    if (!name.compare(l.name()))
      return &l;
  }
  return nullptr;
}

const specter::Locale* SystemLocaleOrEnglish() {
  const char* sysLocale = std::setlocale(LC_ALL, nullptr);
  size_t sysLocaleLen = std::strlen(sysLocale);
  constexpr size_t localeCount = std::extent<decltype(Locales)>::value;
  for (size_t i = 0; i < localeCount; ++i) {
    const specter::Locale& l = Locales[i];
    if (!l.name().compare(0, std::min(l.name().size(), sysLocaleLen), sysLocale))
      return &l;
  }
  return Locales;
}

} // namespace urde
