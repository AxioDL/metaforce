#ifndef __RETRO_CCONTROLMAPPER_HPP__
#define __RETRO_CCONTROLMAPPER_HPP__

namespace Retro
{
class CFinalInput;

class ControlMapper
{
public:
    enum ECommands
    {
        CmdForward,
        CmdBackward,
        CmdTurnLeft,
        CmdTurnRight,
        CmdStrafeLeft,
        CmdStrafeRight,
        CmdLookLeft,
        CmdLookRight,
        CmdLookUp,
        CmdLookDown,
        CmdJumpOrBoost,
        CmdFireOrBomb,
        CmdMissileOrPowerBomb,
        CmdMorph,
        CmdAimUp,
        CmdAimDown,
        CmdCycleBeamUp,
        CmdCycleBeamDown,
        CmdCycleItem,
        CmdPowerBeam,
        CmdIceBeam,
        CmdWaveBeam,
        CmdPlasmaBeam,
        CmdToggleHolster,
        CmdOrbitClose,
        CmdOrbitFar,
        CmdOrbitObject,
        CmdOrbitSelect,
        CmdOrbitConfirm,
        CmdOrbitLeft,
        CmdOrbitRight,
        CmdOrbitUp,
        CmdOrbitDown,
        CmdLookHold1,
        CmdLookHold2,
        CmdLookZoomIn,
        CmdLookZoomOut,
        CmdAimHold,
        CmdMapCircleUp,
        CmdMapCircleDown,
        CmdMapCircleLeft,
        CmdMapCircleRight,
        CmdMapMoveForward,
        CmdMapMoveBack,
        CmdMapMoveLeft,
        CmdMapMoveRight,
        CmdMapZoomIn,
        CmdMapZoomOut,
        CmdChaseCamera,
        CmdXrayVisor,
        CmdThermoVisor,
        CmdInviroVisor,
        CmdNoVisor,
        CmdVisorMenu,
        CmdVisorUp,
        CmdVisorDown,
        CmdUseSheild,
        CmdScanItem,
        CmdUNKNOWN,
        CmdNone,
        CmdMAX
    };

    enum EFunctionList
    {
        FuncNone,
        FuncLeftStickUp,
        FuncLeftStickDown,
        FuncLeftStickLeft,
        FuncLeftStickRight,
        FuncRightStickUp,
        FuncRightStickDown,
        FuncRightStickLeft,
        FuncRightStickRight,
        FuncLeftTrigger,
        FuncRightTrigger,
        FuncDPadUp,
        FuncDPadDown,
        FuncDPadLeft,
        FuncDPadRight,
        FuncAButton,
        FuncBButton,
        FuncXButton,
        FuncYButton,
        FuncZButton,
        FuncLeftTriggerPress,
        FuncRightTriggerPress,
        FuncStart,
        FuncMAX // default case
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
