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

  DECLARE_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;

  void AddSelf(CStateManager& mgr);
  void RemoveSelf(CStateManager& mgr);

  float GetRadius() const { return mRadius; }
  float GetPriority() const { return mPriority; }
  bool IsRepulsor() const { return mIsRepulsor; }
  bool GetSwirl() const { return mSwirl; }
};

CHECK_CHILD_SIZEOF(CFishCloudModifier, CActor, 0x10)

#endif // _CFISHCLOUDMODIFIER
