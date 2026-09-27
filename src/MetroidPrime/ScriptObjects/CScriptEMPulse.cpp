#include "MetroidPrime/ScriptObjects/CScriptEMPulse.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

#include "MetaRender/CCubeRenderer.hpp"

#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"

CScriptEMPulse::CScriptEMPulse(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                               const CTransform4f& xf, const bool active, float initialRadius,
                               float finalRadius, float duration, float interferenceDur, float f5,
                               float interferenceMag, float f7, CAssetId partId)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_Projectile),
         CActorParameters::None().HotInThermal(true), kInvalidUniqueId)
, mDuration(duration)
, mFinalRadius(finalRadius)
, mCurrentRadius(initialRadius)
, mInitialRadius(initialRadius)
, mInterferenceDur(interferenceDur)
, xfc_(f5)
, mInterferenceMag(interferenceMag)
, x104_(f7)
, mParticleDesc(gpSimplePool->GetObj(SObjectTag('PART', partId))) {}

void CScriptEMPulse::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case kSM_Activate:

    mParticleGen =
        rs_new CElementGen(mParticleDesc, CElementGen::kMOT_Normal, CElementGen::kOSF_One);

    mParticleGen->SetOrientation(GetTransform().GetRotation());
    mParticleGen->SetGlobalTranslation(GetTranslation());
    mParticleGen->SetParticleEmission(true);
    mgr.PlayerState()->StaticInterference().AddSource(GetUniqueId(), mInterferenceMag,
                                                      mInterferenceDur);
    break;
  default:
    break;
  }
}

ENTITY_ACCEPT_IMPL(CScriptEMPulse)

void CScriptEMPulse::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  float step = (mFinalRadius - mInitialRadius) / mDuration;
  mCurrentRadius = mCurrentRadius + step * dt;
  if (mCurrentRadius >= mFinalRadius) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }

  mParticleGen->Update(dt);
}

CAABox CScriptEMPulse::CalculateBoundingBox() const {
  float radius = mCurrentRadius;
  CVector3f position(GetTranslation());
  return CAABox(position - CVector3f(radius, radius, radius),
                position + CVector3f(radius, radius, radius));
}

rstl::optional_object< CAABox > CScriptEMPulse::GetTouchBounds() const {
  return CalculateBoundingBox();
}

void CScriptEMPulse::CalculateRenderBounds() { SetRenderBounds(CalculateBoundingBox()); }

void CScriptEMPulse::Touch(CActor& act, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (CPlayer* pl = TCastToPtr< CPlayer >(act)) {
    // CVector3f thisPos(GetTranslation());
    // CVector3f posDiff(thisPos - pl->GetTranslation());
    // float diffMagnitude = posDiff.Magnitude();
    float diffMagnitude = (GetTranslation() - pl->GetTranslation()).Magnitude();
    if (diffMagnitude < mFinalRadius) {
      const float diffMagOp = 1.f - (diffMagnitude / mFinalRadius);
      const float dur = (diffMagOp * (xfc_ - mInterferenceDur)) + mInterferenceDur;
      const float mag = (diffMagOp * (x104_ - mInterferenceMag)) + mInterferenceMag;

      if (dur > pl->GetStaticTimer()) {
        pl->SetHudDisable(dur);
        pl->TryToBreakOrbit(mgr.GetPlayer()->GetOrbitTargetId(), CPlayer::kOB_ActivateOrbitSource,
                            mgr);
      }
      mgr.PlayerState()->StaticInterference().AddSource(GetUniqueId(), mag, dur);
    }
  }
}

void CScriptEMPulse::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  CActor::AddToRenderer(frustum, mgr);
  if (GetActive()) {
    gpRender->AddParticleGen(*mParticleGen);
  }
}
