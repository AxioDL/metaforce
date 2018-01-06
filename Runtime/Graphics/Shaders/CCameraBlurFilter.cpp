#include "CCameraBlurFilter.hpp"

namespace urde
{

CCameraBlurFilter::CCameraBlurFilter()
{
    CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, 32, 4);
        m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
        m_dataBind = TShader<CCameraBlurFilter>::BuildShaderDataBinding(ctx, *this);
        return true;
    });
}

void CCameraBlurFilter::draw(float amount, bool clearDepth)
{
    if (amount <= 0.f)
        return;

    SClipScreenRect clipRect(g_Viewport);
    CGraphics::ResolveSpareTexture(clipRect, 0, clearDepth);
    float aspect = CGraphics::g_CroppedViewport.xc_width / float(CGraphics::g_CroppedViewport.x10_height);
    
    float xFac = CGraphics::g_CroppedViewport.xc_width / float(g_Viewport.x8_width);
    float yFac = CGraphics::g_CroppedViewport.x10_height / float(g_Viewport.xc_height);
    float xBias = CGraphics::g_CroppedViewport.x4_left / float(g_Viewport.x8_width);
    float yBias = CGraphics::g_CroppedViewport.x8_top / float(g_Viewport.xc_height);
    
    Vert verts[4] =
    {
        {{-1.0, -1.0}, {xBias, yBias}},
        {{-1.0,  1.0}, {xBias, yBias + yFac}},
        {{ 1.0, -1.0}, {xBias + xFac, yBias}},
        {{ 1.0,  1.0}, {xBias + xFac, yBias + yFac}},
    };
    m_vbo->load(verts, sizeof(verts));

    for (int i=0 ; i<6 ; ++i)
    {
        float tmp = i;
        tmp *= 2.f * M_PIF;
        tmp /= 6.f;

        float amtX = std::cos(tmp);
        amtX *= amount / 448.f / aspect;

        float amtY = std::sin(tmp);
        amtY *= amount / 448.f;

        m_uniform.m_uv[i][0] = amtX * xFac;
        m_uniform.m_uv[i][1] = amtY * yFac;
    }
    m_uniform.m_opacity = std::min(amount / 2.f, 1.f);
    m_uniBuf->load(&m_uniform, sizeof(m_uniform));

    CGraphics::SetShaderDataBinding(m_dataBind);
    CGraphics::DrawArray(0, 4);
}

URDE_SPECIALIZE_SHADER(CCameraBlurFilter)

}
