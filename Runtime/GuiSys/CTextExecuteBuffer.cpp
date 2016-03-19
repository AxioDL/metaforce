#include "CTextExecuteBuffer.hpp"
#include "CTextRenderBuffer.hpp"
#include "CFontRenderState.hpp"
#include "CFontImageDef.hpp"
#include "CInstruction.hpp"
#include "Graphics/CGraphicsPalette.hpp"

namespace urde
{

CTextRenderBuffer CTextExecuteBuffer::CreateTextRenderBuffer() const
{
    CTextRenderBuffer ret(CTextRenderBuffer::EMode::Zero);

    {
        CFontRenderState rendState;
        for (const std::shared_ptr<CInstruction>& inst : x0_instList)
            inst->Invoke(rendState, &ret);
    }

    ret.SetMode(CTextRenderBuffer::EMode::One);

    {
        CFontRenderState rendState;
        for (const std::shared_ptr<CInstruction>& inst : x0_instList)
            inst->Invoke(rendState, &ret);
    }

    return ret;
}

void CTextExecuteBuffer::StartNewLine()
{
    if (x70_curLine)
        TerminateLine();

    x74_curInst = x0_instList.emplace(x0_instList.cend(),
        new CLineInstruction(x18_.x4c_, x18_.x50_));
    x88_curFontSize = 0;

    StartNewWord();
    ++x6c_curBlock->x34_wordCount;
}

void CTextExecuteBuffer::Clear()
{
    x0_instList.clear();
    x18_ = CSaveableState();
    x6c_curBlock = nullptr;
    x70_curLine = nullptr;
    x74_curInst = x0_instList.begin();
    x80_ = 0;
    x84_ = 0;
    x88_curFontSize = 0;
}

}
