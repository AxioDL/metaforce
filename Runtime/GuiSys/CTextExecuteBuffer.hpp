#pragma once

#include <list>
#include <vector>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/GuiSys/CGuiTextSupport.hpp"
#include "Runtime/GuiSys/CSaveableState.hpp"

#include <zeus/CVector2i.hpp>

namespace metaforce {
class CBlockInstruction;
class CFontImageDef;
class CInstruction;
class CLineInstruction;
class CTextRenderBuffer;

class CTextExecuteBuffer {
  friend class CGuiTextSupport;
  friend class CTextRenderBufferPages;
  using InstList = std::list<std::shared_ptr<CInstruction>>;

  InstList x0_instList;
  u32 x14_ = 0;
  CSaveableState x18_textState;
  CBlockInstruction* xa0_curBlock = nullptr;
  CLineInstruction* xa4_curLine = nullptr;
  InstList::iterator xa8_curWordIt;
  s32 xac_curY = 0;
  s32 xb0_curX = 0;
  s32 xb4_curWordX = 0;
  s32 xb8_curWordY = 0;
  s32 xbc_spaceDistance = 0;
  bool xc0_imageBaseline = false;
  std::list<CSaveableState> xc4_stateStack;
  u32 xd8_ = 0;

public:
  CTextExecuteBuffer() : xa8_curWordIt{x0_instList.begin()} {}

  CTextRenderBuffer BuildRenderBuffer(CGuiWidget::EGuiModelDrawFlags df) const;
  CTextRenderBuffer BuildRenderBufferPage(InstList::const_iterator start, InstList::const_iterator pgStart,
                                          InstList::const_iterator pgEnd, CGuiWidget::EGuiModelDrawFlags df) const;
  std::list<CTextRenderBuffer> BuildRenderBufferPages(const zeus::CVector2i& extent,
                                                      CGuiWidget::EGuiModelDrawFlags df) const;
  std::vector<CToken> GetAssets() const;
  void AddString(const char16_t* str, int len);
  void AddStringFragment(const char16_t* str, int len);
  int WrapOneLTR(const char16_t* str, int len);
  void MoveWordLTR();
  void StartNewLine();
  void StartNewWord();
  void TerminateLine();
  void TerminateLineLTR();
  void AddPopState();
  void AddPushState();
  void AddVerticalJustification(EVerticalJustification vjust);
  void AddJustification(EJustification just);
  void AddLineExtraSpace(s32 space);
  void AddCharacterExtraSpace(s32 space);
  void AddLineSpacing(float spacing);
  void AddRemoveColorOverride(int idx);
  void AddColorOverride(int idx, const CTextColor& color);
  void AddColor(EColorType, const CTextColor& color);
  void AddImage(const CFontImageDef& image);
  void AddFont(const TToken<CRasterFont>& font);
  void EndBlock();
  void BeginBlock(s32 offX, s32 offY, s32 extX, s32 extY, bool imageBaseline, ETextDirection dir, EJustification just,
                  EVerticalJustification vjust);
  void Clear();
};

} // namespace metaforce
