#ifndef _CSCRIPTHUDMEMO
#define _CSCRIPTHUDMEMO

#include "MetroidPrime/CEntity.hpp"

#include "MetroidPrime/HUD/CHUDMemoParms.hpp"

#include "rstl/optional_object.hpp"

class CStringTable;
class CScriptHUDMemo : public CEntity {
public:
  enum EDisplayType {
    kDT_StatusMessage,
    kDT_MessageBox,
  };

private:
  CHUDMemoParms mParms;
  EDisplayType mDispType;
  CAssetId mStringTableId;
  rstl::optional_object< TLockedToken< CStringTable > > mStringTable;

public:
  CScriptHUDMemo(TUniqueId, const rstl::string&, const CEntityInfo&, const CHUDMemoParms&,
                 CScriptHUDMemo::EDisplayType, CAssetId, bool);
  ~CScriptHUDMemo();

  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
};
CHECK_SIZEOF(CScriptHUDMemo, 0x54)

#endif // _CSCRIPTHUDMEMO
