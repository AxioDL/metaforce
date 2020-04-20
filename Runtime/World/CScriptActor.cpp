#include "Runtime/World/CScriptActor.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/CPlayerState.hpp"
#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/Character/IAnimReader.hpp"
#include "Runtime/MP1/World/CActorContraption.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CDamageVulnerability.hpp"
#include "Runtime/World/CScriptColorModulate.hpp"
#include "Runtime/World/CScriptTrigger.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

#include <zeus/CEulerAngles.hpp>

namespace urde {

CScriptActor::CScriptActor(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                           CModelData&& mData, const zeus::CAABox& aabb, float mass, float zMomentum,
                           const CMaterialList& matList, const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                           const CActorParameters& actParms, bool looping, bool active, s32 shaderIdx, float xrayAlpha,
                           bool noThermalHotZ, bool castsShadow, bool scaleAdvancementDelta, bool materialFlag54)
: CPhysicsActor(uid, active, name, info, xf, std::move(mData), matList, aabb, SMoverData(mass), actParms, 0.3f, 0.1f)
, x258_initialHealth(hInfo)
, x260_currentHealth(hInfo)
, x268_damageVulnerability(dVuln)
, x2d0_fadeInTime(actParms.x5c_fadeInTime)
, x2d4_fadeOutTime(actParms.x60_fadeOutTime)
, x2d8_shaderIdx(shaderIdx)
, x2dc_xrayAlpha(xrayAlpha) {
  x2e2_24_noThermalHotZ = noThermalHotZ;
  x2e2_27_xrayAlphaEnabled = !zeus::close_enough(1.f, xrayAlpha);
  x2e2_29_processModelFlags = (x2e2_27_xrayAlphaEnabled || x2e2_24_noThermalHotZ || x2d8_shaderIdx != 0);
  x2e2_30_scaleAdvancementDelta = scaleAdvancementDelta;
  x2e2_31_materialFlag54 = materialFlag54;

  if (x64_modelData && (x64_modelData->HasAnimData() || x64_modelData->HasNormalModel()) && castsShadow)
    CreateShadow(true);

  if (x64_modelData && x64_modelData->HasAnimData())
    x64_modelData->EnableLooping(looping);

  x150_momentum = zeus::CVector3f(0.f, 0.f, zMomentum);
}

void CScriptActor::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptActor::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::InitializedInArea:
    for (const SConnection& conn : x20_conns) {
      if (conn.x0_state != EScriptObjectState::InheritBounds || conn.x4_msg != EScriptObjectMessage::Activate)
        continue;

      auto search = mgr.GetIdListForScript(conn.x8_objId);
      for (auto it = search.first; it != search.second; ++it) {
        if (TCastToConstPtr<CScriptTrigger>(mgr.GetObjectById(it->second))) {
          x2e0_triggerId = it->second;
          break;
        }
      }
    }

    if (x2e2_31_materialFlag54)
      CActor::AddMaterial(EMaterialTypes::Unknown54, mgr);
    break;
  case EScriptObjectMessage::Reset:
    x2e2_25_dead = false;
    x260_currentHealth = x258_initialHealth;
    break;
  case EScriptObjectMessage::Increment:
    if (!GetActive()) {
      mgr.SendScriptMsg(this, x8_uid, EScriptObjectMessage::Activate);
      CScriptColorModulate::FadeInHelper(mgr, x8_uid, x2d0_fadeInTime);
    }
    break;
  case EScriptObjectMessage::Decrement:
    CScriptColorModulate::FadeOutHelper(mgr, x8_uid, x2d4_fadeOutTime);
    break;
  default:
    break;
  }

  CActor::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptActor::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  if (HasModelData() && x64_modelData->HasAnimData()) {
    bool timeRemaining = x64_modelData->GetAnimationData()->IsAnimTimeRemaining(dt - FLT_EPSILON, "Whole Body");
    bool loop = x64_modelData->GetIsLoop();

    SAdvancementDeltas deltas = CActor::UpdateAnimation(dt, mgr, true);

    if (timeRemaining || loop) {
      x2e2_26_animating = true;

      if (x2e2_30_scaleAdvancementDelta)
        MoveToOR(x34_transform.rotate(x64_modelData->GetScale() * x34_transform.transposeRotate(deltas.x0_posDelta)),
                 dt);
      else
        MoveToOR(deltas.x0_posDelta, dt);

      //if (TCastToPtr<MP1::CActorContraption>(this))
        //printf("DEL %f\n", zeus::radToDeg(zeus::CEulerAngles(deltas.xc_rotDelta).z()));
        //printf("DEL %f %f %f %f\n", float(deltas.xc_rotDelta[0]), float(deltas.xc_rotDelta[1]), float(deltas.xc_rotDelta[2]), float(deltas.xc_rotDelta[3]));
      RotateToOR(deltas.xc_rotDelta, dt);
    }

    if (!timeRemaining && x2e2_26_animating && !loop) {
      SendScriptMsgs(EScriptObjectState::MaxReached, mgr, EScriptObjectMessage::None);
      x2e2_26_animating = false;
    }
  }

  if (!x2e2_25_dead && HealthInfo(mgr)->GetHP() <= 0.f) {
    x2e2_25_dead = true;
    SendScriptMsgs(EScriptObjectState::Dead, mgr, EScriptObjectMessage::None);
  }
}

void CScriptActor::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  CActor::PreRender(mgr, frustum);

  if (xe4_30_outOfFrustum && TCastToPtr<CCinematicCamera>(mgr.GetCameraManager()->GetCurrentCamera(mgr)))
    xe4_30_outOfFrustum = false;

  if (!xe4_30_outOfFrustum && x2e2_29_processModelFlags) {
    if (x2e2_27_xrayAlphaEnabled) {
      zeus::CColor col(1.f, x2dc_xrayAlpha);
      CModelFlags xrayFlags(5, 0, 3, col);
      if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay) {
        xb4_drawFlags = xrayFlags;
        x2e2_28_inXrayAlpha = true;
      } else if (x2e2_28_inXrayAlpha) {
        x2e2_28_inXrayAlpha = false;
        if (xb4_drawFlags == xrayFlags)
          xb4_drawFlags = CModelFlags(0, 0, 3, zeus::skWhite);
      }
    }

    if (x2e2_24_noThermalHotZ && xe6_27_thermalVisorFlags == 2) {
      if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::Thermal)
        xb4_drawFlags.x2_flags &= ~3; // Disable Z test/update
      else
        xb4_drawFlags.x2_flags |= 3; // Enable Z test/update
    }

    if (x2d8_shaderIdx != 0)
      xb4_drawFlags.x1_matSetIdx = u8(x2d8_shaderIdx);
  }

  if (mgr.GetObjectById(x2e0_triggerId) == nullptr)
    x2e0_triggerId = kInvalidUniqueId;
}

zeus::CAABox CScriptActor::GetSortingBounds(const CStateManager& mgr) const {
  if (x2e0_triggerId != kInvalidUniqueId) {
    TCastToConstPtr<CScriptTrigger> trigger(mgr.GetObjectById(x2e0_triggerId));
    if (trigger)
      return trigger->GetTriggerBoundsWR();
  }

  return CActor::GetSortingBounds(mgr);
}

EWeaponCollisionResponseTypes CScriptActor::GetCollisionResponseType(const zeus::CVector3f& v1,
                                                                     const zeus::CVector3f& v2,
                                                                     const CWeaponMode& wMode,
                                                                     EProjectileAttrib w) const {
  const CDamageVulnerability* dVuln = GetDamageVulnerability();
  if (dVuln->GetVulnerability(wMode, false) == EVulnerability::Deflect) {
    EDeflectType deflectType = dVuln->GetDeflectionType(wMode);
    if (deflectType < EDeflectType::Four && deflectType >= EDeflectType::One)
      return EWeaponCollisionResponseTypes::Unknown15;
  }
  return CActor::GetCollisionResponseType(v1, v2, wMode, w);
}

std::optional<zeus::CAABox> CScriptActor::GetTouchBounds() const {
  if (GetActive() && x68_material.HasMaterial(EMaterialTypes::Solid)) {
    return {CPhysicsActor::GetBoundingBox()};
  }
  return std::nullopt;
}

void CScriptActor::Touch(CActor&, CStateManager&) {
  // Empty
}
} // namespace urde
