#ifndef _CSCRIPTDOCKAREACHANGE
#define _CSCRIPTDOCKAREACHANGE

#include "MetroidPrime/CEntity.hpp"

class CScriptDockAreaChange : public CEntity {
  int mDockReference;

public:
#if TARGET_PC
  void DrawInspectorPanel() override;
#endif
  CScriptDockAreaChange(const TUniqueId, const rstl::string&, const CEntityInfo&, int, const bool);

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  DECLARE_ACCEPT;
};

#endif // _CSCRIPTDOCKAREACHANGE
