#ifndef _CCONTROLMAPPER
#define _CCONTROLMAPPER

#include "rstl/reserved_vector.hpp"
#include "Kyoto/Input/CFinalInput.hpp"

typedef float (CFinalInput::*FAnalogInput)() const;
typedef bool (CFinalInput::*FDigitalInput)() const;

class ControlMapper {
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

  enum EFunctionList {
    kFL_None,
    kFL_LeftStickUp,
    kFL_LeftStickDown,
    kFL_LeftStickLeft,
    kFL_LeftStickRight,
    kFL_RightStickUp,
    kFL_RightStickDown,
    kFL_RightStickLeft,
    kFL_RightStickRight,
    kFL_LeftTrigger,
    kFL_RightTrigger,
    kFL_DPadUp,
    kFL_DPadDown,
    kFL_DPadLeft,
    kFL_DPadRight,
    kFL_AButton,
    kFL_BButton,
    kFL_XButton,
    kFL_YButton,
    kFL_ZButton,
    kFL_LeftTriggerPress,
    kFL_RightTriggerPress,
    kFL_Start,
    kFL_MAX // default casegDigitalInputs
  };

  static const FAnalogInput gAnalogInputs[];
  static const FDigitalInput gDigitalInputs[];
  static const FDigitalInput gPressInputs[];

  static rstl::reserved_vector<bool, 67> gCommandFilterFlag;

  static const char* GetDescriptionForFunction(EFunctionList function);
  static const char* GetDescriptionForCommand(ECommands function);
  static float GetAnalogInput(ECommands command, const CFinalInput& input);
  static bool GetDigitalInput(ECommands command, const CFinalInput& input);
  static bool GetPressInput(ECommands command, const CFinalInput& input);
  static void ResetCommandFilters();
  static void SetCommandFiltered(ECommands cmd, bool filtered);
};

#endif // _CCONTROLMAPPER
