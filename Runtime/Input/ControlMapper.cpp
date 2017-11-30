#include "../RetroTypes.hpp"
#include "ControlMapper.hpp"
#include "CFinalInput.hpp"
#include "DataSpec/DNACommon/Tweaks/ITweakPlayerControl.hpp"
#include "../GameGlobalObjects.hpp"

namespace urde
{

static const char* skCommandDescs[] =
{
    "Forward",
    "Backward",
    "Turn Left",
    "Turn Right",
    "Strafe Left",
    "Strafe Right",
    "Look Left",
    "Look Right",
    "Look Up",
    "Look Down",
    "Jump/Boost",
    "Fire/Bomb",
    "Missile/PowerBomb",
    "Morph",
    "Aim Up",
    "Aim Down",
    "Cycle Beam Up",
    "Cycle Beam Down",
    "Cycle Item",
    "Power Beam",
    "Ice Beam",
    "Wave Beam",
    "Plasma Beam",
    "Toggle Holster",
    "Orbit Close",
    "Orbit Far",
    "Orbit Object",
    "Orbit Select",
    "Orbit Confirm",
    "Orbit Left",
    "Orbit Right",
    "Orbit Up",
    "Orbit Down",
    "Look Hold1",
    "Look Hold2",
    "Look Zoom In",
    "Look Zoom Out",
    "Aim Hold",
    "Map Circle Up",
    "Map Circle Down",
    "Map Circle Left",
    "Map Circle Right",
    "Map Move Forward",
    "Map Move Back",
    "Map Move Left",
    "Map Move Right",
    "Map Zoom In",
    "Map Zoom Out",
    "SpiderBall",
    "Chase Camera",
    "XRay Visor",
    "Thermo Visor",
    "Enviro Visor",
    "No Visor",
    "Visor Menu",
    "Visor Up",
    "Visor Down",
    "UNKNOWN",
    "UNKNOWN",
    "Use Shield",
    "Scan Item",
    "UNKNOWN"

};

static const char* skFunctionDescs[] =
{
    "None",
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
    "Start"
};

typedef bool (CFinalInput::*BoolReturnFn)() const;
typedef float (CFinalInput::*FloatReturnFn)() const;

static BoolReturnFn skPressFuncs[] =
{
    nullptr,
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
    nullptr
};

static BoolReturnFn skDigitalFuncs[] =
{
    nullptr,
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
    nullptr
};

static FloatReturnFn skAnalogFuncs[] =
{
    nullptr,
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
    nullptr
};

#define kCommandFilterCount 67
static bool skCommandFilterFlag[kCommandFilterCount] = {true};

void ControlMapper::SetCommandFiltered(ECommands cmd, bool filtered)
{
    skCommandFilterFlag[int(cmd)] = filtered;
}

void ControlMapper::ResetCommandFilters()
{
    for (int i=0 ; i<kCommandFilterCount ; ++i)
        skCommandFilterFlag[i] = true;
}

bool ControlMapper::GetPressInput(ECommands cmd, const CFinalInput& input)
{
    if (!skCommandFilterFlag[int(cmd)])
        return false;
    EFunctionList func = EFunctionList(g_currentPlayerControl->GetMapping(atUint32(cmd)));
    if (func > EFunctionList::MAX)
        return false;
    BoolReturnFn fn = skPressFuncs[int(func)];
    if (!fn)
        return false;
    return (input.*fn)();
}

bool ControlMapper::GetDigitalInput(ECommands cmd, const CFinalInput& input)
{
    if (!skCommandFilterFlag[int(cmd)])
        return false;
    EFunctionList func = EFunctionList(g_currentPlayerControl->GetMapping(atUint32(cmd)));
    if (func > EFunctionList::MAX)
        return false;
    BoolReturnFn fn = skDigitalFuncs[int(func)];
    if (!fn)
        return false;
    return (input.*fn)();
}

float ControlMapper::GetAnalogInput(ECommands cmd, const CFinalInput& input)
{
    if (!skCommandFilterFlag[int(cmd)])
        return 0.0;
    EFunctionList func = EFunctionList(g_currentPlayerControl->GetMapping(atUint32(cmd)));
    if (func > EFunctionList::MAX)
        return 0.0;
    FloatReturnFn fn = skAnalogFuncs[int(func)];
    if (!fn)
        return 0.0;
    return (input.*fn)();
}

const char* ControlMapper::GetDescriptionForCommand(ECommands cmd)
{
    if (cmd > ECommands::MAX)
        return nullptr;
    return skCommandDescs[int(cmd)];
}

const char* ControlMapper::GetDescriptionForFunction(EFunctionList func)
{
    if (func > EFunctionList::MAX)
        return nullptr;
    return skFunctionDescs[int(func)];
}

}
