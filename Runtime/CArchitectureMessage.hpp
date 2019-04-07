#pragma once

#include "GCNTypes.hpp"
#include "Input/CFinalInput.hpp"
#include "rstl.hpp"

namespace urde {
class CIOWin;

enum class EArchMsgTarget {
  IOWinManager = 0,
  Game = 1,
};

enum class EArchMsgType {
  RemoveIOWin = 0,
  CreateIOWin = 1,
  ChangeIOWinPriority = 2,
  RemoveAllIOWins = 3,
  TimerTick = 4,
  UserInput = 5,
  SetGameState = 6,
  ControllerStatus = 7,
  QuitGameplay = 8,
  FrameBegin = 10,
  FrameEnd = 11,
};

struct IArchMsgParm {
  virtual ~IArchMsgParm() = default;
};

struct CArchMsgParmInt32 : IArchMsgParm {
  u32 x4_parm;
  CArchMsgParmInt32(u32 parm) : x4_parm(parm) {}
};

struct CArchMsgParmVoidPtr : IArchMsgParm {
  void* x4_parm1;
  CArchMsgParmVoidPtr(void* parm1) : x4_parm1(parm1) {}
};

struct CArchMsgParmInt32Int32VoidPtr : IArchMsgParm {
  u32 x4_parm1;
  u32 x8_parm2;
  void* xc_parm3;
  CArchMsgParmInt32Int32VoidPtr(u32 parm1, u32 parm2, void* parm3)
  : x4_parm1(parm1), x8_parm2(parm2), xc_parm3(parm3) {}
};

struct CArchMsgParmInt32Int32IOWin : IArchMsgParm {
  u32 x4_parm1;
  u32 x8_parm2;
  std::shared_ptr<CIOWin> xc_parm3;
  CArchMsgParmInt32Int32IOWin(u32 parm1, u32 parm2, std::shared_ptr<CIOWin>&& parm3)
  : x4_parm1(parm1), x8_parm2(parm2), xc_parm3(std::move(parm3)) {}
};

struct CArchMsgParmNull : IArchMsgParm {};

struct CArchMsgParmReal32 : IArchMsgParm {
  float x4_parm;
  CArchMsgParmReal32(float parm) : x4_parm(parm) {}
};

struct CArchMsgParmUserInput : IArchMsgParm {
  CFinalInput x4_parm;
  CArchMsgParmUserInput(const CFinalInput& parm) : x4_parm(parm) {}
};

struct CArchMsgParmControllerStatus : IArchMsgParm {
  u16 x4_parm1;
  bool x6_parm2;
  CArchMsgParmControllerStatus(u16 a, bool b) : x4_parm1(a), x6_parm2(b) {}
};

class CArchitectureMessage {
  EArchMsgTarget x0_target;
  EArchMsgType x4_type;
  std::shared_ptr<IArchMsgParm> x8_parm;

public:
  CArchitectureMessage(EArchMsgTarget target, EArchMsgType type, std::shared_ptr<IArchMsgParm>&& parm)
  : x0_target(target), x4_type(type), x8_parm(std::move(parm)) {}

  EArchMsgTarget GetTarget() const { return x0_target; }
  EArchMsgType GetType() const { return x4_type; }
  template <class T>
  const T* GetParm() const {
    return static_cast<T*>(x8_parm.get());
  }
};

class MakeMsg {
public:
  static CArchitectureMessage CreateQuitGameplay(EArchMsgTarget target) {
    return CArchitectureMessage(target, EArchMsgType::QuitGameplay, std::make_shared<CArchMsgParmNull>());
  }
  static CArchitectureMessage CreateControllerStatus(EArchMsgTarget target, u16 a, bool b) {
    return CArchitectureMessage(target, EArchMsgType::ControllerStatus,
                                std::make_shared<CArchMsgParmControllerStatus>(a, b));
  }
  static const CArchMsgParmInt32& GetParmNewGameflowState(const CArchitectureMessage& msg) {
    return *msg.GetParm<CArchMsgParmInt32>();
  }
  static const CArchMsgParmUserInput& GetParmUserInput(const CArchitectureMessage& msg) {
    return *msg.GetParm<CArchMsgParmUserInput>();
  }
  static CArchitectureMessage CreateUserInput(EArchMsgTarget target, const CFinalInput& input) {
    return CArchitectureMessage(target, EArchMsgType::UserInput, std::make_shared<CArchMsgParmUserInput>(input));
  }
  static const CArchMsgParmReal32& GetParmTimerTick(const CArchitectureMessage& msg) {
    return *msg.GetParm<CArchMsgParmReal32>();
  }
  static CArchitectureMessage CreateTimerTick(EArchMsgTarget target, float val) {
    return CArchitectureMessage(target, EArchMsgType::TimerTick, std::make_shared<CArchMsgParmReal32>(val));
  }
  static const CArchMsgParmInt32Int32VoidPtr& GetParmChangeIOWinPriority(const CArchitectureMessage& msg) {
    return *msg.GetParm<CArchMsgParmInt32Int32VoidPtr>();
  }
  static const CArchMsgParmInt32Int32IOWin& GetParmCreateIOWin(const CArchitectureMessage& msg) {
    return *msg.GetParm<CArchMsgParmInt32Int32IOWin>();
  }
  static CArchitectureMessage CreateCreateIOWin(EArchMsgTarget target, int pmin, int pmax,
                                                std::shared_ptr<CIOWin>&& iowin) {
    return CArchitectureMessage(target, EArchMsgType::CreateIOWin,
                                std::make_shared<CArchMsgParmInt32Int32IOWin>(pmin, pmax, std::move(iowin)));
  }
  static const CArchMsgParmVoidPtr& GetParmDeleteIOWin(const CArchitectureMessage& msg) {
    return *msg.GetParm<CArchMsgParmVoidPtr>();
  }
  static CArchitectureMessage CreateFrameBegin(EArchMsgTarget target, s32 a) {
    return CArchitectureMessage(target, EArchMsgType::FrameBegin, std::make_shared<CArchMsgParmInt32>(a));
  }
  static CArchitectureMessage CreateFrameEnd(EArchMsgTarget target, s32 a) {
    return CArchitectureMessage(target, EArchMsgType::FrameEnd, std::make_shared<CArchMsgParmInt32>(a));
  }
  /* URDE Messages */
  static CArchitectureMessage CreateRemoveAllIOWins(EArchMsgTarget target) {
    return CArchitectureMessage(target, EArchMsgType::RemoveAllIOWins, std::make_shared<CArchMsgParmNull>());
  }
};
} // namespace urde
