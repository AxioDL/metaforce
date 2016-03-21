#ifndef __URDE_CINSTRUCTION_HPP__
#define __URDE_CINSTRUCTION_HPP__

#include "CToken.hpp"
#include "CGuiTextSupport.hpp"
#include "CFontImageDef.hpp"
#include <vector>

namespace urde
{
class CFontRenderState;
class CTextRenderBuffer;
class CFontImageDef;

class CInstruction
{
public:
    virtual ~CInstruction() = default;
    virtual void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const=0;
    virtual void GetAssets(std::vector<CToken>& assetsOut) const;
    virtual size_t GetAssetCount() const;
};

class CColorInstruction : public CInstruction
{
    EColorType x4_cType;
    CTextColor x8_color;
public:
    CColorInstruction(EColorType tp, const CTextColor& color)
    : x4_cType(tp), x8_color(color) {}
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
};

class CColorOverrideInstruction : public CInstruction
{
    int x4_overrideIdx;
    CTextColor x8_color;
public:
    CColorOverrideInstruction(int idx, const CTextColor& color)
    : x4_overrideIdx(idx), x8_color(color) {}
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
};

class CFontInstruction : public CInstruction
{
    TLockedToken<CRasterFont> x4_font;
public:
    CFontInstruction(const TToken<CRasterFont>& font) : x4_font(font) {}
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
    void GetAssets(std::vector<CToken>& assetsOut) const;
    size_t GetAssetCount() const;
};

class CLineExtraSpaceInstruction : public CInstruction
{
    s32 x4_extraSpace;
public:
    CLineExtraSpaceInstruction(s32 extraSpace) : x4_extraSpace(extraSpace) {}
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
};

class CLineInstruction : public CInstruction
{
    friend class CTextExecuteBuffer;
    friend class CTextInstruction;
    friend class CImageInstruction;
    friend class CWordInstruction;

    s32 x4_wordCount = 0;
    s32 x8_curX = 0;
    s32 xc_curY = 0;
    s32 x10_largestMonoHeight = 0;
    s32 x14_largestMonoWidth = 0;
    s32 x18_largestBaseline = 0;
    EJustification x1c_just;
    EVerticalJustification x20_vjust;
public:
    CLineInstruction(EJustification just, EVerticalJustification vjust)
    : x1c_just(just), x20_vjust(vjust) {}
    void TestLargestFont(s32 monoW, s32 monoH, s32 baseline);
    void InvokeLTR(CFontRenderState& state) const;
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
};

class CLineSpacingInstruction : public CInstruction
{
    float x4_lineSpacing;
public:
    CLineSpacingInstruction(float spacing) : x4_lineSpacing(spacing) {}
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
};

class CPopStateInstruction : public CInstruction
{
public:
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
};

class CPushStateInstruction : public CInstruction
{
public:
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
};

class CRemoveColorOverrideInstruction : public CInstruction
{
    int x4_idx;
public:
    CRemoveColorOverrideInstruction(int idx) : x4_idx(idx) {}
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
};

class CImageInstruction : public CInstruction
{
    CFontImageDef x4_image;
public:
    CImageInstruction(const CFontImageDef& image) : x4_image(image) {}
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
};

class CTextInstruction : public CInstruction
{
    std::wstring x4_str; /* used to be a placement-new sized allocation */
public:
    CTextInstruction(const wchar_t* str, int len) : x4_str(str, len) {}
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
};

class CBlockInstruction : public CInstruction
{
    friend class CTextExecuteBuffer;
    friend class CLineInstruction;
    friend class CImageInstruction;
    friend class CWordInstruction;

    s32 x4_offsetX;
    s32 x8_offsetY;
    s32 xc_blockPaddingX;
    s32 x10_blockPaddingY;
    ETextDirection x14_direction;
    EJustification x18_justification;
    EVerticalJustification x1c_vertJustification;
    s32 x20_largestMonoW = 0;
    s32 x24_largestMonoH = 0;
    s32 x28_largestBaseline = 0;
    s32 x2c_lineX = 0;
    s32 x30_lineY = 0;
    s32 x34_lineCount = 0;
public:
    CBlockInstruction(s32 offX, s32 offY, s32 padX, s32 padY, ETextDirection dir,
                      EJustification just, EVerticalJustification vjust)
    : x4_offsetX(offX), x8_offsetY(offY),
      xc_blockPaddingX(padX), x10_blockPaddingY(padY), x14_direction(dir),
      x18_justification(just), x1c_vertJustification(vjust) {}
    void TestLargestFont(s32 monoW, s32 monoH, s32 baseline);
    void SetupPositionLTR(CFontRenderState& state) const;
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
};

class CWordInstruction : public CInstruction
{
public:
    void InvokeLTR(CFontRenderState& state) const;
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
};

}

#endif // __URDE_CINSTRUCTION_HPP__
