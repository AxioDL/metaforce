#include "CFontRenderState.hpp"
#include "CRasterFont.hpp"

namespace urde
{

CFontRenderState::CFontRenderState()
{
    x20_[0] = zeus::CColor::skWhite;
    x20_[1] = zeus::CColor::skGrey;
    x20_[2] = zeus::CColor::skWhite;
    RefreshPalette();
}

zeus::CColor CFontRenderState::ConvertToTextureSpace(const CTextColor& col) const
{
    return col;
}

void CFontRenderState::PopState()
{
    static_cast<CSaveableState&>(*this) = xa4_pushedStates.back();
    xa4_pushedStates.pop_back();
    RefreshPalette();
}

void CFontRenderState::PushState()
{
    xa4_pushedStates.push_back(*this);
}

void CFontRenderState::SetColor(EColorType tp, const CTextColor& col)
{
    switch (tp)
    {
    case EColorType::Zero:
    case EColorType::One:
    case EColorType::Two:
        x20_[int(tp)] = col;
        break;
    case EColorType::Three:
        x20_[0] = col;
        break;
    case EColorType::Four:
        x20_[1] = col;
        break;
    }
    RefreshColor(tp);
}

void CFontRenderState::RefreshPalette()
{
    RefreshColor(EColorType::Three);
    RefreshColor(EColorType::Four);
}

void CFontRenderState::RefreshColor(EColorType tp)
{
    switch (tp)
    {
    case EColorType::Zero:
        if (!x14_font)
            return;
        switch (x14_font.GetObj()->GetMode())
        {
        case EColorType::Zero:
            if (!x30_[0])
                x0_drawStrOpts.x4_vec[0] = ConvertToTextureSpace(x20_[0]);
            break;
        case EColorType::One:
            if (!x30_[0])
                x0_drawStrOpts.x4_vec[0] = ConvertToTextureSpace(x20_[0]);
            break;
        default: break;
        }
        break;
    case EColorType::One:
        if (!x14_font)
            return;
        if (x30_[1])
            return;
        if (x14_font.GetObj()->GetMode() == EColorType::One)
            x0_drawStrOpts.x4_vec[1] = ConvertToTextureSpace(x20_[1]);
        break;
    case EColorType::Two:
        if (!x30_[2])
            x0_drawStrOpts.x4_vec[2] = ConvertToTextureSpace(x20_[2]);
        break;
    case EColorType::Three:
        RefreshColor(EColorType::Zero);
        RefreshColor(EColorType::Two);
        break;
    case EColorType::Four:
        RefreshColor(EColorType::One);
        break;
    }
}

}
