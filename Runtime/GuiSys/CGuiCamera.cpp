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
: CGuiWidget(parms), xb8_proj(EProjection::Orthographic),
  xbc_left(left), xc0_right(right),
  xc4_top(top), xc8_bottom(bottom),
  xcc_znear(znear), xd0_zfar(zfar)
{}

CGuiCamera::CGuiCamera(const CGuiWidgetParms& parms,
                       float fov, float aspect,
                       float znear, float zfar)
: CGuiWidget(parms), xb8_proj(EProjection::Perspective),
  xbc_fov(fov), xc0_aspect(aspect),
  xc4_znear(znear), xc8_zfar(zfar)
{}

zeus::CVector3f CGuiCamera::ConvertToScreenSpace(const zeus::CVector3f& vec) const
{
    zeus::CVector3f local = RotateTranslateW2O(vec);
    if (local.isZero())
        return {-1.f, -1.f, 1.f};

    zeus::CMatrix4f mat = CGraphics::CalculatePerspectiveMatrix(xbc_fov, xc0_aspect,
                                                                xc4_znear, xc8_zfar,
                                                                false);
    return mat.multiplyOneOverW(local);
}

void CGuiCamera::Draw(const CGuiWidgetDrawParms& parms) const
{
    if (xb8_proj == EProjection::Perspective)
        CGraphics::SetPerspective(xbc_fov, xc0_aspect, xc4_znear, xc8_zfar);
    else
        CGraphics::SetOrtho(xbc_left, xc0_right, xc4_top, xc8_bottom, xcc_znear, xd0_zfar);
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
