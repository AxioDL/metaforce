#ifndef _CSCRIPTPLAYERSTATECHANGE
#define _CSCRIPTPLAYERSTATECHANGE

#include "MetroidPrime/CEntity.hpp"

class CScriptPlayerStateChange : public CEntity {
public:
  enum EControl { kC_Unfiltered, kC_Filtered };
  enum EControlCommandOption { kCCO_Unfiltered, kCCO_Filtered };

private:
  int x34_itemType;
  int x38_itemCount;
  int x3c_itemCapacity;
  EControl x40_ctrl;
  EControlCommandOption x44_ctrlCmdOpt;

public:
  CScriptPlayerStateChange(TUniqueId, const rstl::string&, const CEntityInfo&, bool, int, int, int, EControl,
                           EControlCommandOption);
  DECLARE_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  ~CScriptPlayerStateChange();

  bool Filtered() { return x44_ctrlCmdOpt == kCCO_Filtered; }
};

#endif // _CSCRIPTPLAYERSTATECHANGE
