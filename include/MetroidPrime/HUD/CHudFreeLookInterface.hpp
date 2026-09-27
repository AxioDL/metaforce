#ifndef _CHUDFREELOOKINTERFACE
#define _CHUDFREELOOKINTERFACE

#include "Kyoto/Math/CTransform4f.hpp"
#include "MetroidPrime/HUD/CHudInterface.hpp"
#include "types.h"

class CGuiFrame;
class CGuiWidget;
class CGuiModel;
class CGuiTextPane;
class CGuiCamera;
class CAuiEnergyBarT01;
class CAuiMeter;
class CStateManager;

class IHudFreeLookInterface {
public:
  virtual void SetFreeLookState(bool freeLook, bool controlHeld, bool lockedOn, float angle) = 0;
  virtual void SetIsVisibleDebug(bool visible) = 0;
  virtual void SetIsVisibleGame(bool visible) = 0;
  virtual void Update(float dt) = 0;
  virtual ~IHudFreeLookInterface() {}
};

class CHudFreeLookInterface : public IHudFreeLookInterface {
public:
  void SetFreeLookState(bool freeLook, bool controlHeld, bool lockedOn, float angle) override;
  void SetIsVisibleDebug(bool visible) override;
  void SetIsVisibleGame(bool visible) override;
  void Update(float dt) override;
  ~CHudFreeLookInterface() override;
  CHudFreeLookInterface(CGuiFrame& hud, EHudType type, bool freeLook, bool controlHeld,
                        bool lockedOn);

private:
  void UpdateVisibility();

  EHudType mHudType;
  CTransform4f mFreeLookLeftXf;
  CTransform4f mFreeLookRightXf;
  float mFreeLookInterp;
  float mLockOnInterp;
  bool mInFreeLook : 1;
  bool mLookControlHeld : 1;
  bool mLockedOnObj : 1;
  bool mVisibleDebug : 1;
  bool mVisibleGame : 1;
  CGuiWidget* mBasewidget_freelookleft;
  CGuiModel* mModel_shieldleft;
  CGuiModel* mModel_freelookleft;
  CGuiWidget* mBasewidget_freelookright;
  CGuiModel* mModel_shieldright;
  CGuiModel* mModel_freelookright;
  CGuiWidget* mBasewidget_outlinesb;
};
CHECK_SIZEOF(CHudFreeLookInterface, 0x90)

class CHudFreeLookInterfaceXRay : public IHudFreeLookInterface {
public:
  void SetFreeLookState(bool freeLook, bool controlHeld, bool lockedOn, float angle) override;
  void SetIsVisibleDebug(bool visible) override;
  void SetIsVisibleGame(bool visible) override;
  void Update(float dt) override;
  ~CHudFreeLookInterfaceXRay() override;
  CHudFreeLookInterfaceXRay(CGuiFrame& hud, bool freeLook, bool controlHeld, bool lockedOn);

private:
  void UpdateVisibility();

  CVector3f mFreeLookLeftPos;
  CVector3f mFreeLookRightPos;
  float mFreeLookInterp;
  bool mInFreeLook;
  bool mLookControlHeld;
  bool mVisibleDebug : 1;
  bool mVisibleGame : 1;
  CGuiWidget* mBasewidget_freelook;
  CGuiModel* mModel_shield;
  CGuiModel* mModel_freelookleft;
  CGuiModel* mModel_freelookright;
};
CHECK_SIZEOF(CHudFreeLookInterfaceXRay, 0x34)

#endif // _CHUDFREELOOKINTERFACE
