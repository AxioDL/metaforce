#include "CFogVolumePlaneShader.hpp"

namespace urde
{

void CFogVolumePlaneShader::CommitResources(size_t capacity)
{
    m_vertCapacity = capacity;
    CGraphicsCommitResources([this, capacity](boo::IGraphicsDataFactory::Context& ctx)
    {
        m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, sizeof(zeus::CVector4f), capacity);
        TShader<CFogVolumePlaneShader>::BuildShaderDataBinding(ctx, *this);
        return true;
    });
}

void CFogVolumePlaneShader::addFan(const zeus::CVector3f* verts, int numVerts)
{
    if (numVerts == 3)
    {
        zeus::CMatrix4f proj = CGraphics::GetPerspectiveProjectionMatrix(true);
        zeus::CVector4f vert0 = proj * zeus::CVector4f(CGraphics::g_GXModelView * verts[0]);
        if (m_verts.size())
        {
            m_verts.push_back(m_verts.back());
            m_verts.push_back(vert0);
        }
        m_verts.push_back(vert0);
        m_verts.push_back(proj * zeus::CVector4f(CGraphics::g_GXModelView * verts[1]));
        m_verts.push_back(proj * zeus::CVector4f(CGraphics::g_GXModelView * verts[2]));
        m_verts.push_back(m_verts.back());
    }
    else if (numVerts == 4)
    {
        zeus::CMatrix4f proj = CGraphics::GetPerspectiveProjectionMatrix(true);
        zeus::CVector4f vert0 = proj * zeus::CVector4f(CGraphics::g_GXModelView * verts[0]);
        if (m_verts.size())
        {
            m_verts.push_back(m_verts.back());
            m_verts.push_back(vert0);
        }
        m_verts.push_back(vert0);
        m_verts.push_back(proj * zeus::CVector4f(CGraphics::g_GXModelView * verts[1]));
        m_verts.push_back(proj * zeus::CVector4f(CGraphics::g_GXModelView * verts[3]));
        m_verts.push_back(proj * zeus::CVector4f(CGraphics::g_GXModelView * verts[2]));
    }
}

void CFogVolumePlaneShader::draw(int pass)
{
    if (m_verts.empty())
        return;
    if (pass == 0)
    {
        if (m_vertCapacity < m_verts.size())
            CommitResources(m_verts.size());
        m_vbo->load(m_verts.data(), m_verts.size() * sizeof(zeus::CVector4f));
    }
    CGraphics::SetShaderDataBinding(m_dataBinds[pass]);
    CGraphics::DrawArray(0, m_verts.size());
}

URDE_SPECIALIZE_SHADER(CFogVolumePlaneShader)

}
