#pragma once

#include "Runtime/CToken.hpp"
#include "Runtime/GuiSys/CScanDisplay.hpp"
#include "Runtime/World/CPlayer.hpp"

#include <zeus/CMatrix3f.hpp>
#include <zeus/CQuaternion.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CGuiFrame;
struct CFinalInput;
class CStateManager;
class CGuiCamera;
class CGuiWidget;
class CAuiEnergyBarT01;
class CGuiModel;

class IHudDecoInterface {
public:
  virtual void SetIsVisibleDebug(bool v) = 0;
  virtual void SetIsVisibleGame(bool v) = 0;
  virtual void SetHudRotation(const zeus::CQuaternion& rot) = 0;
  virtual void SetHudOffset(const zeus::CVector3f& off) = 0;
  virtual void SetReticuleTransform(const zeus::CMatrix3f& xf);
  virtual void SetDecoRotation(float angle);
  virtual void SetDamageTransform(const zeus::CMatrix3f& rotation, const zeus::CVector3f& position) = 0;
  virtual void SetFrameColorValue(float v);
  virtual void Update(float dt, const CStateManager& stateMgr) = 0;
  virtual void Draw();
  virtual void ProcessInput(const CFinalInput& input);
  virtual void UpdateCameraDebugSettings(float fov, float y, float z) = 0;
  virtual void UpdateHudAlpha() = 0;
  virtual float GetHudTextAlpha() const;
  virtual ~IHudDecoInterface() = default;
};

class CHudDecoInterfaceCombat : public IHudDecoInterface {
  zeus::CQuaternion x4_rotation;
  zeus::CVector3f x14_pivotPosition;
  zeus::CVector3f x20_offset;
  zeus::CVector3f x2c_camPos;
  zeus::CVector3f x38_basePosition;
  zeus::CMatrix3f x44_baseRotation;
  bool x68_24_visDebug : 1 = true;
  bool x68_25_visGame : 1 = true;
  CGuiCamera* x6c_camera;
  CGuiWidget* x70_basewidget_pivot;
  CGuiWidget* x74_basewidget_deco;
  CGuiWidget* x78_basewidget_tickdeco0;
  CGuiWidget* x7c_basewidget_frame;
  void UpdateVisibility();

public:
  explicit CHudDecoInterfaceCombat(CGuiFrame& selHud);
  void SetIsVisibleDebug(bool v) override;
  void SetIsVisibleGame(bool v) override;
  void SetHudRotation(const zeus::CQuaternion& rot) override;
  void SetHudOffset(const zeus::CVector3f& off) override;
  void SetDamageTransform(const zeus::CMatrix3f& rotation, const zeus::CVector3f& position) override;
  void SetFrameColorValue(float v) override;
  void Update(float dt, const CStateManager& stateMgr) override;
  void UpdateCameraDebugSettings(float fov, float y, float z) override;
  void UpdateHudAlpha() override;
};

class CHudDecoInterfaceScan : public IHudDecoInterface {
  TLockedToken<CGuiFrame> x4_scanHudFlat;
  CGuiFrame* x10_loadedScanHudFlat = nullptr;
  CGuiFrame& x14_selHud;
  CScanDisplay x18_scanDisplay;
  TUniqueId x1d0_latestHudPoi = kInvalidUniqueId;
  TUniqueId x1d2_latestScanningObject = kInvalidUniqueId;
  CPlayer::EPlayerScanState x1d4_latestScanState = CPlayer::EPlayerScanState::NotScanning;
  float x1d8_scanningTime = 0.f;
  float x1dc_ = 0.f;
  float x1e0_ = 1.f;
  zeus::CQuaternion x1e4_rotation;
  zeus::CVector3f x1f4_pivotPosition;
  zeus::CVector3f x200_offset;
  zeus::CVector3f x20c_camPos;
  zeus::CVector3f x218_leftsidePosition;
  zeus::CVector3f x224_rightsidePosition;
  float x230_sidesTimer = 0.f;
  float x234_sidesPositioner;
  float x238_scanningTextAlpha = 0.f;
  float x23c_scanBarAlpha = 0.f;
  bool x240_24_visDebug : 1 = true;
  bool x240_25_visGame : 1 = true;
  CGuiCamera* x244_camera;
  CGuiWidget* x248_basewidget_pivot;
  CGuiWidget* x24c_basewidget_leftside;
  CGuiWidget* x250_basewidget_rightside;
  CGuiTextPane* x254_flat_textpane_scanning;
  CGuiWidget* x258_flat_basewidget_scanguage;
  CAuiEnergyBarT01* x25c_flat_energybart01_scanbar;
  CGuiWidget* x260_flat_basewidget_textgroup;
  CGuiTextPane* x264_flat_textpane_message;
  CGuiTextPane* x268_flat_textpane_scrollmessage;
  CGuiModel* x26c_flat_model_xmark;
  CGuiModel* x270_flat_model_abutton;
  CGuiModel* x274_flat_model_dash;
  void UpdateVisibility();

public:
  explicit CHudDecoInterfaceScan(CGuiFrame& selHud);
  void SetIsVisibleDebug(bool v) override;
  void SetIsVisibleGame(bool v) override;
  void SetHudRotation(const zeus::CQuaternion& rot) override;
  void SetHudOffset(const zeus::CVector3f& off) override;
  void SetReticuleTransform(const zeus::CMatrix3f& xf) override;
  void SetDamageTransform(const zeus::CMatrix3f& rotation, const zeus::CVector3f& position) override;
  void SetFrameColorValue(float v) override;
  void InitializeFlatFrame();
  const CScannableObjectInfo* GetCurrScanInfo(const CStateManager& stateMgr) const;
  void UpdateScanDisplay(const CStateManager& stateMgr, float dt);
  void Update(float dt, const CStateManager& stateMgr) override;
  void Draw() override;
  void ProcessInput(const CFinalInput& input) override;
  void UpdateCameraDebugSettings(float fov, float y, float z) override;
  void UpdateHudAlpha() override;
  float GetHudTextAlpha() const override;
};

class CHudDecoInterfaceXRay : public IHudDecoInterface {
  float x4_seekerScale = 1.f;
  zeus::CQuaternion x8_rotation;
  zeus::CVector3f x18_pivotPosition;
  zeus::CVector3f x24_offset;
  zeus::CVector3f x30_camPos;
  zeus::CMatrix3f x3c_reticuleXf;
  zeus::CVector3f x60_seekerPosition;
  zeus::CVector3f x6c_;
  zeus::CMatrix3f x78_;
  bool x9c_24_visDebug : 1 = true;
  bool x9c_25_visGame : 1 = true;
  CGuiCamera* xa0_camera;
  CGuiWidget* xa4_basewidget_pivot;
  CGuiWidget* xa8_basewidget_seeker;
  CGuiWidget* xac_basewidget_rotate;
  void UpdateVisibility();

public:
  explicit CHudDecoInterfaceXRay(CGuiFrame& selHud);
  void SetIsVisibleDebug(bool v) override;
  void SetIsVisibleGame(bool v) override;
  void SetHudRotation(const zeus::CQuaternion& rot) override;
  void SetHudOffset(const zeus::CVector3f& off) override;
  void SetReticuleTransform(const zeus::CMatrix3f& xf) override;
  void SetDecoRotation(float angle) override;
  void SetDamageTransform(const zeus::CMatrix3f& rotation, const zeus::CVector3f& position) override;
  void SetFrameColorValue(float v) override;
  void Update(float dt, const CStateManager& stateMgr) override;
  void UpdateCameraDebugSettings(float fov, float y, float z) override;
  void UpdateHudAlpha() override;
};

class CHudDecoInterfaceThermal : public IHudDecoInterface {
  zeus::CQuaternion x4_rotation;
  zeus::CVector3f x14_pivotPosition;
  zeus::CVector3f x20_offset;
  zeus::CVector3f x2c_camPos;
  zeus::CMatrix3f x38_reticuleXf;
  zeus::CVector3f x5c_reticulePosition;
  float x68_lockonScale = 5.f;
  float x6c_retflashTimer = 0.f;
  bool x70_24_visDebug : 1 = true;
  bool x70_25_visGame : 1 = true;
  CGuiCamera* x74_camera;
  CGuiWidget* x78_basewidget_pivot;
  CGuiWidget* x7c_basewidget_reticle;
  CGuiModel* x80_model_retflash;
  std::vector<std::pair<CGuiWidget*, zeus::CTransform>> x84_lockonWidgets;
  void UpdateVisibility();

public:
  explicit CHudDecoInterfaceThermal(CGuiFrame& selHud);
  void SetIsVisibleDebug(bool v) override;
  void SetIsVisibleGame(bool v) override;
  void SetHudRotation(const zeus::CQuaternion& rot) override;
  void SetHudOffset(const zeus::CVector3f& off) override;
  void SetReticuleTransform(const zeus::CMatrix3f& xf) override;
  void SetDamageTransform(const zeus::CMatrix3f& rotation, const zeus::CVector3f& position) override;
  void Update(float dt, const CStateManager& stateMgr) override;
  void UpdateCameraDebugSettings(float fov, float y, float z) override;
  void UpdateHudAlpha() override;
};

} // namespace urde
