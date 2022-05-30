#pragma once

#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/GuiSys/CFontImageDef.hpp"
#include "Runtime/GuiSys/CGuiTextSupport.hpp"

namespace metaforce {
class CFontImageDef;
class CFontRenderState;
class CTextRenderBuffer;

class CInstruction {
public:
  virtual ~CInstruction() = default;
  virtual void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const = 0;
  virtual void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
  virtual void GetAssets(std::vector<CToken>& assetsOut) const;
  virtual size_t GetAssetCount() const;
};

class CColorInstruction : public CInstruction {
  EColorType x4_cType;
  CTextColor x8_color;

public:
  CColorInstruction(EColorType tp, const CTextColor& color) : x4_cType(tp), x8_color(color) {}
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
};

class CColorOverrideInstruction : public CInstruction {
  int x4_overrideIdx;
  CTextColor x8_color;

public:
  CColorOverrideInstruction(int idx, const CTextColor& color) : x4_overrideIdx(idx), x8_color(color) {}
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
};

class CFontInstruction : public CInstruction {
  TLockedToken<CRasterFont> x4_font;

public:
  explicit CFontInstruction(const TToken<CRasterFont>& font) : x4_font(font) {}
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void GetAssets(std::vector<CToken>& assetsOut) const override;
  size_t GetAssetCount() const override;
};

class CLineExtraSpaceInstruction : public CInstruction {
  s32 x4_extraSpace;

public:
  explicit CLineExtraSpaceInstruction(s32 extraSpace) : x4_extraSpace(extraSpace) {}
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
};

class CCharacterExtraSpaceInstruction : public CInstruction {
  s32 x4_extraSpace;
public:
  explicit CCharacterExtraSpaceInstruction(s32 extraSpace) : x4_extraSpace(extraSpace) {}
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
};
class CLineInstruction : public CInstruction {
  friend class CTextExecuteBuffer;
  friend class CTextInstruction;
  friend class CImageInstruction;
  friend class CWordInstruction;

  s32 x4_wordCount = 0;
  s32 x8_curX = 0;
  s32 xc_curY = 0;
  s32 x10_largestMonoHeight = 0;
  s32 x14_largestMonoWidth = 0;
  s32 x18_largestMonoBaseline = 0;
  s32 x1c_largestImageHeight = 0;
  s32 x20_largestImageWidth = 0;
  s32 x24_largestImageBaseline = 0;
  EJustification x28_just;
  EVerticalJustification x2c_vjust;
  bool x30_imageBaseline;

public:
  CLineInstruction(EJustification just, EVerticalJustification vjust, bool imageBaseline)
  : x28_just(just), x2c_vjust(vjust), x30_imageBaseline(imageBaseline) {}
  void TestLargestFont(s32 w, s32 h, s32 b);
  void TestLargestImage(s32 w, s32 h, s32 b);
  void InvokeLTR(CFontRenderState& state) const;
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;

  s32 GetHeight() const {
    if (x10_largestMonoHeight && !x30_imageBaseline)
      return x10_largestMonoHeight;
    else
      return x1c_largestImageHeight;
  }

  s32 GetBaseline() const {
    if (x10_largestMonoHeight && !x30_imageBaseline)
      return x18_largestMonoBaseline;
    else
      return x24_largestImageBaseline;
  }
};

class CLineSpacingInstruction : public CInstruction {
  float x4_lineSpacing;

public:
  explicit CLineSpacingInstruction(float spacing) : x4_lineSpacing(spacing) {}
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
};

class CPopStateInstruction : public CInstruction {
public:
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
};

class CPushStateInstruction : public CInstruction {
public:
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
};

class CRemoveColorOverrideInstruction : public CInstruction {
  int x4_idx;

public:
  explicit CRemoveColorOverrideInstruction(int idx) : x4_idx(idx) {}
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
};

class CImageInstruction : public CInstruction {
  CFontImageDef x4_image;

public:
  explicit CImageInstruction(const CFontImageDef& image) : x4_image(image) {}
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void GetAssets(std::vector<CToken>& assetsOut) const override;
  size_t GetAssetCount() const override;
};

class CTextInstruction : public CInstruction {
  std::u16string x4_str; /* used to be a placement-new sized allocation */
public:
  CTextInstruction(const char16_t* str, int len) : x4_str(str, len) {}
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
};

class CBlockInstruction : public CInstruction {
  friend class CTextExecuteBuffer;
  friend class CLineInstruction;
  friend class CImageInstruction;
  friend class CTextInstruction;
  friend class CWordInstruction;

  s32 x4_offsetX;
  s32 x8_offsetY;
  s32 xc_blockExtentX;
  s32 x10_blockExtentY;
  ETextDirection x14_dir;
  EJustification x18_justification;
  EVerticalJustification x1c_vertJustification;
  s32 x20_largestMonoW = 0;
  s32 x24_largestMonoH = 0;
  s32 x28_largestBaseline = 0;
  s32 x2c_lineX = 0;
  s32 x30_lineY = 0;
  s32 x34_lineCount = 0;

public:
  CBlockInstruction(s32 offX, s32 offY, s32 extX, s32 extY, ETextDirection dir, EJustification just,
                    EVerticalJustification vjust)
  : x4_offsetX(offX)
  , x8_offsetY(offY)
  , xc_blockExtentX(extX)
  , x10_blockExtentY(extY)
  , x14_dir(dir)
  , x18_justification(just)
  , x1c_vertJustification(vjust) {}
  void TestLargestFont(s32 monoW, s32 monoH, s32 baseline);
  void SetupPositionLTR(CFontRenderState& state) const;
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
};

class CWordInstruction : public CInstruction {
public:
  void InvokeLTR(CFontRenderState& state) const;
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
};

} // namespace metaforce
