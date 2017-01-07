#ifndef __URDE_CMATERIALFILTER_HPP__
#define __URDE_CMATERIALFILTER_HPP__

#include "CMaterialList.hpp"
#include "zeus/Math.hpp"
namespace urde
{
class CMaterialFilter
{
public:
    enum class EFilterType
    {
        Include,
        One,
        Exclude,
        IncludeExclude
    };
private:
    CMaterialList x0_include;
    CMaterialList x8_exclude;
    EFilterType x10_type = EFilterType::IncludeExclude;
public:
    static const CMaterialFilter skPassEverything;

    CMaterialFilter(const CMaterialList& include, const CMaterialList& exclude, EFilterType type);

    static CMaterialFilter MakeInclude(const CMaterialList& include);

    static CMaterialFilter MakeExclude(const CMaterialList& exclude);

    static CMaterialFilter MakeIncludeExclude(const CMaterialList& include, const CMaterialList& exclude);

    const CMaterialList& GetIncludeList() const { return x0_include; }
    const CMaterialList& GetExcludeList() const { return x8_exclude; }
    bool Passes(const CMaterialList&) const { return false;}
};
}
#endif // __CMATERIALFILTER_HPP__
