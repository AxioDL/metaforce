#ifndef _CSUSTAINEDPLAYERDAMAGE
#define _CSUSTAINEDPLAYERDAMAGE

#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CDamageInfo.hpp"

class CSustainedPlayerDamage : public CEntity {
public:
#if TARGET_PC
  void DrawInspectorPanel() override;
#endif
  CSustainedPlayerDamage(TUniqueId uid, const CEntityInfo& info, const bool active,
                         const rstl::string& name, const CDamageInfo& dInfo, float f1);
  DECLARE_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  
  void StopDamaging(CStateManager& mgr);

private:
  CDamageInfo mDamage;
  float x50_;
  float x54_;
  bool x58_24 : 1;
};


#endif
