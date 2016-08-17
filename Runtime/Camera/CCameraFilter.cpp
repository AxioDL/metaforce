#include "Graphics/CGraphics.hpp"
#include "CCameraFilter.hpp"
#include "GameGlobalObjects.hpp"
#include "Graphics/CBooRenderer.hpp"

namespace urde
{

void CCameraFilterPass::DrawFilter(EFilterType type, EFilterShape shape, const zeus::CColor& color,
                                   const CTexture* tex, float uvScale)
{
    switch (type)
    {
    case EFilterType::ColorMultiply:
    case EFilterType::InvertDst:
    case EFilterType::AdditiveAlpha:
    case EFilterType::Subtractive:
    case EFilterType::AlphaBlended:
    case EFilterType::AdditiveDestColor:
    case EFilterType::NoColorWrite:
    case EFilterType::None:
    case EFilterType::None2:
    default: return;
    }
    DrawFilterShape(shape, color, tex, uvScale);
}

void CCameraFilterPass::DrawFilterShape(EFilterShape shape, const zeus::CColor& color,
                                        const CTexture* tex, float uvScale)
{
}

}
