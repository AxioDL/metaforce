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
  void SetAmbientContribution(const CColor& color) { mAmbColor = color; }
  void SetConstantAttenuation(float value) { mDistC = value; }
  void SetLinearAttenuation(float value) { mDistL = value; }
  void SetConstantAngleAttenuation(float value) { mAngleC = value; }
  void SetLinearAngleAttenuation(float value) { mAngleL = value; }
  void SetQuadraticAngleAttenuation(float value) { mAngleQ = value; }
  void SetLightIndex(int index) { mLightId = index; }
  FourCC GetWidgetTypeID() const override;
public:
  ELightType mType;
  float mSpotCutoff;
  float mDistC;
  float mDistL;
  float mDistQ;
  float mAngleC;
  float mAngleL;
  float mAngleQ;
  int mLightId;
  CColor mAmbColor;
};

#endif // _CGUILIGHT
