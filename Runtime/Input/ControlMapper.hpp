#ifndef __RETRO_CCONTROLMAPPER_HPP__
#define __RETRO_CCONTROLMAPPER_HPP__

namespace Retro
{
class CFinalInput;

class ControlMapper
{
public:
    enum class ECommands
    {
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
        JumpOrBoost,
        FireOrBomb,
        MissileOrPowerBomb,
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
        ToggleHolster,
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
        ChaseCamera,
        XrayVisor,
        ThermoVisor,
        InviroVisor,
        NoVisor,
        VisorMenu,
        VisorUp,
        VisorDown,
        UseSheild=0x3B,
        ScanItem=0x3C,
        UNKNOWN,
        None,
        MAX
    };

    enum class EFunctionList
    {
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

    static void SetCommandFiltered(ECommands cmd, bool filtered);
    static void ResetCommandFilters();
    static bool GetPressInput(ECommands cmd, const CFinalInput& input);
    static bool GetDigitalInput(ECommands cmd, const CFinalInput& input);
    static float GetAnalogInput(ECommands cmd, const CFinalInput& input);
    static const char* GetDescriptionForCommand(ECommands cmd);
    static const char* GetDescriptionForFunction(EFunctionList func);
};

}

#endif // __RETRO_CCONTROLMAPPER_HPP__
