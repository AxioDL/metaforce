#include "CGuiPane.hpp"

namespace urde
{

CGuiPane::CGuiPane(const CGuiWidgetParms& parms, const zeus::CVector2f& dim,
                   const zeus::CVector3f& scaleCenter)
: CGuiWidget(parms), xb8_dim(dim), x108_scaleCenter(scaleCenter)
{
    InitializeBuffers();
}

void CGuiPane::ScaleDimensions(const zeus::CVector3f& scale)
{
    InitializeBuffers();

    for (specter::View::TexShaderVert& v : x100_verts)
    {
        v.m_pos -= x108_scaleCenter;
        v.m_pos *= scale;
        v.m_pos += x108_scaleCenter;
    }
}

void CGuiPane::SetDimensions(const zeus::CVector2f& dim, bool initVBO)
{
    xb8_dim = dim;
    if (initVBO)
        InitializeBuffers();
}

zeus::CVector2f CGuiPane::GetDimensions() const
{
    return xb8_dim;
}

void CGuiPane::InitializeBuffers()
{
    if (x100_verts.size() < 4)
        x100_verts.resize(4);

    x100_verts[0].m_pos.assign(-xb8_dim.x * 0.5f, 0.f, xb8_dim.y * 0.5f);
    x100_verts[1].m_pos.assign(-xb8_dim.x * 0.5f, 0.f, -xb8_dim.y * 0.5f);
    x100_verts[2].m_pos.assign(xb8_dim.x * 0.5f, 0.f, xb8_dim.y * 0.5f);
    x100_verts[3].m_pos.assign(xb8_dim.x * 0.5f, 0.f, -xb8_dim.y * 0.5f);
}

void CGuiPane::WriteData(COutputStream& out, bool flag) const
{
}

std::shared_ptr<CGuiWidget> CGuiPane::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
    zeus::CVector2f dim = zeus::CVector2f::ReadBig(in);
    zeus::CVector3f scaleCenter = zeus::CVector3f::ReadBig(in);
    std::shared_ptr<CGuiWidget> ret = std::make_shared<CGuiPane>(parms, dim, scaleCenter);
    ret->ParseBaseInfo(frame, in, parms);
    return ret;
}

}
