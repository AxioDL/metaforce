#ifndef __URDE_CSTRINGTABLE_HPP__
#define __URDE_CSTRINGTABLE_HPP__

#include "RetroTypes.hpp"

namespace urde
{
class CStringTable
{
    static const std::vector<FourCC> skLanguages;
    static FourCC mCurrentLanguage;
    u32 x0_stringCount = 0;
    std::unique_ptr<u8[]> x4_data = 0;
    u32 m_bufLen;
public:
    CStringTable(CInputStream& in);
    void LoadStringTable(CInputStream& in);

    std::wstring GetString(s32) const;
    static void SetLanguage(s32);
};
}
#endif // __URDE_CSTRINGTABLE_HPP__
