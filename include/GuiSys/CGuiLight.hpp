#ifndef _CGUILIGHT
#define _CGUILIGHT

#include "GuiSys/CGuiWidget.hpp"
#include "Kyoto/Graphics/CLight.hpp"

class IObjectStore;
class CGuiLight : public CGuiWidget {
public:
  static CGuiLight* Create(CGuiFrame* frame, CInputStream& in, IObjectStore* sp);
  CGuiLight(const CGuiWidgetParms& parms, const CLight& light);
  ~CGuiLight();

  virtual void SetIsVisible(const bool visible);
  CLight BuildLight() const;
  void SetAmbientContribution(const CColor& color) { xdc_ambColor = color; }
  void SetConstantAttenuation(float value) { xc0_distC = value; }
  void SetLinearAttenuation(float value) { xc4_distL = value; }
  void SetConstantAngleAttenuation(float value) { xcc_angleC = value; }
  void SetLinearAngleAttenuation(float value) { xd0_angleL = value; }
  void SetQuadraticAngleAttenuation(float value) { xd4_angleQ = value; }
  void SetLightIndex(int index) { xd8_lightId = index; }
  FourCC GetWidgetTypeID() const override;
public:
  ELightType xb8_type;
  float xbc_spotCutoff;
  float xc0_distC;
  float xc4_distL;
  float xc8_distQ;
  float xcc_angleC;
  float xd0_angleL;
  float xd4_angleQ;
  int xd8_lightId;
  CColor xdc_ambColor;
};

#endif // _CGUILIGHT
