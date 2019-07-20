#include "locale.hpp"
#include <cstring>
#include <clocale>
#include <algorithm>

#undef min
#undef max

namespace locale {

std::vector<std::pair<std::string_view, std::string_view>> ListLocales() {
  std::vector<std::pair<std::string_view, std::string_view>> ret;
  ret.reserve(std::size_t(ELocale::MAXLocale));
  for (ELocale l = ELocale(0); l < ELocale::MAXLocale; l = ELocale(int(l) + 1))
    ret.emplace_back(GetName(l), GetFullName(l));
  return ret;
}

ELocale LookupLocale(std::string_view name) {
  for (ELocale l = ELocale(0); l < ELocale::MAXLocale; l = ELocale(int(l) + 1))
    if (!name.compare(GetName(l)))
      return l;
  return ELocale::Invalid;
}

ELocale SystemLocaleOrEnglish() {
  const char* sysLocale = std::setlocale(LC_ALL, nullptr);
  size_t sysLocaleLen = std::strlen(sysLocale);
  for (ELocale l = ELocale(0); l < ELocale::MAXLocale; l = ELocale(int(l) + 1)) {
    auto name = GetName(l);
    if (!name.compare(0, std::min(name.size(), sysLocaleLen), sysLocale))
      return l;
  }
  return ELocale::en_US;
}

} // namespace locale
