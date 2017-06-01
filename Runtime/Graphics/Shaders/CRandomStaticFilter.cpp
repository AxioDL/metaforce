#include "CRandomStaticFilter.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{

CRandomStaticFilter::CRandomStaticFilter(EFilterType type, bool cookieCutter)
: m_cookieCutter(cookieCutter)
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
        m_dataBind = TMultiBlendShader<CRandomStaticFilter>::BuildShaderDataBinding(ctx, type, *this);
        return true;
    });
}

void CRandomStaticFilter::draw(const zeus::CColor& color, float t)
{
    std::pair<zeus::CVector2f, zeus::CVector2f> rect = g_Renderer->SetViewportOrtho(true, 0.f, 1.f);
}

void CRandomStaticFilter::Shutdown() {}

URDE_SPECIALIZE_MULTI_BLEND_SHADER(CRandomStaticFilter)

}
