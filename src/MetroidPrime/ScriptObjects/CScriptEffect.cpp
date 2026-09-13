#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "Collision/CMaterialList.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Particles/CElectricDescription.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "rstl/math.hpp"
uint CScriptEffect::mNumParticlesDrawing = 0;
uint CScriptEffect::mNumParticlesUpdating = 0;

static inline CTransform4f ClearTrans(const CTransform4f& xf) {
  CTransform4f ret = xf;
  ret.SetTranslation(CVector3f::Zero());
  return ret;
}

CScriptEffect::CScriptEffect(const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const CTransform4f& xf, const CVector3f& scale, const CAssetId partId,
                             const CAssetId elscId, const bool hotInThermal,
                             const bool noTimerUnlessAreaOccluded, bool rebuildSystemsOnActivate,
                             const bool active, const bool useRateInverseCamDist,
                             float rateInverseCamDist, const float rateInverseCamDistRate,
                             const float duration, const float durationResetWhileVisible,
                             const bool useRateCamDistRange, const float rateCamDistRangeMin,
                             const float rateCamDistRangeMax, const float rateCamDistRangeFarRate,
                             const bool combatVisorVisible, const bool thermalVisorVisible,
                             const bool xrayVisorVisible, const CLightParameters& lParms,
                             const bool dieWhenSystemsDone)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None().HotInThermal(hotInThermal), kInvalidUniqueId)
, xe8_electricToken(nullptr)
, xf8_particleSystemToken(nullptr)
, x108_lightId(kInvalidUniqueId)
, x10c_partId(partId)
, x110_24_enable(active)
, x110_25_noTimerUnlessAreaOccluded(noTimerUnlessAreaOccluded)
, x110_26_rebuildSystemsOnActivate(rebuildSystemsOnActivate)
, x110_27_useRateInverseCamDist(useRateInverseCamDist)
, x110_28_combatVisorVisible(combatVisorVisible)
, x110_29_thermalVisorVisible(thermalVisorVisible)
, x110_30_xrayVisorVisible(xrayVisorVisible)
, x110_31_anyVisorVisible(xrayVisorVisible && thermalVisorVisible && combatVisorVisible)
, x111_24_useRateCamDistRange(useRateCamDistRange)
, x111_25_dieWhenSystemsDone(dieWhenSystemsDone)
, x111_26_canRender(false)
, x114_rateInverseCamDist(rateInverseCamDist)
, x118_rateInverseCamDistSq(rateInverseCamDist * rateInverseCamDist)
, x11c_rateInverseCamDistRate(rateInverseCamDistRate)
, x120_rateCamDistRangeMin(rateCamDistRangeMin)
, x124_rateCamDistRangeMax(rateCamDistRangeMax)
, x128_rateCamDistRangeFarRate(rateCamDistRangeFarRate)
, x12c_remTime(duration)
, x130_duration(duration)
, x134_durationResetWhileVisible(durationResetWhileVisible)
, x138_actorLights(lParms.MakeActorLights().release())
, x13c_triggerId(kInvalidUniqueId)
, x140_destroyDelayTimer(0.f) {
  if (partId != kInvalidAssetId) {
    xf8_particleSystemToken = gpSimplePool->GetObj(SObjectTag('PART', partId));
    x104_particleSystem = rs_new CElementGen(xf8_particleSystemToken);
    x104_particleSystem->SetOrientation(ClearTrans(xf));
    x104_particleSystem->SetGlobalTranslation(xf.GetTranslation());
    x104_particleSystem->SetGlobalScale(scale);
    x104_particleSystem->SetParticleEmission(active);
    x104_particleSystem->SetModulationColor(lParms.GetAmbientColor());
    x104_particleSystem->SetLeaveLightsEnabledForModelRender(x138_actorLights.get() != nullptr);
  }
  if (elscId != kInvalidAssetId) {
    xe8_electricToken = gpSimplePool->GetObj(SObjectTag('ELSC', elscId));
    xf4_electric = rs_new CParticleElectric(xe8_electricToken);
    xf4_electric->SetOrientation(ClearTrans(xf));
    xf4_electric->SetGlobalTranslation(xf.GetTranslation());
    xf4_electric->SetGlobalScale(scale);
    xf4_electric->SetParticleEmission(active);
    xf4_electric->SetModulationColor(lParms.GetAmbientColor());
  }
  SetDrawEnabled(true);
}

void CScriptEffect::Think(float dt, CStateManager& mgr) {
  if (GetTransformDirtySpare()) {
    if (x104_particleSystem.get()) {
      x104_particleSystem->SetOrientation(ClearTrans(GetTransform()));
      x104_particleSystem->SetGlobalTranslation(GetTranslation());
    }
    if (xf4_electric.get()) {
      xf4_electric->SetOrientation(ClearTrans(GetTransform()));
      xf4_electric->SetGlobalTranslation(GetTranslation());
    }
    if (CActor* light = TCastToPtr< CActor >(mgr.ObjectById(x108_lightId))) {
      light->SetTransform(GetTransform());
    }
    SetTransformDirtySpare(false);
  }
  if (!x110_25_noTimerUnlessAreaOccluded) {
    if (x12c_remTime <= 0.f) {
      return;
    }
  } else {
    if (mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()).GetOcclusionState() ==
            CGameArea::kOS_Occluded &&
        x12c_remTime <= 0.f) {
      return;
    }
  }
  x12c_remTime -= dt;
  if (x110_24_enable) {
    if (x104_particleSystem.get()) {
      x104_particleSystem->Update(dt);
      mNumParticlesUpdating += x104_particleSystem->GetParticleCountAll();
    }
    if (xf4_electric.get()) {
      xf4_electric->Update(dt);
      mNumParticlesUpdating += xf4_electric->GetParticleCount();
    }
    if (x108_lightId != kInvalidUniqueId) {
      if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(x108_lightId))) {
        if (GetActive()) {
          light->SetLight(x104_particleSystem->GetLight());
        }
      }
    }
    if (x111_25_dieWhenSystemsDone) {
      x140_destroyDelayTimer += dt;
      if (x140_destroyDelayTimer > 15.f) {
        mgr.DeleteObjectRequest(GetUniqueId());
        return;
      }
      if (AreBothSystemsDeleteable()) {
        mgr.DeleteObjectRequest(GetUniqueId());
        return;
      }
    }
  }
  if (x104_particleSystem.get()) {
    if (GetModelFlags().GetTrans() != 0) {
      x104_particleSystem->SetModulationColor(GetModelFlags().GetColorRef());
    } else {
      x104_particleSystem->SetModulationColor(CColor(0xffffffff));
    }
  }
}

void CScriptEffect::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  bool oldActive = GetActive();
  switch (msg) {
  case kSM_InitializedInArea:
    for (AUTO(conn, GetConnectionList().begin()); conn != GetConnectionList().end(); ++conn) {
      if ((conn->x0_state == kSS_Modify && conn->x4_msg == kSM_Follow) ||
          (conn->x0_state == kSS_InheritBounds && conn->x4_msg == kSM_Activate)) {
        CStateManager::TIdListResult ids = mgr.GetIdListForScript(conn->x8_objId);
        for (AUTO(it, ids.first); it != ids.second; ++it) {
          if (TCastToConstPtr< CScriptTrigger >(mgr.GetObjectById(it->second))) {
            x13c_triggerId = it->second;
          }
        }
      }
    }
    break;
  case kSM_Registered:
    if (x104_particleSystem.get() && x104_particleSystem->SystemHasLight()) {
      x108_lightId = mgr.AllocateUniqueId();
      const CAssetId sourceId = x10c_partId;
      mgr.AddObject(rs_new CGameLight(x108_lightId, GetCurrentAreaId(), GetActive(),
                                      rstl::string_l("EffectPLight_") + GetDebugName(),
                                      GetTransform(), GetUniqueId(),
                                      x104_particleSystem->GetLight(), sourceId, 1, 0.f));
    }
    break;
  case kSM_Deleted:
    if (x108_lightId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(x108_lightId);
      x108_lightId = kInvalidUniqueId;
    }
    break;
  case kSM_Activate:
    if (x110_26_rebuildSystemsOnActivate) {
      if (x104_particleSystem.get()) {
        const CVector3f scale = x104_particleSystem->GetGlobalScale();
        const CColor color = x104_particleSystem->GetModulationColor();
        x104_particleSystem = rs_new CElementGen(xf8_particleSystemToken);
        x104_particleSystem->SetOrientation(ClearTrans(GetTransform()));
        x104_particleSystem->SetGlobalTranslation(GetTranslation());
        x104_particleSystem->SetGlobalScale(scale);
        x104_particleSystem->SetLeaveLightsEnabledForModelRender(x138_actorLights.get() != nullptr);
        x104_particleSystem->SetModulationColor(color);
      }
      if (xf4_electric.get()) {
        const CVector3f scale = xf4_electric->GetGlobalScale();
        const CColor color = xf4_electric->GetModulationColor();
        xf4_electric = rs_new CParticleElectric(xe8_electricToken);
        xf4_electric->SetOrientation(ClearTrans(GetTransform()));
        xf4_electric->SetGlobalTranslation(GetTranslation());
        xf4_electric->SetGlobalScale(scale);
        xf4_electric->SetModulationColor(color);
      }
    }
    break;
  default:
    break;
  }
  CActor::AcceptScriptMsg(msg, uid, mgr);
  CActor* light = TCastToPtr< CActor >(mgr.ObjectById(x108_lightId));
  mgr.DeliverScriptMsg(light, uid, msg);
  if (oldActive != GetActive()) {
    if (GetActive()) {
      rstl::vector< TUniqueId > playIds;
      playIds.reserve(GetConnectionList().size());
      for (AUTO(conn, GetConnectionList().begin()); conn != GetConnectionList().end(); ++conn) {
        if (conn->x0_state == kSS_Play && conn->x4_msg == kSM_Activate) {
          TUniqueId id = mgr.GetIdForScript(conn->x8_objId);
          if (id != kInvalidUniqueId) {
            playIds.push_back(id);
          }
        }
      }
      if (playIds.size() != 0) {
        if (const CActor* const other = TCastToConstPtr< CActor >(mgr.GetObjectById(
                playIds[static_cast< int >(0.99f * (mgr.Random()->Float() * playIds.size()))]))) {
          SetTransform(other->GetTransform());
          if (light) {
            light->SetTransform(other->GetTransform());
          }
        }
      }
    }
    x110_24_enable = true;
    if (x104_particleSystem.get()) {
      x104_particleSystem->SetParticleEmission(GetActive());
    }
    if (xf4_electric.get()) {
      xf4_electric->SetParticleEmission(GetActive());
    }
    if (GetActive()) {
      x12c_remTime = rstl::max_val(x130_duration, x12c_remTime);
    }
  }
}

void CScriptEffect::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  if (!x111_26_canRender) {
    x12c_remTime = rstl::max_val(x134_durationResetWhileVisible, x12c_remTime);
    return;
  }
  const CAABox& bounds = GetRenderBoundsCached();
  if (!frustum.BoxInFrustumPlanes(bounds)) {
    return;
  }
  x12c_remTime = rstl::max_val(x134_durationResetWhileVisible, x12c_remTime);
  bool visible = true;
  if (!x110_31_anyVisorVisible) {
    switch (mgr.GetPlayerState()->GetActiveVisor(mgr)) {
    case CPlayerState::kPV_Combat:
    case CPlayerState::kPV_Scan:
      visible = x110_28_combatVisorVisible;
      break;
    case CPlayerState::kPV_XRay:
      visible = x110_30_xrayVisorVisible;
      break;
    case CPlayerState::kPV_Thermal:
      visible = x110_29_thermalVisorVisible;
      break;
    }
  }
  if (visible) {
    if (x138_actorLights.get()) {
      const CVector3f center = bounds.GetCenterPoint();
      x138_actorLights->BuildAreaLightList(mgr, mgr.GetWorld()->GetAreaAlways(GetCurrentAreaId()),
                                           CAABox(center, center));
      x138_actorLights->BuildDynamicLightList(mgr, bounds);
    }
    EnsureRendered(mgr);
  }
}

void CScriptEffect::Render(const CStateManager& mgr) const {
  if (x138_actorLights.get()) {
    x138_actorLights->ActivateLights();
  }
  if (x104_particleSystem.get()) {
    const int count = x104_particleSystem->GetParticleCountAll();
    if (count > 0) {
      mNumParticlesDrawing += count;
      x104_particleSystem->Render();
    }
  }
  if (xf4_electric.get()) {
    const int count = xf4_electric->GetParticleCount();
    if (count > 0) {
      mNumParticlesDrawing += count;
      xf4_electric->Render();
    }
  }
}

void CScriptEffect::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  if (x110_27_useRateInverseCamDist || x111_24_useRateCamDistRange) {
    float genRate = 1.f;
    const float camMagSq =
        (mgr.GetCameraManager()->GetCurrentCamera(mgr).GetTranslation() - GetTranslation())
            .MagSquared();
    const float camMag = camMagSq > 0.001f ? CMath::FastSqrtF(camMagSq) : 0.f;
    if (x110_27_useRateInverseCamDist && camMagSq < x118_rateInverseCamDistSq) {
      genRate = (1.f - x11c_rateInverseCamDistRate) * (camMag / x114_rateInverseCamDist) +
                x11c_rateInverseCamDistRate;
    }
    if (x111_24_useRateCamDistRange) {
      const float range = x124_rateCamDistRangeMax - x120_rateCamDistRangeMin;
      const float t =
          rstl::min_val(1.f, rstl::max_val(0.f, camMag - x120_rateCamDistRangeMin) / range);
      genRate = (1.f - t) * genRate + t * x128_rateCamDistRangeFarRate;
    }
    x104_particleSystem->SetGeneratorRate(genRate);
  }
  if (!mgr.GetObjectById(x13c_triggerId)) {
    x13c_triggerId = kInvalidUniqueId;
  }
}

ENTITY_ACCEPT_IMPL(CScriptEffect)

void CScriptEffect::ResetParticleCounts() {
  mNumParticlesDrawing = 0;
  mNumParticlesUpdating = 0;
}

bool CScriptEffect::AreBothSystemsDeleteable() const {
  return (!x104_particleSystem.get() || x104_particleSystem->IsSystemDeletable()) &&
         (!xf4_electric.get() || xf4_electric->IsSystemDeletable());
}

bool CScriptEffect::CanRenderUnsorted(const CStateManager& mgr) const { return false; }

void CScriptEffect::CalculateRenderBounds() {
  const rstl::optional_object< CAABox > particleBounds = x104_particleSystem.get()
                                                             ? x104_particleSystem->GetBounds()
                                                             : rstl::optional_object< CAABox >();
  const rstl::optional_object< CAABox > electricBounds =
      xf4_electric.get() ? xf4_electric->GetBounds() : rstl::optional_object< CAABox >();
  if (particleBounds.valid() || electricBounds.valid()) {
    CAABox bounds = CAABox::MakeMaxInvertedBox();
    if (particleBounds.valid()) {
      bounds.Include(*particleBounds);
    }
    if (electricBounds.valid()) {
      bounds.Include(*electricBounds);
    }
    SetRenderBounds(bounds);
    x111_26_canRender = true;
  } else {
    const CVector3f translation = GetTranslation();
    SetRenderBounds(CAABox(translation, translation));
    x111_26_canRender = false;
  }
}

CAABox CScriptEffect::GetSortingBounds(const CStateManager& mgr) const {
  if (x13c_triggerId != kInvalidUniqueId) {
    if (const CScriptTrigger* trigger =
            static_cast< const CScriptTrigger* >(mgr.GetObjectById(x13c_triggerId))) {
      return trigger->GetTriggerBoundsWR();
    }
  }
  return GetRenderBoundsCached();
}

void CScriptEffect::SetActive(const bool active) {
  CActor::SetActive(active);
  SetDrawEnabled(true);
}

CScriptEffect::~CScriptEffect() {}
