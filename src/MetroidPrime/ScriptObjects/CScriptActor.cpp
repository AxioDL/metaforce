#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptColorModulate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

CScriptActor::CScriptActor(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf, const CModelData& mData, const CAABox& aabb,
                           const CMaterialList& matList, float mass, float zMomentum,
                           const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                           const CActorParameters& actParms, bool looping, bool active,
                           uint shaderIdx, float xrayAlpha, const bool noThermalHotZ,
                           const bool castsShadow, const bool scaleAdvancementDelta,
                           const bool materialFlag54)
: CPhysicsActor(uid, active, name, info, xf, mData, matList, aabb, SMoverData(mass), actParms, 0.3f,
                0.1f)
, x258_initialHealth(hInfo)
, x260_currentHealth(hInfo)
, x268_damageVulnerability(dVuln)
, x2d0_fadeInTime(actParms.GetFadeInTime())
, x2d4_fadeOutTime(actParms.GetFadeOutTime())
, x2d8_shaderIdx(shaderIdx)
, x2dc_xrayAlpha(xrayAlpha)
, x2e0_triggerId(kInvalidUniqueId)
, x2e2_24_noThermalHotZ(noThermalHotZ)
, x2e2_25_dead(false)
, x2e2_26_animating(true)
, x2e2_27_xrayAlphaEnabled(!close_enough(xrayAlpha, 1.f))
, x2e2_28_inXrayAlpha(false)
, x2e2_29_processModelFlags(x2e2_27_xrayAlphaEnabled || x2e2_24_noThermalHotZ ||
                            x2d8_shaderIdx != 0)
, x2e2_30_scaleAdvancementDelta(scaleAdvancementDelta)
, x2e2_31_materialFlag54(materialFlag54)
, x2e3_24_isPlayerActor(false) {
  if (HasModelData()) {
    if (castsShadow) {
      SetDrawShadow(true);
    }

    if (HasAnimation()) {
      ModelData()->EnableLooping(looping);
    }
  }

  SetMomentumWR(CVector3f(0.f, 0.f, -zMomentum));
}

CScriptActor::~CScriptActor() {}

CHealthInfo* CScriptActor::HealthInfo(CStateManager&) { return &x260_currentHealth; }

const CDamageVulnerability* CScriptActor::GetDamageVulnerability() const {
  return &x268_damageVulnerability;
}

void CScriptActor::Touch(CActor&, CStateManager&) {}

rstl::optional_object< CAABox > CScriptActor::GetTouchBounds() const {
  if (GetActive() && GetMaterialList().HasMaterial(kMT_Solid)) {
    return CPhysicsActor::GetBoundingBox();
  }
  return rstl::optional_object_null();
}

void CScriptActor::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (HasAnimation()) {
    const bool timeRemaining =
        GetAnimationData()->IsAnimTimeRemaining(dt - FLT_EPSILON, rstl::string_l("Whole Body"));
    const bool loop = GetModelData()->GetIsLoop();

    CAdvancementDeltas deltas = CActor::UpdateAnimation(dt, mgr, true);

    if (timeRemaining || loop) {
      x2e2_26_animating = true;

      if (x2e2_30_scaleAdvancementDelta) {
        CVector3f pos = GetTransform().TransposeRotate(deltas.GetOffsetDelta());
        CVector3f scale = GetModelData()->GetScale();
        pos = CVector3f(scale.GetX() * pos.GetX(), scale.GetY() * pos.GetY(),
                        scale.GetZ() * pos.GetZ());
        pos = GetTransform().Rotate(pos);
        MoveToOR(pos, dt);
      } else {
        MoveToOR(deltas.GetOffsetDelta(), dt);
      }

      RotateToOR(deltas.GetOrientationDelta(), dt);
    }

    if (!timeRemaining && x2e2_26_animating && !loop) {
      SendScriptMsgs(kSS_MaxReached, mgr, kSM_None);
      x2e2_26_animating = false;
    }
  }

  if (!x2e2_25_dead && HealthInfo(mgr)->GetHP() <= 0.f) {
    x2e2_25_dead = true;
    SendScriptMsgs(kSS_Dead, mgr, kSM_None);
  }
}

void CScriptActor::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_InitializedInArea: {
    rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
    for (; conn != GetConnectionList().end(); ++conn) {
      if (conn->x0_state != kSS_InheritBounds || conn->x4_msg != kSM_Activate) {
        continue;
      }

      CStateManager::TIdListResult search = mgr.GetIdListForScript(conn->x8_objId);
      CStateManager::TIdList::const_iterator current = search.first;
      CStateManager::TIdList::const_iterator end = search.second;
      while (current != end) {
        if (TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(current->second))) {
          x2e0_triggerId = current->second;
        }
        current++;
      }
    }

    if (x2e2_31_materialFlag54) {
      CActor::AddMaterial(kMT_Unknown54, mgr);
    }
    break;
  }
  case kSM_Reset: {
    x2e2_25_dead = false;
    x260_currentHealth = x258_initialHealth;
    break;
  }
  case kSM_Increment: {
    if (!GetActive()) {
      mgr.DeliverScriptMsg(this, GetUniqueId(), kSM_Activate);
      CScriptColorModulate::FadeInHelper(mgr, GetUniqueId(), x2d0_fadeInTime);
    }
    break;
  }
  case kSM_Decrement: {
    CScriptColorModulate::FadeOutHelper(mgr, GetUniqueId(), x2d4_fadeOutTime);
    break;
  }
  default:
    break;
  }

  CActor::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptActor::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  CActor::PreRender(mgr, frustum);

  if (GetPreRenderClipped() &&
      TCastToConstPtr< CCinematicCamera >(mgr.GetCameraManager()->GetCurrentCamera(mgr))) {
    SetPreRenderClipped(false);
  }

  if (!GetPreRenderClipped() && x2e2_29_processModelFlags) {
    if (x2e2_27_xrayAlphaEnabled) {
      CModelFlags xrayFlags = CModelFlags::AlphaBlended(x2dc_xrayAlpha);
      if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay) {
        SetModelFlags(xrayFlags);
        x2e2_28_inXrayAlpha = true;
      } else if (x2e2_28_inXrayAlpha) {
        x2e2_28_inXrayAlpha = false;
        if (GetModelFlags() == xrayFlags) {
          SetModelFlags(CModelFlags::Normal());
        }
      }
    }

    if (x2e2_24_noThermalHotZ && GetThermalFlags() == kTF_Hot) {
      if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Thermal) {
        SetModelFlags(GetModelFlags().DepthCompareUpdate(false, false));
      } else {
        SetModelFlags(GetModelFlags().DepthCompareUpdate(true, true));
      }
    }

    if (x2d8_shaderIdx != 0) {
      SetModelFlags(GetModelFlags().UseShaderSet(x2d8_shaderIdx));
    }
  }

  if (mgr.GetObjectById(x2e0_triggerId) == nullptr) {
    x2e0_triggerId = kInvalidUniqueId;
  }
}

ENTITY_ACCEPT_IMPL(CScriptActor)

EWeaponCollisionResponseTypes CScriptActor::GetCollisionResponseType(const CVector3f& v1,
                                                                     const CVector3f& v2,
                                                                     const CWeaponMode& wMode,
                                                                     int w) const {

  const CDamageVulnerability* dVuln = GetDamageVulnerability();
  if (dVuln->GetVulnerability(wMode, CDamageVulnerability::kRD_No) == kVN_Deflect) {
    const EDeflectionType deflectType = dVuln->GetDeflectionType(wMode);
    switch (deflectType) {
    case kDT_Ricochet:
    case kDT_RetargetPlayer:
    case kDT_RetargetPlayerCombo:
      return kWCR_Unknown15;
    default:
      break;
    }
  }
  return CActor::GetCollisionResponseType(v1, v2, wMode, w);
}

CAABox CScriptActor::GetSortingBounds(const CStateManager& mgr) const {
  if (x2e0_triggerId != kInvalidUniqueId) {
    const CScriptTrigger* trigger =
        static_cast< const CScriptTrigger* >(mgr.GetObjectById(x2e0_triggerId));
    if (trigger) {
      return trigger->GetTriggerBoundsWR();
    }
  }

  return CActor::GetSortingBounds(mgr);
}
