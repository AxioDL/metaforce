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
    void AddString(const wchar_t* str, int);
    void AddStringFragment(const wchar_t* str, int);
    int WrapOneLTR(const wchar_t* str, int);
    void MoveWordLTR();
    void StartNewLine();
    void StartNewWord();
    void TerminateLine();
    void TerminateLineLTR();
    void AddPopState();
    void AddPushState();
    void AddVerticalJustification(EVerticalJustification);
    void AddJustification(EJustification);
    void AddLineExtraSpace(s32 space);
    void AddLineSpacing(float);
    void AddRemoveColorOverride(int);
    void AddColorOverride(int, const CTextColor& color);
    void AddColor(EColorType, const CTextColor& color);
    void AddImage(const CFontImageDef& image);
    void EndBlock();
    void BeginBlock(int,int,int,int,ETextDirection,EJustification,EVerticalJustification);
    void Clear();
};

}

#endif // __URDE_CTEXTEXECUTEBUFFER_HPP__
