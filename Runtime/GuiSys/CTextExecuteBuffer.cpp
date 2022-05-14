#include "Runtime/GuiSys/CTextExecuteBuffer.hpp"

#include "Runtime/Graphics/CGraphicsPalette.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/GuiSys/CFontImageDef.hpp"
#include "Runtime/GuiSys/CFontRenderState.hpp"
#include "Runtime/GuiSys/CInstruction.hpp"
#include "Runtime/GuiSys/CRasterFont.hpp"
#include "Runtime/GuiSys/CTextRenderBuffer.hpp"
#include "Runtime/GuiSys/CWordBreakTables.hpp"

namespace metaforce {

CTextRenderBuffer CTextExecuteBuffer::BuildRenderBuffer(CGuiWidget::EGuiModelDrawFlags df) const {
  CTextRenderBuffer ret(CTextRenderBuffer::EMode::AllocTally);//, df);

  {
    CFontRenderState rendState;
    for (const std::shared_ptr<CInstruction>& inst : x0_instList)
      inst->Invoke(rendState, &ret);
  }

  ret.SetMode(CTextRenderBuffer::EMode::BufferFill);

  {
    CFontRenderState rendState;
    for (const std::shared_ptr<CInstruction>& inst : x0_instList)
      inst->Invoke(rendState, &ret);
  }

  return ret;
}

CTextRenderBuffer CTextExecuteBuffer::BuildRenderBufferPage(InstList::const_iterator start,
                                                            InstList::const_iterator pgStart,
                                                            InstList::const_iterator pgEnd,
                                                            CGuiWidget::EGuiModelDrawFlags df) const {
  CTextRenderBuffer ret(CTextRenderBuffer::EMode::AllocTally);//, df);

  {
    CFontRenderState rendState;
    for (auto it = start; it != pgStart; ++it) {
      const std::shared_ptr<CInstruction>& inst = *it;
      inst->PageInvoke(rendState, &ret);
    }
    for (auto it = pgStart; it != pgEnd; ++it) {
      const std::shared_ptr<CInstruction>& inst = *it;
      inst->Invoke(rendState, &ret);
    }
  }

  ret.SetMode(CTextRenderBuffer::EMode::BufferFill);

  {
    CFontRenderState rendState;
    for (auto it = start; it != pgStart; ++it) {
      const std::shared_ptr<CInstruction>& inst = *it;
      inst->PageInvoke(rendState, &ret);
    }
    for (auto it = pgStart; it != pgEnd; ++it) {
      const std::shared_ptr<CInstruction>& inst = *it;
      inst->Invoke(rendState, &ret);
    }
  }

  return ret;
}

std::list<CTextRenderBuffer> CTextExecuteBuffer::BuildRenderBufferPages(const zeus::CVector2i& extent,
                                                                        CGuiWidget::EGuiModelDrawFlags df) const {
  std::list<CTextRenderBuffer> ret;

  for (auto it = x0_instList.begin(); it != x0_instList.end();) {
    CTextRenderBuffer rbuf(CTextRenderBuffer::EMode::AllocTally);//, df);

    {
      CFontRenderState rstate;
      for (auto it2 = x0_instList.begin(); it2 != x0_instList.end(); ++it2) {
        const std::shared_ptr<CInstruction>& inst2 = *it2;
        inst2->Invoke(rstate, &rbuf);
      }
    }

    rbuf.SetMode(CTextRenderBuffer::EMode::BufferFill);

    InstList::const_iterator pageEnd = it;
    {
      CFontRenderState rstate;
      bool seekingToPage = true;
      for (auto it2 = x0_instList.begin(); it2 != x0_instList.end(); ++it2) {
        const std::shared_ptr<CInstruction>& inst2 = *it2;
        if (it2 == it)
          seekingToPage = false;
        if (seekingToPage) {
          inst2->PageInvoke(rstate, &rbuf);
        } else {
          inst2->Invoke(rstate, &rbuf);
          if (!rbuf.HasSpaceAvailable(zeus::CVector2i{}, extent))
            break;
          ++pageEnd;
        }
      }
    }

    ret.push_back(BuildRenderBufferPage(x0_instList.cbegin(), it, pageEnd, df));
    it = pageEnd;
  }

  return ret;
}

std::vector<CToken> CTextExecuteBuffer::GetAssets() const {
  size_t totalAssets = 0;
  for (const std::shared_ptr<CInstruction>& inst : x0_instList)
    totalAssets += inst->GetAssetCount();

  std::vector<CToken> ret;
  ret.reserve(totalAssets);

  for (const std::shared_ptr<CInstruction>& inst : x0_instList)
    inst->GetAssets(ret);

  return ret;
}

void CTextExecuteBuffer::AddString(const char16_t* str, int count) {
  if (!xa4_curLine)
    StartNewLine();

  const char16_t* charCur = str;
  const char16_t* wordCur = str;

  for (int ac = 0; *charCur && (ac < count || count == -1); ++charCur, ++ac) {
    if (*charCur == u'\n' || *charCur == u' ') {
      AddStringFragment(wordCur, charCur - wordCur);
      wordCur = charCur + 1;
      if (*charCur == u'\n') {
        StartNewLine();
      } else {
        StartNewWord();
        int w, h;
        char16_t space = u' ';
        x18_textState.x48_font->GetSize(x18_textState.x0_drawStrOpts, w, h, &space, 1);
        if (xa0_curBlock->x14_dir == ETextDirection::Horizontal) {
          xa4_curLine->x8_curX += w;
          xbc_spaceDistance = w;
        } else {
          xa4_curLine->xc_curY += h;
          xbc_spaceDistance = h;
        }
      }
    }
  }

  if (charCur > wordCur)
    AddStringFragment(wordCur, charCur - wordCur);
}

void CTextExecuteBuffer::AddStringFragment(const char16_t* str, int len) {
  if (xa0_curBlock->x14_dir == ETextDirection::Horizontal)
    for (int i = 0; i < len;)
      i += WrapOneLTR(str + i, len - i);
}

int CTextExecuteBuffer::WrapOneLTR(const char16_t* str, int len) {
  if (!x18_textState.x48_font)
    return len;

  CRasterFont* font = x18_textState.x48_font.GetObj();
  int rem = len;
  int w, h;
  x18_textState.x48_font->GetSize(x18_textState.x0_drawStrOpts, w, h, str, len);

  if (x18_textState.x7c_enableWordWrap) {
    if (w + xa4_curLine->x8_curX > xa0_curBlock->xc_blockExtentX && xa4_curLine->x4_wordCount >= 1 &&
        xb0_curX + w < xa0_curBlock->xc_blockExtentX) {
      MoveWordLTR();
    }
    if (w + xa4_curLine->x8_curX > xa0_curBlock->xc_blockExtentX && len > 1) {
      const char16_t* strEnd = str + len;
      int aRank = 5;

      do {
        --rem;
        --strEnd;
        int endRank = 4;
        if (len > 2)
          endRank = CWordBreakTables::GetEndRank(*(strEnd - 1));

        int beginRank = CWordBreakTables::GetBeginRank(*strEnd);

        if (endRank < aRank && endRank <= beginRank) {
          aRank = endRank;
        } else {
          x18_textState.x48_font->GetSize(x18_textState.x0_drawStrOpts, w, h, str, rem);
        }

      } while (w + xa4_curLine->x8_curX > xa0_curBlock->xc_blockExtentX && rem > 1);
    }
  }

  xac_curY = std::max(xac_curY, font->GetMonoHeight());

  xa4_curLine->TestLargestFont(font->GetMonoWidth(), font->GetMonoHeight(), font->GetBaseline());

  xa4_curLine->x8_curX += w;
  xa0_curBlock->x2c_lineX = std::max(xa0_curBlock->x2c_lineX, xa4_curLine->x8_curX);
  xb0_curX += w;

  x0_instList.emplace(x0_instList.cend(), std::make_shared<CTextInstruction>(str, rem));

  if (rem != len)
    StartNewLine();

  return rem;
}

void CTextExecuteBuffer::MoveWordLTR() {
  xa4_curLine->x8_curX -= (xb0_curX + xbc_spaceDistance);
  xa4_curLine->xc_curY = std::min(xa4_curLine->xc_curY, xb8_curWordY);
  xbc_spaceDistance = 0;
  --xa4_curLine->x4_wordCount;
  TerminateLineLTR();

  xa4_curLine = static_cast<CLineInstruction*>(
      x0_instList
          .emplace(xa8_curWordIt, std::make_shared<CLineInstruction>(x18_textState.x80_just, x18_textState.x84_vjust,
                                                                     xc0_imageBaseline))
          ->get());

  // Dunno what's up with this in the original; seems fine without
  x0_instList.emplace(xa8_curWordIt, std::make_shared<CWordInstruction>());

  ++xa0_curBlock->x34_lineCount;
}

void CTextExecuteBuffer::StartNewLine() {
  if (xa4_curLine)
    TerminateLine();

  xa8_curWordIt = x0_instList.emplace(
      x0_instList.cend(),
      std::make_shared<CLineInstruction>(x18_textState.x80_just, x18_textState.x84_vjust, xc0_imageBaseline));
  xa4_curLine = static_cast<CLineInstruction*>(xa8_curWordIt->get());
  xbc_spaceDistance = 0;

  StartNewWord();
  ++xa0_curBlock->x34_lineCount;
}

void CTextExecuteBuffer::StartNewWord() {
  xa8_curWordIt = x0_instList.emplace(x0_instList.cend(), std::make_shared<CWordInstruction>());
  xb0_curX = 0;
  xac_curY = 0;
  xb4_curWordX = xa4_curLine->x8_curX;
  xb8_curWordY = xa4_curLine->xc_curY;
  ++xa4_curLine->x4_wordCount;
}

void CTextExecuteBuffer::TerminateLine() {
  if (xa0_curBlock->x14_dir == ETextDirection::Horizontal)
    TerminateLineLTR();
}

void CTextExecuteBuffer::TerminateLineLTR() {
  if (!xa4_curLine->xc_curY /*&& x18_textState.IsFinishedLoading()*/) {
    xa4_curLine->xc_curY = std::max(xa4_curLine->GetHeight(), x18_textState.x48_font->GetCarriageAdvance());
  }

  if (xa0_curBlock->x1c_vertJustification == EVerticalJustification::Full) {
    xa0_curBlock->x30_lineY += xa4_curLine->xc_curY;
  } else {
    xa0_curBlock->x30_lineY += x18_textState.x78_extraLineSpace + xa4_curLine->xc_curY * x18_textState.x74_lineSpacing;
  }
}

void CTextExecuteBuffer::AddPopState() {
  x0_instList.emplace(x0_instList.cend(), std::make_shared<CPopStateInstruction>());

  x18_textState = xc4_stateStack.back();
  xc4_stateStack.pop_back();

  if (xa4_curLine->x8_curX == 0) {
    xa4_curLine->x28_just = x18_textState.x80_just;
    xa4_curLine->x2c_vjust = x18_textState.x84_vjust;
  }
}

void CTextExecuteBuffer::AddPushState() {
  x0_instList.emplace(x0_instList.cend(), std::make_shared<CPushStateInstruction>());
  xc4_stateStack.push_back(x18_textState);
}

void CTextExecuteBuffer::AddVerticalJustification(EVerticalJustification vjust) {
  x18_textState.x84_vjust = vjust;
  if (!xa4_curLine)
    return;
  if (xa4_curLine->x8_curX)
    return;
  xa4_curLine->x2c_vjust = vjust;
}

void CTextExecuteBuffer::AddJustification(EJustification just) {
  x18_textState.x80_just = just;
  if (!xa4_curLine)
    return;
  if (xa4_curLine->x8_curX)
    return;
  xa4_curLine->x28_just = just;
}

void CTextExecuteBuffer::AddLineExtraSpace(s32 space) {
  x0_instList.emplace(x0_instList.cend(), std::make_shared<CLineExtraSpaceInstruction>(space));
  x18_textState.x78_extraLineSpace = space;
}

void CTextExecuteBuffer::AddLineSpacing(float spacing) {
  x0_instList.emplace(x0_instList.cend(), std::make_shared<CLineSpacingInstruction>(spacing));
  x18_textState.x74_lineSpacing = spacing;
}

void CTextExecuteBuffer::AddRemoveColorOverride(int idx) {
  x0_instList.emplace(x0_instList.cend(), std::make_shared<CRemoveColorOverrideInstruction>(idx));
}

void CTextExecuteBuffer::AddColorOverride(int idx, const CTextColor& color) {
  x0_instList.emplace(x0_instList.cend(), std::make_shared<CColorOverrideInstruction>(idx, color));
}

void CTextExecuteBuffer::AddColor(EColorType tp, const CTextColor& color) {
  x0_instList.emplace(x0_instList.cend(), std::make_shared<CColorInstruction>(tp, color));
}

void CTextExecuteBuffer::AddImage(const CFontImageDef& image) {
  if (!xa4_curLine)
    StartNewLine();

  if (xa0_curBlock) {
    const CTexture* tex = image.x4_texs[0].GetObj();
    int width = tex->GetWidth() * image.x14_cropFactor.x();
    int height = tex->GetHeight() * image.x14_cropFactor.y();

    if (x18_textState.x7c_enableWordWrap && xa4_curLine->x8_curX + width > xa0_curBlock->xc_blockExtentX &&
        xa4_curLine->x4_wordCount > 1)
      StartNewLine();

    xa4_curLine->TestLargestImage(width, height, image.CalculateBaseline());

    xa4_curLine->x8_curX += width;
    if (xa4_curLine->x8_curX > width)
      xa0_curBlock->x2c_lineX = xa4_curLine->x8_curX;
  }

  x0_instList.emplace(x0_instList.cend(), std::make_shared<CImageInstruction>(image));
}

void CTextExecuteBuffer::AddFont(const TToken<CRasterFont>& font) {
  x0_instList.emplace(x0_instList.cend(), std::make_shared<CFontInstruction>(font));
  x18_textState.x48_font = font;

  if (xa0_curBlock)
    xa0_curBlock->TestLargestFont(font->GetMonoWidth(), font->GetMonoHeight(), font->GetBaseline());

  if (xa4_curLine)
    xa4_curLine->TestLargestFont(font->GetMonoWidth(), font->GetMonoHeight(), font->GetBaseline());
}

void CTextExecuteBuffer::EndBlock() {
  if (xa4_curLine)
    TerminateLine();
  xa4_curLine = nullptr;
  xa0_curBlock = nullptr;
}

void CTextExecuteBuffer::BeginBlock(s32 offX, s32 offY, s32 extX, s32 extY, bool imageBaseline, ETextDirection dir,
                                    EJustification just, EVerticalJustification vjust) {
  xc0_imageBaseline = imageBaseline;
  xa0_curBlock = static_cast<CBlockInstruction*>(
      x0_instList
          .emplace(x0_instList.cend(), std::make_shared<CBlockInstruction>(offX, offY, extX, extY, dir, just, vjust))
          ->get());

  if (x18_textState.x48_font) {
    CRasterFont* font = x18_textState.x48_font.GetObj();
    s32 baseline = font->GetBaseline();
    s32 monoH = font->GetMonoHeight();
    s32 monoW = font->GetMonoWidth();
    xa0_curBlock->TestLargestFont(monoW, monoH, baseline);
  }

  x18_textState.x0_drawStrOpts.x0_direction = dir;
  x18_textState.x80_just = just;
  x18_textState.x84_vjust = vjust;
}

void CTextExecuteBuffer::Clear() {
  x0_instList.clear();
  x18_textState = CSaveableState();
  xa0_curBlock = nullptr;
  xa4_curLine = nullptr;
  xa8_curWordIt = x0_instList.begin();
  xb4_curWordX = 0;
  xb8_curWordY = 0;
  xbc_spaceDistance = 0;
}

} // namespace metaforce
