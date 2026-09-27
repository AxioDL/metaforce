#ifndef _CBLOCKINSTRUCTION
#define _CBLOCKINSTRUCTION

#include "Kyoto/Text/CInstruction.hpp"
#include "Kyoto/Text/TextCommon.hpp"

class CBlockInstruction : public CInstruction {
public:
  CBlockInstruction(int offX, int offY, int extX, int extY, ETextDirection dir, EJustification just,
                    EVerticalJustification vjust)
  : mOffsetX(offX)
  , mOffsetY(offY)
  , mBlockExtentX(extX)
  , mBlockExtentY(extY)
  , mDir(dir)
  , mJustification(just)
  , mVertJustification(vjust)
  , mLargestMonoW(0)
  , mLargestMonoH(0)
  , mLargestBaseline(0)
  , mLineX(0)
  , mLineY(0)
  , mLineCount(0) {}

  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
  void SetupPositionLTR(CFontRenderState& state) const;
  void TestLargestFont(int monoW, int monoH, int baseline);
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const;

  int GetOutputX() const { return mOffsetX; }
  int GetOutputY() const { return mOffsetY; }
  int GetOutputWidth() const { return mBlockExtentX; }
  int GetOutputHeight() const { return mBlockExtentY; }
  ETextDirection GetTextDirection() const { return mDir; }
  EJustification GetJustification() const { return mJustification; }
  EVerticalJustification GetVerticalJustification() const { return mVertJustification; }
  int GetLargestFontWidth() const { return mLargestMonoW; }
  int GetLargestFontHeight() const { return mLargestMonoH; }
  int GetLineX() const { return mLineX; }
  int GetLineY() const { return mLineY; }
  int GetLines() const { return mLineCount; }
  void IncLines() { ++mLineCount; }
  void AddHeight(int height) { mLineY += height; }
  void SetWidth(int width) { mLineX = width; }

private:
  int mOffsetX;
  int mOffsetY;
  int mBlockExtentX;
  int mBlockExtentY;
  ETextDirection mDir;
  EJustification mJustification;
  EVerticalJustification mVertJustification;
  int mLargestMonoW;
  int mLargestMonoH;
  int mLargestBaseline;
  int mLineX;
  int mLineY;
  int mLineCount;
};

#endif // _CBLOCKINSTRUCTION
