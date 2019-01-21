#pragma once

#include "boo/IWindow.hpp"

namespace urde {
struct CFinalInput;

class ControlMapper {
public:
  enum class ECommands {
    Forward,
    Backward,
    TurnLeft,
    TurnRight,
    StrafeLeft,
    StrafeRight,
    LookLeft,
    LookRight,
    LookUp,
    LookDown,
    JumpOrBoost = 10,
    FireOrBomb = 11,
    MissileOrPowerBomb = 12,
    Morph,
    AimUp,
    AimDown,
    CycleBeamUp,
    CycleBeamDown,
    CycleItem,
    PowerBeam,
    IceBeam,
    WaveBeam,
    PlasmaBeam,
    ToggleHolster = 23,
    OrbitClose,
    OrbitFar,
    OrbitObject,
    OrbitSelect,
    OrbitConfirm,
    OrbitLeft,
    OrbitRight,
    OrbitUp,
    OrbitDown,
    LookHold1,
    LookHold2,
    LookZoomIn,
    LookZoomOut,
    AimHold,
    MapCircleUp,
    MapCircleDown,
    MapCircleLeft,
    MapCircleRight,
    MapMoveForward,
    MapMoveBack,
    MapMoveLeft,
    MapMoveRight,
    MapZoomIn,
    MapZoomOut,
    SpiderBall,
    ChaseCamera,
    XrayVisor = 50,
    ThermoVisor = 51,
    InviroVisor = 52,
    NoVisor = 53,
    VisorMenu,
    VisorUp,
    VisorDown,
    ShowCrosshairs,
    UseSheild = 0x3B,
    ScanItem = 0x3C,
    PreviousPauseScreen = 0x41,
    NextPauseScreen = 0x42,
    UNKNOWN,
    None,
    MAX
  };

  enum class EFunctionList {
    None,
    LeftStickUp,
    LeftStickDown,
    LeftStickLeft,
    LeftStickRight,
    RightStickUp,
    RightStickDown,
    RightStickLeft,
    RightStickRight,
    LeftTrigger,
    RightTrigger,
    DPadUp,
    DPadDown,
    DPadLeft,
    DPadRight,
    AButton,
    BButton,
    XButton,
    YButton,
    ZButton,
    LeftTriggerPress,
    RightTriggerPress,
    Start,
    MAX // default case
  };

  enum class EKBMFunctionList {
    None,
    KeyPress,
    SpecialKeyPress = 259,
    MousePress = 285,
    MAX = 291 /* Provide space for keys/buttons within base actions */
  };

  static void SetCommandFiltered(ECommands cmd, bool filtered);
  static void ResetCommandFilters();
  static bool GetPressInput(ECommands cmd, const CFinalInput& input);
  static bool GetDigitalInput(ECommands cmd, const CFinalInput& input);
  static float GetAnalogInput(ECommands cmd, const CFinalInput& input);
  static const char* GetDescriptionForCommand(ECommands cmd);
  static const char* GetDescriptionForFunction(EFunctionList func);
};

constexpr ControlMapper::EKBMFunctionList operator+(ControlMapper::EKBMFunctionList a, char b) {
  using T = std::underlying_type_t<ControlMapper::EKBMFunctionList>;
  return ControlMapper::EKBMFunctionList(static_cast<T>(a) + static_cast<T>(b));
}

constexpr ControlMapper::EKBMFunctionList operator+(ControlMapper::EKBMFunctionList a, boo::ESpecialKey b) {
  using T = std::underlying_type_t<ControlMapper::EKBMFunctionList>;
  return ControlMapper::EKBMFunctionList(static_cast<T>(a) + static_cast<T>(b));
}

constexpr ControlMapper::EKBMFunctionList operator+(ControlMapper::EKBMFunctionList a, boo::EMouseButton b) {
  using T = std::underlying_type_t<ControlMapper::EKBMFunctionList>;
  return ControlMapper::EKBMFunctionList(static_cast<T>(a) + static_cast<T>(b));
}

} // namespace urde
