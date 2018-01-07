#include "CXRayBlurFilter.hpp"
#include "Graphics/CGraphics.hpp"
#include "Graphics/CTexture.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{

CXRayBlurFilter::CXRayBlurFilter(TLockedToken<CTexture>& tex)
: m_paletteTex(tex), m_booTex(tex->GetPaletteTexture())
{
    CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        struct Vert
        {
            zeus::CVector2f m_pos;
            zeus::CVector2f m_uv;
        } verts[4] =
        {
        {{-1.f, -1.f}, {0.f, 0.f}},
        {{-1.f,  1.f}, {0.f, 1.f}},
        {{ 1.f, -1.f}, {1.f, 0.f}},
        {{ 1.f,  1.f}, {1.f, 1.f}},
        };
        m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts, 32, 4);
        m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
        m_dataBind = TShader<CXRayBlurFilter>::BuildShaderDataBinding(ctx, *this);
        return true;
    });
}

void CXRayBlurFilter::draw(float amount)
{
    CGraphics::ResolveSpareTexture(CGraphics::g_CroppedViewport);

    float blurL = amount * g_tweakGui->GetXrayBlurScaleLinear() * 0.25f;
    float blurQ = amount * g_tweakGui->GetXrayBlurScaleQuadratic() * 0.25f;

    for (int i=0 ; i<8 ; ++i)
    {
        float iflt = i / 2.f;
        float uvScale = (1.f - (blurL * iflt + blurQ * iflt * iflt));
        float uvOffset = uvScale * -0.5f + 0.5f;
        m_uniform.m_uv[i][0][0] = uvScale;
        m_uniform.m_uv[i][1][1] = uvScale;
        m_uniform.m_uv[i][3][0] = uvOffset;
        m_uniform.m_uv[i][3][1] = uvOffset;
    }
    m_uniBuf->load(&m_uniform, sizeof(m_uniform));

    CGraphics::SetShaderDataBinding(m_dataBind);
    CGraphics::DrawArray(0, 4);
}

URDE_SPECIALIZE_SHADER(CXRayBlurFilter)

}
