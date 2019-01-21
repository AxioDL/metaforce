#include "../RetroTypes.hpp"
#include "ControlMapper.hpp"
#include "CFinalInput.hpp"
#include "DataSpec/DNACommon/Tweaks/ITweakPlayerControl.hpp"
#include "../GameGlobalObjects.hpp"

namespace urde {

static const char* skCommandDescs[] = {
    "Forward",           "Backward",      "Turn Left",     "Turn Right",      "Strafe Left",     "Strafe Right",
    "Look Left",         "Look Right",    "Look Up",       "Look Down",       "Jump/Boost",      "Fire/Bomb",
    "Missile/PowerBomb", "Morph",         "Aim Up",        "Aim Down",        "Cycle Beam Up",   "Cycle Beam Down",
    "Cycle Item",        "Power Beam",    "Ice Beam",      "Wave Beam",       "Plasma Beam",     "Toggle Holster",
    "Orbit Close",       "Orbit Far",     "Orbit Object",  "Orbit Select",    "Orbit Confirm",   "Orbit Left",
    "Orbit Right",       "Orbit Up",      "Orbit Down",    "Look Hold1",      "Look Hold2",      "Look Zoom In",
    "Look Zoom Out",     "Aim Hold",      "Map Circle Up", "Map Circle Down", "Map Circle Left", "Map Circle Right",
    "Map Move Forward",  "Map Move Back", "Map Move Left", "Map Move Right",  "Map Zoom In",     "Map Zoom Out",
    "SpiderBall",        "Chase Camera",  "XRay Visor",    "Thermo Visor",    "Enviro Visor",    "No Visor",
    "Visor Menu",        "Visor Up",      "Visor Down",    "UNKNOWN",         "UNKNOWN",         "Use Shield",
    "Scan Item",         "UNKNOWN"

};

static const char* skFunctionDescs[] = {"None",
                                        "Left Stick Up",
                                        "Left Stick Down",
                                        "Left Stick Left",
                                        "Left Stick Right",
                                        "Right Stick Up",
                                        "Right Stick Down",
                                        "Right Stick Left",
                                        "Right Stick Right",
                                        "Left Trigger",
                                        "Right Trigger",
                                        "D-Pad Up   ",
                                        "D-Pad Down ",
                                        "D-Pad Left ",
                                        "D-Pad Right",
                                        "A Button",
                                        "B Button",
                                        "X Button",
                                        "Y Button",
                                        "Z Button",
                                        "Left Trigger Press",
                                        "Right Trigger Press",
                                        "Start"};

typedef bool (CFinalInput::*BoolReturnFn)() const;
typedef float (CFinalInput::*FloatReturnFn)() const;

static BoolReturnFn skPressFuncs[] = {nullptr,
                                      &CFinalInput::PLAUp,
                                      &CFinalInput::PLADown,
                                      &CFinalInput::PLALeft,
                                      &CFinalInput::PLARight,
                                      &CFinalInput::PRAUp,
                                      &CFinalInput::PRADown,
                                      &CFinalInput::PRALeft,
                                      &CFinalInput::PRARight,
                                      &CFinalInput::PLTrigger,
                                      &CFinalInput::PRTrigger,
                                      &CFinalInput::PDPUp,
                                      &CFinalInput::PDPDown,
                                      &CFinalInput::PDPLeft,
                                      &CFinalInput::PDPRight,
                                      &CFinalInput::PA,
                                      &CFinalInput::PB,
                                      &CFinalInput::PX,
                                      &CFinalInput::PY,
                                      &CFinalInput::PZ,
                                      &CFinalInput::PL,
                                      &CFinalInput::PR,
                                      &CFinalInput::PStart,
                                      nullptr};

static BoolReturnFn skDigitalFuncs[] = {nullptr,
                                        &CFinalInput::DLAUp,
                                        &CFinalInput::DLADown,
                                        &CFinalInput::DLALeft,
                                        &CFinalInput::DLARight,
                                        &CFinalInput::DRAUp,
                                        &CFinalInput::DRADown,
                                        &CFinalInput::DRALeft,
                                        &CFinalInput::DRARight,
                                        &CFinalInput::DLTrigger,
                                        &CFinalInput::DRTrigger,
                                        &CFinalInput::DDPUp,
                                        &CFinalInput::DDPDown,
                                        &CFinalInput::DDPLeft,
                                        &CFinalInput::DDPRight,
                                        &CFinalInput::DA,
                                        &CFinalInput::DB,
                                        &CFinalInput::DX,
                                        &CFinalInput::DY,
                                        &CFinalInput::DZ,
                                        &CFinalInput::DL,
                                        &CFinalInput::DR,
                                        &CFinalInput::DStart,
                                        nullptr};

static FloatReturnFn skAnalogFuncs[] = {nullptr,
                                        &CFinalInput::ALAUp,
                                        &CFinalInput::ALADown,
                                        &CFinalInput::ALALeft,
                                        &CFinalInput::ALARight,
                                        &CFinalInput::ARAUp,
                                        &CFinalInput::ARADown,
                                        &CFinalInput::ARALeft,
                                        &CFinalInput::ARARight,
                                        &CFinalInput::ALTrigger,
                                        &CFinalInput::ARTrigger,
                                        &CFinalInput::ADPUp,
                                        &CFinalInput::ADPDown,
                                        &CFinalInput::ADPLeft,
                                        &CFinalInput::ADPRight,
                                        &CFinalInput::AA,
                                        &CFinalInput::AB,
                                        &CFinalInput::AX,
                                        &CFinalInput::AY,
                                        &CFinalInput::AZ,
                                        &CFinalInput::AL,
                                        &CFinalInput::AR,
                                        &CFinalInput::AStart,
                                        nullptr};

static const ControlMapper::EKBMFunctionList skKBMMapping[] = {
  ControlMapper::EKBMFunctionList::KeyPress + 'w', // Forward,
  ControlMapper::EKBMFunctionList::KeyPress + 's', // Backward,
  ControlMapper::EKBMFunctionList::KeyPress + 'a', // TurnLeft,
  ControlMapper::EKBMFunctionList::KeyPress + 'd', // TurnRight,
  ControlMapper::EKBMFunctionList::KeyPress + 'a', // StrafeLeft,
  ControlMapper::EKBMFunctionList::KeyPress + 'd', // StrafeRight,
  ControlMapper::EKBMFunctionList::KeyPress + 'a', // LookLeft,
  ControlMapper::EKBMFunctionList::KeyPress + 'd', // LookRight,
  ControlMapper::EKBMFunctionList::KeyPress + 's', // LookUp,
  ControlMapper::EKBMFunctionList::KeyPress + 'w', // LookDown,
  ControlMapper::EKBMFunctionList::KeyPress + ' ', // JumpOrBoost = 10,
  ControlMapper::EKBMFunctionList::MousePress + boo::EMouseButton::Primary, // FireOrBomb = 11,
  ControlMapper::EKBMFunctionList::MousePress + boo::EMouseButton::Secondary, // MissileOrPowerBomb = 12,
  ControlMapper::EKBMFunctionList::KeyPress + 'c', // Morph,
  ControlMapper::EKBMFunctionList::None, // AimUp,
  ControlMapper::EKBMFunctionList::None, // AimDown,
  ControlMapper::EKBMFunctionList::None, // CycleBeamUp,
  ControlMapper::EKBMFunctionList::None, // CycleBeamDown,
  ControlMapper::EKBMFunctionList::None, // CycleItem,
  ControlMapper::EKBMFunctionList::KeyPress + '1', // PowerBeam,
  ControlMapper::EKBMFunctionList::KeyPress + '3', // IceBeam,
  ControlMapper::EKBMFunctionList::KeyPress + '2', // WaveBeam,
  ControlMapper::EKBMFunctionList::KeyPress + '4', // PlasmaBeam,
  ControlMapper::EKBMFunctionList::None, // ToggleHolster = 23,
  ControlMapper::EKBMFunctionList::None, // OrbitClose,
  ControlMapper::EKBMFunctionList::KeyPress + 'q', // OrbitFar,
  ControlMapper::EKBMFunctionList::KeyPress + 'q', // OrbitObject,
  ControlMapper::EKBMFunctionList::None, // OrbitSelect,
  ControlMapper::EKBMFunctionList::None, // OrbitConfirm,
  ControlMapper::EKBMFunctionList::KeyPress + 'a', // OrbitLeft,
  ControlMapper::EKBMFunctionList::KeyPress + 'd', // OrbitRight,
  ControlMapper::EKBMFunctionList::KeyPress + 'w', // OrbitUp,
  ControlMapper::EKBMFunctionList::KeyPress + 's', // OrbitDown,
  ControlMapper::EKBMFunctionList::KeyPress + 'e', // LookHold1,
  ControlMapper::EKBMFunctionList::None, // LookHold2,
  ControlMapper::EKBMFunctionList::None, // LookZoomIn,
  ControlMapper::EKBMFunctionList::None, // LookZoomOut,
  ControlMapper::EKBMFunctionList::None, // AimHold,
  ControlMapper::EKBMFunctionList::KeyPress + 's', // MapCircleUp,
  ControlMapper::EKBMFunctionList::KeyPress + 'w', // MapCircleDown,
  ControlMapper::EKBMFunctionList::KeyPress + 'a', // MapCircleLeft,
  ControlMapper::EKBMFunctionList::KeyPress + 'd', // MapCircleRight,
  ControlMapper::EKBMFunctionList::SpecialKeyPress + boo::ESpecialKey::Up, // MapMoveForward,
  ControlMapper::EKBMFunctionList::SpecialKeyPress + boo::ESpecialKey::Down, // MapMoveBack,
  ControlMapper::EKBMFunctionList::SpecialKeyPress + boo::ESpecialKey::Left, // MapMoveLeft,
  ControlMapper::EKBMFunctionList::SpecialKeyPress + boo::ESpecialKey::Right, // MapMoveRight,
  ControlMapper::EKBMFunctionList::KeyPress + 'e', // MapZoomIn,
  ControlMapper::EKBMFunctionList::KeyPress + 'q', // MapZoomOut,
  ControlMapper::EKBMFunctionList::KeyPress + 'e', // SpiderBall,
  ControlMapper::EKBMFunctionList::KeyPress + 'q', // ChaseCamera,
  ControlMapper::EKBMFunctionList::SpecialKeyPress + boo::ESpecialKey::Right, // XrayVisor = 50,
  ControlMapper::EKBMFunctionList::SpecialKeyPress + boo::ESpecialKey::Down, // ThermoVisor = 51,
  ControlMapper::EKBMFunctionList::SpecialKeyPress + boo::ESpecialKey::Left, // InviroVisor = 52,
  ControlMapper::EKBMFunctionList::SpecialKeyPress + boo::ESpecialKey::Up, // NoVisor = 53,
  ControlMapper::EKBMFunctionList::None, // VisorMenu,
  ControlMapper::EKBMFunctionList::None, // VisorUp,
  ControlMapper::EKBMFunctionList::None, // VisorDown,
  ControlMapper::EKBMFunctionList::KeyPress + 'e', // ShowCrosshairs,
  ControlMapper::EKBMFunctionList::None, // UNKNOWN
  ControlMapper::EKBMFunctionList::None, // UseSheild = 0x3B,
  ControlMapper::EKBMFunctionList::KeyPress + 'q', // ScanItem = 0x3C,
  ControlMapper::EKBMFunctionList::None, // UNKNOWN
  ControlMapper::EKBMFunctionList::None, // UNKNOWN
  ControlMapper::EKBMFunctionList::None, // UNKNOWN
  ControlMapper::EKBMFunctionList::None, // UNKNOWN
  ControlMapper::EKBMFunctionList::KeyPress + 'q', // PreviousPauseScreen = 0x41,
  ControlMapper::EKBMFunctionList::KeyPress + 'e', // NextPauseScreen = 0x42,
  ControlMapper::EKBMFunctionList::None, // UNKNOWN,
  ControlMapper::EKBMFunctionList::None, // None,
  ControlMapper::EKBMFunctionList::None
};

#define kCommandFilterCount 67
static bool skCommandFilterFlag[kCommandFilterCount] = {true};

void ControlMapper::SetCommandFiltered(ECommands cmd, bool filtered) { skCommandFilterFlag[int(cmd)] = filtered; }

void ControlMapper::ResetCommandFilters() {
  for (int i = 0; i < kCommandFilterCount; ++i)
    skCommandFilterFlag[i] = true;
}

bool ControlMapper::GetPressInput(ECommands cmd, const CFinalInput& input) {
  if (!skCommandFilterFlag[int(cmd)])
    return false;
  bool ret = false;
  EFunctionList func = EFunctionList(g_currentPlayerControl->GetMapping(atUint32(cmd)));
  if (func < EFunctionList::MAX) {
    if (BoolReturnFn fn = skPressFuncs[int(func)])
      ret = (input.*fn)();
  }
  if (const auto& kbm = input.GetKBM()) {
    EKBMFunctionList kbmfunc = skKBMMapping[int(cmd)];
    if (kbmfunc < EKBMFunctionList::MAX) {
      if (kbmfunc >= EKBMFunctionList::MousePress)
        ret |= input.m_PMouseButtons[int(kbmfunc) - int(EKBMFunctionList::MousePress)];
      else if (kbmfunc >= EKBMFunctionList::SpecialKeyPress)
        ret |= input.m_PSpecialKeys[int(kbmfunc) - int(EKBMFunctionList::SpecialKeyPress)];
      else if (kbmfunc >= EKBMFunctionList::KeyPress)
        ret |= input.m_PCharKeys[int(kbmfunc) - int(EKBMFunctionList::KeyPress)];
    }
  }
  return ret;
}

bool ControlMapper::GetDigitalInput(ECommands cmd, const CFinalInput& input) {
  if (!skCommandFilterFlag[int(cmd)])
    return false;
  bool ret = false;
  EFunctionList func = EFunctionList(g_currentPlayerControl->GetMapping(atUint32(cmd)));
  if (func < EFunctionList::MAX) {
    if (BoolReturnFn fn = skDigitalFuncs[int(func)])
      ret = (input.*fn)();
  }
  if (const auto& kbm = input.GetKBM()) {
    EKBMFunctionList kbmfunc = skKBMMapping[int(cmd)];
    if (kbmfunc < EKBMFunctionList::MAX) {
      if (kbmfunc >= EKBMFunctionList::MousePress)
        ret |= kbm->m_mouseButtons[int(kbmfunc) - int(EKBMFunctionList::MousePress)];
      else if (kbmfunc >= EKBMFunctionList::SpecialKeyPress)
        ret |= kbm->m_specialKeys[int(kbmfunc) - int(EKBMFunctionList::SpecialKeyPress)];
      else if (kbmfunc >= EKBMFunctionList::KeyPress)
        ret |= kbm->m_charKeys[int(kbmfunc) - int(EKBMFunctionList::KeyPress)];
    }
  }
  return ret;
}

static float KBToWASDX(const CKeyboardMouseControllerData& data) {
  float retval = 0.0;
  if (data.m_charKeys[int('a')])
    retval -= 1.0;
  if (data.m_charKeys[int('d')])
    retval += 1.0;
  return retval;
}

static float KBToWASDY(const CKeyboardMouseControllerData& data) {
  float retval = 0.0;
  if (data.m_charKeys[int('s')])
    retval -= 1.0;
  if (data.m_charKeys[int('w')])
    retval += 1.0;
  return retval;
}

static float KBToArrowsX(const CKeyboardMouseControllerData& data) {
  float retval = 0.0;
  if (data.m_specialKeys[int(boo::ESpecialKey::Left)])
    retval -= 1.0;
  if (data.m_specialKeys[int(boo::ESpecialKey::Right)])
    retval += 1.0;
  return retval;
}

static float KBToArrowsY(const CKeyboardMouseControllerData& data) {
  float retval = 0.0;
  if (data.m_specialKeys[int(boo::ESpecialKey::Down)])
    retval -= 1.0;
  if (data.m_specialKeys[int(boo::ESpecialKey::Up)])
    retval += 1.0;
  return retval;
}

float ControlMapper::GetAnalogInput(ECommands cmd, const CFinalInput& input) {
  if (!skCommandFilterFlag[int(cmd)])
    return 0.f;
  float ret = 0.f;
  EFunctionList func = EFunctionList(g_currentPlayerControl->GetMapping(atUint32(cmd)));
  if (func < EFunctionList::MAX) {
    if (FloatReturnFn fn = skAnalogFuncs[int(func)])
      ret = (input.*fn)();
  }
  if (const auto& kbm = input.GetKBM()) {
    switch (cmd) {
    case ECommands::Forward:
    case ECommands::LookDown:
    case ECommands::OrbitUp:
    case ECommands::MapCircleDown:
      ret = std::max(ret, KBToWASDY(*kbm) * input.m_leftMul);
      break;
    case ECommands::Backward:
    case ECommands::LookUp:
    case ECommands::OrbitDown:
    case ECommands::MapCircleUp:
      ret = std::max(ret, -KBToWASDY(*kbm) * input.m_leftMul);
      break;
    case ECommands::TurnLeft:
    case ECommands::StrafeLeft:
    case ECommands::LookLeft:
    case ECommands::OrbitLeft:
    case ECommands::MapCircleLeft:
      ret = std::max(ret, -KBToWASDX(*kbm) * input.m_leftMul);
      break;
    case ECommands::TurnRight:
    case ECommands::StrafeRight:
    case ECommands::LookRight:
    case ECommands::OrbitRight:
    case ECommands::MapCircleRight:
      ret = std::max(ret, KBToWASDX(*kbm) * input.m_leftMul);
      break;
    case ECommands::MapMoveForward:
      ret = std::max(ret, KBToArrowsY(*kbm));
      break;
    case ECommands::MapMoveBack:
      ret = std::max(ret, -KBToArrowsY(*kbm));
      break;
    case ECommands::MapMoveLeft:
      ret = std::max(ret, -KBToArrowsX(*kbm));
      break;
    case ECommands::MapMoveRight:
      ret = std::max(ret, KBToArrowsX(*kbm));
      break;
    default: {
      EKBMFunctionList kbmfunc = skKBMMapping[int(cmd)];
      if (kbmfunc < EKBMFunctionList::MAX) {
        if (kbmfunc >= EKBMFunctionList::MousePress)
          ret = std::max(ret, kbm->m_mouseButtons[int(kbmfunc) - int(EKBMFunctionList::MousePress)] ? 1.f : 0.f);
        else if (kbmfunc >= EKBMFunctionList::SpecialKeyPress)
          ret = std::max(ret, kbm->m_specialKeys[int(kbmfunc) - int(EKBMFunctionList::SpecialKeyPress)] ? 1.f : 0.f);
        else if (kbmfunc >= EKBMFunctionList::KeyPress)
          ret = std::max(ret, kbm->m_charKeys[int(kbmfunc) - int(EKBMFunctionList::KeyPress)] ? 1.f : 0.f);
      }
      break;
    }
    }
  }
  return ret;
}

const char* ControlMapper::GetDescriptionForCommand(ECommands cmd) {
  if (cmd > ECommands::MAX)
    return nullptr;
  return skCommandDescs[int(cmd)];
}

const char* ControlMapper::GetDescriptionForFunction(EFunctionList func) {
  if (func > EFunctionList::MAX)
    return nullptr;
  return skFunctionDescs[int(func)];
}

} // namespace urde
