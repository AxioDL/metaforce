#ifndef __URDE_CCONTROLMAPPER_HPP__
#define __URDE_CCONTROLMAPPER_HPP__

namespace urde
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
        UseSheild=0x3B,
        ScanItem=0x3C,
        PreviousPauseScreen=0x41,
        NextPauseScreen=0x42,
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

#endif // __URDE_CCONTROLMAPPER_HPP__
