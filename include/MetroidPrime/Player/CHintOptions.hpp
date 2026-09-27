#ifndef _CHINTOPTIONS
#define _CHINTOPTIONS

#include "types.h"

#include "rstl/vector.hpp"
#include "rstl/string.hpp"

enum EHintState { kHS_Zero, kHS_Waiting, kHS_Displaying, kHS_Delayed };

class CStateManager;
class CInputStream;
class COutputStream;

class CHintOptions {
public:
  struct SHintState {
    SHintState();
    SHintState(EHintState state, float time);

    EHintState mState;
    float mTime;
    bool mDismissed;

    bool CanContinue();
  };

  CHintOptions();
  explicit CHintOptions(CInputStream& in);
  void PutTo(COutputStream& out) const;
  void InitializeMemoryState();

  void SetHintNextTime();
#if VERSION >= VERSION_GM8P_00
  void EnsureHintNextTime();
#endif
  void Update(float dt, const CStateManager& mgr);

  void DelayHint(const rstl::string& name);
  void ActivateImmediateHintTimer(const rstl::string& name);
  void ActivateContinueDelayHintTimer(const rstl::string& name);
  void DismissDisplayedHint();

  const SHintState* GetCurrentDisplayedHint() const;
  int GetNextHintIdx();
  const rstl::vector< SHintState >& GetHintStates() const { return mHintStates; }

private:
  static uint GetBitCount(uint value);

  rstl::vector< SHintState > mHintStates;
  int mNextHintIdx;
#if VERSION >= VERSION_GM8P_00
  bool mPalHintFlag;
#endif
};

NESTED_CHECK_SIZEOF(CHintOptions, SHintState, 0xc)
#if VERSION >= VERSION_GM8P_00
CHECK_SIZEOF(CHintOptions, 0x18)
#else
CHECK_SIZEOF(CHintOptions, 0x14)
#endif

typedef CHintOptions::SHintState SHintState;

#endif // _CHINTOPTIONS
