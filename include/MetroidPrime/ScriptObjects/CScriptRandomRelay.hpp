#ifndef _CSCRIPTRANDOMRELAY
#define _CSCRIPTRANDOMRELAY

#include "MetroidPrime/CEntity.hpp"

class CScriptRandomRelay : public CEntity {
public:
  CScriptRandomRelay(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     int sendSetSize, int sendSetVariance, bool percentSize, bool active);
  ~CScriptRandomRelay();

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  void SendLocalScriptMsgs(EScriptObjectState state, CStateManager& stateMgr);

private:
  int mSendSetSize;
  int mSendSetVariance;
  bool mPercentSize;
};

#endif // _CSCRIPTRANDOMRELAY
