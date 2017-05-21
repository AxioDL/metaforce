#include "CGuiCamera.hpp"
#include "CGuiFrame.hpp"
#include "Graphics/CGraphics.hpp"
#include "CGuiWidgetDrawParms.hpp"

namespace urde
{

CGuiCamera::CGuiCamera(const CGuiWidgetParms& parms,
                       float left, float right,
                       float top, float bottom,
                       float znear, float zfar)
: CGuiWidget(parms), xb8_projtype(EProjection::Orthographic),
  m_proj(left, right, top, bottom, znear, zfar)
{}

CGuiCamera::CGuiCamera(const CGuiWidgetParms& parms,
                       float fov, float aspect,
                       float znear, float zfar)
: CGuiWidget(parms), xb8_projtype(EProjection::Perspective),
  m_proj(fov, aspect, znear, zfar)
{}

zeus::CVector3f CGuiCamera::ConvertToScreenSpace(const zeus::CVector3f& vec) const
{
    zeus::CVector3f local = RotateTranslateW2O(vec);
    if (local.isZero())
        return {-1.f, -1.f, 1.f};

    zeus::CMatrix4f mat = CGraphics::CalculatePerspectiveMatrix(m_proj.xbc_fov, m_proj.xc0_aspect,
                                                                m_proj.xc4_znear, m_proj.xc8_zfar,
                                                                false);
    return mat.multiplyOneOverW(local);
}

void CGuiCamera::Draw(const CGuiWidgetDrawParms& parms) const
{
    if (xb8_projtype == EProjection::Perspective)
        CGraphics::SetPerspective(m_proj.xbc_fov, m_proj.xc0_aspect, m_proj.xc4_znear, m_proj.xc8_zfar);
    else
        CGraphics::SetOrtho(m_proj.xbc_left, m_proj.xc0_right, m_proj.xc4_top, m_proj.xc8_bottom,
                            m_proj.xcc_znear, m_proj.xd0_zfar);
    CGraphics::SetViewPointMatrix(GetGuiFrame()->GetAspectTransform() *
                                  zeus::CTransform::Translate(parms.x4_cameraOffset) * x34_worldXF);
    CGuiWidget::Draw(parms);
}

std::shared_ptr<CGuiWidget> CGuiCamera::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
    EProjection proj = EProjection(in.readUint32Big());
    std::shared_ptr<CGuiCamera> ret = {};
    switch (proj)
    {
    case EProjection::Perspective:
    {
        float fov = in.readFloatBig();
        float aspect = in.readFloatBig();
        float znear = in.readFloatBig();
        float zfar = in.readFloatBig();
        ret = std::make_shared<CGuiCamera>(parms, fov, aspect, znear, zfar);
        break;
    }
    case EProjection::Orthographic:
    {
        float left = in.readFloatBig();
        float right = in.readFloatBig();
        float top = in.readFloatBig();
        float bottom = in.readFloatBig();
        float znear = in.readFloatBig();
        float zfar = in.readFloatBig();
        ret = std::make_shared<CGuiCamera>(parms, left, right, top, bottom, znear, zfar);
        break;
    }
    break;
    }
    frame->SetFrameCamera(ret->shared_from_this());
    ret->ParseBaseInfo(frame, in, parms);
    return ret;
}

}
