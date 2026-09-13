#include "MetroidPrime/ScriptObjects/CSustainedPlayerDamage.hpp"

#include "MetroidPrime/Player/CPlayer.hpp"

CSustainedPlayerDamage::CSustainedPlayerDamage(TUniqueId uid, const CEntityInfo& info,
                                               const bool active, const rstl::string& name,
                                               const CDamageInfo& dInfo, float f1)
: CEntity(uid, info, active, name), mDamage(dInfo), x50_(f1), x54_(0.f), x58_24(false) {}

void CSustainedPlayerDamage::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (x54_ > x50_) {
    StopDamaging(mgr);
    mgr.DeleteObjectRequest(GetUniqueId());
  } else {
    mgr.ApplyDamage(
        GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), mDamage.MakeScaledForTime(dt),
        CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()));
  }

  x54_ += dt;
}

void CSustainedPlayerDamage::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                             CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered:
    mgr.Player()->IncrementEnvironmentDamage();
    x58_24 = true;
    break;
  case kSM_Deleted:
    StopDamaging(mgr);
    break;
  }
}

void CSustainedPlayerDamage::StopDamaging(CStateManager& mgr) {
  if (!x58_24) {
    return;
  }

  mgr.Player()->DecrementEnvironmentDamage();
  x58_24 = false;
}

ENTITY_ACCEPT_IMPL(CSustainedPlayerDamage)
