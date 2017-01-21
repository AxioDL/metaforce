#include "CGuiPane.hpp"

namespace urde
{

CGuiPane::CGuiPane(const CGuiWidgetParms& parms, float xDim, float zDim,
                   const zeus::CVector3f& scaleCenter)
: CGuiWidget(parms), xf8_xDim(xDim), xfc_zDim(zDim), x108_scaleCenter(scaleCenter)
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
    xf8_xDim = dim.x;
    xfc_zDim = dim.y;
    if (initVBO)
        InitializeBuffers();
}

zeus::CVector2f CGuiPane::GetDimensions() const
{
    return {xf8_xDim, xfc_zDim};
}

void CGuiPane::InitializeBuffers()
{
    if (x100_verts.size() < 4)
        x100_verts.resize(4);

    x100_verts[0].m_pos.assign(-xf8_xDim * 0.5f, 0.f, xfc_zDim * 0.5f);
    x100_verts[1].m_pos.assign(-xf8_xDim * 0.5f, 0.f, -xfc_zDim * 0.5f);
    x100_verts[2].m_pos.assign(xf8_xDim * 0.5f, 0.f, xfc_zDim * 0.5f);
    x100_verts[3].m_pos.assign(xf8_xDim * 0.5f, 0.f, -xfc_zDim * 0.5f);
}

void CGuiPane::WriteData(COutputStream& out, bool flag) const
{
}

CGuiPane* CGuiPane::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);
    float x = in.readFloatBig();
    float z = in.readFloatBig();
    zeus::CVector3f scaleCenter;
    scaleCenter.readBig(in);
    CGuiPane* pane =  new CGuiPane(parms, x, z, scaleCenter);
    pane->ParseBaseInfo(frame, in, parms);
    return pane;
}

}
