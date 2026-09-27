#ifndef _CSCRIPTPLAYERSTATECHANGE
#define _CSCRIPTPLAYERSTATECHANGE

#include "MetroidPrime/CEntity.hpp"

class CScriptPlayerStateChange : public CEntity {
public:
  enum EControl { kC_Unfiltered, kC_Filtered };
  enum EControlCommandOption { kCCO_Unfiltered, kCCO_Filtered };

private:
  int mItemType;
  int mItemCount;
  int mItemCapacity;
  EControl mCtrl;
  EControlCommandOption mCtrlCmdOpt;

public:
  CScriptPlayerStateChange(TUniqueId, const rstl::string&, const CEntityInfo&, bool, int, int, int, EControl,
                           EControlCommandOption);
  DECLARE_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;

  bool Filtered() { return mCtrlCmdOpt == kCCO_Filtered; }
};

#endif // _CSCRIPTPLAYERSTATECHANGE
