#ifndef _CIOWINMANAGER
#define _CIOWINMANAGER

#include "types.h"

#include "MetroidPrime/CArchitectureQueue.hpp"

#include "rstl/list.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/string.hpp"

class CIOWin;

class CIOWinManager {
public:
  struct IOWinPQNode {
    rstl::ncrc_ptr< CIOWin > mIowin;
    int mPrio;
    IOWinPQNode* mNext;

    IOWinPQNode(rstl::ncrc_ptr< CIOWin > iowin, int prio, IOWinPQNode* next);

    rstl::ncrc_ptr< CIOWin > GetIOWin() const;
    IOWinPQNode* GetNext() const { return mNext; }
    void SetNext(IOWinPQNode* next) { mNext = next; }
    int GetPriority() const { return mPrio; }
    void SetPriority(int prio) { mPrio = prio; }
  };

  CIOWinManager();
  ~CIOWinManager();

  void Draw() const;
  void AddIOWin(rstl::ncrc_ptr< CIOWin >, int, int);
  void RemoveIOWin(rstl::ncrc_ptr< CIOWin > chIow);
  void RemoveAllIOWins();
  void ChangeIOWinPriority(rstl::ncrc_ptr< CIOWin > toChange, int pumpPrio, int drawPrio);
  rstl::ncrc_ptr< CIOWin > FindIOWin(const rstl::string& name) const;

  void PumpMessages(CArchitectureQueue& queue);
  bool DistributeOneMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue);
  bool OnIOWinMessage(const CArchitectureMessage& msg);

  inline bool IsEmpty() const { return mPumpRoot == nullptr && mDrawRoot == nullptr; }

private:
  IOWinPQNode* mDrawRoot;
  IOWinPQNode* mPumpRoot;
  CArchitectureQueue mLocalGatherQueue;
};
CHECK_SIZEOF(CIOWinManager, 0x20)

#endif // _CIOWINMANAGER
