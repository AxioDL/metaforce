#ifndef _CCONTROLMAPPER
#define _CCONTROLMAPPER

#include "types.h"

#if VERSION >= VERSION_R3IJ_00

#include "Kyoto/Input/CFinalInput.hpp"
#include "rstl/reserved_vector.hpp"

class CVector2f;
class CStateManager;
class CPlayer;

class CControlMapper {
public:
  // Command values recovered from Trilogy's 91 control descriptions.
  enum ECommands {
    kC_None = -1,
    kC_Command0 = 0,
    kC_Forward = 1,
    kC_Backward = 2,
    kC_TurnLeft = 3,
    kC_TurnRight = 4,
    kC_StrafeLeft = 5,
    kC_StrafeRight = 6,
    kC_LookLeft = 7,
    kC_LookRight = 8,
    kC_LookUp = 9,
    kC_LookDown = 10,
    kC_JumpOrBoost = 11,
    kC_Command12 = 12,
    kC_FireOrBomb = 13,
    kC_Command14 = 14,
    kC_Command15 = 15,
    kC_Command16 = 16,
    kC_Command17 = 17,
    kC_MissileOrPowerBomb = 18,
    kC_Command19 = 19,
    kC_Command20 = 20,
    kC_Command21 = 21,
    kC_Command22 = 22,
    kC_Command23 = 23,
    kC_PowerBeam = 24,
    kC_IceBeam = 25,
    kC_WaveBeam = 26,
    kC_PlasmaBeam = 27,
    kC_Command28 = 28,
    kC_OrbitClose = 29,
    kC_OrbitFar = 30,
    kC_OrbitObject = 31,
    kC_Command32 = 32,
    kC_Command33 = 33,
    kC_Command34 = 34,
    kC_Command35 = 35,
    kC_Command36 = 36,
    kC_Command37 = 37,
    kC_Command38 = 38,
    kC_Command39 = 39,
    kC_Command40 = 40,
    kC_Command41 = 41,
    kC_Command42 = 42,
    kC_MapCircleUp = 43,
    kC_MapCircleDown = 44,
    kC_MapCircleLeft = 45,
    kC_MapCircleRight = 46,
    kC_MapMoveForward = 47,
    kC_MapMoveBack = 48,
    kC_MapMoveLeft = 49,
    kC_MapMoveRight = 50,
    kC_MapZoomIn = 51,
    kC_MapZoomOut = 52,
    kC_SpiderBall = 53,
    kC_ChaseCamera = 54,
    kC_XRayVisor = 55,
    kC_ThermalVisor = 56,
    kC_ScanVisor = 57,
    kC_CombatVisor = 58,
    kC_Command59 = 59,
    kC_Command60 = 60,
    kC_Command61 = 61,
    kC_Command62 = 62,
    kC_ShowCrosshairs = 63,
    kC_Command64 = 64,
    kC_Command65 = 65,
    kC_ScanItem = 66,
    kC_PauseScreen = 67,
    kC_Command68 = 68,
    kC_Command69 = 69,
    kC_Command70 = 70,
    kC_PreviousPauseScreen = 71,
    kC_NextPauseScreen = 72,
    kC_Command73 = 73,
    kC_Morph = 74,
    kC_Command75 = 75,
    kC_VisorMenu = 76,
    kC_Command77 = 77,
    kC_BeamMenu = 78,
    kC_Command79 = 79,
    kC_BallTurnLeft = 80,
    kC_BallTurnRight = 81,
    kC_Command82 = 82,
    kC_Command83 = 83,
    kC_Command84 = 84,
    kC_Command85 = 85,
    kC_PowerBeamAlternative = 86,
    kC_CombatVisorAlternative = 87,
    kC_PointerAimHold = 88,
    kC_SpringBall = 89,
    kC_Screenshot = 90,
    kC_MAX = 91
  };
  enum EFilterType { kFT_Filtered, kFT_Unfiltered };

  // Mapping record names are inferred from the override and preset consumers.
  struct SCommandMapping {
    SCommandMapping(int type, int primary, int secondary)
    : mControlType(type), mPrimaryControl(primary), mSecondaryControl(secondary) {}

    int mControlType;
    int mPrimaryControl;
    int mSecondaryControl;
  };
  struct SCommandOverride {
    SCommandOverride(ECommands command, const SCommandMapping& mapping)
    : mCommand(command), mMapping(mapping) {}

    ECommands mCommand;
    SCommandMapping mMapping;
  };

  static const float skDefaultTapHoldThreshold;

  explicit CControlMapper(float tapHoldThreshold = skDefaultTapHoldThreshold);
  void Update(const CFinalInput& input, const CStateManager& mgr, const CPlayer& player);
  float GetAnalogInput(ECommands command, const CFinalInput& input, EFilterType filter) const;
  bool GetDigitalInput(ECommands command, const CFinalInput& input, EFilterType filter) const;
  bool GetPressInput(ECommands command, const CFinalInput& input, EFilterType filter) const;
  bool GetReleaseInput(ECommands command, const CFinalInput& input, EFilterType filter) const;
  bool GetTapInput(ECommands command, const CFinalInput& input, EFilterType filter) const;
  float GetDigitalTime(ECommands command) const {
#if NONMATCHING
    if (command < 0 || command >= kC_MAX) {
      return 0.f;
    }
#endif
    return mDigitalTime[command];
  }
  float GetReleaseTime(ECommands command) const {
#if NONMATCHING
    if (command < 0 || command >= kC_MAX) {
      return 0.f;
    }
#endif
    return mReleaseTime[command];
  }
  int GetSelectorActive() const { return mSelectorActive; }
  float GetSelectorFade() const;
  bool GetSelectorReleaseInput(ECommands command, const CFinalInput& input,
                               const CStateManager& mgr, const CPlayer& player) const;
  void SetCommandEnabled(ECommands command, bool enabled);
  void SetCommandMapping(ECommands command, const SCommandMapping& mapping);
  void RestoreCommandMapping(ECommands command);
  bool IsCommandRemapped(ECommands command) const;
  SCommandMapping GetCommandMapping(ECommands command) const;
  void SwapCommands(ECommands commandA, ECommands commandB, bool swap);
  void ResetCommandFilters();
  void ResetCommandMappings();
  void ResetCommandTimes();
  void ResetInputState();
  void ResetSelector();
  void Reset();

private:
  void UpdateCommandTimes(const CFinalInput& input);
  bool CanOpenSelector(const CStateManager& mgr, const CPlayer& player, ECommands command) const;
  void UpdateSelector(ECommands command, const CFinalInput& input, const CStateManager& mgr,
                      const CPlayer& player);
  void UpdateCommandSwaps();
  static bool IsSplineControl(int control);
  bool TestVirtualMenu(ECommands command, const CVector2f& pointer) const;

  rstl::reserved_vector< bool, 91 > mCommandEnabled;
  rstl::reserved_vector< bool, 91 > mCommandOverridden;
  rstl::reserved_vector< SCommandOverride, 12 > mCommandOverrides;
  int mSelectorActive;
  float mSelectorFadeTime;
  ECommands mActiveSelectorCommand;
  ECommands mReleasedSelectorCommand;
  float mTapHoldThreshold;
  rstl::reserved_vector< float, 91 > mDigitalTime;
  rstl::reserved_vector< float, 91 > mReleaseTime;
};
CHECK_SIZEOF(CControlMapper, 0x478)


#else

#include "Kyoto/Input/CFinalInput.hpp"

#include "rstl/reserved_vector.hpp"

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

  static rstl::reserved_vector< bool, 67 > gCommandFilterFlag;

  static const char* GetDescriptionForFunction(EFunctionList function);
  static const char* GetDescriptionForCommand(ECommands function);
  static float GetAnalogInput(ECommands command, const CFinalInput& input);
  static bool GetDigitalInput(ECommands command, const CFinalInput& input);
  static bool GetPressInput(ECommands command, const CFinalInput& input);
  static void ResetCommandFilters();
  static void SetCommandFiltered(ECommands cmd, bool filtered);
};


#endif

#endif // _CCONTROLMAPPER
