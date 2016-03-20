#include "CTextExecuteBuffer.hpp"
#include "CTextRenderBuffer.hpp"
#include "CFontRenderState.hpp"
#include "CFontImageDef.hpp"
#include "CInstruction.hpp"
#include "CRasterFont.hpp"
#include "CWordBreakTables.hpp"
#include "Graphics/CGraphicsPalette.hpp"
#include "Graphics/CTexture.hpp"

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

std::vector<CToken> CTextExecuteBuffer::GetAssets() const
{
    size_t totalAssets = 0;
    for (const std::shared_ptr<CInstruction>& inst : x0_instList)
        totalAssets += inst->GetAssetCount();

    std::vector<CToken> ret;
    ret.reserve(totalAssets);

    for (const std::shared_ptr<CInstruction>& inst : x0_instList)
        inst->GetAssets(ret);

    return ret;
}

void CTextExecuteBuffer::AddString(const wchar_t* str, int count)
{
    if (!x70_curLine)
        StartNewLine();

    const wchar_t* charCur = str;
    const wchar_t* wordCur = str;

    for (int ac=0 ; *charCur && (ac < count || count == -1) ; ++charCur, ++ac)
    {
        if (*charCur == L'\n' || *charCur == L' ')
        {
            AddStringFragment(wordCur, charCur - wordCur);
            wordCur = charCur + 1;
            if (*charCur == L'\n')
            {
                StartNewLine();
            }
            else
            {
                StartNewWord();
                int w, h;
                wchar_t space = L' ';
                x18_textState.x14_font.GetObj()->GetSize(x18_textState.x0_drawStrOpts,
                                                          w, h, &space, 1);
                if (x6c_curBlock->x14_direction == ETextDirection::Horizontal)
                {
                    x70_curLine->x8_curX += w;
                    x88_spaceDistance = w;
                }
                else
                {
                    x70_curLine->xc_curY += h;
                    x88_spaceDistance = h;
                }
            }
        }
    }

    if (charCur > wordCur)
        AddStringFragment(wordCur, charCur - wordCur);
}

void CTextExecuteBuffer::AddStringFragment(const wchar_t* str, int len)
{
    if (x6c_curBlock->x14_direction == ETextDirection::Horizontal)
        for (int i=0 ; i<len ;)
            i += WrapOneLTR(str + i, len - i);
}

int CTextExecuteBuffer::WrapOneLTR(const wchar_t* str, int len)
{
    if (!x18_textState.x14_font)
        return len;

    CRasterFont* font = x18_textState.x14_font.GetObj();
    int rem = len;
    int w, h;
    x18_textState.x14_font.GetObj()->GetSize(x18_textState.x0_drawStrOpts,
                                              w, h, str, len);

    if (x18_textState.x48_)
    {
        if (w + x70_curLine->x8_curX > x6c_curBlock->xc_blockPaddingX &&
            x70_curLine->x4_ > 1 &&
            x7c_curX + w < x6c_curBlock->xc_blockPaddingX)
        {
            MoveWordLTR();
        }
        if (w + x70_curLine->x8_curX > x6c_curBlock->xc_blockPaddingX && len > 1)
        {
            const wchar_t* strEnd = str + len;
            int aRank = 5;

            do
            {
                --rem;
                --strEnd;
                int endRank = 4;
                if (len > 2)
                    endRank = CWordBreakTables::GetEndRank(*(strEnd - 1));

                int beginRank = CWordBreakTables::GetBeginRank(*strEnd);

                if (endRank < aRank && endRank <= beginRank)
                {
                    aRank = endRank;
                }
                else
                {
                    x18_textState.x14_font.GetObj()->GetSize(x18_textState.x0_drawStrOpts,
                                                              w, h, str, rem);
                }

            } while (w + x70_curLine->x8_curX > x6c_curBlock->xc_blockPaddingX && rem > 1);
        }
    }

    x78_curY = std::max(x78_curY, font->GetMonoHeight());

    x70_curLine->TestLargestFont(font->GetMonoWidth(),
                                 font->GetMonoHeight(),
                                 font->GetBaseline());

    x70_curLine->x8_curX += w;
    x6c_curBlock->x2c_lineX = std::max(x6c_curBlock->x2c_lineX, x70_curLine->x8_curX);
    x7c_curX += w;

    x0_instList.emplace(x0_instList.cend(), new CTextInstruction(str, rem));

    if (rem != len)
        StartNewLine();

    return rem;
}

void CTextExecuteBuffer::MoveWordLTR()
{
    x70_curLine->xc_curY = std::min(x70_curLine->xc_curY, x84_);
    x88_spaceDistance = 0;
    --x70_curLine->x4_;
    TerminateLineLTR();

    x70_curLine = static_cast<CLineInstruction*>(x0_instList.emplace(x74_curWordIt,
        new CLineInstruction(x18_textState.x4c_just, x18_textState.x50_vjust))->get());

    x0_instList.emplace(x74_curWordIt, new CWordInstruction());

    ++x6c_curBlock->x34_lineCount;
}

void CTextExecuteBuffer::StartNewLine()
{
    if (x70_curLine)
        TerminateLine();

    x74_curWordIt = x0_instList.emplace(x0_instList.cend(),
        new CLineInstruction(x18_textState.x4c_just, x18_textState.x50_vjust));
    x88_spaceDistance = 0;

    StartNewWord();
    ++x6c_curBlock->x34_lineCount;
}

void CTextExecuteBuffer::StartNewWord()
{
    x74_curWordIt = x0_instList.emplace(x0_instList.cend(), new CWordInstruction());
    x7c_curX = 0;
    x78_curY = 0;
    x80_ = x70_curLine->x8_curX;
    x84_ = x70_curLine->xc_curY;
    ++x70_curLine->x4_;
}

void CTextExecuteBuffer::TerminateLine()
{
    if (x6c_curBlock->x14_direction == ETextDirection::Horizontal)
        TerminateLineLTR();
}

void CTextExecuteBuffer::TerminateLineLTR()
{
    if (!x70_curLine->xc_curY && x18_textState.x14_font)
    {
        x70_curLine->xc_curY = x70_curLine->x10_largestMonoHeight;
    }

    if (x6c_curBlock->x1c_vertJustification == EVerticalJustification::Three)
    {
        x6c_curBlock->x30_lineY += x70_curLine->xc_curY;
    }
    else
    {
        x6c_curBlock->x30_lineY += x18_textState.x44_extraLineSpace +
                                   x70_curLine->xc_curY * x18_textState.x40_lineSpacing;
    }
}

void CTextExecuteBuffer::AddPopState()
{
    x0_instList.emplace(x0_instList.cend(), new CPopStateInstruction());

    x18_textState = x8c_stateStack.back();
    x8c_stateStack.pop_back();

    if (!x70_curLine->x8_curX)
    {
        x70_curLine->x1c_just = x18_textState.x4c_just;
        x70_curLine->x20_vjust = x18_textState.x50_vjust;
    }
}

void CTextExecuteBuffer::AddPushState()
{
    x0_instList.emplace(x0_instList.cend(), new CPushStateInstruction());
    x8c_stateStack.push_back(x18_textState);
}

void CTextExecuteBuffer::AddVerticalJustification(EVerticalJustification vjust)
{
    x18_textState.x50_vjust = vjust;
    if (!x70_curLine)
        return;
    if (x70_curLine->x8_curX)
        return;
    x70_curLine->x20_vjust = vjust;
}

void CTextExecuteBuffer::AddJustification(EJustification just)
{
    x18_textState.x4c_just = just;
    if (!x70_curLine)
        return;
    if (x70_curLine->x8_curX)
        return;
    x70_curLine->x1c_just = just;
}

void CTextExecuteBuffer::AddLineExtraSpace(s32 space)
{
    x0_instList.emplace(x0_instList.cend(), new CLineExtraSpaceInstruction(space));
    x18_textState.x44_extraLineSpace = space;
}

void CTextExecuteBuffer::AddLineSpacing(float spacing)
{
    x0_instList.emplace(x0_instList.cend(), new CLineSpacingInstruction(spacing));
    x18_textState.x40_lineSpacing = spacing;
}

void CTextExecuteBuffer::AddRemoveColorOverride(int idx)
{
    x0_instList.emplace(x0_instList.cend(), new CRemoveColorOverrideInstruction(idx));
}

void CTextExecuteBuffer::AddColorOverride(int idx, const CTextColor& color)
{
    x0_instList.emplace(x0_instList.cend(), new CColorOverrideInstruction(idx, color));
}

void CTextExecuteBuffer::AddColor(EColorType tp, const CTextColor& color)
{
    x0_instList.emplace(x0_instList.cend(), new CColorInstruction(tp, color));
}

void CTextExecuteBuffer::AddImage(const CFontImageDef& image)
{
    if (!x70_curLine)
        StartNewLine();

    if (x6c_curBlock)
    {
        const CTexture* tex = image.x4_texs[0].GetObj();
        int width = tex->GetWidth() * image.x14_pointsPerTexel.x;
        int height = tex->GetHeight() * image.x14_pointsPerTexel.y;
        x70_curLine->TestLargestFont(width, height, height);

        if (x6c_curBlock->x14_direction == ETextDirection::Horizontal)
            if (x70_curLine->x8_curX > width)
                x6c_curBlock->x2c_lineX = x70_curLine->x8_curX;
    }

    x0_instList.emplace(x0_instList.cend(), new CImageInstruction(image));
}

void CTextExecuteBuffer::EndBlock()
{
    if (x70_curLine)
        TerminateLine();
    x70_curLine = nullptr;
    x6c_curBlock = nullptr;
}

void CTextExecuteBuffer::BeginBlock(s32 offX, s32 offY, s32 padX, s32 padY,
                                    ETextDirection dir, EJustification just,
                                    EVerticalJustification vjust)
{
    x0_instList.emplace(x0_instList.cend(),
        new CBlockInstruction(offX, offY, padX, padY, dir, just, vjust));

    if (x18_textState.x14_font)
    {
        CRasterFont* font = x18_textState.x14_font.GetObj();
        s32 baseline = font->GetBaseline();
        s32 monoH = font->GetMonoHeight();
        s32 monoW = font->GetMonoWidth();
        static_cast<CBlockInstruction&>(*x0_instList.back()).
            TestLargestFont(monoW, monoH, baseline);
    }

    x18_textState.x0_drawStrOpts.x0_direction = dir;
    x18_textState.x4c_just = just;
    x18_textState.x50_vjust = vjust;
}

void CTextExecuteBuffer::Clear()
{
    x0_instList.clear();
    x18_textState = CSaveableState();
    x6c_curBlock = nullptr;
    x70_curLine = nullptr;
    x74_curWordIt = x0_instList.begin();
    x80_ = 0;
    x84_ = 0;
    x88_spaceDistance = 0;
}

}
