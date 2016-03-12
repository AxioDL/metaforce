#ifndef __URDE_CRASTERFONT_HPP__
#define __URDE_CRASTERFONT_HPP__

#include "IOStreams.hpp"

namespace urde
{

class IObjectStore;

class CGlyph
{
    wchar_t x0_char;
    s16     x2_leftPadding;
    s16     x4_advance;
    s16     x6_rightPadding;
    float   x8_startU;
    float   xc_startV;
    float   x10_endU;
    float   x14_endV;
    s16     x18_cellWidth;
    s16     x1a_cellHeight;
    s16     x1c_baseline;
    s16     x1e_kernStart;

    s16 GetA()          const { return x2_leftPadding; }
    s16 GetB()          const { return x4_advance; }
    s16 GetC()          const { return x6_rightPadding; }
    float GetStartU()   const { return x8_startU; }
    float GetStartV()   const { return xc_startV; }
    float GetEndU()     const { return x10_endU; }
    float GetEndV()     const { return x14_endV; }
    s16 GetCellWidth()  const { return x18_cellWidth; }
    s16 GetCellHeight() const { return x1a_cellHeight; }
    s16 GetBaseline()   const { return x1c_baseline; }
    s16 GetKernStart()  const { return x1e_kernStart; }
};

struct CKernPair
{
    wchar_t x0_first;
    wchar_t x2_second;
    s32     x4_howMuch;

    wchar_t GetFirst()  const { return x0_first; }
    wchar_t GetSecond() const { return x2_second; }
    s32 GetHowMuch()    const { return x4_howMuch; }
};

class CRasterFont
{
    bool x0_ = false;
    s32 x4_ = 16;
    s32 x8_ = 16;
    std::vector<std::pair<wchar_t, CGlyph>> xc_glyphs;
    std::vector<CKernPair> x1c_kerning;
    s32 x30_;
    s32 x8c_;
    s32 x90_;
public:
    CRasterFont(CInputStream& in, IObjectStore& store);

    static s32 KernLookup(const std::vector<CKernPair>& kernTable, s32 kernStart, s32 chr)
    {
        auto iter = kernTable.cbegin() + kernStart;
        for (; iter != kernTable.cend() && iter->GetFirst() == kernTable[kernStart].GetFirst() ; ++iter)
        {
            if (iter->GetSecond() == chr)
                return iter->GetHowMuch();
        }

        return 0;
    }
};

}

#endif // __URDE_CRASTERFONT_HPP__
