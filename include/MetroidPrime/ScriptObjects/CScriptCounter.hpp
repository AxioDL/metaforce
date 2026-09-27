#ifndef _CSCRIPTCOUNTER
#define _CSCRIPTCOUNTER

#include "MetroidPrime/CEntity.hpp"

class CScriptCounter : public CEntity {
  int mInitial;
  int mCurrent;
  int mMax;
  bool mAutoReset;

public:
  CScriptCounter(TUniqueId, const rstl::string& name, const CEntityInfo& info, int, int, bool,
                 bool);
  ~CScriptCounter();

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  DECLARE_ACCEPT;
};

#endif // _CSCRIPTCOUNTER
