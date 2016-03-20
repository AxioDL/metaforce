#include "CInstruction.hpp"
#include "CFontRenderState.hpp"
#include "CTextRenderBuffer.hpp"
#include "CRasterFont.hpp"

namespace urde
{

void CInstruction::GetAssets(std::vector<CToken>& assetsOut) const
{
}

size_t CInstruction::GetAssetCount() const
{
    return 0;
}

void CColorInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const
{
    state.SetColor(x4_cType, x8_color);
}

void CColorOverrideInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const
{
    state.x30_[x4_overrideIdx] = true;
    zeus::CColor convCol = state.ConvertToTextureSpace(x8_color);
    state.x0_drawStrOpts.x4_vec[x4_overrideIdx] = convCol;
}

void CFontInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const
{
    buf->AddFontChange(x4_font);
    state.x14_font = x4_font;
    state.RefreshPalette();
}

void CFontInstruction::GetAssets(std::vector<CToken>& assetsOut) const
{
    assetsOut.push_back(x4_font);
}

size_t CFontInstruction::GetAssetCount() const
{
    return 1;
}

void CLineExtraSpaceInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const
{
    state.x44_extraLineSpace = x4_extraSpace;
}

void CLineInstruction::TestLargestFont(s32 monoW, s32 monoH, s32 baseline)
{
    if (!x18_largestBaseline)
        x18_largestBaseline = baseline;

    if (x14_largestMonoWidth < monoW)
        monoW = x14_largestMonoWidth;

    if (x10_largestMonoHeight < monoH)
    {
        x10_largestMonoHeight = monoH;
        x18_largestBaseline = baseline;
    }
}

void CLineInstruction::InvokeLTR(CFontRenderState& state) const
{
    switch (x1c_just)
    {
    case EJustification::Zero:
    case EJustification::Three:
    case EJustification::Four:
    case EJustification::Seven:
        state.x6c_curX = state.x54_curBlock->x4_offsetX;
        break;
    case EJustification::One:
    case EJustification::Eight:
        state.x6c_curX = state.x54_curBlock->x4_offsetX +
            state.x54_curBlock->xc_blockPaddingX / 2 - x8_curX / 2;
        break;
    case EJustification::Five:
        if (x4_ == 1)
        {
            state.x6c_curX = state.x54_curBlock->x4_offsetX +
                state.x54_curBlock->xc_blockPaddingX / 2 - x8_curX / 2;
        }
        else
        {
            state.x6c_curX = state.x54_curBlock->x4_offsetX +
                state.x54_curBlock->xc_blockPaddingX / 2 -
                state.x54_curBlock->x2c_lineX / 2;
        }
        break;
    case EJustification::Two:
    case EJustification::Nine:
        state.x6c_curX = state.x54_curBlock->x4_offsetX +
            state.x54_curBlock->xc_blockPaddingX - x8_curX;
        break;
    case EJustification::Six:
        state.x6c_curX = state.x54_curBlock->x4_offsetX +
            state.x54_curBlock->xc_blockPaddingX -
            state.x54_curBlock->x2c_lineX;
        break;
    default: break;
    }

    if (state.x74_currentLineInst)
    {
        const CLineInstruction& inst = *state.x74_currentLineInst;
        s32 val = 0;
        switch (state.x54_curBlock->x1c_vertJustification)
        {
        case EVerticalJustification::Zero:
        case EVerticalJustification::One:
        case EVerticalJustification::Two:
        case EVerticalJustification::Four:
        case EVerticalJustification::Five:
        case EVerticalJustification::Six:
            val = inst.xc_curY;
            break;
        case EVerticalJustification::Three:
            val = state.x54_curBlock->x10_blockPaddingY - state.x54_curBlock->x30_lineY;
            if (state.x54_curBlock->x34_lineCount > 1)
                val /= state.x54_curBlock->x34_lineCount - 1;
            else
                val = 0;
            val += inst.xc_curY;
            break;
        case EVerticalJustification::Seven:
            val = state.x54_curBlock->x24_largestMonoH;
            break;
        case EVerticalJustification::Eight:
            val = (inst.xc_curY - state.x54_curBlock->x24_largestMonoH) / 2 +
                state.x54_curBlock->x24_largestMonoH;
            break;
        case EVerticalJustification::Nine:
            val = state.x54_curBlock->x24_largestMonoH * 2 - inst.xc_curY;
            break;
        }

        if (state.x54_curBlock->x1c_vertJustification != EVerticalJustification::Three)
            val = val * state.x40_lineSpacing + state.x44_extraLineSpace;

        state.x70_curY += val;
    }
}

void CLineInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const
{
    InvokeLTR(state);
    state.xa0_ = true;
    state.x74_currentLineInst = this;
}

void CTextInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const
{
    int xOut, yOut;
    state.x14_font.GetObj()->DrawString(state.x0_drawStrOpts, state.x6c_curX,
                                        state.x74_currentLineInst->x18_largestBaseline + state.x70_curY,
                                        xOut, yOut, buf, x4_str.c_str(), x4_str.size());
    state.x6c_curX = xOut;
}

void CBlockInstruction::TestLargestFont(s32 monoW, s32 monoH, s32 baseline)
{
    if (!x28_largestBaseline)
        x28_largestBaseline = baseline;

    if (x20_largestMonoW < monoW)
        monoW = x20_largestMonoW;

    if (x24_largestMonoH < monoH)
    {
        x24_largestMonoH = monoH;
        x28_largestBaseline = baseline;
    }
}

void CBlockInstruction::SetupPositionLTR(CFontRenderState& state) const
{
    switch (x1c_vertJustification)
    {
    case EVerticalJustification::Zero:
    case EVerticalJustification::Three:
    case EVerticalJustification::Four:
    case EVerticalJustification::Seven:
        state.x70_curY = x8_offsetY;
        break;
    case EVerticalJustification::One:
    case EVerticalJustification::Five:
        state.x70_curY = x8_offsetY + (x10_blockPaddingY - x30_lineY) / 2;
        break;
    case EVerticalJustification::Eight:
        state.x70_curY = x8_offsetY + (x10_blockPaddingY - x34_lineCount * x24_largestMonoH) / 2;
        break;
    case EVerticalJustification::Two:
    case EVerticalJustification::Six:
        state.x70_curY = x8_offsetY + x10_blockPaddingY - x30_lineY;
        break;
    case EVerticalJustification::Nine:
        state.x70_curY = x8_offsetY + x10_blockPaddingY - x34_lineCount * x24_largestMonoH;
        break;
    }
}

void CBlockInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const
{
    state.x0_drawStrOpts.x0_direction = x14_direction;
    state.x54_curBlock = (CBlockInstruction*)this;
    if (x14_direction == ETextDirection::Horizontal)
        SetupPositionLTR(state);
}

}
