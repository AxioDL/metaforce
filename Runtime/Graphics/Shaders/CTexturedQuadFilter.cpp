#include "CTexturedQuadFilter.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

CTexturedQuadFilter::CTexturedQuadFilter(CCameraFilterPass::EFilterType type, boo::ITexture* tex)
: m_booTex(tex)
{
    m_token = CGraphics::g_BooFactory->commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, 32, 4);
        m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
        m_dataBind = TMultiBlendShader<CTexturedQuadFilter>::BuildShaderDataBinding(ctx, type, *this);
        return true;
    });
}

CTexturedQuadFilter::CTexturedQuadFilter(CCameraFilterPass::EFilterType type,
                                         TLockedToken<CTexture>& tex)
: CTexturedQuadFilter(type, tex->GetBooTexture())
{
    m_tex = tex;
}

void CTexturedQuadFilter::draw(const zeus::CColor& color, float uvScale)
{
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

    m_uniform.m_color = color;
    m_uniform.m_uvScale = uvScale;
    m_uniBuf->load(&m_uniform, sizeof(m_uniform));

    CGraphics::g_BooMainCommandQueue->setShaderDataBinding(m_dataBind);
    CGraphics::g_BooMainCommandQueue->draw(0, 4);
}

URDE_SPECIALIZE_MULTI_BLEND_SHADER(CTexturedQuadFilter)

}
