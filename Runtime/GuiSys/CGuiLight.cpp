#include "Runtime/GuiSys/CGuiLight.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"

namespace metaforce {

CGuiLight::CGuiLight(const CGuiWidgetParms& parms, const CLight& light)
: CGuiWidget(parms)
, xb8_type(light.x1c_type)
, xbc_spotCutoff(light.x20_spotCutoff)
, xc0_distC(light.x24_distC)
, xc4_distL(light.x28_distL)
, xc8_distQ(light.x2c_distQ)
, xcc_angleC(light.x30_angleC)
, xd0_angleL(light.x34_angleL)
, xd4_angleQ(light.x38_angleQ)
, xd8_lightId(light.x40_lightId) {}

CGuiLight::~CGuiLight() { xb0_frame->RemoveLight(this); }

CLight CGuiLight::BuildLight() const {
  CLight ret = CLight::BuildLocalAmbient(zeus::skZero3f, zeus::skBlack);

  switch (xb8_type) {
  case ELightType::Spot:
    ret = CLight::BuildSpot(GetWorldPosition(), x34_worldXF.basis[1], xa4_color, xbc_spotCutoff);
    break;
  case ELightType::Point:
    ret = CLight::BuildPoint(GetWorldPosition(), xa4_color);
    break;
  case ELightType::Directional:
    ret = CLight::BuildDirectional(x34_worldXF.basis[1], xa4_color);
    break;
  default:
    break;
  }

  ret.SetAttenuation(xc0_distC, xc4_distL, xc8_distQ);
  ret.SetAngleAttenuation(xcc_angleC, xd0_angleL, xd4_angleQ);
  return ret;
}

void CGuiLight::SetIsVisible(bool vis) {
  if (vis)
    xb0_frame->AddLight(this);
  else
    xb0_frame->RemoveLight(this);
  CGuiWidget::SetIsVisible(vis);
}

std::shared_ptr<CGuiWidget> CGuiLight::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);

  ELightType tp = ELightType(in.ReadLong());
  float distC = in.ReadFloat();
  float distL = in.ReadFloat();
  float distQ = in.ReadFloat();
  float angC = in.ReadFloat();
  float angL = in.ReadFloat();
  float angQ = in.ReadFloat();
  u32 lightId = in.ReadLong();

  std::shared_ptr<CGuiLight> ret = {};
  switch (tp) {
  case ELightType::Spot: {
    float cutoff = in.ReadFloat();
    CLight lt = CLight::BuildSpot(zeus::skZero3f, zeus::skZero3f, parms.x10_color, cutoff);
    lt.SetAttenuation(distC, distL, distQ);
    lt.SetAngleAttenuation(angC, angL, angQ);
    lt.x40_lightId = lightId;
    ret = std::make_shared<CGuiLight>(parms, lt);
    break;
  }
  case ELightType::Point: {
    CLight lt = CLight::BuildPoint(zeus::skZero3f, parms.x10_color);
    lt.SetAttenuation(distC, distL, distQ);
    lt.x40_lightId = lightId;
    ret = std::make_shared<CGuiLight>(parms, lt);
    break;
  }
  case ELightType::Directional: {
    CLight lt = CLight::BuildDirectional(zeus::skZero3f, parms.x10_color);
    lt.x40_lightId = lightId;
    ret = std::make_shared<CGuiLight>(parms, lt);
    break;
  }
  default:
    break;
  }

  ret->ParseBaseInfo(frame, in, parms);
  frame->RegisterLight(ret->shared_from_this());
  frame->AddLight(ret.get());
  return ret;
}

} // namespace metaforce
