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
        Zero,
        One,
        Two,
        Three
    };
private:

    CMaterialList x0_include;
    CMaterialList x8_exclude;
    EFilterType x10_type = EFilterType::Three;
public:

    CMaterialFilter(const CMaterialList& include, const CMaterialList& exclude, EFilterType type)
        : x0_include(include),
          x8_exclude(exclude),
          x10_type(type)
    {
    }

    static CMaterialFilter MakeInclude(const CMaterialList& include)
    {
        return CMaterialFilter(include, {EMaterialTypes::Zero}, EFilterType::Zero);
    }

    static CMaterialFilter MakeExclude(const CMaterialList& exclude)
    {
        return CMaterialFilter({u64(0x00000000FFFFFFFF)}, exclude, EFilterType::Two);
    }

    static CMaterialFilter MakeIncludeExclude(const CMaterialList& include, const CMaterialList& exclude)
    {
        return CMaterialFilter(include, exclude, EFilterType::Three);
    }

    const CMaterialList& GetIncludeList() const { return x0_include; }
    const CMaterialList& GetExcludeList() const { return x8_exclude; }
    bool Passes(const CMaterialList&) const;
};
}
#endif // __CMATERIALFILTER_HPP__
