#include "CCameraBlurFilter.hpp"
#include "Graphics/CGraphics.hpp"

namespace urde
{

CCameraBlurFilter::CCameraBlurFilter()
{
    m_token = CGraphics::g_BooFactory->commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, 32, 4);
        m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
        m_dataBind = TShader<CCameraBlurFilter>::BuildShaderDataBinding(ctx, *this);
        return true;
    });
}

void CCameraBlurFilter::draw(float amount)
{
    if (amount <= 0.f)
        return;

    SClipScreenRect clipRect = {};
    clipRect.xc_width = CGraphics::g_ViewportResolution.x;
    clipRect.x10_height = CGraphics::g_ViewportResolution.y;
    CGraphics::ResolveSpareTexture(clipRect);
    float aspect = CGraphics::g_CroppedViewport.xc_width / float(CGraphics::g_CroppedViewport.x10_height);
    
    float xFac = CGraphics::g_CroppedViewport.xc_width / float(CGraphics::g_ViewportResolution.x);
    float yFac = CGraphics::g_CroppedViewport.x10_height / float(CGraphics::g_ViewportResolution.y);
    float xBias = CGraphics::g_CroppedViewport.x4_left / float(CGraphics::g_ViewportResolution.x);
    float yBias = CGraphics::g_CroppedViewport.x8_top / float(CGraphics::g_ViewportResolution.y);
    
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

    CGraphics::g_BooMainCommandQueue->setShaderDataBinding(m_dataBind);
    CGraphics::g_BooMainCommandQueue->draw(0, 4);
}

void CCameraBlurFilter::Shutdown() {}

URDE_SPECIALIZE_SHADER(CCameraBlurFilter)

}
