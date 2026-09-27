#ifndef _CHUDDECOINTERFACE
#define _CHUDDECOINTERFACE

#include "Kyoto/Math/CQuaternion.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CScanDisplay.hpp"
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"
#include "types.h"

class CGuiCamera;
class CGuiFrame;
class CGuiWidget;
class CGuiTextPane;
class CGuiModel;
class CAuiEnergyBarT01;
class CFinalInput;
class CStateManager;

class IHudDecoInterface {
public:
  virtual void SetIsVisibleDebug(const bool visible) = 0;
  virtual void SetIsVisibleGame(bool visible) = 0;
  virtual void SetHudRotation(const CQuaternion& rotation) = 0;
  virtual void SetHudOffset(const CVector3f& offset) = 0;
  virtual void SetReticuleTransform(const CMatrix3f& transform) {}
  virtual void SetDecoRotation(float angle) {}
  virtual void SetDamageTransform(const CMatrix3f& rotation, const CVector3f& position) = 0;
  virtual void SetFrameColorValue(float value) {}
  virtual void Update(float dt, const CStateManager& mgr) = 0;
  virtual void Draw() const {}
  virtual void ProcessInput(const CFinalInput& input) {}
  virtual void UpdateCameraDebugSettings(float fov, float y, float z) = 0;
  virtual void UpdateHudAlpha() = 0;
  virtual float GetMessageTextAlpha() const { return 1.f; }
  virtual ~IHudDecoInterface() {}
};

class CHudDecoInterfaceCombat : public IHudDecoInterface {
public:
  void SetIsVisibleDebug(const bool visible) override;
  void SetIsVisibleGame(bool visible) override;
  void SetHudRotation(const CQuaternion& rotation) override;
  void SetHudOffset(const CVector3f& offset) override;
  void SetDamageTransform(const CMatrix3f& rotation, const CVector3f& position) override;
  void SetFrameColorValue(float value) override;
  void Update(float dt, const CStateManager& mgr) override;
  void UpdateCameraDebugSettings(float fov, float y, float z) override;
  void UpdateHudAlpha() override;
  ~CHudDecoInterfaceCombat() override;
  explicit CHudDecoInterfaceCombat(CGuiFrame& hud);

private:
  void UpdateVisibility();
  CQuaternion mRotation;
  CVector3f mPivotPosition;
  CVector3f mOffset;
  CVector3f mCamPos;
  CVector3f mBasePosition;
  CMatrix3f mBaseRotation;
  bool mVisDebug : 1;
  bool mVisGame : 1;
  CGuiCamera* mCamera;
  CGuiWidget* mBasewidget_pivot;
  CGuiWidget* mBasewidget_deco;
  CGuiWidget* mBasewidget_tickdeco0;
  CGuiWidget* mBasewidget_frame;
};
CHECK_SIZEOF(CHudDecoInterfaceCombat, 0x80)

class CHudDecoInterfaceScan : public IHudDecoInterface {
public:
  void SetIsVisibleDebug(const bool visible) override;
  void SetIsVisibleGame(bool visible) override;
  void SetHudRotation(const CQuaternion& rotation) override;
  void SetHudOffset(const CVector3f& offset) override;
  void SetReticuleTransform(const CMatrix3f& transform) override;
  void SetDamageTransform(const CMatrix3f& rotation, const CVector3f& position) override;
  void SetFrameColorValue(float value) override;
  void Update(float dt, const CStateManager& mgr) override;
  void Draw() const override;
  void ProcessInput(const CFinalInput& input) override;
  void UpdateCameraDebugSettings(float fov, float y, float z) override;
  void UpdateHudAlpha() override;
  float GetMessageTextAlpha() const override;
  ~CHudDecoInterfaceScan() override;
  explicit CHudDecoInterfaceScan(CGuiFrame& hud);

private:
  void UpdateVisibility();
  void InitializeFlatFrame();
  void UpdateScanDisplay(const CStateManager& mgr, float dt);
  const CScannableObjectInfo* GetCurrScanInfo(const CStateManager& mgr) const;
  TCachedToken< CGuiFrame > mScanHudFlat;
  CGuiFrame* mLoadedScanHudFlat;
  CGuiFrame& mSelHud;
  CScanDisplay mScanDisplay;
  TUniqueId mLatestHudPoi;
  TUniqueId mLatestScanningObject;
  CPlayer::EPlayerScanState mLatestScanState;
  float mScanningTime;
  float x1dc_;
  float x1e0_;
  CQuaternion mRotation;
  CVector3f mPivotPosition;
  CVector3f mOffset;
  CVector3f mCamPos;
  CVector3f mLeftsidePosition;
  CVector3f mRightsidePosition;
  float mSidesTimer;
  float mSidesPositioner;
  float mScanningTextAlpha;
  float mScanBarAlpha;
  bool mVisDebug : 1;
  bool mVisGame : 1;
  CGuiCamera* mCamera;
  CGuiWidget* mBasewidget_pivot;
  CGuiWidget* mBasewidget_leftside;
  CGuiWidget* mBasewidget_rightside;
  CGuiTextPane* mFlat_textpane_scanning;
  CGuiWidget* mFlat_basewidget_scanguage;
  CAuiEnergyBarT01* mFlat_energybart01_scanbar;
  CGuiWidget* mFlat_basewidget_textgroup;
  CGuiTextPane* mFlat_textpane_message;
  CGuiTextPane* mFlat_textpane_scrollmessage;
  CGuiModel* mFlat_model_xmark;
  CGuiModel* mFlat_model_abutton;
  CGuiModel* mFlat_model_dash;
};
CHECK_SIZEOF(CHudDecoInterfaceScan, 0x278)

class CHudDecoInterfaceXRay : public IHudDecoInterface {
public:
  void SetIsVisibleDebug(const bool visible) override;
  void SetIsVisibleGame(bool visible) override;
  void SetHudRotation(const CQuaternion& rotation) override;
  void SetHudOffset(const CVector3f& offset) override;
  void SetReticuleTransform(const CMatrix3f& transform) override;
  void SetDecoRotation(float angle) override;
  void SetDamageTransform(const CMatrix3f& rotation, const CVector3f& position) override;
  void SetFrameColorValue(float value) override;
  void Update(float dt, const CStateManager& mgr) override;
  void UpdateCameraDebugSettings(float fov, float y, float z) override;
  void UpdateHudAlpha() override;
  ~CHudDecoInterfaceXRay() override;
  explicit CHudDecoInterfaceXRay(CGuiFrame& hud);

private:
  void UpdateVisibility();
  float mSeekerScale;
  CQuaternion mRotation;
  CVector3f mPivotPosition;
  CVector3f mOffset;
  CVector3f mCamPos;
  CMatrix3f mReticuleXf;
  CVector3f mSeekerPosition;
  CVector3f x6c_;
  CMatrix3f x78_;
  bool mVisDebug : 1;
  bool mVisGame : 1;
  CGuiCamera* mCamera;
  CGuiWidget* mBasewidget_pivot;
  CGuiWidget* mBasewidget_seeker;
  CGuiWidget* mBasewidget_rotate;
};
CHECK_SIZEOF(CHudDecoInterfaceXRay, 0xb0)

class CHudDecoInterfaceThermal : public IHudDecoInterface {
public:
  void SetIsVisibleDebug(const bool visible) override;
  void SetIsVisibleGame(bool visible) override;
  void SetHudRotation(const CQuaternion& rotation) override;
  void SetHudOffset(const CVector3f& offset) override;
  void SetReticuleTransform(const CMatrix3f& transform) override;
  void SetDamageTransform(const CMatrix3f& rotation, const CVector3f& position) override;
  void Update(float dt, const CStateManager& mgr) override;
  void UpdateCameraDebugSettings(float fov, float y, float z) override;
  void UpdateHudAlpha() override;
  ~CHudDecoInterfaceThermal() override;
  explicit CHudDecoInterfaceThermal(CGuiFrame& hud);

private:
  struct SLockWidget {
    explicit SLockWidget(CGuiWidget* widget);
    CGuiWidget* mWidget;
    CTransform4f mTransform;
  };
  void UpdateVisibility();
  CQuaternion mRotation;
  CVector3f mPivotPosition;
  CVector3f mOffset;
  CVector3f mCamPos;
  CMatrix3f mReticuleXf;
  CVector3f mReticulePosition;
  float mLockonScale;
  float mRetflashTimer;
  bool mVisDebug : 1;
  bool mVisGame : 1;
  CGuiCamera* mCamera;
  CGuiWidget* mBasewidget_pivot;
  CGuiWidget* mBasewidget_reticle;
  CGuiModel* mModel_retflash;
  rstl::vector< SLockWidget > mLockonWidgets;
};
CHECK_SIZEOF(CHudDecoInterfaceThermal, 0x94)

#endif // _CHUDDECOINTERFACE
