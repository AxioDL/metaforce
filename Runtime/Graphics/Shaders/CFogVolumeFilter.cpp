#include "CFogVolumeFilter.hpp"

namespace urde
{

CFogVolumeFilter::CFogVolumeFilter()
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
        m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts, sizeof(Vert), 4);
        m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(zeus::CColor), 1);
        TShader<CFogVolumeFilter>::BuildShaderDataBinding(ctx, *this);
        return true;
    });
}

void CFogVolumeFilter::draw2WayPass(const zeus::CColor& color)
{
    m_uniBuf->load(&color, sizeof(zeus::CColor));
    CGraphics::SetShaderDataBinding(m_dataBind2Way);
    CGraphics::DrawArray(0, 4);
}

void CFogVolumeFilter::draw1WayPass(const zeus::CColor& color)
{
    m_uniBuf->load(&color, sizeof(zeus::CColor));
    CGraphics::SetShaderDataBinding(m_dataBind1Way);
    CGraphics::DrawArray(0, 4);
}

URDE_SPECIALIZE_SHADER(CFogVolumeFilter)

}
