#ifndef _CSCRIPTSWITCH
#define _CSCRIPTSWITCH

#include "types.h"

#include "MetroidPrime/CEntity.hpp"

class CScriptSwitch : public CEntity {
public:
  CScriptSwitch(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, bool, bool, bool);

  DECLARE_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr);

private:
  bool mOpened;
  bool mCloseOnOpened;
};

#endif // _CSCRIPTSWITCH
