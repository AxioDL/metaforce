#ifndef _CSCRIPTMEMORYRELAY
#define _CSCRIPTMEMORYRELAY

#include "MetroidPrime/CEntity.hpp"

class CScriptMemoryRelay : public CEntity {
  uchar mDefaultActive : 1;
  bool mSkipSendActive : 1;
  bool mIgnoreMessages : 1;
public:
  CScriptMemoryRelay(TUniqueId, const rstl::string&, const CEntityInfo&, bool, bool, const bool);
  ~CScriptMemoryRelay();

  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  DECLARE_ACCEPT;
};

#endif // _CSCRIPTMEMORYRELAY
