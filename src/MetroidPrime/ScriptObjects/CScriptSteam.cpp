#include "MetroidPrime/ScriptObjects/CScriptSteam.hpp"

#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

#include "Kyoto/Math/CloseEnough.hpp"

#include "rstl/math.hpp"

CScriptSteam::CScriptSteam(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CVector3f& pos, const CAABox& aabb, const CDamageInfo& dInfo,
                           const CVector3f& orientedForce, unsigned int flags, bool active,
                           CAssetId texture, float f1, float f2, float f3, const float f4, bool b1)
: CScriptTrigger(uid, name, info, pos, aabb, dInfo, orientedForce, flags, active, false, false)
, x150_(b1)
, mTexture(texture)
, mStrength(f1)
, mAlphaInDur(f2 / f1)
, mAlphaOutDur(f3 / f1)
, mMaxDist(0.f)
, mOoMaxDist(0.f) {
  float r3 = rstl::min_val(aabb.GetMaxPoint().GetX(),
                           rstl::min_val(aabb.GetMaxPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  mMaxDist = close_enough(f4, 0.f) ? r3 : rstl::min_val(f4, r3);
  mOoMaxDist = 1.f / mMaxDist;
}

CScriptSteam::~CScriptSteam() {}

void CScriptSteam::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_Deactivate:
    mgr.Player()->SetVisorSteam(0.f, mAlphaInDur, mAlphaOutDur, kInvalidAssetId, !x150_);
    break;
  }

  CScriptTrigger::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptSteam::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  CScriptTrigger::Think(dt, mgr);

  if (mPlayerTriggerProc && mgr.GetCameraManager()->GetFluidCounter() == 0) {
    CVector3f eyePos = mgr.GetPlayer()->GetEyePosition();
    const float mag = (GetTranslation() - eyePos).Magnitude();
    float distance;
    if (mag >= GetMaxDist()) {
      distance = 0.f;
    } else {
      distance = CMath::FastCosR((mag * 1.5707964f /* 90 deg */) * GetOOMaxDist()) * GetStrength();
    }
    mgr.Player()->SetVisorSteam(distance, GetFadeInRate(), GetFadeOutRate(), GetTextureId(),
                                !x150_);
    if (x150_) {
      mgr.EnvFxManager()->SetSplashRate(2.f * distance);
    }
  } else {
    mgr.Player()->SetVisorSteam(0.f, GetFadeInRate(), GetFadeOutRate(), kInvalidAssetId, !x150_);
  }
}

ENTITY_ACCEPT_IMPL(CScriptSteam)
