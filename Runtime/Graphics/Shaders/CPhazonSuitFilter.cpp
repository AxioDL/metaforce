#include "CPhazonSuitFilter.hpp"

namespace urde
{

#define BLUR_SCALE (1.f / 128.f)

void CPhazonSuitFilter::drawBlurPasses(float radius, const CTexture* indTex)
{
    if (!m_dataBind || indTex != m_indTex)
    {
        m_indTex = indTex;
        CGraphics::CommitResources([this](boo::IGraphicsDataFactory::Context& ctx)
        {
            m_uniBufBlurX = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(zeus::CVector4f), 1);
            m_uniBufBlurY = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(zeus::CVector4f), 1);
            m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(zeus::CVector4f) * 2, 1);

            struct BlurVert
            {
                zeus::CVector3f pos;
                zeus::CVector2f uv;
            } blurVerts[4] =
            {
                {{-1.f,  1.f, 0.f}, {0.f, 1.f}},
                {{-1.f, -1.f, 0.f}, {0.f, 0.f}},
                {{ 1.f,  1.f, 0.f}, {1.f, 1.f}},
                {{ 1.f, -1.f, 0.f}, {1.f, 0.f}}
            };
            m_blurVbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, blurVerts, sizeof(BlurVert), 4);

            float aspect = g_Viewport.x8_width / float(g_Viewport.xc_height);
            struct Vert
            {
                zeus::CVector3f pos;
                zeus::CVector2f screenUv;
                zeus::CVector2f indUv;
                zeus::CVector2f maskUv;
            } verts[4] =
            {
                {{-1.f,  1.f, 0.f}, {0.01f, 0.99f}, {0.f, 4.f}, {0.f, 1.f}},
                {{-1.f, -1.f, 0.f}, {0.01f, 0.01f}, {0.f, 0.f}, {0.f, 0.f}},
                {{ 1.f,  1.f, 0.f}, {0.99f, 0.99f}, {aspect * 4.f, 4.f}, {1.f, 1.f}},
                {{ 1.f, -1.f, 0.f}, {0.99f, 0.01f}, {aspect * 4.f, 0.f}, {1.f, 0.f}}
            };
            m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts, sizeof(Vert), 4);

            m_dataBind = TShader<CPhazonSuitFilter>::BuildShaderDataBinding(ctx, *this);
            return true;
        });
    }

    SClipScreenRect rect;
    rect.x4_left = g_Viewport.x0_left;
    rect.x8_top = g_Viewport.x4_top;
    rect.xc_width = g_Viewport.x8_width;
    rect.x10_height = g_Viewport.xc_height;

    /* X Pass */
    zeus::CVector4f blurDir = zeus::CVector4f{g_Viewport.xc_height / float(g_Viewport.x8_width) *
                                              radius * BLUR_SCALE, 0.f, 0.f, 0.f};
    m_uniBufBlurX->load(&blurDir, sizeof(zeus::CVector4f));

    CGraphics::SetShaderDataBinding(m_dataBindBlurX);
    CGraphics::DrawArray(0, 4);
    CGraphics::ResolveSpareTexture(rect, 2);

    /* Y Pass */
    blurDir = zeus::CVector4f{0.f, radius * BLUR_SCALE, 0.f, 0.f};
    m_uniBufBlurY->load(&blurDir, sizeof(zeus::CVector4f));

    CGraphics::SetShaderDataBinding(m_dataBindBlurY);
    CGraphics::DrawArray(0, 4);
    CGraphics::ResolveSpareTexture(rect, 2);
}

void CPhazonSuitFilter::draw(const zeus::CColor& color,
                             float indScale, float indOffX, float indOffY)
{
    struct Uniform
    {
        zeus::CColor color;
        zeus::CVector4f indScaleOff;
    } uniform =
    {
        color,
        zeus::CVector4f(indScale, indScale, indOffX, indOffY)
    };

    m_uniBuf->load(&uniform, sizeof(Uniform));
    CGraphics::SetShaderDataBinding(m_dataBind);
    CGraphics::DrawArray(0, 4);
}

URDE_SPECIALIZE_SHADER(CPhazonSuitFilter)

}
