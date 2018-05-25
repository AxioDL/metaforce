#include "CRadarPaintShader.hpp"

namespace urde
{

void CRadarPaintShader::draw(const std::vector<Instance>& instances, const CTexture* tex)
{
    if (!instances.size())
        return;

    if (instances.size() > m_maxInsts)
    {
        m_maxInsts = instances.size();
        m_tex = tex;
        CGraphics::CommitResources([this](boo::IGraphicsDataFactory::Context& ctx)
        {
            m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, sizeof(Instance), m_maxInsts);
            m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(zeus::CMatrix4f), 1);
            m_dataBind = TShader<CRadarPaintShader>::BuildShaderDataBinding(ctx, *this);
            return true;
        } BooTrace);
    }

    zeus::CMatrix4f uniMtx = CGraphics::GetPerspectiveProjectionMatrix(true) * CGraphics::g_GXModelView.toMatrix4f();
    m_uniBuf->load(&uniMtx, sizeof(zeus::CMatrix4f));

    size_t mapSz = sizeof(Instance) * instances.size();
    Instance* insts = reinterpret_cast<Instance*>(m_vbo->map(mapSz));
    memmove(insts, instances.data(), mapSz);
    m_vbo->unmap();

    CGraphics::SetShaderDataBinding(m_dataBind);
    CGraphics::DrawInstances(0, 4, instances.size());
}

URDE_SPECIALIZE_SHADER(CRadarPaintShader)

}
