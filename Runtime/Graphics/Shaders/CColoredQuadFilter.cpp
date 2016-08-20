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
        {{0.0, 0.0}},
        {{0.0, 1.0}},
        {{1.0, 0.0}},
        {{1.0, 1.0}},
        };
        m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts, 16, 4);
        m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
        m_dataBind = TMultiBlendShader<CColoredQuadFilter>::BuildShaderDataBinding(ctx, type, *this);
        return true;
    });
}

void CColoredQuadFilter::draw(const zeus::CColor& color, const zeus::CRectangle& rect)
{
    m_uniform.m_matrix[0][0] = rect.size.x * 2.f;
    m_uniform.m_matrix[1][1] = rect.size.y * 2.f;
    m_uniform.m_matrix[3][0] = rect.position.x * 2.f - 1.f;
    m_uniform.m_matrix[3][1] = rect.position.y * 2.f - 1.f;
    m_uniform.m_color = color;
    m_uniBuf->load(&m_uniform, sizeof(m_uniform));

    CGraphics::g_BooMainCommandQueue->setShaderDataBinding(m_dataBind);
    CGraphics::g_BooMainCommandQueue->draw(0, 4);
}

void CWideScreenFilter::draw(const zeus::CColor& color, float t)
{
    float aspect = CGraphics::g_ViewportResolution.x / float(CGraphics::g_ViewportResolution.y);
    if (aspect < 1.7777f)
    {
        float targetHeight = CGraphics::g_ViewportResolution.x / 1.7777f;
        float delta = (CGraphics::g_ViewportResolution.y - targetHeight) * t / 2.f;
        delta /= float(CGraphics::g_ViewportResolution.y);
        zeus::CRectangle rect(0.f, 0.f, 1.f, delta);
        m_bottom.draw(color, rect);
        rect.position.y = 1.f - delta;
        m_top.draw(color, rect);
    }
}

const zeus::CRectangle CColoredQuadFilter::DefaultRect = {0.f, 0.f, 1.f, 1.f};

URDE_SPECIALIZE_MULTI_BLEND_SHADER(CColoredQuadFilter)

}
