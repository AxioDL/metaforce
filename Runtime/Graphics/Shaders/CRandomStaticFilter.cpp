#include "CRandomStaticFilter.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"

namespace urde
{

CRandomStaticFilter::CRandomStaticFilter(EFilterType type, bool cookieCutter)
: m_cookieCutter(cookieCutter)
{
    CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx)
    {
        struct Vert
        {
            zeus::CVector2f m_pos;
            zeus::CVector2f m_uv;
        } verts[4] =
        {
        {{-1.0, -1.0}, {0.0,   0.0}},
        {{-1.0,  1.0}, {0.0,   448.0}},
        {{ 1.0, -1.0}, {640.0, 0.0}},
        {{ 1.0,  1.0}, {640.0, 448.0}},
        };
        m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts, sizeof(Vert), 4);
        m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
        m_dataBind = TMultiBlendShader<CRandomStaticFilter>::BuildShaderDataBinding(ctx, type, *this);
        return true;
    } BooTrace);
}

void CRandomStaticFilter::draw(const zeus::CColor& color, float t)
{
    m_uniform.color = color;
    m_uniform.randOff = ROUND_UP_32(rand() * 32767 / RAND_MAX);
    m_uniform.discardThres = 1.f - t;

    m_uniBuf->load(&m_uniform, sizeof(Uniform));

    CGraphics::SetShaderDataBinding(m_dataBind);
    CGraphics::DrawArray(0, 4);
}

URDE_SPECIALIZE_MULTI_BLEND_SHADER(CRandomStaticFilter)

}
