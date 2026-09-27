#include "MetroidPrime/ScriptObjects/CFire.hpp"

#include "Kyoto/CToken.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"

#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "MetaRender/CCubeRenderer.hpp"

#include "rstl/optional_object.hpp"
#include <rstl/math.hpp>

CFire::CFire(const TToken< CGenDescription >& effect, TUniqueId uid, TAreaId area,
             const bool active, TUniqueId owner, const CTransform4f& xf, const CDamageInfo& dInfo,
             const CAABox& aabox, const CVector3f& vec, bool b1, CAssetId visorEffect, bool b2,
             bool b3, bool b4, float f1, float f2, float f3, float f4)
: CActor(uid, active, "Fire", CEntityInfo(area, NullConnectionList), xf,
         CModelData::CModelDataNull(), CMaterialList(kMT_Projectile), CActorParameters::None(),
         kInvalidUniqueId)
, xe8_(rs_new CElementGen(effect))
, mOwner(owner)
, mDamageInfo1(dInfo)
, mDamageInfo2(dInfo)
, x128_(aabox)
, x144_(f1)
, x148_24_(b2)
, x148_25_(b3)
, x148_26_(b4)
, x148_27_(b4 && b3 && b2)
, x148_28_(false)
, x148_29_(b1)
, x14c_(f2)
, x150_(visorEffect)
, x154_(f3)
, x158_(f4)
, x15c_(0.f) {
  xe8_->SetGlobalScale(vec);
  xe8_->SetTranslation(xf.GetTranslation());
}

CFire::~CFire() {}

void CFire::Touch(CActor& act, CStateManager& mgr) {
  if (act.GetUniqueId() == mOwner) {
    return;
  }

  mgr.ApplyDamage(
      GetUniqueId(), act.GetUniqueId(), GetUniqueId(), mDamageInfo2,
      CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
      CVector3f::Zero());
}

rstl::optional_object< CAABox > CFire::GetTouchBounds() const {
  if (GetActive()) {
    return x128_;
  }

  return rstl::optional_object_null();
}

void CFire::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  bool drawParticles = true;
  if (!x148_27_) {
    switch (mgr.GetPlayerState()->GetActiveVisor(mgr)) {
    case CPlayerState::kPV_Combat:
    case CPlayerState::kPV_Scan:
      drawParticles = x148_24_;
      break;
    case CPlayerState::kPV_XRay:
      drawParticles = x148_26_;
      break;
    case CPlayerState::kPV_Thermal:
      drawParticles = x148_25_;
      break;
    }
  }

  if (drawParticles)
    gpRender->AddParticleGen(*xe8_);
  CActor::AddToRenderer(frustum, mgr);
}

ENTITY_ACCEPT_IMPL(CFire)

void CFire::Think(float dt, CStateManager& mgr) {
  const float particleCount = static_cast< float >(xe8_->GetParticleCount()) /
                              static_cast< float >(xe8_->GetMaxParticles());
  if (GetActive()) {
    xe8_->Update(dt * x144_);
    mDamageInfo2 =
        mDamageInfo1.MakeScaledForTime((particleCount > 0.5f ? particleCount : 0.f) * dt);
  }

  bool doFree = false;
  if (xe8_->IsSystemDeletable()) {
    doFree = true;
  }

  if (x148_29_) {
    if (mgr.GetPlayer()->GetTouchBounds()->DoBoundsOverlap(*GetTouchBounds()) && !doFree &&
        particleCount > 0.5f) {
      mgr.Player()->SetVisorSteam(particleCount * x14c_, x154_, x158_, x150_, true);
    } else {
      mgr.Player()->SetVisorSteam(0.f, 1.f, 1.f, kInvalidAssetId, true);
    }
  }

  x15c_ += dt;

  if (x15c_ > 45.f)
    doFree = true;

  if (doFree) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

void CFire::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case kSM_Registered:
    xe8_->SetParticleEmission(true);
    SetActive(true);
    break;
  default:
    break;
  }
}
