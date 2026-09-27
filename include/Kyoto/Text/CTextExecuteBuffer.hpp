#ifndef _CTEXTEXECUTEBUFFER
#define _CTEXTEXECUTEBUFFER

#include "Kyoto/Text/CInstruction.hpp"
#include "Kyoto/Text/CSaveableState.hpp"
#include "Kyoto/Text/TextCommon.hpp"
#include "rstl/list.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/string.hpp"

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
  void AddWordWrapping(const bool wrap) { mState.SetWordWrapping(wrap); }
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
    mInstructions.push_back(instruction);
    return rstl::advance_iterator(mInstructions.begin(), -1);
  }
  void AddStringFragment(const wchar_t* str, int len);
  int WrapOneLTR(const wchar_t* str, int len);
  void MoveWordLTR();
  void StartNewLine();
  void StartNewWord();
  void TerminateLine();
  void TerminateLineLTR();

  InstList mInstructions;
  CSaveableState mState;
  CBlockInstruction* mCurBlock;
  CLineInstruction* mCurLine;
  InstList::iterator mCurWordIt;
  int mCurY;
  int mCurX;
  int mCurWordX;
  int mCurWordY;
  int mSpaceDistance;
  bool mImageBaseline;
  rstl::list< CSaveableState > mStateStack;
};

CHECK_SIZEOF(CTextExecuteBuffer,
             (VERSION >= VERSION_GM8P_00 ? 0xe0 : 0xdc))

#endif // _CTEXTEXECUTEBUFFER
