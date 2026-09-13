#ifndef _CSCRIPTCONTROLLERACTION
#define _CSCRIPTCONTROLLERACTION

#include "MetroidPrime/CEntity.hpp"

#include "MetroidPrime/CControlMapper.hpp"

class CScriptControllerAction : public CEntity {
public:
  enum ECommands {
    kC_Forward,
    kC_Backward,
    kC_TurnLeft,
    kC_TurnRight,
    kC_StrafeLeft,
    kC_StrafeRight,
    kC_LookLeft,
    kC_LookRight,
    kC_LookUp,
    kC_LookDown,
    kC_JumpOrBoost = 10,
    kC_FireOrBomb = 11,
    kC_MissileOrPowerBomb = 12,
    kC_Morph,
    kC_AimUp,
    kC_AimDown,
    kC_CycleBeamUp,
    kC_CycleBeamDown,
    kC_CycleItem,
    kC_PowerBeam,
    kC_IceBeam,
    kC_WaveBeam,
    kC_PlasmaBeam,
    kC_ToggleHolster = 23,
    kC_OrbitClose,
    kC_OrbitFar,
    kC_OrbitObject,
    kC_OrbitSelect,
    kC_OrbitConfirm,
    kC_OrbitLeft,
    kC_OrbitRight,
    kC_OrbitUp,
    kC_OrbitDown,
    kC_LookHold1,
    kC_LookHold2,
    kC_LookZoomIn,
    kC_LookZoomOut,
    kC_AimHold,
    kC_MapCircleUp,
    kC_MapCircleDown,
    kC_MapCircleLeft,
    kC_MapCircleRight,
    kC_MapMoveForward,
    kC_MapMoveBack,
    kC_MapMoveLeft,
    kC_MapMoveRight,
    kC_MapZoomIn,
    kC_MapZoomOut,
    kC_SpiderBall,
    kC_ChaseCamera,
    kC_XrayVisor = 50,
    kC_ThermoVisor = 51,
    kC_EnviroVisor = 52,
    kC_NoVisor = 53,
    kC_VisorMenu,
    kC_VisorUp,
    kC_VisorDown,
    kC_ShowCrosshairs,
    kC_UseShield = 0x3B,
    kC_ScanItem = 0x3C,
    kC_PreviousPauseScreen = 0x41,
    kC_NextPauseScreen = 0x42,
    kC_UNKNOWN,
    kC_None,
    kC_MAX
  };

  CScriptControllerAction(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                          bool active, ECommands command, bool b1, uint w1, bool b2);

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float, CStateManager&) override;

  static ControlMapper::ECommands GetCommand(ECommands cmd);

private:
  ECommands x34_command;
  uint x38_mapScreenSubaction;
  uchar x3c_24_mapScreenResponse : 1;
  bool x3c_25_deactivateOnClose : 1;
  bool x3c_26_pressed : 1;
};

#endif // _CSCRIPTCONTROLLERACTION
