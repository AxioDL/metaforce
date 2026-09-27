#ifndef _CSCRIPTTIMER
#define _CSCRIPTTIMER

#include "MetroidPrime/CEntity.hpp"

class CScriptTimer : public CEntity {
#if VERSION >= VERSION_GM8P_00
  uint mStartFrame;
#endif
  float mTime;
  float mStartTime;
  float mMaxRandDelay;
  uchar mLoop;
  bool mAutoStart;
  bool mIsTiming;

  public:
    CScriptTimer(TUniqueId, const rstl::string&, const CEntityInfo&, float, float, bool, bool, bool);
    ~CScriptTimer();

    DECLARE_TYPES_MATCH_OR_ACCEPT;
    void Reset(CStateManager&);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
    void ApplyTime(float, CStateManager&);
    void Think(float, CStateManager&) override;

    bool IsTiming() const { return mIsTiming; }
    void StartTiming(bool isTiming) {
      mIsTiming = isTiming;
    }
};

CHECK_SIZEOF(CScriptTimer, (VERSION >= VERSION_GM8P_00 ? 0x48 : 0x44))

#endif // _CSCRIPTTIMER
