#ifndef _CGSFREELOOK
#define _CGSFREELOOK

#include "types.h"

class CAnimData;
class CStateManager;

class CGSFreeLook {
  float mDelay;
  int mCueAnimId;
  int mLoopState; // In, loop, out
  int mGunId;
  int mSetId;
  bool mIdle : 1;

public:
  CGSFreeLook();

  int GetSetId() const { return mSetId; }
  void SetLoopState(const int l) { mLoopState = l; }
  int GetLoopState() const { return mLoopState; }
  void SetIdle(const bool l) { mIdle = l; }
  int GetGunId() const { return mGunId; }
  bool Update(CAnimData& data, float dt, CStateManager& mgr);
  int SetAnim(CAnimData& data, int gunId, int setId, int loopState, CStateManager& mgr,
              float delay);
};
CHECK_SIZEOF(CGSFreeLook, 0x18)

#endif // _CGSFREELOOK
