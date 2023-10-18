#include "Runtime/World/CScriptSteam.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

CScriptSteam::CScriptSteam(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CVector3f& pos,
                           const zeus::CAABox& aabb, const CDamageInfo& dInfo, const zeus::CVector3f& orientedForce,
                           ETriggerFlags flags, bool active, CAssetId texture, float f1, float f2, float f3, float f4,
                           bool b1)
: CScriptTrigger(uid, name, info, pos, aabb, dInfo, orientedForce, flags, active, false, false)
, x150_(b1)
, x154_texture(texture)
, x158_(f1)
, x15c_alphaInDur(f2 / f1)
, x160_alphaOutDur(f3 / f1) {
  float r3 = std::min(aabb.max.x(), std::min(aabb.max.y(), aabb.max.z()));
  x164_ = zeus::close_enough(f4, 0.f) ? r3 : std::min(f4, r3);
  x168_ = 1.f / x164_;
}

void CScriptSteam::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptSteam::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::Deactivate) {
    mgr.Player()->SetVisorSteam(0.f, x158_, x160_alphaOutDur, CAssetId(), !x150_);
  }

  CScriptTrigger::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptSteam::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  CScriptTrigger::Think(dt, mgr);

  if (x148_28_playerTriggerProc && mgr.GetCameraManager()->GetFluidCounter() == 0) {
    const zeus::CVector3f eyePos = mgr.GetPlayer().GetEyePosition();
    const zeus::CVector3f posDiff = (GetTranslation() - eyePos);
    const float mag = posDiff.magnitude();
    const float distance = (mag >= x164_ ? 0.f : std::cos((1.5707964f * mag) * x168_) * x158_);
    mgr.Player()->SetVisorSteam(distance, x15c_alphaInDur, x160_alphaOutDur, x154_texture, !x150_);
    if (x150_) {
      mgr.GetEnvFxManager()->SetSplashRate(2.f * distance);
    }
  } else {
    mgr.Player()->SetVisorSteam(0.f, x15c_alphaInDur, x160_alphaOutDur, CAssetId(), !x150_);
  }
}

} // namespace metaforce
