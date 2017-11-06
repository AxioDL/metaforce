#include "CMapSurfaceShader.hpp"

namespace urde
{

CMapSurfaceShader::CMapSurfaceShader(boo::IGraphicsDataFactory::Context& ctx,
                                     const boo::ObjToken<boo::IGraphicsBufferS>& vbo,
                                     const boo::ObjToken<boo::IGraphicsBufferS>& ibo)
: m_vbo(vbo), m_ibo(ibo)
{
    m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
    m_dataBind = TShader<CMapSurfaceShader>::BuildShaderDataBinding(ctx, *this);
}

void CMapSurfaceShader::draw(const zeus::CColor& color, u32 start, u32 count)
{
    Uniform uniform = {
        CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f(), color
    };
    m_uniBuf->load(&uniform, sizeof(uniform));
    CGraphics::SetShaderDataBinding(m_dataBind);
    CGraphics::DrawArrayIndexed(start, count);
}

URDE_SPECIALIZE_SHADER(CMapSurfaceShader)

}
