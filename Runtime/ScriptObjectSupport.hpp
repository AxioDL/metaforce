#ifndef __SCRIPT_OBJECT_SUPPORT_HPP__
#define __SCRIPT_OBJECT_SUPPORT_HPP__

namespace pshag
{

enum class EScriptObjectState
{
    Active,
    Arrived,
    Closed,
    Entered,
    Exited,
    Inactive,
    Inside,
    MaxReached,
    Open,
    Zero,
    Attack,
    UNKS1,
    Retreat,
    Patrol,
    Dead,
    CameraPath,
    CameraTarget,
    UNKS2,
    Play,
    UNKS3,
    DeathRattle,
    UNKS4,
    Damage,
    UNKS6,
    UNKS5,
    Modify,
    UNKS7,
    UNKS8,
    ScanDone,
    UNKS9,
    DFST,
    ReflectedDamage,
    InheritBounds
};

enum class EScriptObjectMessage
{
    None = -1,
    UNKM1 = 0,
    Activate,
    UNKM2,
    Close,
    Deactivate,
    Decrement,
    Follow,
    Increment,
    Next,
    Open,
    Reset,
    ResetAndStart,
    SetToMax,
    SetToZero,
    Start,
    Stop,
    StopAndReset,
    ToggleActive,
    UNKM3,
    Action,
    Play,
    Alert
};

}

#endif // __SCRIPT_OBJECT_SUPPORT_HPP__
