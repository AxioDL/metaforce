#include "CThermalHotFilter.hpp"
#include "Graphics/CGraphics.hpp"

namespace urde
{

CThermalHotFilter::CThermalHotFilter()
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
        m_dataBind = TShader<CThermalHotFilter>::BuildShaderDataBinding(ctx, *this);
        return true;
    });
}

void CThermalHotFilter::draw()
{
    SClipScreenRect clipRect = {};
    clipRect.xc_width = CGraphics::g_ViewportResolution.x;
    clipRect.x10_height = CGraphics::g_ViewportResolution.y;
    CGraphics::ResolveSpareTexture(clipRect);

    //m_uniBuf->load(&m_uniform, sizeof(m_uniform));

    CGraphics::g_BooMainCommandQueue->setShaderDataBinding(m_dataBind);
    CGraphics::g_BooMainCommandQueue->draw(0, 4);
}

URDE_SPECIALIZE_SHADER(CThermalHotFilter)

}
