#ifndef __URDE_CTEXTEXECUTEBUFFER_HPP__
#define __URDE_CTEXTEXECUTEBUFFER_HPP__

#include "CSaveableState.hpp"
#include "CGuiTextSupport.hpp"
#include <list>

namespace urde
{
class CInstruction;
class CFontImageDef;
class CTextRenderBuffer;
class CBlockInstruction;
class CLineInstruction;

class CTextExecuteBuffer
{
    friend class CGuiTextSupport;
    friend class CTextRenderBufferPages;
    using InstList = std::list<std::shared_ptr<CInstruction>>;

    InstList x0_instList;
    u32 x14_ = 0;
    CSaveableState x18_textState;
    CBlockInstruction* xa0_curBlock = nullptr;
    CLineInstruction* xa4_curLine = nullptr;
    std::list<std::shared_ptr<CInstruction>>::iterator xa8_curWordIt;
    s32 xac_curY;
    s32 xb0_curX;
    s32 xb4_curWordX = 0;
    s32 xb8_curWordY = 0;
    s32 xbc_spaceDistance = 0;
    bool xc0_ = false;
    std::list<CSaveableState> xc4_stateStack;
    u32 xd8_ = 0;

public:
    CTextExecuteBuffer()
    {
        xa8_curWordIt = x0_instList.begin();
    }

    CTextRenderBuffer BuildRenderBuffer() const;
    CTextRenderBuffer BuildRenderBufferPage(InstList::const_iterator start,
                                            InstList::const_iterator pgStart,
                                            InstList::const_iterator pgEnd) const;
    std::list<CTextRenderBuffer> BuildRenderBufferPages(const zeus::CVector2i& extent) const;
    std::vector<CToken> GetAssets() const;
    void AddString(const wchar_t* str, int len);
    void AddStringFragment(const wchar_t* str, int len);
    int WrapOneLTR(const wchar_t* str, int len);
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
    void AddLineSpacing(float spacing);
    void AddRemoveColorOverride(int idx);
    void AddColorOverride(int idx, const CTextColor& color);
    void AddColor(EColorType, const CTextColor& color);
    void AddImage(const CFontImageDef& image);
    void AddFont(const TToken<CRasterFont>& font);
    void EndBlock();
    void BeginBlock(s32 offX, s32 offY, s32 padX, s32 padY,
                    ETextDirection dir, EJustification just,
                    EVerticalJustification vjust);
    void Clear();
};

}

#endif // __URDE_CTEXTEXECUTEBUFFER_HPP__
