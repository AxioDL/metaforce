#include "CColoredQuadFilter.hpp"

namespace urde
{

CColoredQuadFilter::CColoredQuadFilter(CCameraFilterPass::EFilterType type)
{
    m_token = CGraphics::g_BooFactory->commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        struct Vert
        {
            zeus::CVector2f m_pos;
        } verts[4] =
        {
        {{-1.0, -1.0}},
        {{-1.0,  1.0}},
        {{ 1.0, -1.0}},
        {{ 1.0,  1.0}},
        };
        m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts, 16, 4);
        m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
        m_dataBind = TMultiBlendShader<CColoredQuadFilter>::BuildShaderDataBinding(ctx, type, *this);
        return true;
    });
}

void CColoredQuadFilter::draw(const zeus::CColor& color)
{
    m_uniform.m_color = color;
    m_uniBuf->load(&m_uniform, sizeof(m_uniform));

    CGraphics::g_BooMainCommandQueue->setShaderDataBinding(m_dataBind);
    CGraphics::g_BooMainCommandQueue->draw(0, 4);
}

URDE_SPECIALIZE_MULTI_BLEND_SHADER(CColoredQuadFilter)

}
