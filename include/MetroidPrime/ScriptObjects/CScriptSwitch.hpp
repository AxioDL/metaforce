#ifndef _CSCRIPTSWITCH
#define _CSCRIPTSWITCH

#include "types.h"

#include "MetroidPrime/CEntity.hpp"

class CScriptSwitch : public CEntity {
public:
#if TARGET_PC
  void DrawInspectorPanel() override;
#endif
  CScriptSwitch(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, bool, bool, bool);

  DECLARE_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

private:
  bool mOpened;
  bool mCloseOnOpened;
};

#endif // _CSCRIPTSWITCH
