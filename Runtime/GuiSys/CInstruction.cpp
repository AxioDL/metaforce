#include "CInstruction.hpp"
#include "CFontRenderState.hpp"
#include "CTextRenderBuffer.hpp"
#include "CRasterFont.hpp"
#include "Graphics/CTexture.hpp"

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
    state.x30_colorOverrides[x4_overrideIdx] = true;
    zeus::CColor convCol = state.ConvertToTextureSpace(x8_color);
    state.x0_drawStrOpts.x4_colors[x4_overrideIdx] = convCol;
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
    case EJustification::Left:
    case EJustification::Full:
    case EJustification::NLeft:
    case EJustification::Seven:
        state.x6c_curX = state.x54_curBlock->x4_offsetX;
        break;
    case EJustification::Center:
    case EJustification::Eight:
        state.x6c_curX = state.x54_curBlock->x4_offsetX +
            state.x54_curBlock->xc_blockPaddingX / 2 - x8_curX / 2;
        break;
    case EJustification::NCenter:
        if (x4_wordCount == 1)
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
    case EJustification::Right:
    case EJustification::Nine:
        state.x6c_curX = state.x54_curBlock->x4_offsetX +
            state.x54_curBlock->xc_blockPaddingX - x8_curX;
        break;
    case EJustification::NRight:
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
        case EVerticalJustification::Top:
        case EVerticalJustification::Center:
        case EVerticalJustification::Bottom:
        case EVerticalJustification::NTop:
        case EVerticalJustification::NCenter:
        case EVerticalJustification::NBottom:
            val = inst.xc_curY;
            break;
        case EVerticalJustification::Full:
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

        if (state.x54_curBlock->x1c_vertJustification != EVerticalJustification::Full)
            val = val * state.x40_lineSpacing + state.x44_extraLineSpace;

        state.x70_curY += val;
    }
}

void CLineInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const
{
    InvokeLTR(state);
    state.xa0_lineInitialized = true;
    state.x74_currentLineInst = this;
}

void CLineSpacingInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const
{
    state.x40_lineSpacing = x4_lineSpacing;
}

void CPopStateInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const
{
    state.PopState();
}

void CPushStateInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const
{
    state.PushState();
}

void CRemoveColorOverrideInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const
{
    state.x30_colorOverrides[x4_idx] = false;
}

void CImageInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const
{
    if (x4_image.IsLoaded() && x4_image.x4_texs.size())
    {
        if (state.x54_curBlock->x14_direction == ETextDirection::Horizontal)
        {
            const CTexture* tex = x4_image.x4_texs[0].GetObj();
            if (buf)
            {
                zeus::CVector2i coords(state.x6c_curX,
                state.x70_curY + state.x74_currentLineInst->x18_largestBaseline -
                tex->GetHeight() * x4_image.x14_pointsPerTexel.y * 2 / 3);
                buf->AddImage(coords, x4_image);
            }
            state.x6c_curX += tex->GetWidth() * x4_image.x14_pointsPerTexel.x;
        }
    }
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
    case EVerticalJustification::Top:
    case EVerticalJustification::Full:
    case EVerticalJustification::NTop:
    case EVerticalJustification::Seven:
        state.x70_curY = x8_offsetY;
        break;
    case EVerticalJustification::Center:
    case EVerticalJustification::NCenter:
        state.x70_curY = x8_offsetY + (x10_blockPaddingY - x30_lineY) / 2;
        break;
    case EVerticalJustification::Eight:
        state.x70_curY = x8_offsetY + (x10_blockPaddingY - x34_lineCount * x24_largestMonoH) / 2;
        break;
    case EVerticalJustification::Bottom:
    case EVerticalJustification::NBottom:
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

void CWordInstruction::InvokeLTR(CFontRenderState& state) const
{
    CRasterFont* font = state.x14_font.GetObj();
    wchar_t space = L' ';
    int w, h;
    font->GetSize(state.x0_drawStrOpts, w, h, &space, 1);

    const CLineInstruction& inst = *state.x74_currentLineInst;
    switch (state.x54_curBlock->x18_justification)
    {
    case EJustification::Full:
        w += (state.x54_curBlock->xc_blockPaddingX - inst.x8_curX) / (inst.x4_wordCount - 1);
        break;
    case EJustification::NLeft:
    case EJustification::NCenter:
    case EJustification::NRight:
        w += (state.x54_curBlock->x2c_lineX - inst.x8_curX) / (inst.x4_wordCount - 1);
        break;
    default: break;
    }

    int wOut = state.x6c_curX;
    font->DrawSpace(state.x0_drawStrOpts, wOut,
                    inst.xc_curY - font->GetMonoHeight() + state.x70_curY, wOut, h, w);
    state.x6c_curX = wOut;
}

void CWordInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const
{
    if (state.xa0_lineInitialized)
    {
        state.xa0_lineInitialized = false;
        return;
    }

    if (state.x0_drawStrOpts.x0_direction == ETextDirection::Horizontal)
        InvokeLTR(state);
}

}
