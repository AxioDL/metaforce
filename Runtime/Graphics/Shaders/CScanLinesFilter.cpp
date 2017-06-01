#include "CScanLinesFilter.hpp"

namespace urde
{

CScanLinesFilter::CScanLinesFilter(EFilterType type)
{
    m_token = CGraphics::g_BooFactory->commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        struct Vert
        {
            zeus::CVector2f m_pos;
        } verts[4] =
        {
        {{0.0, 0.0}},
        {{0.0, 1.0}},
        {{1.0, 0.0}},
        {{1.0, 1.0}},
        };
        m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts, 16, 4);
        m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
        m_dataBind = TMultiBlendShader<CScanLinesFilter>::BuildShaderDataBinding(ctx, type, *this);
        return true;
    });
}

void CScanLinesFilter::DrawFilter(EFilterShape shape, const zeus::CColor& color, float t)
{

}

void CScanLinesFilter::Shutdown() {}

URDE_SPECIALIZE_MULTI_BLEND_SHADER(CScanLinesFilter)

}
