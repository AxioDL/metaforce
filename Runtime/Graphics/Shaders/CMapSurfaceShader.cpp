#include "CMapSurfaceShader.hpp"

namespace urde
{

CMapSurfaceShader::CMapSurfaceShader(boo::IGraphicsDataFactory::Context& ctx,
                                     boo::IGraphicsBufferS* vbo,
                                     boo::IGraphicsBufferS* ibo)
: m_vbo(vbo), m_ibo(ibo)
{
    m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
    m_dataBind = TShader<CMapSurfaceShader>::BuildShaderDataBinding(ctx, *this);
}

void CMapSurfaceShader::draw(const zeus::CColor& color, u32 start, u32 count)
{
    m_uniform.mtx = CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f();
    m_uniform.color = color;
    m_uniBuf->load(&m_uniform, sizeof(m_uniform));
    CGraphics::SetShaderDataBinding(m_dataBind);
    CGraphics::DrawArrayIndexed(start, count);
}

URDE_SPECIALIZE_SHADER(CMapSurfaceShader)

void CMapSurfaceShader::Shutdown() {}

}
