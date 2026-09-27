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
: mCurBlock(nullptr)
, mCurLine(nullptr)
, mCurWordIt(mInstructions.end())
, mCurWordX(0)
, mCurWordY(0)
, mSpaceDistance(0)
, mImageBaseline(false) {}

void CTextExecuteBuffer::Clear() {
  mInstructions.clear();
  mState = CSaveableState();
  mCurBlock = nullptr;
  mCurLine = nullptr;
  mCurWordIt = mInstructions.end();
  mCurWordX = 0;
  mCurWordY = 0;
  mSpaceDistance = 0;
}

void CTextExecuteBuffer::BeginBlock(int x, int y, int width, int height, bool imageBaseline,
                                    ETextDirection dir, EJustification just,
                                    EVerticalJustification vjust) {
  mImageBaseline = imageBaseline;
  const rstl::ncrc_ptr< CInstruction > instruction =
      rs_new CBlockInstruction(x, y, width, height, dir, just, vjust);
  mCurBlock = static_cast< CBlockInstruction* >(instruction.GetPtr());
  if (mState.IsFinishedLoading()) {
    mCurBlock->TestLargestFont((*mState.GetFont())->GetMonoWidth(),
                                  (*mState.GetFont())->GetCarriageAdvance(),
                                  (*mState.GetFont())->GetBaseLine());
  }
  Add(instruction);
  mState.GetOptions().SetTextDirection(dir);
  mState.SetJustification(just);
  mState.SetVerticalJustification(vjust);
}

void CTextExecuteBuffer::EndBlock() {
  if (mCurLine) {
    TerminateLine();
  }
  mCurLine = nullptr;
  mCurBlock = nullptr;
}

void CTextExecuteBuffer::AddFont(const TToken< CRasterFont >& font) {
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CFontInstruction(font);
  Add(instruction);
  mState.SetFont(font);
  if (font.IsLoaded()) {
    if (mCurBlock) {
      mCurBlock->TestLargestFont((*mState.GetFont())->GetMonoWidth(),
                                    (*mState.GetFont())->GetCarriageAdvance(),
                                    (*mState.GetFont())->GetBaseLine());
    }
    if (mCurLine) {
      mCurLine->TestLargestFont((*mState.GetFont())->GetMonoWidth(),
                                   (*mState.GetFont())->GetCarriageAdvance(),
                                   (*mState.GetFont())->GetBaseLine());
    }
  }
}

void CTextExecuteBuffer::AddImage(const CFontImageDef& image) {
  if (!mCurLine) {
    StartNewLine();
  }
  if (mCurBlock && image.IsLoaded()) {
    bool wrap = mState.mEnableWordWrap;
    if (wrap) {
      const int width = mCurLine->GetWidth() + image.GetWidth();
      wrap = width > mCurBlock->GetOutputWidth();
    }
    if (wrap) {
      wrap = mCurLine->GetWordCount() > 1;
    }
    if (wrap) {
      StartNewLine();
    }
    // For PAL only, using GetHeight instead of GetMonoHeight improves the match, but not fully
    mCurLine->TestLargestImage(image.GetMonoWidth(), image.GetMonoHeight(),
                                  image.CalculateBaseline());
    if (mCurBlock->GetTextDirection() == kTD_Horizontal) {
      mCurLine->AddWidth(image.GetWidth());
      if (mCurLine->GetWidth() > image.GetWidth()) {
        mCurBlock->SetWidth(mCurLine->GetWidth());
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
  mState.SetLineSpacing(spacing);
}

void CTextExecuteBuffer::AddLineExtraSpace(int space) {
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CLineExtraSpaceInstruction(space);
  Add(instruction);
  mState.SetLineExtraSpace(space);
}

void CTextExecuteBuffer::AddJustification(EJustification just) {
  mState.SetJustification(just);
  if (mCurLine && mCurLine->GetWidth() == 0) {
    mCurLine->SetJustification(just);
  }
}

void CTextExecuteBuffer::AddVerticalJustification(EVerticalJustification just) {
  mState.SetVerticalJustification(just);
  if (mCurLine && mCurLine->GetWidth() == 0) {
    mCurLine->SetVerticalJustification(just);
  }
}

void CTextExecuteBuffer::AddPushState() {
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CPushStateInstruction();
  Add(instruction);
  mStateStack.push_front(mState);
}

void CTextExecuteBuffer::AddPopState() {
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CPopStateInstruction();
  Add(instruction);
  mState = mStateStack.front();
  mStateStack.pop_front();
  if (mCurLine->GetWidth() == 0) {
    mCurLine->SetJustification(mState.GetJustification());
    mCurLine->SetVerticalJustification(mState.GetVerticalJustification());
  }
}

void CTextExecuteBuffer::TerminateLineLTR() {
  if (mCurLine->GetY() == 0 && mState.IsFinishedLoading()) {
    mCurLine->SetHeight(
        rstl::max_val((*mState.GetFont())->GetCarriageAdvance(), mCurLine->GetHeight()));
  }
  mCurBlock->AddHeight(
      mCurBlock->GetVerticalJustification() == kVerticalJustification_Full
          ? mCurLine->GetY()
          : mState.GetLineExtraSpacing() +
                static_cast< int >(mCurLine->GetY() * mState.GetLineSpacing()));
}

void CTextExecuteBuffer::TerminateLine() {
  if (mCurBlock->GetTextDirection() == kTD_Horizontal) {
    TerminateLineLTR();
  }
}

void CTextExecuteBuffer::StartNewWord() {
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CWordInstruction();
  mCurWordIt = Add(instruction);
  mCurX = 0;
  mCurY = 0;
  mCurWordX = mCurLine->GetWidth();
  mCurWordY = mCurLine->GetY();
  mCurLine->IncWords();
}

void CTextExecuteBuffer::StartNewLine() {
  if (mCurLine) {
    TerminateLine();
  }
  const rstl::ncrc_ptr< CInstruction > instruction =
      rstl::ncrc_ptr< CInstruction >(rs_new CLineInstruction(
          0, 0, 0, mImageBaseline, mState.GetJustification(), mState.GetVerticalJustification()));
  mCurWordIt = Add(instruction);
  mCurLine = static_cast< CLineInstruction* >(instruction.GetPtr());
  mSpaceDistance = 0;
  StartNewWord();
  mCurBlock->IncLines();
}

void CTextExecuteBuffer::MoveWordLTR() {
  mCurLine->SubWidth(mCurX + mSpaceDistance);
  if (mCurLine->GetY() > mCurWordY) {
    mCurLine->SetHeight(mCurWordY);
  }
  mSpaceDistance = 0;
  mCurLine->DecWords();
  TerminateLineLTR();
  const rstl::ncrc_ptr< CInstruction > instruction =
      rs_new CLineInstruction(1, mCurX, mCurY, mImageBaseline, mState.GetJustification(),
                              mState.GetVerticalJustification());
  mCurLine = static_cast< CLineInstruction* >(instruction.GetPtr());
  mInstructions.insert(mCurWordIt, instruction);
  mInstructions.insert(mCurWordIt, rs_new CWordInstruction());
  mCurBlock->IncLines();
}

int CTextExecuteBuffer::WrapOneLTR(const wchar_t* str, int len) {
  int rem = len;
  if (mState.IsFinishedLoading()) {
    int width, height;
    (*mState.GetFont())->GetSize(mState.GetOptions(), width, height, str, len);
    if (mState.IsWordWrapping()) {
      if (width + mCurLine->GetWidth() > mCurBlock->GetOutputWidth() &&
          mCurLine->GetWordCount() > 1 && mCurX + width < mCurBlock->GetOutputWidth()) {
        MoveWordLTR();
      }
      if (width + mCurLine->GetWidth() > mCurBlock->GetOutputWidth() && len > 1) {
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
            (*mState.GetFont())->GetSize(mState.GetOptions(), width, height, str, rem);
          }
        } while (width + mCurLine->GetWidth() > mCurBlock->GetOutputWidth() && rem > 1);
      }
    }
    if ((*mState.GetFont())->GetCarriageAdvance() > mCurY) {
      mCurY = (*mState.GetFont())->GetCarriageAdvance();
    }
    mCurLine->TestLargestFont((*mState.GetFont())->GetMonoWidth(),
                                 (*mState.GetFont())->GetCarriageAdvance(),
                                 (*mState.GetFont())->GetBaseLine());
    mCurLine->AddWidth(width);
    if (mCurLine->GetWidth() > mCurBlock->GetLineX()) {
      mCurBlock->SetWidth(mCurLine->GetWidth());
    }
    mCurX += width;
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
  if (mCurBlock->GetTextDirection() == kTD_Horizontal) {
    while (i != len) {
      i += WrapOneLTR(str + i, len - i);
    }
  }
}

void CTextExecuteBuffer::AddString(const wchar_t* str, int len) {
  if (!mCurLine) {
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
        if (mState.IsFinishedLoading()) {
          wchar_t space = L' ';
          (*mState.GetFont())->GetSize(mState.GetOptions(), width, height, &space, 1);
        }
        if (mCurBlock->GetTextDirection() == kTD_Horizontal) {
          mCurLine->AddWidth(width);
          mSpaceDistance = width;
        } else {
          mCurLine->AddHeight(height);
          mSpaceDistance = height;
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
  for (AUTO(it, mInstructions.begin()); it != mInstructions.end(); ++it) {
    count += (*it)->GetAssetCount();
  }
  rstl::vector< CToken > assets;
  if (count > 0) {
    assets.reserve(count);
    for (AUTO(it, mInstructions.begin()); it != mInstructions.end(); ++it) {
      (*it)->GetAssets(assets);
    }
  }
  return assets;
}

CTextRenderBuffer CTextExecuteBuffer::BuildRenderBuffer() const {
  CTextRenderBuffer buffer(CTextRenderBuffer::kM_AllocTally);
  {
    CFontRenderState state;
    for (AUTO(it, mInstructions.begin()); it != mInstructions.end(); ++it) {
      (*it)->Invoke(state, &buffer);
    }
  }
  buffer.SetMode(CTextRenderBuffer::kM_BufferFill);
  {
    CFontRenderState state;
    for (AUTO(it, mInstructions.begin()); it != mInstructions.end(); ++it) {
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
  InstList::const_iterator it = mInstructions.begin();
  while (it != mInstructions.end()) {
    CTextRenderBuffer buffer(CTextRenderBuffer::kM_AllocTally);
    {
      CFontRenderState state;
      for (AUTO(it2, mInstructions.begin()); it2 != mInstructions.end(); ++it2) {
        (*it2)->Invoke(state, &buffer);
      }
    }
    buffer.SetMode(CTextRenderBuffer::kM_BufferFill);
    CFontRenderState state;
    InstList::const_iterator pageEnd = it;
    bool seeking = true;
    for (AUTO(it2, mInstructions.begin()); it2 != mInstructions.end(); ++it2) {
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
    pages.push_back(BuildRenderBufferPage(mInstructions.begin(), it, pageEnd));
    it = pageEnd;
  }
  return pages;
}
