#include "Runtime/World/CScriptDamageableTrigger.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CScriptActor.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
CActorParameters MakeDamageableTriggerActorParms(const CActorParameters& aParams, const CVisorParameters& vParams) {
  CActorParameters ret = aParams;
  ret.SetVisorParameters(vParams);
  return ret;
}

CMaterialList MakeDamageableTriggerMaterial(CScriptDamageableTrigger::ECanOrbit canOrbit) {
  if (canOrbit == CScriptDamageableTrigger::ECanOrbit::Orbit)
    return CMaterialList(EMaterialTypes::Orbit, EMaterialTypes::Trigger, EMaterialTypes::Immovable,
                         EMaterialTypes::NonSolidDamageable, EMaterialTypes::ExcludeFromLineOfSightTest);
  return CMaterialList(EMaterialTypes::Trigger, EMaterialTypes::Immovable, EMaterialTypes::NonSolidDamageable,
                       EMaterialTypes::ExcludeFromLineOfSightTest);
}

CScriptDamageableTrigger::CScriptDamageableTrigger(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                                   const zeus::CVector3f& position, const zeus::CVector3f& extent,
                                                   const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                                                   u32 faceFlag, CAssetId patternTex1, CAssetId patternTex2,
                                                   CAssetId colorTex, ECanOrbit canOrbit, bool active,
                                                   const CVisorParameters& vParams)
: CActor(uid, active, name, info, zeus::CTransform::Translate(position), CModelData::CModelDataNull(),
         MakeDamageableTriggerMaterial(canOrbit), MakeDamageableTriggerActorParms(CActorParameters::None(), vParams),
         kInvalidUniqueId)
, x14c_bounds(-extent * 0.5f, extent * 0.5f)
, x164_origHInfo(hInfo)
, x16c_hInfo(hInfo)
, x174_dVuln(dVuln)
, x1dc_faceFlag(faceFlag)
, x254_fluidPlane(patternTex1, patternTex2, colorTex, 1.f, 2, EFluidType::NormalWater, 1.f, CFluidUVMotion(6.f, 0.f))
, x300_28_canOrbit(canOrbit == ECanOrbit::Orbit) {
  if (x1dc_faceFlag & 0x1) {
    x244_faceTranslate = zeus::CVector3f(0.f, x14c_bounds.max.y(), 0.f);
    x1e4_faceDir = zeus::CTransform::RotateX(-M_PIF / 2.f);
  } else if (x1dc_faceFlag & 0x2) {
    x244_faceTranslate = zeus::CVector3f(0.f, x14c_bounds.min.y(), 0.f);
    x1e4_faceDir = zeus::CTransform::RotateX(M_PIF / 2.f);
  } else if (x1dc_faceFlag & 0x4) {
    x244_faceTranslate = zeus::CVector3f(x14c_bounds.min.x(), 0.f, 0.f);
    x1e4_faceDir = zeus::CTransform::RotateY(-M_PIF / 2.f);
  } else if (x1dc_faceFlag & 0x8) {
    x244_faceTranslate = zeus::CVector3f(x14c_bounds.max.x(), 0.f, 0.f);
    x1e4_faceDir = zeus::CTransform::RotateY(M_PIF / 2.f);
  } else if (x1dc_faceFlag & 0x10) {
    x244_faceTranslate = zeus::CVector3f(0.f, 0.f, x14c_bounds.max.z());
    x1e4_faceDir = zeus::CTransform();
  } else if (x1dc_faceFlag & 0x20) {
    x244_faceTranslate = zeus::CVector3f(0.f, 0.f, x14c_bounds.min.z());
    x1e4_faceDir = zeus::CTransform::RotateY(M_PIF);
  }

  x214_faceDirInv = x1e4_faceDir.inverse();
}

void CScriptDamageableTrigger::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptDamageableTrigger::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::Deactivate:
    if (x30_24_active && x300_25_alphaOut)
      return;
    [[fallthrough]];
  case EScriptObjectMessage::Activate:
    if (!x30_24_active || x300_25_alphaOut) {
      x250_alphaTimer = 0.f;
      x16c_hInfo = x164_origHInfo;
      x300_25_alphaOut = false;
      if (x300_28_canOrbit)
        AddMaterial(EMaterialTypes::Orbit, mgr);
      SetLinkedObjectAlpha(0.f, mgr);
      x1e0_alpha = 0.f;
    }
    break;
  case EScriptObjectMessage::Damage:
    if (x300_27_invulnerable)
      x16c_hInfo = x164_origHInfo;
    break;
  case EScriptObjectMessage::Increment:
    x300_27_invulnerable = true;
    break;
  case EScriptObjectMessage::Decrement:
    x300_27_invulnerable = false;
    break;
  default:
    break;
  }

  CActor::AcceptScriptMsg(msg, sender, mgr);
}

EWeaponCollisionResponseTypes CScriptDamageableTrigger::GetCollisionResponseType(const zeus::CVector3f&,
                                                                                 const zeus::CVector3f&,
                                                                                 const CWeaponMode& weapMode,
                                                                                 EProjectileAttrib) const {
  return x174_dVuln.WeaponHurts(weapMode, false) ? EWeaponCollisionResponseTypes::OtherProjectile
                                                 : EWeaponCollisionResponseTypes::Unknown15;
}

void CScriptDamageableTrigger::Render(CStateManager& mgr) {
  if (x30_24_active && x1dc_faceFlag != 0 && std::fabs(x1e0_alpha) >= 0.00001f) {
    zeus::CAABox aabb = x14c_bounds.getTransformedAABox(x214_faceDirInv);
    zeus::CTransform xf = x34_transform * zeus::CTransform::Translate(x244_faceTranslate) * x1e4_faceDir;
    x254_fluidPlane.Render(mgr, x1e0_alpha, aabb, xf, zeus::CTransform(), false, xe8_frustum, {},
                           kInvalidUniqueId, nullptr, 0, 0, zeus::skZero3f);
  }

  CActor::Render(mgr);
}

void CScriptDamageableTrigger::AddToRenderer(const zeus::CFrustum& /*frustum*/, CStateManager& mgr) {
  if (x300_26_outOfFrustum) {
    return;
  }

  EnsureRendered(mgr, GetTranslation() - x244_faceTranslate, GetSortingBounds(mgr));
}

void CScriptDamageableTrigger::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  x300_26_outOfFrustum = !frustum.aabbFrustumTest(x14c_bounds.getTransformedAABox(x34_transform));
  if (!x300_26_outOfFrustum) {
    xe8_frustum = frustum;
    CActor::PreRender(mgr, frustum);
  }
}

void CScriptDamageableTrigger::SetLinkedObjectAlpha(float a, CStateManager& mgr) {
  for (const SConnection& conn : x20_conns) {
    if (conn.x0_state != EScriptObjectState::MaxReached || conn.x4_msg != EScriptObjectMessage::Activate)
      continue;
    if (TCastToPtr<CScriptActor> act = mgr.ObjectById(mgr.GetIdForScript(conn.x8_objId))) {
      if (!act->GetActive())
        act->SetActive(true);
      act->SetDrawFlags(CModelFlags(5, 0, 3, zeus::CColor(1.f, a)));
    }
  }
}

float CScriptDamageableTrigger::GetPuddleAlphaScale() const {
  if (x250_alphaTimer <= 0.75f) {
    if (x300_25_alphaOut)
      return 1.f - x250_alphaTimer / 0.75f;
    return x250_alphaTimer / 0.75f;
  }

  if (x300_25_alphaOut)
    return 0.f;
  return 1.f;
}

void CScriptDamageableTrigger::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  const CGameArea* area = mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways());
  CGameArea::EOcclusionState occState =
      area->IsPostConstructed() ? area->GetOcclusionState() : CGameArea::EOcclusionState::Occluded;
  x300_24_notOccluded = occState == CGameArea::EOcclusionState::Visible;

  if (x300_25_alphaOut) {
    if (x250_alphaTimer >= 0.75f) {
      SetActive(false);
      for (const SConnection& conn : x20_conns) {
        if (conn.x0_state != EScriptObjectState::MaxReached || conn.x4_msg != EScriptObjectMessage::Activate)
          continue;
        if (TCastToPtr<CScriptActor> act = mgr.ObjectById(mgr.GetIdForScript(conn.x8_objId)))
          act->SetActive(false);
      }

      SetLinkedObjectAlpha(0.f, mgr);
      x300_25_alphaOut = false;
      return;
    }
  } else if (x16c_hInfo.GetHP() <= 0.f && x30_24_active) {
    SendScriptMsgs(EScriptObjectState::Dead, mgr, EScriptObjectMessage::None);
    RemoveMaterial(EMaterialTypes::Orbit, mgr);
    x300_25_alphaOut = true;
    x250_alphaTimer = 0.f;
  }

  if (x250_alphaTimer <= 0.75f)
    x250_alphaTimer += dt;

  float objAlpha = GetPuddleAlphaScale();
  x1e0_alpha = 0.2f * objAlpha;
  SetLinkedObjectAlpha(objAlpha, mgr);
}

std::optional<zeus::CAABox> CScriptDamageableTrigger::GetTouchBounds() const {
  if (x30_24_active && x300_24_notOccluded) {
    return zeus::CAABox(x14c_bounds.min + GetTranslation(), x14c_bounds.max + GetTranslation());
  }
  return std::nullopt;
}

} // namespace urde
