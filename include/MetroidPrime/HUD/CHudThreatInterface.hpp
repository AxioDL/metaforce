#ifndef _CHUDTHREATINTERFACE
#define _CHUDTHREATINTERFACE

#include "Kyoto/Math/CTransform4f.hpp"
#include "MetroidPrime/HUD/CHudInterface.hpp"
#include "rstl/rc_ptr.hpp"
#include "types.h"
#include "rstl/pair.hpp"
#include "rstl/string.hpp"

class CGuiFrame;
class CGuiWidget;
class CGuiModel;
class CGuiTextPane;
class CGuiCamera;
class CAuiEnergyBarT01;
class CAuiMeter;
class CStateManager;
class CStringTable;

class IHudThreatInterface {
public:
  virtual void SetThreatDistance(float distance) = 0;
  virtual void Update(float dt) = 0;
  virtual void SetIsVisibleDebug(bool visible) = 0;
  virtual void SetIsVisibleGame(bool visible) = 0;
  virtual ~IHudThreatInterface() {}
};

class CHudThreatInterface
#if VERSION < VERSION_GM8E_02
: public IHudThreatInterface 
#endif
{
public:
  struct StringTableHolder {
    public:
      int mPad0;
      int mPad1;
      CStringTable* mTable;
  };

  void SetThreatDistance(float distance);
  void Update(float dt);
  void SetIsVisibleDebug(bool visible);
  void SetIsVisibleGame(bool visible);
#if VERSION >= VERSION_GM8E_02
  ~CHudThreatInterface();
#endif
  CHudThreatInterface(
    CGuiFrame& hud,
#if VERSION >= VERSION_GM8E_02
    StringTableHolder* stringTable,
#endif
    EHudType type, 
    float distance
  );
  static rstl::pair< CVector3f, CVector3f > CombatThreatBarCoordFunc(float t);
  static rstl::pair< CVector3f, CVector3f > ThermalThreatBarCoordFunc(float t);
  static rstl::pair< CVector3f, CVector3f > XRayThreatBarCoordFunc(float t);

private:
  void ReinitializeStrings();
  void UpdateVisibility();
  enum EThreatStatus { kTS_Normal, kTS_Warning, kTS_Damage };

  const rstl::wstring BuildWarningString(EThreatStatus status);

#if VERSION >= VERSION_GM8E_02
  StringTableHolder* mStringTable;
#endif

  EHudType mHudType;
  float mDamagePulseTimer;
  float mDamagePulse;
  float mThreatDist;
  float mArrowTimer;
  CTransform4f mThreatIconXf;
  float mWarningLerpAlpha;
  EThreatStatus mThreatStatus;
  float mWarningColorLerp;
  bool mVisibleDebug : 1;
  bool mVisibleGame : 1;
  bool mHasArrows : 1;
  bool mNotXRay : 1;
  CGuiWidget* mBasewidget_threatstuff;
  CGuiWidget* mBasewidget_threaticon;
  CGuiModel* mModel_threatarrowup;
  CGuiModel* mModel_threatarrowdown;
  CGuiTextPane* mTextpane_threatwarning;
  CAuiEnergyBarT01* mEnergybart01_threatbar;
  CGuiTextPane* mTextpane_threatdigits;
};
CHECK_SIZEOF(CHudThreatInterface, 0x74)

#endif // _CHUDTHREATINTERFACE
