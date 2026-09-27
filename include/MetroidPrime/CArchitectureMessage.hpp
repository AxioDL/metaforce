#ifndef _CARCHITECTUREMESSAGE
#define _CARCHITECTUREMESSAGE

#include "types.h"

#include "rstl/rc_ptr.hpp"

enum EArchMsgTarget {
  kAMT_IOWinManager,
  kAMT_Game,
};

enum EArchMsgType {
  kAM_RemoveIOWin = 0,
  kAM_CreateIOWin = 1,
  kAM_ChangeIOWinPriority = 2,
  kAM_RemoveAllIOWins = 3,
  kAM_TimerTick = 4,
  kAM_UserInput = 5,
  kAM_SetGameState = 6,
  kAM_ControllerStatus = 7,
  kAM_QuitGameplay = 8,
  kAM_FrameBegin = 10,
  kAM_FrameEnd = 11,
};

struct IArchitectureMessageParm {
  virtual ~IArchitectureMessageParm() {}
};

class CArchitectureMessage {

public:
  CArchitectureMessage(EArchMsgTarget target, int type,
                       const rstl::rc_ptr< IArchitectureMessageParm >& parm)
  : mTarget(target), mType(static_cast< EArchMsgType >(type)), mParm(parm) {}

  EArchMsgType GetType() const { return mType; }
  const IArchitectureMessageParm* GetParm() const { return mParm.GetPtr(); }
  EArchMsgTarget GetTarget() const { return mTarget; }

private:
  EArchMsgTarget mTarget;
  EArchMsgType mType;
  rstl::rc_ptr< IArchitectureMessageParm > mParm;
};

#endif // _CARCHITECTUREMESSAGE
