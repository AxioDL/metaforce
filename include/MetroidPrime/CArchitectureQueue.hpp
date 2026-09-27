#ifndef _CARCHITECTUREQUEUE
#define _CARCHITECTUREQUEUE

#include "types.h"

#include "MetroidPrime/CArchitectureMessage.hpp"

#include "rstl/list.hpp"

class CArchitectureQueue {
public:
  void Push(const CArchitectureMessage& msg) { mQueue.push_back(msg); }
  CArchitectureMessage Pop() {
    CArchitectureMessage result = *mQueue.begin();
    mQueue.pop_front();
    return result;
  }
  void Clear() { mQueue.clear(); }
  bool IsEmpty() const { return mQueue.empty(); }

private:
  rstl::list< CArchitectureMessage > mQueue;
};

#endif // _CARCHITECTUREQUEUE
