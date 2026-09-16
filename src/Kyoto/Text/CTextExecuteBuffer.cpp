#include "Kyoto/Text/CTextExecuteBuffer.hpp"

#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/Text/CBlockInstruction.hpp"
#include "Kyoto/Text/CColorInstruction.hpp"
#include "Kyoto/Text/CColorOverrideInstruction.hpp"
#include "Kyoto/Text/CFontImageDef.hpp"
#include "Kyoto/Text/CFontInstruction.hpp"
#include "Kyoto/Text/CFontRenderState.hpp"
#include "Kyoto/Text/CImageInstruction.hpp"
#include "Kyoto/Text/CLineExtraSpaceInstruction.hpp"
#include "Kyoto/Text/CLineInstruction.hpp"
#include "Kyoto/Text/CLineSpacingInstruction.hpp"
#include "Kyoto/Text/CPopStateInstruction.hpp"
#include "Kyoto/Text/CPushStateInstruction.hpp"
#include "Kyoto/Text/CRemoveColorOverrideInstruction.hpp"
#include "Kyoto/Text/CTextInstruction.hpp"
#include "Kyoto/Text/CTextRenderBuffer.hpp"
#include "Kyoto/Text/CWordBreakTables.hpp"
#include "Kyoto/Text/CWordInstruction.hpp"

#include "rstl/math.hpp"

CTextExecuteBuffer::CTextExecuteBuffer()
: xa0_curBlock(nullptr)
, xa4_curLine(nullptr)
, xa8_curWordIt(x0_instructions.end())
, xb4_curWordX(0)
, xb8_curWordY(0)
, xbc_spaceDistance(0)
, xc0_imageBaseline(false) {}

void CTextExecuteBuffer::Clear() {
  x0_instructions.clear();
  x18_state = CSaveableState();
  xa0_curBlock = nullptr;
  xa4_curLine = nullptr;
  xa8_curWordIt = x0_instructions.end();
  xb4_curWordX = 0;
  xb8_curWordY = 0;
  xbc_spaceDistance = 0;
}

void CTextExecuteBuffer::BeginBlock(int x, int y, int width, int height, bool imageBaseline,
                                    ETextDirection dir, EJustification just,
                                    EVerticalJustification vjust) {
  xc0_imageBaseline = imageBaseline;
  const rstl::ncrc_ptr< CInstruction > instruction =
      rs_new CBlockInstruction(x, y, width, height, dir, just, vjust);
  xa0_curBlock = static_cast< CBlockInstruction* >(instruction.GetPtr());
  if (x18_state.IsFinishedLoading()) {
    xa0_curBlock->TestLargestFont((*x18_state.GetFont())->GetMonoWidth(),
                                  (*x18_state.GetFont())->GetCarriageAdvance(),
                                  (*x18_state.GetFont())->GetBaseLine());
  }
  Add(instruction);
  x18_state.GetOptions().SetTextDirection(dir);
  x18_state.SetJustification(just);
  x18_state.SetVerticalJustification(vjust);
}

void CTextExecuteBuffer::EndBlock() {
  if (xa4_curLine) {
    TerminateLine();
  }
  xa4_curLine = nullptr;
  xa0_curBlock = nullptr;
}

void CTextExecuteBuffer::AddFont(const TToken< CRasterFont >& font) {
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CFontInstruction(font);
  Add(instruction);
  x18_state.SetFont(font);
  if (font.IsLoaded()) {
    if (xa0_curBlock) {
      xa0_curBlock->TestLargestFont((*x18_state.GetFont())->GetMonoWidth(),
                                    (*x18_state.GetFont())->GetCarriageAdvance(),
                                    (*x18_state.GetFont())->GetBaseLine());
    }
    if (xa4_curLine) {
      xa4_curLine->TestLargestFont((*x18_state.GetFont())->GetMonoWidth(),
                                   (*x18_state.GetFont())->GetCarriageAdvance(),
                                   (*x18_state.GetFont())->GetBaseLine());
    }
  }
}

void CTextExecuteBuffer::AddImage(const CFontImageDef& image) {
  if (!xa4_curLine) {
    StartNewLine();
  }
  if (xa0_curBlock && image.IsLoaded()) {
    bool wrap = x18_state.x7c_enableWordWrap;
    if (wrap) {
      const int width = xa4_curLine->GetWidth() + image.GetWidth();
      wrap = width > xa0_curBlock->GetOutputWidth();
    }
    if (wrap) {
      wrap = xa4_curLine->GetWordCount() > 1;
    }
    if (wrap) {
      StartNewLine();
    }
    // For PAL only, using GetHeight instead of GetMonoHeight improves the match, but not fully
    xa4_curLine->TestLargestImage(image.GetMonoWidth(), image.GetMonoHeight(),
                                  image.CalculateBaseline());
    if (xa0_curBlock->GetTextDirection() == kTD_Horizontal) {
      xa4_curLine->AddWidth(image.GetWidth());
      if (xa4_curLine->GetWidth() > image.GetWidth()) {
        xa0_curBlock->SetWidth(xa4_curLine->GetWidth());
      }
    }
  }
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CImageInstruction(image);
  Add(instruction);
}

void CTextExecuteBuffer::AddColor(EColorType type, const CTextColor& color) {
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CColorInstruction(type, color);
  Add(instruction);
}

void CTextExecuteBuffer::AddColorOverride(int idx, const CTextColor& color) {
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CColorOverrideInstruction(idx, color);
  Add(instruction);
}

void CTextExecuteBuffer::AddRemoveColorOverride(int idx) {
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CRemoveColorOverrideInstruction(idx);
  Add(instruction);
}

void CTextExecuteBuffer::AddLineSpacing(float spacing) {
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CLineSpacingInstruction(spacing);
  Add(instruction);
  x18_state.SetLineSpacing(spacing);
}

void CTextExecuteBuffer::AddLineExtraSpace(int space) {
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CLineExtraSpaceInstruction(space);
  Add(instruction);
  x18_state.SetLineExtraSpace(space);
}

void CTextExecuteBuffer::AddJustification(EJustification just) {
  x18_state.SetJustification(just);
  if (xa4_curLine && xa4_curLine->GetWidth() == 0) {
    xa4_curLine->SetJustification(just);
  }
}

void CTextExecuteBuffer::AddVerticalJustification(EVerticalJustification just) {
  x18_state.SetVerticalJustification(just);
  if (xa4_curLine && xa4_curLine->GetWidth() == 0) {
    xa4_curLine->SetVerticalJustification(just);
  }
}

void CTextExecuteBuffer::AddPushState() {
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CPushStateInstruction();
  Add(instruction);
  xc4_stateStack.push_front(x18_state);
}

void CTextExecuteBuffer::AddPopState() {
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CPopStateInstruction();
  Add(instruction);
  x18_state = xc4_stateStack.front();
  xc4_stateStack.pop_front();
  if (xa4_curLine->GetWidth() == 0) {
    xa4_curLine->SetJustification(x18_state.GetJustification());
    xa4_curLine->SetVerticalJustification(x18_state.GetVerticalJustification());
  }
}

void CTextExecuteBuffer::TerminateLineLTR() {
  if (xa4_curLine->GetY() == 0 && x18_state.IsFinishedLoading()) {
    xa4_curLine->SetHeight(
        rstl::max_val((*x18_state.GetFont())->GetCarriageAdvance(), xa4_curLine->GetHeight()));
  }
  xa0_curBlock->AddHeight(
      xa0_curBlock->GetVerticalJustification() == kVerticalJustification_Full
          ? xa4_curLine->GetY()
          : x18_state.GetLineExtraSpacing() +
                static_cast< int >(xa4_curLine->GetY() * x18_state.GetLineSpacing()));
}

void CTextExecuteBuffer::TerminateLine() {
  if (xa0_curBlock->GetTextDirection() == kTD_Horizontal) {
    TerminateLineLTR();
  }
}

void CTextExecuteBuffer::StartNewWord() {
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CWordInstruction();
  xa8_curWordIt = Add(instruction);
  xb0_curX = 0;
  xac_curY = 0;
  xb4_curWordX = xa4_curLine->GetWidth();
  xb8_curWordY = xa4_curLine->GetY();
  xa4_curLine->IncWords();
}

void CTextExecuteBuffer::StartNewLine() {
  if (xa4_curLine) {
    TerminateLine();
  }
  const rstl::ncrc_ptr< CInstruction > instruction =
      rstl::ncrc_ptr< CInstruction >(rs_new CLineInstruction(
          0, 0, 0, xc0_imageBaseline, x18_state.GetJustification(), x18_state.GetVerticalJustification()));
  xa8_curWordIt = Add(instruction);
  xa4_curLine = static_cast< CLineInstruction* >(instruction.GetPtr());
  xbc_spaceDistance = 0;
  StartNewWord();
  xa0_curBlock->IncLines();
}

void CTextExecuteBuffer::MoveWordLTR() {
  xa4_curLine->SubWidth(xb0_curX + xbc_spaceDistance);
  if (xa4_curLine->GetY() > xb8_curWordY) {
    xa4_curLine->SetHeight(xb8_curWordY);
  }
  xbc_spaceDistance = 0;
  xa4_curLine->DecWords();
  TerminateLineLTR();
  const rstl::ncrc_ptr< CInstruction > instruction =
      rs_new CLineInstruction(1, xb0_curX, xac_curY, xc0_imageBaseline, x18_state.GetJustification(),
                              x18_state.GetVerticalJustification());
  xa4_curLine = static_cast< CLineInstruction* >(instruction.GetPtr());
  x0_instructions.insert(xa8_curWordIt, instruction);
  x0_instructions.insert(xa8_curWordIt, rs_new CWordInstruction());
  xa0_curBlock->IncLines();
}

int CTextExecuteBuffer::WrapOneLTR(const wchar_t* str, int len) {
  int rem = len;
  if (x18_state.IsFinishedLoading()) {
    int width, height;
    (*x18_state.GetFont())->GetSize(x18_state.GetOptions(), width, height, str, len);
    if (x18_state.IsWordWrapping()) {
      if (width + xa4_curLine->GetWidth() > xa0_curBlock->GetOutputWidth() &&
          xa4_curLine->GetWordCount() > 1 && xb0_curX + width < xa0_curBlock->GetOutputWidth()) {
        MoveWordLTR();
      }
      if (width + xa4_curLine->GetWidth() > xa0_curBlock->GetOutputWidth() && len > 1) {
        int rank = 5;
        do {
          --rem;
          int endRank = rem > 1 ? CWordBreakTables::GetEndRank(str[rem - 1]) : 4;
          int beginRank = CWordBreakTables::GetBeginRank(str[rem]);
          if (endRank < rank && endRank <= beginRank) {
            rank = endRank;
          } else if (beginRank < rank && beginRank <= endRank) {
            rank = endRank;
          } else {
            (*x18_state.GetFont())->GetSize(x18_state.GetOptions(), width, height, str, rem);
          }
        } while (width + xa4_curLine->GetWidth() > xa0_curBlock->GetOutputWidth() && rem > 1);
      }
    }
    if ((*x18_state.GetFont())->GetCarriageAdvance() > xac_curY) {
      xac_curY = (*x18_state.GetFont())->GetCarriageAdvance();
    }
    xa4_curLine->TestLargestFont((*x18_state.GetFont())->GetMonoWidth(),
                                 (*x18_state.GetFont())->GetCarriageAdvance(),
                                 (*x18_state.GetFont())->GetBaseLine());
    xa4_curLine->AddWidth(width);
    if (xa4_curLine->GetWidth() > xa0_curBlock->GetLineX()) {
      xa0_curBlock->SetWidth(xa4_curLine->GetWidth());
    }
    xb0_curX += width;
    const rstl::ncrc_ptr< CInstruction > instruction = CTextInstruction::Create(str, rem);
    Add(instruction);
    if (rem != len) {
      StartNewLine();
    }
  }
  return rem;
}

void CTextExecuteBuffer::AddStringFragment(const wchar_t* str, int len) {
  int i = 0;
  if (xa0_curBlock->GetTextDirection() == kTD_Horizontal) {
    while (i != len) {
      i += WrapOneLTR(str + i, len - i);
    }
  }
}

void CTextExecuteBuffer::AddString(const wchar_t* str, int len) {
  if (!xa4_curLine) {
    StartNewLine();
  }
  int wordStart = 0;
  int i = 0;
  for (; str[i] && (i < len || len == -1); ++i) {
    if (str[i] == L'\n' || str[i] == L' ') {
      AddStringFragment(str + wordStart, i - wordStart);
      wordStart = i + 1;
      if (str[i] == L'\n') {
        StartNewLine();
      } else {
        StartNewWord();
        int width = 0;
        int height = 0;
        if (x18_state.IsFinishedLoading()) {
          wchar_t space = L' ';
          (*x18_state.GetFont())->GetSize(x18_state.GetOptions(), width, height, &space, 1);
        }
        if (xa0_curBlock->GetTextDirection() == kTD_Horizontal) {
          xa4_curLine->AddWidth(width);
          xbc_spaceDistance = width;
        } else {
          xa4_curLine->AddHeight(height);
          xbc_spaceDistance = height;
        }
      }
    }
  }
  if (i > wordStart) {
    AddStringFragment(str + wordStart, i - wordStart);
  }
}

rstl::vector< CToken > CTextExecuteBuffer::GetAssets() const {
  int count = 0;
  for (AUTO(it, x0_instructions.begin()); it != x0_instructions.end(); ++it) {
    count += (*it)->GetAssetCount();
  }
  rstl::vector< CToken > assets;
  if (count > 0) {
    assets.reserve(count);
    for (AUTO(it, x0_instructions.begin()); it != x0_instructions.end(); ++it) {
      (*it)->GetAssets(assets);
    }
  }
  return assets;
}

CTextRenderBuffer CTextExecuteBuffer::BuildRenderBuffer() const {
  CTextRenderBuffer buffer(CTextRenderBuffer::kM_AllocTally);
  {
    CFontRenderState state;
    for (AUTO(it, x0_instructions.begin()); it != x0_instructions.end(); ++it) {
      (*it)->Invoke(state, &buffer);
    }
  }
  buffer.SetMode(CTextRenderBuffer::kM_BufferFill);
  {
    CFontRenderState state;
    for (AUTO(it, x0_instructions.begin()); it != x0_instructions.end(); ++it) {
      (*it)->Invoke(state, &buffer);
    }
  }
  return buffer;
}

CTextRenderBuffer CTextExecuteBuffer::BuildRenderBufferPage(InstList::const_iterator start,
                                                            InstList::const_iterator pageStart,
                                                            InstList::const_iterator pageEnd) {
  CTextRenderBuffer buffer(CTextRenderBuffer::kM_AllocTally);
  {
    CFontRenderState state;
    for (InstList::const_iterator it = start; it != pageStart; ++it) {
      (*it)->PageInvoke(state, &buffer);
    }
    for (InstList::const_iterator it = pageStart; it != pageEnd; ++it) {
      (*it)->Invoke(state, &buffer);
    }
  }
  buffer.SetMode(CTextRenderBuffer::kM_BufferFill);
  {
    CFontRenderState state;
    for (InstList::const_iterator it = start; it != pageStart; ++it) {
      (*it)->PageInvoke(state, &buffer);
    }
    for (InstList::const_iterator it = pageStart; it != pageEnd; ++it) {
      (*it)->Invoke(state, &buffer);
    }
  }
  return buffer;
}

rstl::list< CTextRenderBuffer >
CTextExecuteBuffer::BuildRenderBufferPages(const CVector2i& extent) const {
  rstl::list< CTextRenderBuffer > pages;
  InstList::const_iterator it = x0_instructions.begin();
  while (it != x0_instructions.end()) {
    CTextRenderBuffer buffer(CTextRenderBuffer::kM_AllocTally);
    {
      CFontRenderState state;
      for (AUTO(it2, x0_instructions.begin()); it2 != x0_instructions.end(); ++it2) {
        (*it2)->Invoke(state, &buffer);
      }
    }
    buffer.SetMode(CTextRenderBuffer::kM_BufferFill);
    CFontRenderState state;
    InstList::const_iterator pageEnd = it;
    bool seeking = true;
    for (AUTO(it2, x0_instructions.begin()); it2 != x0_instructions.end(); ++it2) {
      if (it2 == it) {
        seeking = false;
      }
      if (seeking) {
        (*it2)->PageInvoke(state, &buffer);
      } else {
        (*it2)->Invoke(state, &buffer);
        if (!buffer.HasSpaceAvailable(CVector2i(0, 0), extent)) {
          break;
        }
        ++pageEnd;
      }
    }
    pages.push_back(BuildRenderBufferPage(x0_instructions.begin(), it, pageEnd));
    it = pageEnd;
  }
  return pages;
}
