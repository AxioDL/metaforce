#ifndef __RETRO_CARCHITECTUREMESSAGE_HPP__
#define __RETRO_CARCHITECTUREMESSAGE_HPP__

#include "GCNTypes.hpp"
#include "Input/CFinalInput.hpp"
#include "rstl.hpp"

namespace Retro
{
class CIOWin;

enum EArchMsgTarget
{
    TargetIOWinManager = 0,
    TargetGame = 1
};

enum EArchMsgType
{
    MsgRemoveIOWin = 0,
    MsgCreateIOWin = 1,
    MsgChangeIOWinPriority = 2,
    MsgRemoveAllIOWins = 3,
    MsgTimerTick = 4,
    MsgUserInput = 5,
    MsgSetGameState = 6,
    MsgControllerStatus = 7,
    MsgQuitGameplay = 8,
    MsgUpdateBegin = 10,
    MsgFrameBegin = 11,
};

struct IArchMsgParm
{
    virtual ~IArchMsgParm() {}
};

struct CArchMsgParmInt32 : IArchMsgParm
{
    u32 x4_parm;
    CArchMsgParmInt32(u32 parm) : x4_parm(parm) {}
};

struct CArchMsgParmVoidPtr : IArchMsgParm
{
    void* x4_parm1;
    CArchMsgParmVoidPtr(void* parm1)
    : x4_parm1(parm1) {}
};

struct CArchMsgParmInt32Int32VoidPtr : IArchMsgParm
{
    u32 x4_parm1;
    u32 x8_parm2;
    void* xc_parm3;
    CArchMsgParmInt32Int32VoidPtr(u32 parm1, u32 parm2, void* parm3)
    : x4_parm1(parm1), x8_parm2(parm2), xc_parm3(parm3) {}
};

struct CArchMsgParmNull : IArchMsgParm
{
};

struct CArchMsgParmReal32 : IArchMsgParm
{
    float x4_parm;
    CArchMsgParmReal32(float parm) : x4_parm(parm) {}
};

struct CArchMsgParmUserInput : IArchMsgParm
{
    CFinalInput x4_parm;
    CArchMsgParmUserInput(const CFinalInput& parm) : x4_parm(parm) {}
};

struct CArchMsgParmControllerStatus : IArchMsgParm
{
    u16 x4_parm1;
    bool x6_parm2;
    CArchMsgParmControllerStatus(u16 a, bool b)
    : x4_parm1(a), x6_parm2(b) {}
};

class CArchitectureMessage
{
    EArchMsgTarget x0_target;
    EArchMsgType x4_type;
    rstl::rc_ptr<IArchMsgParm> x8_parm;
public:
    CArchitectureMessage(EArchMsgTarget target, EArchMsgType type, IArchMsgParm* parm)
    : x0_target(target), x4_type(type), x8_parm(parm) {}

    EArchMsgTarget GetTarget() const {return x0_target;}
    EArchMsgType GetType() const {return x4_type;}
    template <class T>
    const T* GetParm() const {return dynamic_cast<T*>(x8_parm.get());}
};

class MakeMsg
{
public:
    static CArchitectureMessage CreateQuitGameplay(EArchMsgTarget target)
    {
        return CArchitectureMessage(target, MsgQuitGameplay, new CArchMsgParmNull());
    }
    static CArchitectureMessage CreateControllerStatus(EArchMsgTarget target, u16 a, bool b)
    {
        return CArchitectureMessage(target, MsgControllerStatus, new CArchMsgParmControllerStatus(a, b));
    }
    static const CArchMsgParmInt32& GetParmNewGameflowState(const CArchitectureMessage& msg)
    {
        return *msg.GetParm<CArchMsgParmInt32>();
    }
    static const CArchMsgParmUserInput& GetParmUserInput(const CArchitectureMessage& msg)
    {
        return *msg.GetParm<CArchMsgParmUserInput>();
    }
    static CArchitectureMessage CreateUserInput(EArchMsgTarget target, const CFinalInput& input)
    {
        return CArchitectureMessage(target, MsgUserInput, new CArchMsgParmUserInput(input));
    }
    static const CArchMsgParmReal32& GetParmTimerTick(const CArchitectureMessage& msg)
    {
        return *msg.GetParm<CArchMsgParmReal32>();
    }
    static CArchitectureMessage CreateTimerTick(EArchMsgTarget target, float val)
    {
        return CArchitectureMessage(target, MsgTimerTick, new CArchMsgParmReal32(val));
    }
    static const CArchMsgParmInt32Int32VoidPtr& GetParmChangeIOWinPriority(const CArchitectureMessage& msg)
    {
        return *msg.GetParm<CArchMsgParmInt32Int32VoidPtr>();
    }
    static const CArchMsgParmInt32Int32VoidPtr& GetParmCreateIOWin(const CArchitectureMessage& msg)
    {
        return *msg.GetParm<CArchMsgParmInt32Int32VoidPtr>();
    }
    static CArchitectureMessage CreateCreateIOWin(EArchMsgTarget target, int pmin, int pmax, CIOWin* iowin)
    {
        return CArchitectureMessage(target, MsgCreateIOWin, new CArchMsgParmInt32Int32VoidPtr(pmin, pmax, iowin));
    }
    static const CArchMsgParmVoidPtr& GetParmDeleteIOWin(const CArchitectureMessage& msg)
    {
        return *msg.GetParm<CArchMsgParmVoidPtr>();
    }
};

}

#endif // __RETRO_CARCHITECTUREMESSAGE_HPP__
