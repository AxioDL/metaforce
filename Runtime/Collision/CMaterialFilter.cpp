#include "CMaterialFilter.hpp"

namespace urde
{
const CMaterialFilter CMaterialFilter::skPassEverything({0x00000000FFFFFFFF}, {0}, CMaterialFilter::EFilterType::Include);

CMaterialFilter::CMaterialFilter(const CMaterialList& include, const CMaterialList& exclude, CMaterialFilter::EFilterType type)
    : x0_include(include),
      x8_exclude(exclude),
      x10_type(type)
{
}

CMaterialFilter CMaterialFilter::MakeInclude(const CMaterialList& include)
{
    return CMaterialFilter(include, {0ull}, EFilterType::Include);
}

CMaterialFilter CMaterialFilter::MakeExclude(const CMaterialList& exclude)
{
    return CMaterialFilter({u64(0x00000000FFFFFFFF)}, exclude, EFilterType::Exclude);
}

CMaterialFilter CMaterialFilter::MakeIncludeExclude(const CMaterialList& include, const CMaterialList& exclude)
{
    return CMaterialFilter(include, exclude, EFilterType::IncludeExclude);
}

}
