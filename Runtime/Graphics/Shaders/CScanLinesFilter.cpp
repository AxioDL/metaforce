#include "CScanLinesFilter.hpp"

namespace urde
{

CScanLinesFilter::CScanLinesFilter(EFilterType type, bool even)
: m_even(even)
{
    CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx)
    {
        m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
        m_dataBind = TMultiBlendShader<CScanLinesFilter>::BuildShaderDataBinding(ctx, type, *this);
        return true;
    } BooTrace);
}

void CScanLinesFilter::draw(const zeus::CColor& color)
{
    m_uniform.color = color;
    m_uniBuf->load(&m_uniform, sizeof(Uniform));

    CGraphics::SetShaderDataBinding(m_dataBind);
    CGraphics::DrawArray(0, 670);
}

URDE_SPECIALIZE_MULTI_BLEND_SHADER(CScanLinesFilter)

}
