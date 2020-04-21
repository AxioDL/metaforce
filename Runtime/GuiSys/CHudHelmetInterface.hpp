#pragma once

#include "Runtime/RetroTypes.hpp"
#include <zeus/CMatrix3f.hpp>

namespace urde {
class CGuiCamera;
class CGuiFrame;
class CGuiWidget;

class CHudHelmetInterface {
  zeus::CMatrix3f x0_hudLagRotation;
  zeus::CVector3f x24_pivotPosition;
  zeus::CVector3f x30_hudLagPosition;
  bool x3c_24_helmetVisibleDebug : 1 = true;
  bool x3c_25_helmetVisibleGame : 1 = true;
  bool x3c_26_glowVisibleDebug : 1 = true;
  bool x3c_27_glowVisibleGame : 1 = true;
  bool x3c_28_hudLagDirty : 1 = false;
  CGuiCamera* x40_camera;
  CGuiWidget* x44_BaseWidget_Pivot;
  CGuiWidget* x48_BaseWidget_Helmet;
  CGuiWidget* x4c_BaseWidget_Glow;
  CGuiWidget* x50_BaseWidget_HelmetLight;
  void UpdateVisibility();

public:
  explicit CHudHelmetInterface(CGuiFrame& helmetFrame);
  void Update(float dt);
  void SetHudLagOffset(const zeus::CVector3f& off);
  void SetHudLagRotation(const zeus::CMatrix3f& rot);
  void AddHelmetLightValue(float val);
  void UpdateCameraDebugSettings(float fov, float y, float z);
  void UpdateHelmetAlpha();
  void SetIsVisibleDebug(bool helmet, bool glow);
};

} // namespace urde
