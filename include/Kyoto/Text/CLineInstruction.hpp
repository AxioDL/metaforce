#ifndef _CLINEINSTRUCTION
#define _CLINEINSTRUCTION

#include "Kyoto/Text/CInstruction.hpp"
#include "Kyoto/Text/TextCommon.hpp"

class CLineInstruction : public CInstruction {
public:
  CLineInstruction(int words, int width, int height, const bool imageBaseline,
                   EJustification justification, EVerticalJustification verticalJustification)
  : mWordCount(words)
  , mCurX(width)
  , mCurY(height)
  , mLargestFontHeight(0)
  , mLargestFontWidth(0)
  , mLargestFontBaseline(0)
  , mLargestImageHeight(0)
  , mLargestImageWidth(0)
  , mLargestImageBaseline(0)
  , mJustification(justification)
  , mVerticalJustification(verticalJustification)
  , mImageBaseline(imageBaseline) {}

  int GetWordCount() const { return mWordCount; }
  int GetHeight() const;
  int GetBaseline() const;

  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void InvokeTTB(CFontRenderState& state) const;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
#if VERSION >= VERSION_GM8P_00
  bool IsLineInstruction() const override { return true; }
#endif

  void TestLargestFont(const int width, const int height, const int baseLine);
  void TestLargestImage(const int width, const int height, const int baseline);

  int GetX() const { return mCurX; }
  int GetWidth() const { return mCurX; }
  int GetY() const { return mCurY; }
  void IncWords() { ++mWordCount; }
  void DecWords() { --mWordCount; }
  void AddWidth(int width) { mCurX += width; }
  void SubWidth(int width) { mCurX -= width; }
  void AddHeight(int height) { mCurY += height; }
  void SetHeight(int height) { mCurY = height; }

  EJustification GetJustification() const { return mJustification; }
  void SetJustification(const EJustification justification) { mJustification = justification; }
  EVerticalJustification GetVerticalJustification() const { return mVerticalJustification; }
  void SetVerticalJustification(const EVerticalJustification justification) {
    mVerticalJustification = justification;
  }

private:
  int mWordCount;
  int mCurX;
  int mCurY;
  int mLargestFontHeight;
  int mLargestFontWidth;
  int mLargestFontBaseline;
  int mLargestImageHeight;
  int mLargestImageWidth;
  int mLargestImageBaseline;
  EJustification mJustification;
  EVerticalJustification mVerticalJustification;
  bool mImageBaseline;
};

CHECK_SIZEOF(CLineInstruction, 0x34)

#endif // _CLINEINSTRUCTION
