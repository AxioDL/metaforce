#ifndef _CFISHCLOUDMODIFIER
#define _CFISHCLOUDMODIFIER

#include "MetroidPrime/CActor.hpp"

class CFishCloudModifier : public CActor {
  float xe8_radius;
  float xec_priority;
  bool xf0_isRepulsor;
  bool xf1_swirl;

public:
  CFishCloudModifier(TUniqueId uid, bool active, const rstl::string& name, const CEntityInfo& info,
                     const CVector3f& pos, bool isRepulsor, bool swirl, float radius,
                     float priority);

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;

  void AddSelf(CStateManager& mgr);
  void RemoveSelf(CStateManager& mgr);

  float GetRadius() const { return xe8_radius; }
  float GetPriority() const { return xec_priority; }
  bool IsRepulsor() const { return xf0_isRepulsor; }
  bool GetSwirl() const { return xf1_swirl; }
};

CHECK_SIZEOF(CFishCloudModifier, (VERSION >= VERSION_GM8P_00 ? 0x108 : 0xf8))

#endif // _CFISHCLOUDMODIFIER
