#include "Runtime/GuiSys/CGuiCamera.hpp"

#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CGuiWidgetDrawParms.hpp"

namespace metaforce {

CGuiCamera::CGuiCamera(const CGuiWidgetParms& parms, float left, float right, float top, float bottom, float znear,
                       float zfar)
: CGuiWidget(parms), xb8_projtype(EProjection::Orthographic), m_proj(left, right, top, bottom, znear, zfar) {}

CGuiCamera::CGuiCamera(const CGuiWidgetParms& parms, float fov, float aspect, float znear, float zfar)
: CGuiWidget(parms), xb8_projtype(EProjection::Perspective), m_proj(fov, aspect, znear, zfar) {}

zeus::CVector3f CGuiCamera::ConvertToScreenSpace(const zeus::CVector3f& vec) const {
  zeus::CVector3f local = RotateTranslateW2O(vec);
  if (local.isZero())
    return {-1.f, -1.f, 1.f};

  zeus::CMatrix4f mat =
      CGraphics::CalculatePerspectiveMatrix(m_proj.xbc_fov, m_proj.xc0_aspect, m_proj.xc4_znear, m_proj.xc8_zfar);
  local = zeus::CVector3f(local.x(), local.z(), -local.y());
  return mat.multiplyOneOverW(local);
}

void CGuiCamera::Draw(const CGuiWidgetDrawParms& parms) {
  if (xb8_projtype == EProjection::Perspective) {
    CGraphics::SetPerspective(m_proj.xbc_fov, m_proj.xc0_aspect, m_proj.xc4_znear, m_proj.xc8_zfar);
  } else {
    CGraphics::SetOrtho(m_proj.xbc_left, m_proj.xc0_right, m_proj.xc4_top, m_proj.xc8_bottom, m_proj.xcc_znear,
                        m_proj.xd0_zfar);
  }
  CGraphics::SetViewPointMatrix(GetGuiFrame()->GetAspectTransform() *
                                zeus::CTransform::Translate(parms.x4_cameraOffset) * x34_worldXF);
  CGuiWidget::Draw(parms);
}

std::shared_ptr<CGuiWidget> CGuiCamera::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  EProjection proj = EProjection(in.ReadLong());
  std::shared_ptr<CGuiCamera> ret = {};
  switch (proj) {
  case EProjection::Perspective: {
    float fov = in.ReadFloat();
    float aspect = in.ReadFloat();
    float znear = in.ReadFloat();
    float zfar = in.ReadFloat();
    ret = std::make_shared<CGuiCamera>(parms, fov, aspect, znear, zfar);
    break;
  }
  case EProjection::Orthographic: {
    float left = in.ReadFloat();
    float right = in.ReadFloat();
    float top = in.ReadFloat();
    float bottom = in.ReadFloat();
    float znear = in.ReadFloat();
    float zfar = in.ReadFloat();
    ret = std::make_shared<CGuiCamera>(parms, left, right, top, bottom, znear, zfar);
    break;
  }
  }
  frame->SetFrameCamera(ret->shared_from_this());
  ret->ParseBaseInfo(frame, in, parms);
  return ret;
}

} // namespace metaforce
