#include "GuiSys/CGuiLight.hpp"
#include "GuiSys/CGuiFrame.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CGuiLight* CGuiLight::Create(CGuiFrame* parent, CInputStream& in, IObjectStore* sp) {
  CGuiWidgetParms parms = ReadWidgetHeader(parent, in);
  CColor color = parms.mColor;
  ELightType type = (ELightType)in.ReadInt32();
  float distC = in.ReadFloat();
  float distL = in.ReadFloat();
  float distQ = in.ReadFloat();
  float angC = in.ReadFloat();
  float angL = in.ReadFloat();
  float angQ = in.ReadFloat();
  int lightId = in.ReadInt32();

  CGuiLight* ret = nullptr;

  switch (type) {
  case kLT_Spot: {
    float cutoff = in.ReadFloat();
    CLight lt = CLight::BuildSpot(CVector3f::Zero(), CVector3f::Zero(), color, cutoff);
    lt.SetAttenuation(distC, distL, distQ);
    lt.SetAngleAttenuation(angC, angL, angQ);
    lt.SetId(lightId);

    ret = rs_new CGuiLight(parms, lt);
    break;
  }
  case kLT_Point: {
    CLight lt = CLight::BuildPoint(CVector3f::Zero(), color);
    lt.SetAttenuation(distC, distL, distQ);
    lt.SetId(lightId);
    ret = rs_new CGuiLight(parms, lt);
    break;
  }
  case kLT_Directional: {
    CLight lt = CLight::BuildDirectional(CVector3f::Zero(), color);
    lt.SetId(lightId);
    ret = rs_new CGuiLight(parms, lt);
    break;
  }
  default:
    break;
  }

  ret->ParseBaseInfo(parent, in, parms);
  parent->AddLight(ret);
  return ret;
}

CGuiLight::CGuiLight(const CGuiWidgetParms& parms, const CLight& light)
: CGuiWidget(parms)
, mType(light.GetType())
, mSpotCutoff(light.GetSpotCutoff())
, mDistC(light.GetAttenuationConstant())
, mDistL(light.GetAttenuationLinear())
, mDistQ(light.GetAttenuationQuadratic())
, mAngleC(light.GetAngleAttenuationConstant())
, mAngleL(light.GetAngleAttenuationLinear())
, mAngleQ(light.GetAngleAttenuationQuadratic())
, mLightId(light.GetId())
, mAmbColor(CColor::Black()) {}

CGuiLight::~CGuiLight() { GetParentFrame()->RemoveLight(this); }

void CGuiLight::SetIsVisible(const bool visible) {
  if (visible) {
    GetParentFrame()->AddLight(this);
  } else {
    GetParentFrame()->RemoveLight(this);
  }

  CGuiWidget::SetIsVisible(visible);
}

CLight CGuiLight::BuildLight() const {
  CLight ret = CLight::BuildLocalAmbient(CVector3f::Zero(), CColor(0.f, 0.f, 0.f));
  CVector3f pos = GetWorldPosition();
  CVector3f dir = GetWorldTransform().GetColumn(kDY);
  CColor color = GetColor();
  switch (mType) {
  case kLT_Spot:
    ret = CLight::BuildSpot(pos, dir, color, mSpotCutoff);
    break;
  case kLT_Point:
    ret = CLight::BuildPoint(pos, color);
    break;
  case kLT_Directional:
    ret = CLight::BuildDirectional(dir, color);
    break;
  case kLT_Custom:
    ret = CLight::BuildCustom(pos, dir, color, mDistC, mDistL, mDistQ, mAngleC,
                              mAngleL, mAngleQ);
  default:
    break;
  }

  ret.SetAttenuation(mDistC, mDistL, mDistQ);
  ret.SetAngleAttenuation(mAngleC, mAngleL, mAngleQ);
  ret.SetId(mLightId);
  return ret;
}

FourCC CGuiLight::GetWidgetTypeID() const {
  return 'LITE';
}
