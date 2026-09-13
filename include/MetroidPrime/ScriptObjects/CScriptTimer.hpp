#ifndef _CSCRIPTTIMER
#define _CSCRIPTTIMER

#include "MetroidPrime/CEntity.hpp"

class CScriptTimer : public CEntity {
  float x34_time;
  float x38_startTime;
  float x3c_maxRandDelay;
  uchar x40_loop;
  bool x41_autoStart;
  bool x42_isTiming;

  public:
    CScriptTimer(TUniqueId, const rstl::string&, const CEntityInfo&, float, float, bool, bool, bool);
    ~CScriptTimer();

    DECLARE_TYPES_MATCH_OR_ACCEPT;
    void Reset(CStateManager&);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void ApplyTime(float, CStateManager&);
    void Think(float, CStateManager&);

    bool IsTiming() const { return x42_isTiming; }
    void StartTiming(bool isTiming) {
      x42_isTiming = isTiming;
    }
};

#endif // _CSCRIPTTIMER
