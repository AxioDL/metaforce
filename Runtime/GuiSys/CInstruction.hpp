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

class CExtraLineSpaceInstruction : public CInstruction
{
    s32 x4_extraSpace;
public:
    CExtraLineSpaceInstruction(s32 extraSpace) : x4_extraSpace(extraSpace) {}
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
};

class CLineInstruction : public CInstruction
{
    u32 x4_ = 0;
    u32 x8_ = 0;
    u32 xc_ = 0;
    u32 x10_ = 0;
    u32 x14_ = 0;
    u32 x18_ = 0;
    u32 x1c_;
    u32 x20_;
public:
    CLineInstruction(u32 a, u32 b) : x1c_(a), x20_(b) {}
    void TestLargestFont(int, int, int);
    void InvokeLTR(CFontRenderState& state) const;
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
};

class CLineSpacingInstruction : public CInstruction
{
};

class CPopStateInstruction : public CInstruction
{
};

class CPushStateInstruction : public CInstruction
{
};

class CRemoveColorOverrideInstruction : public CInstruction
{
};

class CImageInstruction : public CInstruction
{
};

class CTextInstruction : public CInstruction
{
};

class CBlockInstruction : public CInstruction
{
    friend class CTextExecuteBuffer;
    int x4_;
    int x8_;
    int xc_;
    int x10_;
    ETextDirection x14_direction;
    EJustification x18_justification;
    EVerticalJustification x1c_vertJustification;
    int x20_ = 0;
    int x24_ = 0;
    int x28_ = 0;
    int x2c_ = 0;
    int x30_ = 0;
    int x34_wordCount = 0;
public:
    CBlockInstruction(int a, int b, int c, int d, ETextDirection dir,
                      EJustification just, EVerticalJustification vjust)
    : x4_(a), x8_(b), xc_(c), x10_(d), x14_direction(dir),
      x18_justification(just), x1c_vertJustification(vjust) {}
    void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
};

class CWordInstruction : public CInstruction
{
};

}

#endif // __URDE_CINSTRUCTION_HPP__
