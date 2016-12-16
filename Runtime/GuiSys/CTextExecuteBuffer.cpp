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
    CTextRenderBuffer ret(CTextRenderBuffer::EMode::AllocTally);

    {
        CFontRenderState rendState;
        for (const std::shared_ptr<CInstruction>& inst : x0_instList)
            inst->Invoke(rendState, &ret);
    }

    ret.SetMode(CTextRenderBuffer::EMode::BufferFill);

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
    if (!xa4_curLine)
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
                x18_textState.x48_font.GetObj()->GetSize(x18_textState.x0_drawStrOpts,
                                                          w, h, &space, 1);
                if (xa0_curBlock->x14_direction == ETextDirection::Horizontal)
                {
                    xa4_curLine->x8_curX += w;
                    xbc_spaceDistance = w;
                }
                else
                {
                    xa4_curLine->xc_curY += h;
                    xbc_spaceDistance = h;
                }
            }
        }
    }

    if (charCur > wordCur)
        AddStringFragment(wordCur, charCur - wordCur);
}

void CTextExecuteBuffer::AddStringFragment(const wchar_t* str, int len)
{
    if (xa0_curBlock->x14_direction == ETextDirection::Horizontal)
        for (int i=0 ; i<len ;)
            i += WrapOneLTR(str + i, len - i);
}

int CTextExecuteBuffer::WrapOneLTR(const wchar_t* str, int len)
{
    if (!x18_textState.x48_font)
        return len;

    CRasterFont* font = x18_textState.x48_font.GetObj();
    int rem = len;
    int w, h;
    x18_textState.x48_font.GetObj()->GetSize(x18_textState.x0_drawStrOpts,
                                              w, h, str, len);

    if (x18_textState.x7c_enableWordWrap)
    {
        if (w + xa4_curLine->x8_curX > xa0_curBlock->xc_blockExtentX &&
            xa4_curLine->x4_wordCount > 1 &&
            xb0_curX + w < xa0_curBlock->xc_blockExtentX)
        {
            MoveWordLTR();
        }
        if (w + xa4_curLine->x8_curX > xa0_curBlock->xc_blockExtentX && len > 1)
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
                    x18_textState.x48_font.GetObj()->GetSize(x18_textState.x0_drawStrOpts,
                                                              w, h, str, rem);
                }

            } while (w + xa4_curLine->x8_curX > xa0_curBlock->xc_blockExtentX && rem > 1);
        }
    }

    xac_curY = std::max(xac_curY, font->GetMonoHeight());

    xa4_curLine->TestLargestFont(font->GetMonoWidth(),
                                 font->GetMonoHeight(),
                                 font->GetBaseline());

    xa4_curLine->x8_curX += w;
    xa0_curBlock->x2c_lineX = std::max(xa0_curBlock->x2c_lineX, xa4_curLine->x8_curX);
    xb0_curX += w;

    x0_instList.emplace(x0_instList.cend(), new CTextInstruction(str, rem));

    if (rem != len)
        StartNewLine();

    return rem;
}

void CTextExecuteBuffer::MoveWordLTR()
{
    xa4_curLine->xc_curY = std::min(xa4_curLine->xc_curY, xb8_);
    xbc_spaceDistance = 0;
    --xa4_curLine->x4_wordCount;
    TerminateLineLTR();

    xa4_curLine = static_cast<CLineInstruction*>(x0_instList.emplace(xa8_curWordIt,
        new CLineInstruction(x18_textState.x80_just, x18_textState.x84_vjust))->get());

    x0_instList.emplace(xa8_curWordIt, new CWordInstruction());

    ++xa0_curBlock->x34_lineCount;
}

void CTextExecuteBuffer::StartNewLine()
{
    if (xa4_curLine)
        TerminateLine();

    xa8_curWordIt = x0_instList.emplace(x0_instList.cend(),
        new CLineInstruction(x18_textState.x80_just, x18_textState.x84_vjust));
    xbc_spaceDistance = 0;

    StartNewWord();
    ++xa0_curBlock->x34_lineCount;
}

void CTextExecuteBuffer::StartNewWord()
{
    xa8_curWordIt = x0_instList.emplace(x0_instList.cend(), new CWordInstruction());
    xb0_curX = 0;
    xac_curY = 0;
    xb4_ = xa4_curLine->x8_curX;
    xb8_ = xa4_curLine->xc_curY;
    ++xa4_curLine->x4_wordCount;
}

void CTextExecuteBuffer::TerminateLine()
{
    if (xa0_curBlock->x14_direction == ETextDirection::Horizontal)
        TerminateLineLTR();
}

void CTextExecuteBuffer::TerminateLineLTR()
{
    if (!xa4_curLine->xc_curY && x18_textState.x48_font)
    {
        xa4_curLine->xc_curY = xa4_curLine->x10_largestMonoHeight;
    }

    if (xa0_curBlock->x1c_vertJustification == EVerticalJustification::Full)
    {
        xa0_curBlock->x30_lineY += xa4_curLine->xc_curY;
    }
    else
    {
        xa0_curBlock->x30_lineY += x18_textState.x78_extraLineSpace +
                                   xa4_curLine->xc_curY * x18_textState.x74_lineSpacing;
    }
}

void CTextExecuteBuffer::AddPopState()
{
    x0_instList.emplace(x0_instList.cend(), new CPopStateInstruction());

    x18_textState = xc4_stateStack.back();
    xc4_stateStack.pop_back();

    if (!xa4_curLine->x8_curX)
    {
        xa4_curLine->x1c_just = x18_textState.x80_just;
        xa4_curLine->x20_vjust = x18_textState.x84_vjust;
    }
}

void CTextExecuteBuffer::AddPushState()
{
    x0_instList.emplace(x0_instList.cend(), new CPushStateInstruction());
    xc4_stateStack.push_back(x18_textState);
}

void CTextExecuteBuffer::AddVerticalJustification(EVerticalJustification vjust)
{
    x18_textState.x84_vjust = vjust;
    if (!xa4_curLine)
        return;
    if (xa4_curLine->x8_curX)
        return;
    xa4_curLine->x20_vjust = vjust;
}

void CTextExecuteBuffer::AddJustification(EJustification just)
{
    x18_textState.x80_just = just;
    if (!xa4_curLine)
        return;
    if (xa4_curLine->x8_curX)
        return;
    xa4_curLine->x1c_just = just;
}

void CTextExecuteBuffer::AddLineExtraSpace(s32 space)
{
    x0_instList.emplace(x0_instList.cend(), new CLineExtraSpaceInstruction(space));
    x18_textState.x78_extraLineSpace = space;
}

void CTextExecuteBuffer::AddLineSpacing(float spacing)
{
    x0_instList.emplace(x0_instList.cend(), new CLineSpacingInstruction(spacing));
    x18_textState.x74_lineSpacing = spacing;
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
    if (!xa4_curLine)
        StartNewLine();

    if (xa0_curBlock)
    {
        const CTexture* tex = image.x4_texs[0].GetObj();
        int width = tex->GetWidth() * image.x14_pointsPerTexel.x;
        int height = tex->GetHeight() * image.x14_pointsPerTexel.y;
        xa4_curLine->TestLargestFont(width, height, height);

        if (xa0_curBlock->x14_direction == ETextDirection::Horizontal)
            if (xa4_curLine->x8_curX > width)
                xa0_curBlock->x2c_lineX = xa4_curLine->x8_curX;
    }

    x0_instList.emplace(x0_instList.cend(), new CImageInstruction(image));
}

void CTextExecuteBuffer::AddFont(const TToken<CRasterFont>& font)
{
    x0_instList.emplace(x0_instList.cend(), new CFontInstruction(font));
    x18_textState.x48_font = font;

    if (xa0_curBlock)
        xa0_curBlock->TestLargestFont(font->GetMonoWidth(),
                                      font->GetMonoHeight(),
                                      font->GetBaseline());

    if (xa4_curLine)
        xa4_curLine->TestLargestFont(font->GetMonoWidth(),
                                     font->GetMonoHeight(),
                                     font->GetBaseline());
}

void CTextExecuteBuffer::EndBlock()
{
    if (xa4_curLine)
        TerminateLine();
    xa4_curLine = nullptr;
    xa0_curBlock = nullptr;
}

void CTextExecuteBuffer::BeginBlock(s32 offX, s32 offY, s32 padX, s32 padY,
                                    ETextDirection dir, EJustification just,
                                    EVerticalJustification vjust)
{
    x0_instList.emplace(x0_instList.cend(),
        new CBlockInstruction(offX, offY, padX, padY, dir, just, vjust));

    if (x18_textState.x48_font)
    {
        CRasterFont* font = x18_textState.x48_font.GetObj();
        s32 baseline = font->GetBaseline();
        s32 monoH = font->GetMonoHeight();
        s32 monoW = font->GetMonoWidth();
        static_cast<CBlockInstruction&>(*x0_instList.back()).
            TestLargestFont(monoW, monoH, baseline);
    }

    x18_textState.x0_drawStrOpts.x0_direction = dir;
    x18_textState.x80_just = just;
    x18_textState.x84_vjust = vjust;
}

void CTextExecuteBuffer::Clear()
{
    x0_instList.clear();
    x18_textState = CSaveableState();
    xa0_curBlock = nullptr;
    xa4_curLine = nullptr;
    xa8_curWordIt = x0_instList.begin();
    xb4_ = 0;
    xb8_ = 0;
    xbc_spaceDistance = 0;
}

}
