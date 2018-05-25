#include "CColoredQuadFilter.hpp"

namespace urde
{

CColoredQuadFilter::CColoredQuadFilter(EFilterType type)
{
    CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx)
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
    } BooTrace);
}

void CColoredQuadFilter::draw(const zeus::CColor& color, const zeus::CRectangle& rect)
{
    m_uniform.m_matrix[0][0] = rect.size.x * 2.f;
    m_uniform.m_matrix[1][1] = rect.size.y * 2.f;
    m_uniform.m_matrix[3][0] = rect.position.x * 2.f - 1.f;
    m_uniform.m_matrix[3][1] = rect.position.y * 2.f - 1.f;
    m_uniform.m_color = color;
    m_uniBuf->load(&m_uniform, sizeof(m_uniform));

    CGraphics::SetShaderDataBinding(m_dataBind);
    CGraphics::DrawArray(0, 4);
}

void CWideScreenFilter::draw(const zeus::CColor& color, float t)
{
    float aspect = g_Viewport.x8_width / float(g_Viewport.xc_height);
    if (aspect < 1.7777f)
    {
        float targetHeight = g_Viewport.x8_width / 1.7777f;
        float delta = (g_Viewport.xc_height - targetHeight) * t / 2.f;
        delta /= float(g_Viewport.xc_height);
        zeus::CRectangle rect(0.f, 0.f, 1.f, delta);
        m_bottom.draw(color, rect);
        rect.position.y = 1.f - delta;
        m_top.draw(color, rect);
    }
}

void CWideScreenFilter::DrawFilter(EFilterShape shape, const zeus::CColor& color, float t)
{

}
    
float CWideScreenFilter::SetViewportToMatch(float t)
{
    float aspect = g_Viewport.x8_width / float(g_Viewport.xc_height);
    if (aspect < 1.7777f)
    {
        float targetHeight = g_Viewport.x8_width / 1.7777f;
        float delta = (g_Viewport.xc_height - targetHeight) * t / 2.f;
        boo::SWindowRect rect = {};
        rect.size[0] = g_Viewport.x8_width;
        rect.size[1] = g_Viewport.xc_height - delta * 2.f;
        rect.location[1] = delta;
        CGraphics::g_CroppedViewport = rect;
        CGraphics::g_BooMainCommandQueue->setViewport(rect);
        return 1.7777f;
    }
    else
    {
        SetViewportToFull();
        return aspect;
    }
}
    
void CWideScreenFilter::SetViewportToFull()
{
    boo::SWindowRect rect = {};
    rect.size[0] = g_Viewport.x8_width;
    rect.size[1] = g_Viewport.xc_height;
    CGraphics::g_CroppedViewport = rect;
    CGraphics::g_BooMainCommandQueue->setViewport(rect);
}

const zeus::CRectangle CColoredQuadFilter::DefaultRect = {0.f, 0.f, 1.f, 1.f};

URDE_SPECIALIZE_MULTI_BLEND_SHADER(CColoredQuadFilter)

}
