#include "CMaterialFilter.hpp"

namespace urde {
const CMaterialFilter CMaterialFilter::skPassEverything({0x00000000FFFFFFFF}, {0},
                                                        CMaterialFilter::EFilterType::Always);

CMaterialFilter::CMaterialFilter(const CMaterialList& include, const CMaterialList& exclude,
                                 CMaterialFilter::EFilterType type)
: x0_include(include), x8_exclude(exclude), x10_type(type) {}

CMaterialFilter CMaterialFilter::MakeInclude(const CMaterialList& include) {
  return CMaterialFilter(include, {0ull}, EFilterType::Include);
}

CMaterialFilter CMaterialFilter::MakeExclude(const CMaterialList& exclude) {
  return CMaterialFilter({u64(0x00000000FFFFFFFF)}, exclude, EFilterType::Exclude);
}

CMaterialFilter CMaterialFilter::MakeIncludeExclude(const CMaterialList& include, const CMaterialList& exclude) {
  return CMaterialFilter(include, exclude, EFilterType::IncludeExclude);
}

bool CMaterialFilter::Passes(const CMaterialList& list) const {
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

} // namespace urde
