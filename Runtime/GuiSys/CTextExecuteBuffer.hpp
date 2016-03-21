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

    std::list<std::shared_ptr<CInstruction>> x0_instList;
    u32 x14_ = 0;
    CSaveableState x18_textState;
    CBlockInstruction* x6c_curBlock = nullptr;
    CLineInstruction* x70_curLine = nullptr;
    std::list<std::shared_ptr<CInstruction>>::iterator x74_curWordIt;
    s32 x78_curY;
    s32 x7c_curX;
    s32 x80_ = 0;
    s32 x84_ = 0;
    s32 x88_spaceDistance = 0;
    std::vector<CSaveableState> x8c_stateStack;

public:
    CTextExecuteBuffer()
    {
        x74_curWordIt = x0_instList.begin();
    }

    CTextRenderBuffer CreateTextRenderBuffer() const;
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
