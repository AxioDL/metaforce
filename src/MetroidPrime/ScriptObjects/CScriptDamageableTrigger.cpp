#include "MetroidPrime/ScriptObjects/CScriptDamageableTrigger.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Collision/CMaterialList.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

CScriptDamageableTrigger::CScriptDamageableTrigger(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CVector3f& position,
    const CVector3f& extent, const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
    uint faceFlag, CAssetId patternTex1, CAssetId patternTex2, CAssetId colorTex,
    ECanOrbit canOrbit, bool active, const CVisorParameters& vParams)
: CActor(uid, active, name, info, CTransform4f::Translate(position), CModelData::CModelDataNull(),
         canOrbit == kCO_Orbit
             ? CMaterialList(kMT_Orbit, kMT_Trigger, kMT_Immovable, kMT_NonSolidDamageable,
                             kMT_ExcludeFromLineOfSightTest)
             : CMaterialList(kMT_Trigger, kMT_Immovable, kMT_NonSolidDamageable,
                             kMT_ExcludeFromLineOfSightTest),
         CActorParameters::None().MakeDamageableTriggerActorParms(vParams), kInvalidUniqueId)
, mBounds(CVector3f(-(extent.GetX() * 0.5f), -(extent.GetY() * 0.5f), -(extent.GetZ() * 0.5f)),
              CVector3f(extent.GetX() * 0.5f, extent.GetY() * 0.5f, extent.GetZ() * 0.5f))
, mOrigHInfo(hInfo)
, mHInfo(hInfo)
, mDVuln(dVuln)
, mFaceFlag(faceFlag)
, mAlpha(1.f)
, mFaceDir(CTransform4f::Identity())
, mFaceDirInv(CTransform4f::Identity())
, mFaceTranslate(0.f, 0.f, 0.f)
, mAlphaTimer(0.f)
, mFluidPlane(patternTex1, patternTex2, colorTex, 1.f, 2, CFluidPlane::kFT_NormalWater, 1.f,
                  CFluidUVMotion(6.f, 0.f))
, mNotOccluded(false)
, mAlphaOut(false)
, mOutOfFrustum(false)
, mInvulnerable(false)
, mCanOrbit(canOrbit == kCO_Orbit) {
  if (mFaceFlag & 1) {
    mFaceTranslate = CVector3f(0.f, mBounds.GetMaxPoint().GetY(), 0.f);
    mFaceDir = CTransform4f::RotateX(CRelAngle::FromDegrees(-90.f));
  } else if (mFaceFlag & 2) {
    mFaceTranslate = CVector3f(0.f, mBounds.GetMinPoint().GetY(), 0.f);
    mFaceDir = CTransform4f::RotateX(CRelAngle::FromDegrees(90.f));
  } else if (mFaceFlag & 4) {
    mFaceTranslate = CVector3f(mBounds.GetMinPoint().GetX(), 0.f, 0.f);
    mFaceDir = CTransform4f::RotateY(CRelAngle::FromDegrees(-90.f));
  } else if (mFaceFlag & 8) {
    mFaceTranslate = CVector3f(mBounds.GetMaxPoint().GetX(), 0.f, 0.f);
    mFaceDir = CTransform4f::RotateY(CRelAngle::FromDegrees(90.f));
  } else if (mFaceFlag & 0x10) {
    mFaceTranslate = CVector3f(0.f, 0.f, mBounds.GetMaxPoint().GetZ());
    mFaceDir = CTransform4f::Identity();
  } else if (mFaceFlag & 0x20) {
    mFaceTranslate = CVector3f(0.f, 0.f, mBounds.GetMinPoint().GetZ());
    mFaceDir = CTransform4f::RotateY(CRelAngle::FromDegrees(180.f));
  }
  mFaceDirInv = mFaceDir.GetQuickInverse();
}

CScriptDamageableTrigger::~CScriptDamageableTrigger() {}

rstl::optional_object< CAABox > CScriptDamageableTrigger::GetTouchBounds() const {
  if (GetActive() && mNotOccluded) {
    return CAABox(mBounds.GetMinPoint() + GetTranslation(),
                  mBounds.GetMaxPoint() + GetTranslation());
  }
  return rstl::optional_object_null();
}

void CScriptDamageableTrigger::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender,
                                               CStateManager& mgr) {
  switch (msg) {
  case kSM_Deactivate:
    if (GetActive() && mAlphaOut) {
      return;
    }
    // Fall through.
  case kSM_Activate:
    if (!GetActive() || mAlphaOut) {
      mAlphaTimer = 0.f;
      mHInfo = mOrigHInfo;
      mAlphaOut = false;
      if (mCanOrbit) {
        AddMaterial(kMT_Orbit, mgr);
      }
      SetLinkedObjectAlpha(0.f, mgr);
      mAlpha = 0.f;
    }
    break;
  case kSM_Damage:
    if (mInvulnerable) {
      mHInfo = mOrigHInfo;
    }
    break;
  case kSM_Increment:
    mInvulnerable = true;
    break;
  case kSM_Decrement:
    mInvulnerable = false;
    break;
  default:
    break;
  }
  CActor::AcceptScriptMsg(msg, sender, mgr);
}

float CScriptDamageableTrigger::GetPuddleAlphaScale() const {
  if (mAlphaTimer <= 0.75f) {
    if (mAlphaOut) {
      return 1.f - mAlphaTimer / 0.75f;
    }
    return mAlphaTimer / 0.75f;
  }
  if (mAlphaOut) {
    return 0.f;
  }
  return 1.f;
}

void CScriptDamageableTrigger::SetLinkedObjectAlpha(float alpha, CStateManager& mgr) {
  const rstl::vector< SConnection >& connections = GetConnectionList();
  for (AUTO(it, connections.begin()); it != connections.end(); ++it) {
    if (it->mState == kSS_MaxReached && it->mMsg == kSM_Activate) {
      if (CScriptActor* actor =
              TCastToPtr< CScriptActor >(mgr.ObjectById(mgr.GetIdForScript(it->mObjId)))) {
        if (!actor->GetActive()) {
          actor->SetActive(true);
        }
        actor->SetModelFlags(CModelFlags(CModelFlags::kT_Blend, alpha));
      }
    }
  }
}

void CScriptDamageableTrigger::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  const CGameArea& area = mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId());
  mNotOccluded = area.GetOcclusionState() == CGameArea::kOS_Visible;

  if (mAlphaOut) {
    if (mAlphaTimer >= 0.75f) {
      SetActive(false);
      const rstl::vector< SConnection >& connections = GetConnectionList();
      for (AUTO(it, connections.begin()); it != connections.end(); ++it) {
        if (it->mState == kSS_MaxReached && it->mMsg == kSM_Activate) {
          if (CScriptActor* actor =
                  TCastToPtr< CScriptActor >(mgr.ObjectById(mgr.GetIdForScript(it->mObjId)))) {
            actor->SetActive(false);
          }
        }
      }
      SetLinkedObjectAlpha(0.f, mgr);
      mAlphaOut = false;
      return;
    }
  } else if (mHInfo.GetHP() <= 0.f && GetActive()) {
    SendScriptMsgs(kSS_Dead, mgr, kSM_None);
    RemoveMaterial(kMT_Orbit, mgr);
    mAlphaOut = true;
    mAlphaTimer = 0.f;
  }

  if (mAlphaTimer <= 0.75f) {
    mAlphaTimer += dt;
  }
  const float alpha = GetPuddleAlphaScale();
  mAlpha = 0.2f * alpha;
  SetLinkedObjectAlpha(alpha, mgr);
}

CHealthInfo* CScriptDamageableTrigger::HealthInfo(CStateManager&) { return &mHInfo; }

const CDamageVulnerability* CScriptDamageableTrigger::GetDamageVulnerability() const {
  return &mDVuln;
}

void CScriptDamageableTrigger::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  const CAABox bounds = mBounds;
  mOutOfFrustum = !frustum.BoxInFrustumPlanes(bounds.GetTransformedAABox(GetTransform()));
  if (mOutOfFrustum) {
    return;
  }
  mFrustum = frustum;
  CActor::PreRender(mgr, frustum);
}

void CScriptDamageableTrigger::AddToRenderer(const CFrustumPlanes&,
                                             const CStateManager& mgr) const {
  if (mOutOfFrustum) {
    return;
  }
  EnsureRendered(mgr, GetTranslation() - mFaceTranslate, GetSortingBounds(mgr));
}

void CScriptDamageableTrigger::Render(const CStateManager& mgr) const {
  if (GetActive() && mFaceFlag != 0 && !close_enough(mAlpha, 0.f)) {
    const CAABox bounds(
        CVector3f(mBounds.GetMinPoint().GetX(), mBounds.GetMinPoint().GetY(),
                  mBounds.GetMinPoint().GetZ()),
        CVector3f(mBounds.GetMaxPoint().GetX(), mBounds.GetMaxPoint().GetY(),
                  mBounds.GetMaxPoint().GetZ()));
    const CAABox faceBounds = bounds.GetTransformedAABox(mFaceDirInv);
    GetFluidPlane().Render(
        mgr, mAlpha, faceBounds,
        GetTransform() * CTransform4f::Translate(mFaceTranslate) * mFaceDir,
        CTransform4f::Identity(), true, GetFrustumPlanes(), rstl::optional_object_null(),
        kInvalidUniqueId, nullptr, 0, 0, CVector3f(0.f, 0.f, 0.f));
  }
  CActor::Render(mgr);
}

EWeaponCollisionResponseTypes
CScriptDamageableTrigger::GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                   const CWeaponMode& weapon, int) const {
  return mDVuln.WeaponHurts(weapon, CDamageVulnerability::kRD_No) ? kWCR_OtherProjectile
                                                                      : kWCR_Unknown15;
}

ENTITY_ACCEPT_IMPL(CScriptDamageableTrigger)
