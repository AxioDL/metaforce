#ifndef _CFISHCLOUDMODIFIER
#define _CFISHCLOUDMODIFIER

#include "MetroidPrime/CActor.hpp"

class CFishCloudModifier : public CActor {
  float mRadius;
  float mPriority;
  bool mIsRepulsor;
  bool mSwirl;

public:
  CFishCloudModifier(TUniqueId uid, bool active, const rstl::string& name, const CEntityInfo& info,
                     const CVector3f& pos, bool isRepulsor, bool swirl, float radius,
                     float priority);

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;

  void AddSelf(CStateManager& mgr);
  void RemoveSelf(CStateManager& mgr);

  float GetRadius() const { return mRadius; }
  float GetPriority() const { return mPriority; }
  bool IsRepulsor() const { return mIsRepulsor; }
  bool GetSwirl() const { return mSwirl; }
};

CHECK_SIZEOF(CFishCloudModifier, (VERSION >= VERSION_GM8E_02 ? 0x108 : 0xf8))

#endif // _CFISHCLOUDMODIFIER
