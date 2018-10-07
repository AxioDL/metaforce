#include "CThermalColdFilter.hpp"
#include "Graphics/CGraphics.hpp"
#include "hecl/Pipeline.hpp"

namespace urde
{

static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

void CThermalColdFilter::Initialize()
{
    s_Pipeline = hecl::conv->convert(Shader_CThermalColdFilter{});
}

void CThermalColdFilter::Shutdown()
{
    s_Pipeline.reset();
}

CThermalColdFilter::CThermalColdFilter()
{
    CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx)
    {
        m_shiftTex = ctx.newDynamicTexture(8, 4, boo::TextureFormat::RGBA8, boo::TextureClampMode::Repeat);

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
        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {m_uniBuf.get()};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        boo::ObjToken<boo::ITexture> texs[] = {CGraphics::g_SpareTexture.get(), m_shiftTex.get()};
        m_dataBind = ctx.newShaderDataBinding(s_Pipeline, m_vbo.get(), nullptr, nullptr,
                                              1, bufs, stages, nullptr, nullptr, 2, texs, nullptr, nullptr);
        return true;
    } BooTrace);

    setShift(0);
    setScale(0.f);
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
            m_shiftTexture[y][x][0] = u8(nx * 255 / 7);
            m_shiftTexture[y][x][1] = u8(ny * 255 / 3);
        }
    }
    m_shiftTex->load(m_shiftTexture[0][0], sizeof(m_shiftTexture));
}

void CThermalColdFilter::draw()
{
    CGraphics::ResolveSpareTexture(CGraphics::g_CroppedViewport);

    m_uniform.m_shiftTexMtx[0][0] = 80.f * (CGraphics::g_ProjAspect / 1.33f);
    m_uniform.m_shiftTexMtx[1][1] = 120.f;
    m_uniform.m_shiftTexScale[0] = 1.f / m_uniform.m_shiftTexMtx[0][0];
    m_uniform.m_shiftTexScale[1] = 1.f / m_uniform.m_shiftTexMtx[1][1];
    m_uniBuf->load(&m_uniform, sizeof(m_uniform));

    CGraphics::SetShaderDataBinding(m_dataBind);
    CGraphics::DrawArray(0, 4);
}

}
