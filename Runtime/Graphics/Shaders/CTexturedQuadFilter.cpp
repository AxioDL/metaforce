#include "CTexturedQuadFilter.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

CTexturedQuadFilter::CTexturedQuadFilter(const boo::ObjToken<boo::ITexture>& tex)
: m_booTex(tex)
{
}

CTexturedQuadFilter::CTexturedQuadFilter(EFilterType type, const boo::ObjToken<boo::ITexture>& tex, bool gequal)
: m_booTex(tex), m_gequal(gequal)
{
    CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, 32, 16);
        m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
        m_dataBind = TMultiBlendShader<CTexturedQuadFilter>::BuildShaderDataBinding(ctx, type, *this);
        return true;
    });
}

CTexturedQuadFilter::CTexturedQuadFilter(EFilterType type,
                                         TLockedToken<CTexture> tex)
: CTexturedQuadFilter(type, (tex ? tex->GetBooTexture() : nullptr))
{
    m_tex = tex;
}

void CTexturedQuadFilter::draw(const zeus::CColor& color, float uvScale, const zeus::CRectangle& rect, float z)
{
    Vert verts[4] =
    {
        {{0.f, 0.f, z}, {0.f, 0.f}},
        {{0.f, 1.f, z}, {0.f, uvScale}},
        {{1.f, 0.f, z}, {uvScale, 0.f}},
        {{1.f, 1.f, z}, {uvScale, uvScale}},
    };
    m_vbo->load(verts, sizeof(verts));

    m_uniform.m_matrix[0][0] = rect.size.x * 2.f;
    m_uniform.m_matrix[1][1] = rect.size.y * 2.f;
    m_uniform.m_matrix[3][0] = rect.position.x * 2.f - 1.f;
    m_uniform.m_matrix[3][1] = rect.position.y * 2.f - 1.f;
    m_uniform.m_color = color;
    m_uniBuf->load(&m_uniform, sizeof(m_uniform));

    CGraphics::SetShaderDataBinding(m_dataBind);
    CGraphics::DrawArray(0, 4);
}

void CTexturedQuadFilter::drawCropped(const zeus::CColor& color, float uvScale)
{
    float xFac = CGraphics::g_CroppedViewport.xc_width / float(g_Viewport.x8_width);
    float yFac = CGraphics::g_CroppedViewport.x10_height / float(g_Viewport.xc_height);
    float xBias = CGraphics::g_CroppedViewport.x4_left / float(g_Viewport.x8_width);
    float yBias = CGraphics::g_CroppedViewport.x8_top / float(g_Viewport.xc_height);

    Vert verts[4] =
    {
        {{-1.0, -1.0, 0.f}, {xBias * uvScale, yBias * uvScale}},
        {{-1.0,  1.0, 0.f}, {xBias * uvScale, (yBias + yFac) * uvScale}},
        {{ 1.0, -1.0, 0.f}, {(xBias + xFac) * uvScale, yBias * uvScale}},
        {{ 1.0,  1.0, 0.f}, {(xBias + xFac) * uvScale, (yBias + yFac) * uvScale}},
    };
    m_vbo->load(verts, sizeof(verts));

    m_uniform.m_color = color;
    m_uniBuf->load(&m_uniform, sizeof(m_uniform));

    CGraphics::SetShaderDataBinding(m_dataBind);
    CGraphics::DrawArray(0, 4);
}

void CTexturedQuadFilter::drawVerts(const zeus::CColor& color, const Vert verts[4], float lod)
{
    m_vbo->load(verts, sizeof(Vert) * 4);

    m_uniform.m_matrix = CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f();
    m_uniform.m_color = color;
    m_uniform.m_lod = lod;
    m_uniBuf->load(&m_uniform, sizeof(m_uniform));

    CGraphics::SetShaderDataBinding(m_dataBind);
    CGraphics::DrawArray(0, 4);
}

void CTexturedQuadFilter::DrawFilter(EFilterShape shape, const zeus::CColor& color, float t)
{
    m_uniform.m_matrix = zeus::CMatrix4f::skIdentityMatrix4f;
    m_uniform.m_lod = 0.f;
    m_uniform.m_color = color;
    m_uniBuf->load(&m_uniform, sizeof(m_uniform));

    CGraphics::SetShaderDataBinding(m_dataBind);

    if (shape == EFilterShape::FullscreenQuarters)
    {
        Vert QuadVerts[] =
        {
            {{-1.f, -1.f, 0.f}, {0.f, 0.f}},
            {{-1.f,  0.f, 0.f}, {0.f, t}},
            {{ 0.f, -1.f, 0.f}, {t,   0.f}},
            {{ 0.f,  0.f, 0.f}, {t,   t}},
            {{-1.f,  1.f, 0.f}, {0.f, 0.f}},
            {{-1.f,  0.f, 0.f}, {0.f, t}},
            {{ 0.f,  1.f, 0.f}, {t,   0.f}},
            {{ 0.f,  0.f, 0.f}, {t,   t}},
            {{ 1.f, -1.f, 0.f}, {0.f, 0.f}},
            {{ 1.f,  0.f, 0.f}, {0.f, t}},
            {{ 0.f, -1.f, 0.f}, {t,   0.f}},
            {{ 0.f,  0.f, 0.f}, {t,   t}},
            {{ 1.f,  1.f, 0.f}, {0.f, 0.f}},
            {{ 1.f,  0.f, 0.f}, {0.f, t}},
            {{ 0.f,  1.f, 0.f}, {t,   0.f}},
            {{ 0.f,  0.f, 0.f}, {t,   t}},
        };
        m_vbo->load(QuadVerts, sizeof(Vert) * 16);
        CGraphics::DrawArray(0, 4);
        CGraphics::DrawArray(4, 4);
        CGraphics::DrawArray(8, 4);
        CGraphics::DrawArray(12, 4);
    }
    else
    {
        Vert FullscreenVerts[] =
        {
            {{-1.f, -1.f, 0.f}, {0.f, 0.f}},
            {{-1.f,  1.f, 0.f}, {0.f, t}},
            {{ 1.f, -1.f, 0.f}, {t,   0.f}},
            {{ 1.f,  1.f, 0.f}, {t,   t}},
        };
        m_vbo->load(FullscreenVerts, sizeof(Vert) * 4);
        CGraphics::DrawArray(0, 4);
    }
}

const zeus::CRectangle CTexturedQuadFilter::DefaultRect = {0.f, 0.f, 1.f, 1.f};

URDE_SPECIALIZE_MULTI_BLEND_SHADER(CTexturedQuadFilter)

CTexturedQuadFilterAlpha::CTexturedQuadFilterAlpha(EFilterType type, const boo::ObjToken<boo::ITexture>& tex)
: CTexturedQuadFilter(tex)
{
    CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, 32, 4);
        m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
        m_dataBind = TMultiBlendShader<CTexturedQuadFilterAlpha>::BuildShaderDataBinding(ctx, type, *this);
        return true;
    });
}

CTexturedQuadFilterAlpha::CTexturedQuadFilterAlpha(EFilterType type,
                                                   TLockedToken<CTexture> tex)
: CTexturedQuadFilterAlpha(type, (tex ? tex->GetBooTexture() : nullptr))
{
    m_tex = tex;
}

URDE_SPECIALIZE_MULTI_BLEND_SHADER(CTexturedQuadFilterAlpha)

}
