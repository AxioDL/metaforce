#include "CEnergyBarShader.hpp"

namespace urde
{

void CEnergyBarShader::updateModelMatrix()
{
    m_uniform.m_matrix = CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f();
}

void CEnergyBarShader::draw(const zeus::CColor& color0, const std::vector<Vertex>& verts0,
                            const zeus::CColor& color1, const std::vector<Vertex>& verts1,
                            const zeus::CColor& color2, const std::vector<Vertex>& verts2,
                            const CTexture* tex)
{
    size_t totalVerts = verts0.size() + verts1.size() + verts2.size();
    if (!totalVerts)
        return;

    if (totalVerts > m_maxVerts)
    {
        m_maxVerts = totalVerts;
        m_tex = tex;
        CGraphicsCommitResources([this](boo::IGraphicsDataFactory::Context& ctx)
        {
            m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, sizeof(Vertex), m_maxVerts);
            for (int i=0 ; i<3 ; ++i)
                m_uniBuf[i] = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
            TShader<CEnergyBarShader>::BuildShaderDataBinding(ctx, *this);
            return true;
        });
    }

    size_t vertIter = 0;
    Vertex* verts = reinterpret_cast<Vertex*>(m_vbo->map(sizeof(Vertex) * totalVerts));
    if (verts0.size())
    {
        memmove(verts, verts0.data(), sizeof(Vertex) * verts0.size());
        vertIter += verts0.size();
    }
    if (verts1.size())
    {
        memmove(verts + vertIter, verts1.data(), sizeof(Vertex) * verts1.size());
        vertIter += verts1.size();
    }
    if (verts2.size())
    {
        memmove(verts + vertIter, verts2.data(), sizeof(Vertex) * verts2.size());
    }
    m_vbo->unmap();

    vertIter = 0;
    if (verts0.size())
    {
        m_uniform.m_color = color0;
        m_uniBuf[0]->load(&m_uniform, sizeof(Uniform));
        CGraphics::SetShaderDataBinding(m_dataBind[0]);
        CGraphics::DrawArray(0, verts0.size());
        vertIter += verts0.size();
    }
    if (verts1.size())
    {
        m_uniform.m_color = color1;
        m_uniBuf[1]->load(&m_uniform, sizeof(Uniform));
        CGraphics::SetShaderDataBinding(m_dataBind[1]);
        CGraphics::DrawArray(vertIter, verts1.size());
        vertIter += verts1.size();
    }
    if (verts2.size())
    {
        m_uniform.m_color = color2;
        m_uniBuf[2]->load(&m_uniform, sizeof(Uniform));
        CGraphics::SetShaderDataBinding(m_dataBind[2]);
        CGraphics::DrawArray(vertIter, verts2.size());
    }
}

URDE_SPECIALIZE_SHADER(CEnergyBarShader)

}
