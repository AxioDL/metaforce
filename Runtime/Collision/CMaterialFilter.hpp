#pragma once

#include "Runtime/Collision/CMaterialList.hpp"

namespace urde {
class CMaterialFilter {
public:
  enum class EFilterType { Always, Include, Exclude, IncludeExclude };

private:
  CMaterialList x0_include;
  CMaterialList x8_exclude;
  EFilterType x10_type = EFilterType::IncludeExclude;

public:
  static const CMaterialFilter skPassEverything;

  constexpr CMaterialFilter(const CMaterialList& include, const CMaterialList& exclude, EFilterType type) noexcept
  : x0_include(include), x8_exclude(exclude), x10_type(type) {}

  static constexpr CMaterialFilter MakeInclude(const CMaterialList& include) noexcept {
    return CMaterialFilter(include, {}, EFilterType::Include);
  }

  static constexpr CMaterialFilter MakeExclude(const CMaterialList& exclude) noexcept {
    return CMaterialFilter({u64(0x00000000FFFFFFFF)}, exclude, EFilterType::Exclude);
  }

  static constexpr CMaterialFilter MakeIncludeExclude(const CMaterialList& include,
                                                      const CMaterialList& exclude) noexcept {
    return CMaterialFilter(include, exclude, EFilterType::IncludeExclude);
  }

  constexpr const CMaterialList& GetIncludeList() const noexcept { return x0_include; }
  constexpr const CMaterialList& GetExcludeList() const noexcept { return x8_exclude; }
  constexpr CMaterialList& IncludeList() noexcept { return x0_include; }
  constexpr CMaterialList& ExcludeList() noexcept { return x8_exclude; }

  constexpr bool Passes(const CMaterialList& list) const noexcept {
    switch (x10_type) {
    case EFilterType::Always:
      return true;
    case EFilterType::Include:
      return (list.x0_list & x0_include.x0_list) != 0;
    case EFilterType::Exclude:
      return (list.x0_list & x8_exclude.x0_list) == 0;
    case EFilterType::IncludeExclude:
      if ((list.x0_list & x0_include.x0_list) == 0)
        return false;
      return (list.x0_list & x8_exclude.x0_list) == 0;
    default:
      return true;
    }
  }
};
} // namespace urde
