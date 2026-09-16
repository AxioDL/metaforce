#ifndef _CTEXTEXECUTEBUFFER
#define _CTEXTEXECUTEBUFFER

#include "Kyoto/Text/TextCommon.hpp"
#include "rstl/list.hpp"
#include "rstl/rc_ptr.hpp"

#include "Kyoto/Text/CSaveableState.hpp"
#include "rstl/string.hpp"

class CInstruction;
class CBlockInstruction;
class CLineInstruction;
class CFontImageDef;
class CTextRenderBuffer;
class CVector2i;

class CTextExecuteBuffer {
  typedef rstl::list< rstl::ncrc_ptr< CInstruction > > InstList;

public:
  CTextExecuteBuffer();

  CTextRenderBuffer BuildRenderBuffer() const;
  rstl::list< CTextRenderBuffer > BuildRenderBufferPages(const CVector2i& extent) const;
  rstl::vector< CToken > GetAssets() const;

  void AddFont(const TToken< CRasterFont >& font);
  void AddLineSpacing(float spacing);
  void AddLineExtraSpace(int space);
  void AddJustification(EJustification just);
  void AddVerticalJustification(EVerticalJustification just);
  void AddWordWrapping(const bool wrap) { x18_state.SetWordWrapping(wrap); }
  void AddPushState();
  void AddPopState();
  void AddImage(const CFontImageDef& image);
  void AddColor(EColorType type, const CTextColor& color);
  void AddColor(EColorType type, float r, float g, float b, float a) {
    AddColor(type, CTextColor(static_cast< uchar >(255.f * r), static_cast< uchar >(255.f * g),
                              static_cast< uchar >(255.f * b), static_cast< uchar >(255.f * a)));
  }
  void AddRemoveColorOverride(int idx);
  void AddColorOverride(int idx, const CTextColor& color);
  void AddString(const rstl::wstring& str) { AddString(str.data(), str.size()); }
  void AddString(const wchar_t* str, const int len);

  void BeginBlock(int x, int y, int width, int height, const bool imageBaseline, ETextDirection dir,
                  EJustification just, EVerticalJustification vjust);
  void EndBlock();

  void Clear();

private:
  static CTextRenderBuffer BuildRenderBufferPage(InstList::const_iterator start,
                                                 InstList::const_iterator pageStart,
                                                 InstList::const_iterator pageEnd);
  InstList::iterator Add(const rstl::ncrc_ptr< CInstruction >& instruction) {
    x0_instructions.push_back(instruction);
    return rstl::advance_iterator(x0_instructions.begin(), -1);
  }
  void AddStringFragment(const wchar_t* str, int len);
  int WrapOneLTR(const wchar_t* str, int len);
  void MoveWordLTR();
  void StartNewLine();
  void StartNewWord();
  void TerminateLine();
  void TerminateLineLTR();

  InstList x0_instructions;
  CSaveableState x18_state;
  CBlockInstruction* xa0_curBlock;
  CLineInstruction* xa4_curLine;
  InstList::iterator xa8_curWordIt;
  int xac_curY;
  int xb0_curX;
  int xb4_curWordX;
  int xb8_curWordY;
  int xbc_spaceDistance;
  bool xc0_imageBaseline;
  rstl::list< CSaveableState > xc4_stateStack;
};

CHECK_SIZEOF(CTextExecuteBuffer,
             (VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02 ? 0xe0 : 0xdc))

#endif // _CTEXTEXECUTEBUFFER
