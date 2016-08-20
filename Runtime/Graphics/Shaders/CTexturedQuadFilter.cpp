#include "CTexturedQuadFilter.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

CTexturedQuadFilter::CTexturedQuadFilter(CCameraFilterPass::EFilterType type, boo::ITexture* tex)
: m_booTex(tex)
{
    m_token = CGraphics::g_BooFactory->commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        struct Vert
        {
            zeus::CVector2f m_pos;
            zeus::CVector2f m_uv;
        } verts[4] =
        {
        {{-1.0, -1.0}, {0.0, 0.0}},
        {{-1.0,  1.0}, {0.0, 1.0}},
        {{ 1.0, -1.0}, {1.0, 0.0}},
        {{ 1.0,  1.0}, {1.0, 1.0}},
        };
        m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts, 32, 4);
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
    m_uniform.m_color = color;
    m_uniform.m_uvScale = uvScale;
    m_uniBuf->load(&m_uniform, sizeof(m_uniform));

    CGraphics::g_BooMainCommandQueue->setShaderDataBinding(m_dataBind);
    CGraphics::g_BooMainCommandQueue->draw(0, 4);
}

URDE_SPECIALIZE_MULTI_BLEND_SHADER(CTexturedQuadFilter)

}
