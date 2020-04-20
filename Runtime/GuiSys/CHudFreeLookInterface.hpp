#pragma once

#include "Runtime/RetroTypes.hpp"
#include "Runtime/GuiSys/CHudInterface.hpp"
#include <zeus/CTransform.hpp>

namespace urde {
class CGuiFrame;
class CGuiModel;
class CGuiWidget;

class IFreeLookInterface {
public:
  virtual ~IFreeLookInterface() = default;
  virtual void Update(float dt) = 0;
  virtual void SetIsVisibleDebug(bool v) = 0;
  virtual void SetIsVisibleGame(bool v) = 0;
  virtual void SetFreeLookState(bool inFreeLook, bool lookControlHeld, bool lockedOnObj, float vertLookAngle) = 0;
};

class CHudFreeLookInterface : public IFreeLookInterface {
  EHudType x4_hudType;
  zeus::CTransform x8_freeLookLeftXf;
  zeus::CTransform x38_freeLookRightXf;
  float x68_freeLookInterp = 0.f;
  float x6c_lockOnInterp;
  bool x70_24_inFreeLook : 1;
  bool x70_25_lookControlHeld : 1;
  bool x70_26_lockedOnObj : 1;
  bool x70_27_visibleDebug : 1 = true;
  bool x70_28_visibleGame : 1 = true;
  CGuiWidget* x74_basewidget_freelookleft;
  CGuiModel* x78_model_shieldleft;
  CGuiModel* x7c_model_freelookleft;
  CGuiWidget* x80_basewidget_freelookright;
  CGuiModel* x84_model_shieldright;
  CGuiModel* x88_model_freelookright;
  CGuiWidget* x8c_basewidget_outlinesb;
  void UpdateVisibility();

public:
  CHudFreeLookInterface(CGuiFrame& selHud, EHudType hudType, bool inFreeLook, bool lookControlHeld, bool lockedOnObj);
  void Update(float dt) override;
  void SetIsVisibleDebug(bool v) override;
  void SetIsVisibleGame(bool v) override;
  void SetFreeLookState(bool inFreeLook, bool lookControlHeld, bool lockedOnObj, float vertLookAngle) override;
};

class CHudFreeLookInterfaceXRay : public IFreeLookInterface {
  zeus::CVector3f x4_freeLookLeftPos;
  zeus::CVector3f x10_freeLookRightPos;
  float x1c_freeLookInterp = 0.f;
  bool x20_inFreeLook;
  bool x21_lookControlHeld;
  bool x22_24_visibleDebug : 1 = true;
  bool x22_25_visibleGame : 1 = true;
  CGuiWidget* x24_basewidget_freelook;
  CGuiModel* x28_model_shield;
  CGuiModel* x2c_model_freelookleft;
  CGuiModel* x30_model_freelookright;
  void UpdateVisibility();

public:
  CHudFreeLookInterfaceXRay(CGuiFrame& selHud, bool inFreeLook, bool lookControlHeld, bool lockedOnObj);
  void Update(float dt) override;
  void SetIsVisibleDebug(bool v) override;
  void SetIsVisibleGame(bool v) override;
  void SetFreeLookState(bool inFreeLook, bool lookControlHeld, bool lockedOnObj, float vertLookAngle) override;
};

} // namespace urde
