#ifndef __RETRO_CARCHITECTUREMESSAGE_HPP__
#define __RETRO_CARCHITECTUREMESSAGE_HPP__

#include <memory>
#include "GCNTypes.hpp"
#include "Input/CFinalInput.hpp"

namespace Retro
{

class CIOWin;

enum EArchMsgTarget
{
    TargetMainFlow = 0
};

enum EArchMsgType
{
    MsgDeleteIOWin = 0,
    MsgCreateIOWin = 1,
    MsgChangeIOWinPriority = 2,
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
    u32 m_parm;
    CArchMsgParmInt32(u32 parm) : m_parm(parm) {}
};

struct CArchMsgParmInt32Int32VoidPtr : IArchMsgParm
{
    u32 m_parm1;
    u32 m_parm2;
    const void* m_parm3;
    CArchMsgParmInt32Int32VoidPtr(u32 parm1, u32 parm2, const void* parm3)
    : m_parm1(parm1), m_parm2(parm2), m_parm3(parm3) {}
};

struct CArchMsgParmNull : IArchMsgParm
{
};

struct CArchMsgParmReal32 : IArchMsgParm
{
    float m_parm;
    CArchMsgParmReal32(float parm) : m_parm(parm) {}
};

struct CArchMsgParmUserInput : IArchMsgParm
{
    CFinalInput m_parm;
    CArchMsgParmUserInput(const CFinalInput& parm) : m_parm(parm) {}
};

struct CArchMsgParmControllerStatus : IArchMsgParm
{
    u16 m_parm1;
    bool m_parm2;
    CArchMsgParmControllerStatus(u16 a, bool b)
    : m_parm1(a), m_parm2(b) {}
};

class CArchitectureMessage
{
    EArchMsgTarget m_target;
    EArchMsgType m_type;
    std::unique_ptr<IArchMsgParm> m_parm;
public:
    CArchitectureMessage(EArchMsgTarget target, EArchMsgType type, IArchMsgParm* parm)
    : m_target(target), m_type(type), m_parm(parm) {}

    EArchMsgTarget GetTarget() const {return m_target;}
    EArchMsgType GetType() const {return m_type;}
    template <class T>
    const T* GetParm() const {return dynamic_cast<T*>(m_parm.get());}
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
    static CArchitectureMessage CreateCreateIOWin(EArchMsgTarget target, int pmin, int pmax, const CIOWin* iowin)
    {
        return CArchitectureMessage(target, MsgCreateIOWin, new CArchMsgParmInt32Int32VoidPtr(pmin, pmax, iowin));
    }
    static const CArchMsgParmInt32Int32VoidPtr& GetParmDeleteIOWin(const CArchitectureMessage& msg)
    {
        return *msg.GetParm<CArchMsgParmInt32Int32VoidPtr>();
    }
};

}

#endif // __RETRO_CARCHITECTUREMESSAGE_HPP__
