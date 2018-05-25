#include "CAABoxShader.hpp"

namespace urde
{

CAABoxShader::CAABoxShader(bool zOnly)
: m_zOnly(zOnly)
{
    CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx)
    {
        m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, sizeof(zeus::CVector3f), 34);
        m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
        m_dataBind = TShader<CAABoxShader>::BuildShaderDataBinding(ctx, *this);
        return true;
    } BooTrace);
}

void CAABoxShader::setAABB(const zeus::CAABox& aabb)
{
    zeus::CVector3f vboData[] =
    {
        {aabb.max.x, aabb.max.y, aabb.min.z},
        {aabb.max.x, aabb.min.y, aabb.min.z},
        {aabb.max.x, aabb.max.y, aabb.max.z},
        {aabb.max.x, aabb.min.y, aabb.max.z},
        {aabb.max.x, aabb.min.y, aabb.max.z},

        {aabb.min.x, aabb.max.y, aabb.min.z},
        {aabb.min.x, aabb.max.y, aabb.min.z},
        {aabb.max.x, aabb.max.y, aabb.min.z},
        {aabb.min.x, aabb.max.y, aabb.max.z},
        {aabb.max.x, aabb.max.y, aabb.max.z},
        {aabb.max.x, aabb.max.y, aabb.max.z},

        {aabb.min.x, aabb.max.y, aabb.min.z},
        {aabb.min.x, aabb.max.y, aabb.min.z},
        {aabb.min.x, aabb.min.y, aabb.min.z},
        {aabb.min.x, aabb.max.y, aabb.max.z},
        {aabb.min.x, aabb.min.y, aabb.max.z},
        {aabb.min.x, aabb.min.y, aabb.max.z},

        {aabb.min.x, aabb.min.y, aabb.min.z},
        {aabb.min.x, aabb.min.y, aabb.min.z},
        {aabb.max.x, aabb.min.y, aabb.min.z},
        {aabb.min.x, aabb.min.y, aabb.max.z},
        {aabb.max.x, aabb.min.y, aabb.max.z},
        {aabb.max.x, aabb.min.y, aabb.max.z},

        {aabb.min.x, aabb.min.y, aabb.max.z},
        {aabb.min.x, aabb.min.y, aabb.max.z},
        {aabb.max.x, aabb.min.y, aabb.max.z},
        {aabb.min.x, aabb.max.y, aabb.max.z},
        {aabb.max.x, aabb.max.y, aabb.max.z},
        {aabb.max.x, aabb.max.y, aabb.max.z},

        {aabb.min.x, aabb.min.y, aabb.min.z},
        {aabb.min.x, aabb.min.y, aabb.min.z},
        {aabb.max.x, aabb.min.y, aabb.min.z},
        {aabb.min.x, aabb.max.y, aabb.min.z},
        {aabb.max.x, aabb.max.y, aabb.min.z},
    };

    m_vbo->load(vboData, sizeof(zeus::CVector3f) * 34);
}

void CAABoxShader::draw(const zeus::CColor& color)
{
    m_uniform.m_xf = CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f();
    m_uniform.m_color = color;
    m_uniBuf->load(&m_uniform, sizeof(Uniform));

    CGraphics::SetShaderDataBinding(m_dataBind);
    CGraphics::DrawArray(0, 34);
}

URDE_SPECIALIZE_SHADER(CAABoxShader)

}
