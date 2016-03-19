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
    CSaveableState x18_;
    CBlockInstruction* x6c_curBlock = nullptr;
    CLineInstruction* x70_curLine = nullptr;
    std::list<std::shared_ptr<CInstruction>>::iterator x74_curInst;
    u32 x80_ = 0;
    u32 x84_ = 0;
    u32 x88_curFontSize = 0;
    u32 x90_ = 0;
    u32 x94_ = 0;
    u32 x98_ = 0;

public:
    CTextExecuteBuffer()
    {
        x74_curInst = x0_instList.begin();
    }

    CTextRenderBuffer CreateTextRenderBuffer() const;
    std::vector<TResId> GetAssets() const;
    void AddString(const wchar_t* str, int);
    void AddStringFragment(const wchar_t* str, int);
    void WrapOneTTB(const wchar_t* str, int);
    void WrapOneLTR(const wchar_t* str, int);
    void MoveWordTTB();
    void MoveWordLTR();
    void StartNewLine();
    void StartNewWord();
    void TerminateLine();
    void TerminateLineTTB();
    void TerminateLineLTR();
    void AddPopState();
    void AddPushState();
    void AddVerticalJustification(EVerticalJustification);
    void AddJustification(EJustification);
    void AddLineExtraSpace(int);
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
