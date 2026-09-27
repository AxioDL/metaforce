#ifndef _CSCRIPTMAILBOX
#define _CSCRIPTMAILBOX

#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/reserved_vector.hpp"

class CWorldSaveGameInfo;
class CInputStream;
class COutputStream;
class CStateManager;
class CScriptMailbox {
public:
  CScriptMailbox();
  CScriptMailbox(CInputStream& in, const CWorldSaveGameInfo& world);
  ~CScriptMailbox();
  void PutTo(COutputStream& out, CWorldSaveGameInfo& world) const;
  void SendMsgs(const TAreaId& area, CStateManager& mgr);
  void AddMsg(TEditorId id);
  void RemoveMsg(TEditorId id);
  bool HasMsg(TEditorId id) const;

private:
  rstl::reserved_vector< TEditorId, 512 > mRelays;
};

#endif // _CSCRIPTMAILBOX
