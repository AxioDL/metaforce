#ifndef __URDE_CRASTERFONT_HPP__
#define __URDE_CRASTERFONT_HPP__

#include "IOStreams.hpp"
#include "CToken.hpp"
#include "zeus/CVector2i.hpp"
#include "CGuiTextSupport.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{
class IObjectStore;
class CDrawStringOptions;
class CTextRenderBuffer;

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
    s16     m_layer;

public:
    CGlyph() = default;
    CGlyph(s16 a, s16 b, s32 c, float startU, float startV, float endU, float endV,
           s16 cellWidth, s16 cellHeight, s16 baseline, s16 kernStart, s16 layer=0)
        : x0_leftPadding(a), x2_advance(b), x4_rightPadding(c),
          x8_startU(startU), xc_startV(startV), x10_endU(endU), x14_endV(endV),
          x18_cellWidth(cellWidth), x1a_cellHeight(cellHeight),
          x1c_baseline(baseline), x1e_kernStart(kernStart), m_layer(layer)
    {}

    s16 GetLeftPadding()  const { return x0_leftPadding; }
    s16 GetAdvance()      const { return x2_advance; }
    s16 GetRightPadding() const { return x4_rightPadding; }
    float GetStartU()     const { return x8_startU; }
    float GetStartV()     const { return xc_startV; }
    float GetEndU()       const { return x10_endU; }
    float GetEndV()       const { return x14_endV; }
    s16 GetCellWidth()    const { return x18_cellWidth; }
    s16 GetCellHeight()   const { return x1a_cellHeight; }
    s16 GetBaseline()     const { return x1c_baseline; }
    s16 GetKernStart()    const { return x1e_kernStart; }
    s16 GetLayer()        const { return m_layer; }
};

class CKernPair
{
private:
    char16_t x0_first;
    char16_t x2_second;
    s32     x4_howMuch;

public:
    CKernPair() = default;
    CKernPair(char16_t first, char16_t second, s32 howMuch)
    : x0_first(first), x2_second(second), x4_howMuch(howMuch)
    {}

    char16_t GetFirst()  const { return x0_first; }
    char16_t GetSecond() const { return x2_second; }
    s32 GetHowMuch()     const { return x4_howMuch; }
};

class CFontInfo
{
    bool x0_ = false;
    bool x1_ = false;
    s32 x4_ = 0;
    s32 x8_fontSize = 0;
    char xc_name[40];

public:
    CFontInfo() = default;
    CFontInfo(bool a, bool b, s32 c, s32 fontSize, const char* name)
        : x0_(a), x1_(b), x4_(c), x8_fontSize(fontSize)
    {
        strcpy(xc_name, name);
        (void)x0_;
        (void)x1_;
        (void)x4_;
        (void)x8_fontSize;
    }
};

class CRasterFont
{
    bool x0_initialized = false;
    s32 x4_monoWidth = 16;
    s32 x8_monoHeight = 16;
    std::vector<std::pair<char16_t, CGlyph>> xc_glyphs;
    std::vector<CKernPair> x1c_kerning;
    EColorType x2c_mode = EColorType::Main;
    CFontInfo x30_fontInfo;
    TLockedToken<CTexture> x80_texture;
    s32 x8c_baseline;
    s32 x90_lineMargin = 0;

    const CGlyph* InternalGetGlyph(char16_t chr) const
    {
        u32 i = 0;
        for (; i < xc_glyphs.size(); ++i)
            if (chr == xc_glyphs[i].first)
                break;

        if (i == xc_glyphs.size())
            return nullptr;
        return &xc_glyphs[i].second;
    }

public:
    CRasterFont(CInputStream& in, IObjectStore& store);

    s32 GetMonoWidth()        const { return x4_monoWidth; }
    s32 GetMonoHeight()       const { return x8_monoHeight; }
    EColorType GetMode()      const { return x2c_mode; }
    s32 GetLineMargin()       const { return x90_lineMargin; }
    s32 GetCarriageAdvance()  const { return GetLineMargin() + GetMonoHeight(); }

    s32 GetBaseline() const  { return x8c_baseline; }
    static s32 KernLookup(const std::vector<CKernPair>& kernTable, s32 kernStart, char16_t chr)
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
                              CTextRenderBuffer* renderBuf, const char16_t* str, s32 len) const;
    void DrawSpace(const CDrawStringOptions& opts, int x, int y, int& xout, int& yout, int len) const;
    void DrawString(const CDrawStringOptions& opts, int x, int y, int& xout, int& yout,
                    CTextRenderBuffer* renderBuf,
                    const char16_t* str, int len) const;
    const CGlyph* GetGlyph(char16_t chr) const
    {
        return InternalGetGlyph(chr);
    }
    void GetSize(const CDrawStringOptions& opts, int& width, int& height,
                 const char16_t* str, int len) const;
    const boo::ObjToken<boo::ITexture>& GetTexture()
    { return x80_texture->GetFontTexture(CTexture::EFontType(x2c_mode)); }

    bool IsFinishedLoading() const;
};

std::unique_ptr<IObj> FRasterFontFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                         CObjectReference* selfRef);
}

#endif // __URDE_CRASTERFONT_HPP__
