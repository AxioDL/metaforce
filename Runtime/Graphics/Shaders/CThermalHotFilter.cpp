#include "CThermalHotFilter.hpp"
#include "Graphics/CGraphics.hpp"

namespace urde
{

CThermalHotFilter::CThermalHotFilter()
{
    CGraphicsCommitResources([&](boo::IGraphicsDataFactory::Context& ctx)
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
        m_dataBind = TShader<CThermalHotFilter>::BuildShaderDataBinding(ctx, *this);
        return true;
    });
}

void CThermalHotFilter::draw()
{
    CGraphics::ResolveSpareTexture(CGraphics::g_CroppedViewport);

    //m_uniBuf->load(&m_uniform, sizeof(m_uniform));

    CGraphics::SetShaderDataBinding(m_dataBind);
    CGraphics::DrawArray(0, 4);
}

URDE_SPECIALIZE_SHADER(CThermalHotFilter)

}
