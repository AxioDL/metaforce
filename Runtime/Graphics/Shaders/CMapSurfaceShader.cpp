#include "CMapSurfaceShader.hpp"
#include "hecl/Pipeline.hpp"
#include "Graphics/CGraphics.hpp"

namespace urde
{

static boo::ObjToken<boo::IShaderPipeline> s_Pipeline;

void CMapSurfaceShader::Initialize()
{
    s_Pipeline = hecl::conv->convert(Shader_CMapSurfaceShader{});
}

void CMapSurfaceShader::Shutdown()
{
    s_Pipeline.reset();
}

CMapSurfaceShader::CMapSurfaceShader(boo::IGraphicsDataFactory::Context& ctx,
                                     const boo::ObjToken<boo::IGraphicsBufferS>& vbo,
                                     const boo::ObjToken<boo::IGraphicsBufferS>& ibo)
: m_vbo(vbo), m_ibo(ibo)
{
    m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
    boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {m_uniBuf.get()};
    boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
    m_dataBind = ctx.newShaderDataBinding(s_Pipeline, m_vbo.get(), nullptr, m_ibo.get(),
                                          1, bufs, stages, nullptr, nullptr, 0, nullptr, nullptr, nullptr);
}

void CMapSurfaceShader::draw(const zeus::CColor& color, u32 start, u32 count)
{
    Uniform uniform = {
        CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f(), color
    };
    m_uniBuf->load(&uniform, sizeof(Uniform));
    CGraphics::SetShaderDataBinding(m_dataBind);
    CGraphics::DrawArrayIndexed(start, count);
}

}
