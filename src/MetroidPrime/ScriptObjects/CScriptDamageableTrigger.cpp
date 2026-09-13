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
, x14c_bounds(CVector3f(-(extent.GetX() * 0.5f), -(extent.GetY() * 0.5f), -(extent.GetZ() * 0.5f)),
              CVector3f(extent.GetX() * 0.5f, extent.GetY() * 0.5f, extent.GetZ() * 0.5f))
, x164_origHInfo(hInfo)
, x16c_hInfo(hInfo)
, x174_dVuln(dVuln)
, x1dc_faceFlag(faceFlag)
, x1e0_alpha(1.f)
, x1e4_faceDir(CTransform4f::Identity())
, x214_faceDirInv(CTransform4f::Identity())
, x244_faceTranslate(0.f, 0.f, 0.f)
, x250_alphaTimer(0.f)
, x254_fluidPlane(patternTex1, patternTex2, colorTex, 1.f, 2, CFluidPlane::kFT_NormalWater, 1.f,
                  CFluidUVMotion(6.f, 0.f))
, x300_24_notOccluded(false)
, x300_25_alphaOut(false)
, x300_26_outOfFrustum(false)
, x300_27_invulnerable(false)
, x300_28_canOrbit(canOrbit == kCO_Orbit) {
  if (x1dc_faceFlag & 1) {
    x244_faceTranslate = CVector3f(0.f, x14c_bounds.GetMaxPoint().GetY(), 0.f);
    x1e4_faceDir = CTransform4f::RotateX(CRelAngle::FromDegrees(-90.f));
  } else if (x1dc_faceFlag & 2) {
    x244_faceTranslate = CVector3f(0.f, x14c_bounds.GetMinPoint().GetY(), 0.f);
    x1e4_faceDir = CTransform4f::RotateX(CRelAngle::FromDegrees(90.f));
  } else if (x1dc_faceFlag & 4) {
    x244_faceTranslate = CVector3f(x14c_bounds.GetMinPoint().GetX(), 0.f, 0.f);
    x1e4_faceDir = CTransform4f::RotateY(CRelAngle::FromDegrees(-90.f));
  } else if (x1dc_faceFlag & 8) {
    x244_faceTranslate = CVector3f(x14c_bounds.GetMaxPoint().GetX(), 0.f, 0.f);
    x1e4_faceDir = CTransform4f::RotateY(CRelAngle::FromDegrees(90.f));
  } else if (x1dc_faceFlag & 0x10) {
    x244_faceTranslate = CVector3f(0.f, 0.f, x14c_bounds.GetMaxPoint().GetZ());
    x1e4_faceDir = CTransform4f::Identity();
  } else if (x1dc_faceFlag & 0x20) {
    x244_faceTranslate = CVector3f(0.f, 0.f, x14c_bounds.GetMinPoint().GetZ());
    x1e4_faceDir = CTransform4f::RotateY(CRelAngle::FromDegrees(180.f));
  }
  x214_faceDirInv = x1e4_faceDir.GetQuickInverse();
}

CScriptDamageableTrigger::~CScriptDamageableTrigger() {}

rstl::optional_object< CAABox > CScriptDamageableTrigger::GetTouchBounds() const {
  if (GetActive() && x300_24_notOccluded) {
    return CAABox(x14c_bounds.GetMinPoint() + GetTranslation(),
                  x14c_bounds.GetMaxPoint() + GetTranslation());
  }
  return rstl::optional_object_null();
}

void CScriptDamageableTrigger::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender,
                                               CStateManager& mgr) {
  switch (msg) {
  case kSM_Deactivate:
    if (GetActive() && x300_25_alphaOut) {
      return;
    }
    // Fall through.
  case kSM_Activate:
    if (!GetActive() || x300_25_alphaOut) {
      x250_alphaTimer = 0.f;
      x16c_hInfo = x164_origHInfo;
      x300_25_alphaOut = false;
      if (x300_28_canOrbit) {
        AddMaterial(kMT_Orbit, mgr);
      }
      SetLinkedObjectAlpha(0.f, mgr);
      x1e0_alpha = 0.f;
    }
    break;
  case kSM_Damage:
    if (x300_27_invulnerable) {
      x16c_hInfo = x164_origHInfo;
    }
    break;
  case kSM_Increment:
    x300_27_invulnerable = true;
    break;
  case kSM_Decrement:
    x300_27_invulnerable = false;
    break;
  default:
    break;
  }
  CActor::AcceptScriptMsg(msg, sender, mgr);
}

float CScriptDamageableTrigger::GetPuddleAlphaScale() const {
  if (x250_alphaTimer <= 0.75f) {
    if (x300_25_alphaOut) {
      return 1.f - x250_alphaTimer / 0.75f;
    }
    return x250_alphaTimer / 0.75f;
  }
  if (x300_25_alphaOut) {
    return 0.f;
  }
  return 1.f;
}

void CScriptDamageableTrigger::SetLinkedObjectAlpha(float alpha, CStateManager& mgr) {
  const rstl::vector< SConnection >& connections = GetConnectionList();
  for (AUTO(it, connections.begin()); it != connections.end(); ++it) {
    if (it->x0_state == kSS_MaxReached && it->x4_msg == kSM_Activate) {
      if (CScriptActor* actor =
              TCastToPtr< CScriptActor >(mgr.ObjectById(mgr.GetIdForScript(it->x8_objId)))) {
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
  x300_24_notOccluded = area.GetOcclusionState() == CGameArea::kOS_Visible;

  if (x300_25_alphaOut) {
    if (x250_alphaTimer >= 0.75f) {
      SetActive(false);
      const rstl::vector< SConnection >& connections = GetConnectionList();
      for (AUTO(it, connections.begin()); it != connections.end(); ++it) {
        if (it->x0_state == kSS_MaxReached && it->x4_msg == kSM_Activate) {
          if (CScriptActor* actor =
                  TCastToPtr< CScriptActor >(mgr.ObjectById(mgr.GetIdForScript(it->x8_objId)))) {
            actor->SetActive(false);
          }
        }
      }
      SetLinkedObjectAlpha(0.f, mgr);
      x300_25_alphaOut = false;
      return;
    }
  } else if (x16c_hInfo.GetHP() <= 0.f && GetActive()) {
    SendScriptMsgs(kSS_Dead, mgr, kSM_None);
    RemoveMaterial(kMT_Orbit, mgr);
    x300_25_alphaOut = true;
    x250_alphaTimer = 0.f;
  }

  if (x250_alphaTimer <= 0.75f) {
    x250_alphaTimer += dt;
  }
  const float alpha = GetPuddleAlphaScale();
  x1e0_alpha = 0.2f * alpha;
  SetLinkedObjectAlpha(alpha, mgr);
}

CHealthInfo* CScriptDamageableTrigger::HealthInfo(CStateManager&) { return &x16c_hInfo; }

const CDamageVulnerability* CScriptDamageableTrigger::GetDamageVulnerability() const {
  return &x174_dVuln;
}

void CScriptDamageableTrigger::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  const CAABox bounds = x14c_bounds;
  x300_26_outOfFrustum = !frustum.BoxInFrustumPlanes(bounds.GetTransformedAABox(GetTransform()));
  if (x300_26_outOfFrustum) {
    return;
  }
  xe8_frustum = frustum;
  CActor::PreRender(mgr, frustum);
}

void CScriptDamageableTrigger::AddToRenderer(const CFrustumPlanes&,
                                             const CStateManager& mgr) const {
  if (x300_26_outOfFrustum) {
    return;
  }
  EnsureRendered(mgr, GetTranslation() - x244_faceTranslate, GetSortingBounds(mgr));
}

void CScriptDamageableTrigger::Render(const CStateManager& mgr) const {
  if (GetActive() && x1dc_faceFlag != 0 && !close_enough(x1e0_alpha, 0.f)) {
    const CAABox bounds(
        CVector3f(x14c_bounds.GetMinPoint().GetX(), x14c_bounds.GetMinPoint().GetY(),
                  x14c_bounds.GetMinPoint().GetZ()),
        CVector3f(x14c_bounds.GetMaxPoint().GetX(), x14c_bounds.GetMaxPoint().GetY(),
                  x14c_bounds.GetMaxPoint().GetZ()));
    const CAABox faceBounds = bounds.GetTransformedAABox(x214_faceDirInv);
    GetFluidPlane().Render(
        mgr, x1e0_alpha, faceBounds,
        GetTransform() * CTransform4f::Translate(x244_faceTranslate) * x1e4_faceDir,
        CTransform4f::Identity(), true, GetFrustumPlanes(), rstl::optional_object_null(),
        kInvalidUniqueId, nullptr, 0, 0, CVector3f(0.f, 0.f, 0.f));
  }
  CActor::Render(mgr);
}

EWeaponCollisionResponseTypes
CScriptDamageableTrigger::GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                   const CWeaponMode& weapon, int) const {
  return x174_dVuln.WeaponHurts(weapon, CDamageVulnerability::kRD_No) ? kWCR_OtherProjectile
                                                                      : kWCR_Unknown15;
}

ENTITY_ACCEPT_IMPL(CScriptDamageableTrigger)
