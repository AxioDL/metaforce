#pragma once

#include "zeus/CVector3f.hpp"

namespace urde {

struct CGuiWidgetDrawParms {
  float x0_alphaMod = 1.f;
  zeus::CVector3f x4_cameraOffset;

  CGuiWidgetDrawParms() = default;
  CGuiWidgetDrawParms(float alphaMod, const zeus::CVector3f& cameraOff)
  : x0_alphaMod(alphaMod), x4_cameraOffset(cameraOff) {}
  static const CGuiWidgetDrawParms Default;
};

} // namespace urde
