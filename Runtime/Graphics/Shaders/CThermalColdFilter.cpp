#include "CThermalColdFilter.hpp"
#include "Graphics/CGraphics.hpp"

namespace urde
{

CThermalColdFilter::CThermalColdFilter()
{
    m_token = CGraphics::g_BooFactory->commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_shiftTex = ctx.newDynamicTexture(8, 8, boo::TextureFormat::RGBA8);
        m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, 32, 1);
        m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
        m_dataBind = TFilterShader<CThermalColdFilter>::BuildShaderDataBinding(ctx, *this);
        return true;
    });

    struct Vert
    {
        zeus::CVector2f m_pos;
        zeus::CVector2f m_uv;
    } verts[4] =
    {
    {{-1.0, -1.0}, {0.0, 0.0}},
    {{-1.0,  1.0}, {0.0, 1.0}},
    {{ 1.0,  1.0}, {1.0, 1.0}},
    {{ 1.0, -1.0}, {1.0, 0.0}}
    };
    m_vbo->load(verts, sizeof(verts));

    setShift(0);
}

void CThermalColdFilter::setShift(unsigned shift)
{
    shift = std::min(shift, 31u);
    for (unsigned y=0 ; y<4 ; ++y)
    {
        unsigned bx = y * 8;
        for (unsigned x=0 ; x<8 ; ++x)
        {
            unsigned px = bx + x;
            unsigned spx = px + shift;
            unsigned ny = spx / 8;
            if (ny > 3)
                ny = 3;
            unsigned nx = spx % 8;
            m_shiftTexture[y][x][0] = nx / 7.f;
            m_shiftTexture[y][x][1] = ny / 3.f;
        }
    }
    m_shiftTex->load(m_shiftTexture[0][0], sizeof(m_shiftTexture));
}

void CThermalColdFilter::draw()
{
    m_uniform.m_shiftTexMtx[0][0] = 80.f * CGraphics::g_ProjAspect;
    m_uniform.m_shiftTexMtx[1][1] = 120.f;
    m_uniBuf->load(&m_uniform, sizeof(m_uniform));
    CGraphics::g_BooMainCommandQueue->setShaderDataBinding(m_dataBind);
    CGraphics::g_BooMainCommandQueue->draw(0, 4);
}

URDE_SPECIALIZE_FILTER(CThermalColdFilter)

}
