#ifndef __URDE_CRASTERFONT_HPP__
#define __URDE_CRASTERFONT_HPP__

#include "IOStreams.hpp"
#include "CToken.hpp"
#include "zeus/CVector2i.hpp"

namespace urde
{

class IObjectStore;
class CTexture;
/* TODO: Move these elsewhere */
struct CDrawStringOptions
{
    s32 x0_;
    u32 x10_;
};

class CTextRenderBuffer
{
public:
    void AddCharacter(const zeus::CVector2i& pos, s16 chr, u32 unk)
    {
    }
};

/* NOTE: Is this a good place for CGlyph and CKernPair? */
class CGlyph
{
private:
    s16     x0_leftPadding;
    s16     x2_advance;
    s16     x4_rightPadding;
    float   x8_startU;
    float   xc_startV;
    float   x10_endU;
    float   x14_endV;
    s16     x18_cellWidth;
    s16     x1a_cellHeight;
    s16     x1c_baseline;
    s16     x1e_kernStart;
public:
    CGlyph() = default;
    CGlyph(s16 a, s16 b, s32 c, float startU, float startV, float endU, float endV,
           s16 cellWidth, s16 cellHeight, s16 baseline, s16 kernStart)
        : x0_leftPadding(a), x2_advance(b), x4_rightPadding(c),
          x8_startU(startU), xc_startV(startV), x10_endU(endU), x14_endV(endV),
          x18_cellWidth(cellWidth), x1a_cellHeight(cellHeight),
          x1c_baseline(baseline), x1e_kernStart(kernStart)
    {}

    s16 GetA()          const { return x0_leftPadding; }
    s16 GetB()          const { return x2_advance; }
    s16 GetC()          const { return x4_rightPadding; }
    float GetStartU()   const { return x8_startU; }
    float GetStartV()   const { return xc_startV; }
    float GetEndU()     const { return x10_endU; }
    float GetEndV()     const { return x14_endV; }
    s16 GetCellWidth()  const { return x18_cellWidth; }
    s16 GetCellHeight() const { return x1a_cellHeight; }
    s16 GetBaseline()   const { return x1c_baseline; }
    s16 GetKernStart()  const { return x1e_kernStart; }

};

class CKernPair
{
private:
    wchar_t x0_first;
    wchar_t x2_second;
    s32     x4_howMuch;

public:
    CKernPair() = default;
    CKernPair(wchar_t first, wchar_t second, s32 howMuch)
        : x0_first(first), x2_second(second), x4_howMuch(howMuch)
    {}

    wchar_t GetFirst()  const { return x0_first; }
    wchar_t GetSecond() const { return x2_second; }
    s32 GetHowMuch()    const { return x4_howMuch; }
};

class CFontInfo
{
    bool x0_ = false;
    bool x1_ = false;
    s32 x4_ = 0;
    s32 x8_fontSize = 0;
    char* xc_name = 0;

public:
    CFontInfo() = default;
    CFontInfo(bool a, bool b, s32 c, s32 fontSize, const char* name)
        : x0_(a), x1_(b), x4_(c), x8_fontSize(fontSize)
    {
        strcpy(xc_name, name);
    }
};

class CRasterFont
{
    bool x0_ = false;
    s32 x4_monoWidth = 16;
    s32 x8_monoHeight = 16;
    std::vector<std::pair<wchar_t, CGlyph>> xc_glyphs;
    std::vector<CKernPair> x1c_kerning;
    s32 x28_lineMargin = 0;
    s32 x2c_mode = 0;
    CFontInfo x30_fontInfo;
    TToken<CTexture> x80_texture;
    bool x88_ = false;
    s32 x8c_baseline;
    s32 x90_lineMargin = 0;
    char* fontName;

    const CGlyph* InternalGetGlyph(wchar_t chr) const
    {
        u32 i = 0;
        for (; i < xc_glyphs.size(); ++i)
            if (chr == xc_glyphs[i].first)
                break;

        return &xc_glyphs[i].second;
    }

public:
    CRasterFont(CInputStream& in, IObjectStore& store);

    s32 GetMonoWidth()        const { return x4_monoWidth; }
    s32 GetMonoHeight()       const { return x8_monoHeight; }
    s32 GetMode()             const { return x2c_mode; }
    s32 GetLineMargin()       const { return x90_lineMargin; }
    s32 GetCarriageAdvance()  const { return GetLineMargin() + GetMonoHeight(); }

    s32 GetBaseline() const  { return x8c_baseline; }
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


    void SinglePassDrawString(const CDrawStringOptions&, int x, int y, int& xout, int& yout,
                              CTextRenderBuffer* renderBuf,
                              const wchar_t* str, s32 len) const;
    void DrawSpace(const CDrawStringOptions& opts, int x, int y, int& xout, int& yout, int len) const;
    void DrawString(const CDrawStringOptions& opts, int x, int y, int& xout, int& yout,
                    CTextRenderBuffer* renderBuf,
                    const wchar_t* str, int len) const;
    const CGlyph* GetGlyph(wchar_t chr) const
    {
        return InternalGetGlyph(chr);
    }
    void GetSize(const CDrawStringOptions& opts, int& width, int& height,
                 const wchar_t* str, int len) const;
};

std::unique_ptr<IObj> FRasterFontFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms);
}

#endif // __URDE_CRASTERFONT_HPP__
