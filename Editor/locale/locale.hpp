#pragma once
#include <type_traits>
#include <string_view>
#include <vector>

#define FMT_STRING_ALIAS 1
#define FMT_ENFORCE_COMPILE_STRING 1
#define FMT_USE_GRISU 0
#include <fmt/format.h>

namespace locale {
using namespace std::literals;
#include <locales-inl.hpp>

struct DoGetName {
  template <typename L>
  constexpr auto Do() { return L::Name; }
};
constexpr auto GetName(ELocale l) {
  return Do(l, DoGetName());
}

struct DoGetFullName {
  template <typename L>
  constexpr auto Do() { return L::FullName; }
};
constexpr auto GetFullName(ELocale l) {
  return Do(l, DoGetFullName());
}

template <typename Key>
struct DoTranslate {
  template <typename L, typename... Args>
  constexpr auto Do(Args&&... args) { return fmt::format(Lookup<L, Key>::Value(), std::forward<Args>(args)...); }
};
template <typename Key, typename... Args>
constexpr auto Translate(ELocale l, Args&&... args) {
  return Do(l, DoTranslate<Key>(), std::forward<Args>(args)...);
}

std::vector<std::pair<std::string_view, std::string_view>> ListLocales();
ELocale LookupLocale(std::string_view name);
ELocale SystemLocaleOrEnglish();

} // namespace locale
