#ifndef _CHUDHELMETINTERFACE
#define _CHUDHELMETINTERFACE

#include "Kyoto/Math/CMatrix3f.hpp"
#include "types.h"

class CGuiFrame;
class CGuiWidget;
class CGuiModel;
class CGuiTextPane;
class CGuiCamera;
class CAuiEnergyBarT01;
class CAuiMeter;
class CStateManager;

class CHudHelmetInterface {
public:
  explicit CHudHelmetInterface(CGuiFrame& frame);
  void Update(float dt);
  void SetHudLagOffset(const CVector3f& offset);
  void SetHudLagRotation(const CMatrix3f& rotation);
  void AddHelmetLightValue(float value);
  void UpdateCameraDebugSettings(float fov, float y, float z);
  void UpdateHelmetAlpha();
  void SetIsVisibleDebug(const bool helmet, const bool glow);

private:
  void UpdateVisibility();

  CMatrix3f mHudLagRotation;
  CVector3f mPivotPosition;
  CVector3f mHudLagPosition;
  bool mHelmetVisibleDebug : 1;
  bool mHelmetVisibleGame : 1;
  bool mGlowVisibleDebug : 1;
  bool mGlowVisibleGame : 1;
  bool mHudLagDirty : 1;
  CGuiCamera* mCamera;
  CGuiWidget* mBaseWidget_Pivot;
  CGuiWidget* mBaseWidget_Helmet;
  CGuiWidget* mBaseWidget_Glow;
  CGuiWidget* mBaseWidget_HelmetLight;
};
CHECK_SIZEOF(CHudHelmetInterface, 0x54)

#endif // _CHUDHELMETINTERFACE
