#ifndef __SCRIPT_OBJECT_SUPPORT_HPP__
#define __SCRIPT_OBJECT_SUPPORT_HPP__

namespace Retro
{

enum EScriptObjectState
{
    StActive,
    StArrived,
    StClosed,
    StEntered,
    StExited,
    StInactive,
    StInside,
    StMaxReached,
    StOpen,
    StZero,
    StAttack,
    StUNKS1,
    StRetreat,
    StPatrol,
    StDead,
    StCameraPath,
    StCameraTarget,
    StUNKS2,
    StPlay,
    StUNKS3,
    StDeathRattle,
    StUNKS4,
    StDamage,
    StUNKS6,
    StUNKS5,
    StModify,
    StUNKS7,
    StUNKS8,
    StScanDone,
    StUNKS9,
    StDFST,
    StReflectedDamage,
    StInheritBounds
};

enum EScriptObjectMessage
{
    MsgNone = -1,
    MsgUNKM1 = 0,
    MsgActivate,
    MsgUNKM2,
    MsgClose,
    MsgDeactivate,
    MsgDecrement,
    MsgFollow,
    MsgIncrement,
    MsgNext,
    MsgOpen,
    MsgReset,
    MsgResetAndStart,
    MsgSetToMax,
    MsgSetToZero,
    MsgStart,
    MsgStop,
    MsgStopAndReset,
    MsgToggleActive,
    MsgUNKM3,
    MsgAction,
    MsgPlay,
    MsgAlert
};

}

#endif // __SCRIPT_OBJECT_SUPPORT_HPP__
