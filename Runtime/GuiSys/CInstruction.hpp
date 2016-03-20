#ifndef __URDE_CINSTRUCTION_HPP__
#define __URDE_CINSTRUCTION_HPP__

#include "CToken.hpp"
#include "CGuiTextSupport.hpp"
#include <vector>

namespace urde
{
class CFontRenderState;
class CTextRenderBuffer;

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
    s32 x4_ = 0;
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
    void TestLargestFont(s32 w, s32 h, s32 b);
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
public:
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
};

class CTextInstruction : public CInstruction
{
public:
    CTextInstruction(const wchar_t* str, int len);
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
};

class CBlockInstruction : public CInstruction
{
    friend class CTextExecuteBuffer;
    s32 x4_;
    s32 x8_;
    s32 xc_;
    s32 x10_;
    ETextDirection x14_direction;
    EJustification x18_justification;
    EVerticalJustification x1c_vertJustification;
    s32 x20_ = 0;
    s32 x24_ = 0;
    s32 x28_ = 0;
    s32 x2c_ = 0;
    s32 x30_lineY = 0;
    s32 x34_lineCount = 0;
public:
    CBlockInstruction(s32 a, s32 b, s32 c, s32 d, ETextDirection dir,
                      EJustification just, EVerticalJustification vjust)
    : x4_(a), x8_(b), xc_(c), x10_(d), x14_direction(dir),
      x18_justification(just), x1c_vertJustification(vjust) {}
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
};

class CWordInstruction : public CInstruction
{
public:
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
};

}

#endif // __URDE_CINSTRUCTION_HPP__
