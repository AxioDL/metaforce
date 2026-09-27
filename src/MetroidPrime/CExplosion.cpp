#include "MetroidPrime/CExplosion.hpp"

#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "MetaRender/CCubeRenderer.hpp"

CExplosion::CExplosion(const TLockedToken< CGenDescription >& particle, TUniqueId uid, bool active,
                       const CEntityInfo& info, const rstl::string& name, const CTransform4f& xf,
                       const uint flags, const CVector3f& scale, const CColor& color)
: CEffect(uid, info, active, name, xf)
, mParticleGen(rs_new CElementGen(TToken< CGenDescription >(particle), CElementGen::kMOT_Normal,
                                     flags & 0x2 ? CElementGen::kOSF_Two : CElementGen::kOSF_One))
, mExplosionLight(kInvalidUniqueId)
, mSourceId(CToken(particle).GetTag().GetId())
, mRenderThermalHot(flags & 0x4)
, mHasRenderBounds(true)
, mRenderXray(flags & 0x8)
, mTime(0.0f) {
  SetThermalFlags(flags & 0x1 ? kTF_Cold : kTF_Hot);
  mParticleGen->SetGlobalTranslation(xf.GetTranslation());
  mParticleGen->SetOrientation(xf.GetRotation());
  mParticleGen->SetGlobalScale(scale);
  mParticleGen->SetModulationColor(color);
}

CExplosion::CExplosion(const TLockedToken< CElectricDescription >& electric, TUniqueId uid,
                       bool active, const CEntityInfo& info, const rstl::string& name,
                       const CTransform4f& xf, uint flags, const CVector3f& scale,
                       const CColor& color)
: CEffect(uid, info, active, name, xf)
, mParticleGen(rs_new CParticleElectric(electric))
, mExplosionLight(kInvalidUniqueId)
, mSourceId(CToken(electric).GetTag().GetId())
, mRenderThermalHot(flags & 0x4)
, mHasRenderBounds(true)
, mRenderXray(flags & 0x8)
#if NONMATCHING
, mTime(0.0f)
#endif
{
  SetThermalFlags(flags & 0x1 ? kTF_Cold : kTF_Hot);
  mParticleGen->SetGlobalTranslation(xf.GetTranslation());
  mParticleGen->SetOrientation(xf.GetRotation());
  mParticleGen->SetGlobalScale(scale);
  mParticleGen->SetModulationColor(color);
}

CExplosion::~CExplosion() {}

void CExplosion::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  if (GetPreRenderClipped()) {
    return;
  }

  if ((mRenderThermalHot && mgr.GetThermalDrawFlag() == kTD_Hot) ||
      (mRenderXray && mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay)) {
    EnsureRendered(mgr);
  } else {
    gpRender->AddParticleGen(*mParticleGen);
  }
}

void CExplosion::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  CActor::PreRender(mgr, frustum);
  SetPreRenderClipped(!mHasRenderBounds || !frustum.BoxInFrustumPlanes(GetRenderBoundsCached()));
}

void CExplosion::Think(float dt, CStateManager& mgr) {
  if (GetTransformDirtySpare()) {
    mParticleGen->SetGlobalTranslation(GetTranslation());
    mParticleGen->SetOrientation(GetTransform().GetRotation());
    SetTransformDirtySpare(false);
  }
  mParticleGen->Update(dt);

  if (mExplosionLight != kInvalidUniqueId) {
    CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mExplosionLight));
    if (light && GetActive())
      light->SetLight(mParticleGen->GetLight());
  }

  mTime += dt;

  if (mTime > 15.f) {
    mgr.DeleteObjectRequest(GetUniqueId());
  } else if (mParticleGen->IsSystemDeletable()) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

ENTITY_ACCEPT_IMPL(CExplosion)

void CExplosion::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered:
    if (mParticleGen->SystemHasLight()) {
      mExplosionLight = mgr.AllocateUniqueId();
      uint sourceId = mSourceId;
      mgr.AddObject(rs_new CGameLight(mExplosionLight, GetCurrentAreaId(), GetActive(),
                                      rstl::string_l("ExplodePLight_") + GetDebugName(),
                                      GetTransform(), GetUniqueId(), mParticleGen->GetLight(),
                                      sourceId, 1, 0.f));
    }
    break;
  case kSM_Deleted:
    if (mExplosionLight != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mExplosionLight);
      mExplosionLight = kInvalidUniqueId;
    }
    break;

  default:
    break;
  }
  CActor::AcceptScriptMsg(msg, sender, mgr);

  if (mExplosionLight != kInvalidUniqueId)
    mgr.SendScriptMsgAlways(mExplosionLight, sender, msg);
}

void CExplosion::CalculateRenderBounds() {
  rstl::optional_object< CAABox > bounds = mParticleGen->GetBounds();
  if (bounds) {
    SetRenderBounds(*bounds);
    mHasRenderBounds = true;
  } else {
    mHasRenderBounds = false;
    CVector3f pos = GetTransform().GetTranslation();
    SetRenderBounds(CAABox(pos, pos));
  }
}

void CExplosion::Render(const CStateManager& mgr) const {
  if (mgr.GetThermalDrawFlag() == kTD_Hot && mRenderThermalHot) {
    CElementGen::SetSubtractBlend(true);
    CCubeModel::SetRenderModelBlack(true);
    mParticleGen->Render();
    CCubeModel::SetRenderModelBlack(false);
    CElementGen::SetSubtractBlend(false);
    return;
  }

  CElementGen::SetSubtractBlend(!mRenderThermalHot);
  CGraphics::SetFog(kRFM_PerspLin, 0.f, 75.f, CColor::Black());
  mParticleGen->Render();
  mgr.SetupFogForArea(GetCurrentAreaId());
  CElementGen::SetSubtractBlend(false);
}
