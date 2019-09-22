#pragma once

#include <memory>

#include "Runtime/Graphics/CLight.hpp"
#include "Runtime/GuiSys/CGuiWidget.hpp"

#include <zeus/CColor.hpp>

namespace urde {
class CSimplePool;

class CGuiLight : public CGuiWidget {
  ELightType xb8_type;
  float xbc_spotCutoff;
  float xc0_distC;
  float xc4_distL;
  float xc8_distQ;
  float xcc_angleC;
  float xd0_angleL;
  float xd4_angleQ;
  u32 xd8_lightId;
  zeus::CColor xdc_ambColor = zeus::skBlack;

public:
  ~CGuiLight() override;
  CGuiLight(const CGuiWidgetParms& parms, const CLight& light);
  FourCC GetWidgetTypeID() const override { return FOURCC('LITE'); }

  CLight BuildLight() const;
  void SetIsVisible(bool vis) override;
  u32 GetLightId() const { return xd8_lightId; }
  const zeus::CColor& GetAmbientLightColor() const { return xdc_ambColor; }
  void SetSpotCutoff(float v) { xbc_spotCutoff = v; }
  void SetDistC(float v) { xc0_distC = v; }
  void SetDistL(float v) { xc4_distL = v; }
  void SetDistQ(float v) { xc8_distQ = v; }
  void SetAngleC(float v) { xcc_angleC = v; }
  void SetAngleL(float v) { xd0_angleL = v; }
  void SetAngleQ(float v) { xd4_angleQ = v; }
  void SetLightId(u32 idx) { xd8_lightId = idx; }
  void SetAmbientLightColor(const zeus::CColor& color) { xdc_ambColor = color; }

  static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);

  std::shared_ptr<CGuiLight> shared_from_this() {
    return std::static_pointer_cast<CGuiLight>(CGuiObject::shared_from_this());
  }
};

} // namespace urde
