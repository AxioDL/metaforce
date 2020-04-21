#include "Runtime/World/CScriptEffect.hpp"

#include "Runtime/CPlayerState.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/Character/CModelData.hpp"
#include "Runtime/Collision/CMaterialList.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CParticleElectric.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CGameLight.hpp"
#include "Runtime/World/CScriptTrigger.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

u32 CScriptEffect::g_NumParticlesUpdating = 0;
u32 CScriptEffect::g_NumParticlesRendered = 0;

CScriptEffect::CScriptEffect(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                             const zeus::CVector3f& scale, CAssetId partId, CAssetId elscId, bool hotInThermal,
                             bool noTimerUnlessAreaOccluded, bool rebuildSystemsOnActivate, bool active,
                             bool useRateInverseCamDist, float rateInverseCamDist, float rateInverseCamDistRate,
                             float duration, float durationResetWhileVisible, bool useRateCamDistRange,
                             float rateCamDistRangeMin, float rateCamDistRangeMax, float rateCamDistRangeFarRate,
                             bool combatVisorVisible, bool thermalVisorVisible, bool xrayVisorVisible,
                             const CLightParameters& lParms, bool dieWhenSystemsDone)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(),
         CActorParameters::None().HotInThermal(hotInThermal), kInvalidUniqueId)
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
, x114_rateInverseCamDist(rateInverseCamDist)
, x118_rateInverseCamDistSq(rateInverseCamDist * rateInverseCamDist)
, x11c_rateInverseCamDistRate(rateInverseCamDistRate)
, x120_rateCamDistRangeMin(rateCamDistRangeMin)
, x124_rateCamDistRangeMax(rateCamDistRangeMax)
, x128_rateCamDistRangeFarRate(rateCamDistRangeFarRate)
, x12c_remTime(duration)
, x130_duration(duration)
, x134_durationResetWhileVisible(durationResetWhileVisible)
, x138_actorLights(lParms.MakeActorLights()) {
  if (partId.IsValid()) {
    xf8_particleSystemToken = g_SimplePool->GetObj({FOURCC('PART'), partId});
    x104_particleSystem = std::make_unique<CElementGen>(xf8_particleSystemToken);
    zeus::CTransform newXf = xf;
    newXf.origin = zeus::skZero3f;
    x104_particleSystem->SetOrientation(newXf);
    x104_particleSystem->SetGlobalTranslation(xf.origin);
    x104_particleSystem->SetGlobalScale(scale);
    x104_particleSystem->SetParticleEmission(active);
    x104_particleSystem->SetModulationColor(lParms.GetNoLightsAmbient());
    x104_particleSystem->SetModelsUseLights(x138_actorLights != nullptr);
  }

  if (elscId.IsValid()) {
    xe8_electricToken = g_SimplePool->GetObj({FOURCC('ELSC'), elscId});
    xf4_electric = std::make_unique<CParticleElectric>(xe8_electricToken);
    zeus::CTransform newXf = xf;
    newXf.origin = zeus::skZero3f;
    xf4_electric->SetOrientation(newXf);
    xf4_electric->SetGlobalTranslation(xf.origin);
    xf4_electric->SetGlobalScale(scale);
    xf4_electric->SetParticleEmission(active);
    xf4_electric->SetModulationColor(lParms.GetNoLightsAmbient());
  }
  xe7_29_drawEnabled = true;
}

void CScriptEffect::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptEffect::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  bool oldActive = GetActive();

  switch (msg) {
  case EScriptObjectMessage::Activate:
    if (x110_26_rebuildSystemsOnActivate) {
      if (x104_particleSystem) {
        const zeus::CVector3f scale = x104_particleSystem->GetGlobalScale();
        const zeus::CColor color = x104_particleSystem->GetModulationColor();
        x104_particleSystem = std::make_unique<CElementGen>(xf8_particleSystemToken);
        zeus::CTransform newXf = GetTransform();
        newXf.origin = zeus::skZero3f;
        x104_particleSystem->SetOrientation(newXf);
        x104_particleSystem->SetGlobalTranslation(GetTranslation());
        x104_particleSystem->SetGlobalScale(scale);
        x104_particleSystem->SetModulationColor(color);
        x104_particleSystem->SetModelsUseLights(x138_actorLights != nullptr);
      }

      if (xf4_electric) {
        const zeus::CVector3f scale = xf4_electric->GetGlobalScale();
        const zeus::CColor color = xf4_electric->GetModulationColor();
        xf4_electric = std::make_unique<CParticleElectric>(xe8_electricToken);
        zeus::CTransform newXf = GetTransform();
        newXf.origin = zeus::skZero3f;
        xf4_electric->SetOrientation(newXf);
        xf4_electric->SetGlobalTranslation(GetTranslation());
        xf4_electric->SetGlobalScale(scale);
        xf4_electric->SetModulationColor(color);
      }
    }
    break;
  case EScriptObjectMessage::Registered:
    if (x104_particleSystem && x104_particleSystem->SystemHasLight()) {
      x108_lightId = mgr.AllocateUniqueId();
      mgr.AddObject(new CGameLight(x108_lightId, GetAreaIdAlways(), GetActive(),
                                   std::string("EffectPLight_") + GetName().data(), x34_transform, GetUniqueId(),
                                   x104_particleSystem->GetLight(), x10c_partId.Value(), 1, 0.f));
    }
    break;
  case EScriptObjectMessage::Deleted:
    if (x108_lightId != kInvalidUniqueId) {
      mgr.FreeScriptObject(x108_lightId);
      x108_lightId = kInvalidUniqueId;
    }
    break;
  case EScriptObjectMessage::InitializedInArea:
    for (const SConnection& conn : x20_conns) {
      if ((conn.x0_state == EScriptObjectState::Modify && conn.x4_msg == EScriptObjectMessage::Follow) ||
          (conn.x0_state == EScriptObjectState::InheritBounds && conn.x4_msg == EScriptObjectMessage::Activate)) {
        auto search = mgr.GetIdListForScript(conn.x8_objId);
        for (auto it = search.first; it != search.second; ++it) {
          if (TCastToConstPtr<CScriptTrigger>(mgr.GetObjectById(it->second)))
            x13c_triggerId = it->second;
        }
      }
    }
    break;
  default:
    break;
  }

  CActor::AcceptScriptMsg(msg, uid, mgr);

  TCastToPtr<CActor> light = mgr.ObjectById(x108_lightId);
  mgr.SendScriptMsg(light, uid, msg);

  if (oldActive != GetActive()) {
    if (GetActive()) {
      std::vector<TUniqueId> playIds;
      for (const SConnection& conn : x20_conns) {
        if (conn.x0_state != EScriptObjectState::Play || conn.x4_msg != EScriptObjectMessage::Activate) {
          continue;
        }

        const TUniqueId scriptId = mgr.GetIdForScript(conn.x8_objId);
        if (scriptId != kInvalidUniqueId) {
          playIds.push_back(scriptId);
        }
      }

      if (!playIds.empty()) {
        TCastToConstPtr<CActor> otherAct =
            mgr.GetObjectById(playIds[u32(0.99f * playIds.size() * mgr.GetActiveRandom()->Float())]);
        if (otherAct) {
          SetTransform(otherAct->GetTransform());
          if (light)
            light->SetTransform(otherAct->GetTransform());
        }
      }
    }

    x110_24_enable = true;
    if (x104_particleSystem)
      x104_particleSystem->SetParticleEmission(GetActive());
    if (xf4_electric)
      xf4_electric->SetParticleEmission(GetActive());

    if (GetActive())
      x12c_remTime = zeus::max(x12c_remTime, x130_duration);
  }
}

void CScriptEffect::PreRender(CStateManager& mgr, const zeus::CFrustum&) {
  if (x110_27_useRateInverseCamDist || x111_24_useRateCamDistRange) {
    float genRate = 1.f;
    const CGameCamera* cam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
    float camMagSq = (cam->GetTranslation() - GetTranslation()).magSquared();

    float camMag = 0.f;
    if (camMagSq > 0.001f)
      camMag = std::sqrt(camMagSq);
    if (x110_27_useRateInverseCamDist && camMagSq < x118_rateInverseCamDistSq)
      genRate = (1.f - x11c_rateInverseCamDistRate) * (camMag / x114_rateInverseCamDist) + x11c_rateInverseCamDistRate;
    if (x111_24_useRateCamDistRange) {
      float t = zeus::min(1.f, zeus::max(0.f, camMag - x120_rateCamDistRangeMin) /
                                   (x124_rateCamDistRangeMax - x120_rateCamDistRangeMin));
      genRate = (1.f - t) * genRate + t * x128_rateCamDistRangeFarRate;
    }

    x104_particleSystem->SetGeneratorRate(genRate);
  }

  if (!mgr.GetObjectById(x13c_triggerId))
    x13c_triggerId = kInvalidUniqueId;
}

void CScriptEffect::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  if (!x111_26_canRender) {
    x12c_remTime = zeus::max(x12c_remTime, x134_durationResetWhileVisible);
    return;
  }

  if (!frustum.aabbFrustumTest(x9c_renderBounds)) {
    return;
  }
  x12c_remTime = zeus::max(x12c_remTime, x134_durationResetWhileVisible);

  if (x110_31_anyVisorVisible) {
    bool visible = false;
    const CPlayerState::EPlayerVisor visor = mgr.GetPlayerState()->GetActiveVisor(mgr);
    if (visor == CPlayerState::EPlayerVisor::Combat || visor == CPlayerState::EPlayerVisor::Scan) {
      visible = x110_28_combatVisorVisible;
    } else if (visor == CPlayerState::EPlayerVisor::XRay) {
      visible = x110_30_xrayVisorVisible;
    } else if (visor == CPlayerState::EPlayerVisor::Thermal) {
      visible = x110_29_thermalVisorVisible;
    }

    if (visible && x138_actorLights) {
      const CGameArea* area = mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways());
      x138_actorLights->BuildAreaLightList(
          mgr, *area, zeus::CAABox{x9c_renderBounds.center(), x9c_renderBounds.center()});
      x138_actorLights->BuildDynamicLightList(mgr, x9c_renderBounds);
    }
    EnsureRendered(mgr);
  }
}

void CScriptEffect::Render(CStateManager& mgr) {
  /* The following code is kept for reference, this is now performed in CElementGen
  if (x138_actorLights)
  x138_actorLights->ActivateLights();
  */
  if (x104_particleSystem && x104_particleSystem->GetParticleCountAll() > 0) {
    g_NumParticlesRendered += x104_particleSystem->GetParticleCountAll();
    x104_particleSystem->Render(x138_actorLights.get());
  }

  if (xf4_electric && xf4_electric->GetParticleCount() > 0) {
    g_NumParticlesRendered += xf4_electric->GetParticleCount();
    xf4_electric->Render(x138_actorLights.get());
  }
}

void CScriptEffect::Think(float dt, CStateManager& mgr) {
  if (xe4_28_transformDirty) {
    if (x104_particleSystem) {
      zeus::CTransform newXf = x34_transform;
      newXf.origin = zeus::skZero3f;
      x104_particleSystem->SetOrientation(newXf);
      x104_particleSystem->SetGlobalTranslation(x34_transform.origin);
    }
    if (xf4_electric) {
      zeus::CTransform newXf = x34_transform;
      newXf.origin = zeus::skZero3f;
      xf4_electric->SetOrientation(newXf);
      xf4_electric->SetGlobalTranslation(x34_transform.origin);
    }

    if (TCastToPtr<CActor> act = mgr.ObjectById(x108_lightId))
      act->SetTransform(GetTransform());

    xe4_28_transformDirty = false;
  }

  if (x110_25_noTimerUnlessAreaOccluded) {
    const CGameArea* area = mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways());
    CGameArea::EOcclusionState visible =
        area->IsPostConstructed() ? area->GetOcclusionState() : CGameArea::EOcclusionState::Occluded;

    if (visible == CGameArea::EOcclusionState::Occluded && x12c_remTime <= 0.f)
      return;
  } else if (x12c_remTime <= 0.f)
    return;

  x12c_remTime -= dt;

  if (x110_24_enable) {
    if (x104_particleSystem) {
      x104_particleSystem->Update(dt);
      g_NumParticlesUpdating += x104_particleSystem->GetParticleCountAll();
    }

    if (xf4_electric) {
      xf4_electric->Update(dt);
      g_NumParticlesUpdating += xf4_electric->GetParticleCount();
    }

    if (x108_lightId != kInvalidUniqueId) {
      if (TCastToPtr<CGameLight> light = mgr.ObjectById(x108_lightId)) {
        if (x30_24_active)
          light->SetLight(x104_particleSystem->GetLight());
      }
    }

    if (x111_25_dieWhenSystemsDone) {
      x140_destroyDelayTimer += dt;
      if (x140_destroyDelayTimer > 15.f || AreBothSystemsDeleteable()) {
        mgr.FreeScriptObject(GetUniqueId());
        return;
      }
    }
  }

  if (x104_particleSystem) {
    if (xb4_drawFlags.x0_blendMode != 0)
      x104_particleSystem->SetModulationColor(xb4_drawFlags.x4_color);
    else
      x104_particleSystem->SetModulationColor(zeus::skWhite);
  }
}

void CScriptEffect::CalculateRenderBounds() {
  std::optional<zeus::CAABox> particleBounds;
  if (x104_particleSystem)
    particleBounds = x104_particleSystem->GetBounds();

  std::optional<zeus::CAABox> electricBounds;
  if (xf4_electric)
    electricBounds = xf4_electric->GetBounds();

  if (particleBounds || electricBounds) {
    zeus::CAABox renderBounds = zeus::CAABox();
    if (particleBounds) {
      renderBounds.accumulateBounds(particleBounds->min);
      renderBounds.accumulateBounds(particleBounds->max);
    }
    if (electricBounds) {
      renderBounds.accumulateBounds(electricBounds->min);
      renderBounds.accumulateBounds(electricBounds->max);
    }
    x9c_renderBounds = renderBounds;
    x111_26_canRender = true;
  } else {
    x9c_renderBounds = {GetTranslation(), GetTranslation()};
    x111_26_canRender = false;
  }
}

zeus::CAABox CScriptEffect::GetSortingBounds(const CStateManager& mgr) const {
  if (x13c_triggerId == kInvalidUniqueId)
    return x9c_renderBounds;
  else
    return static_cast<const CScriptTrigger*>(mgr.GetObjectById(x13c_triggerId))->GetTriggerBoundsWR();
}

bool CScriptEffect::AreBothSystemsDeleteable() const {
  bool ret = true;
  if (x104_particleSystem && !x104_particleSystem->IsSystemDeletable())
    ret = false;

  if (xf4_electric && !xf4_electric->IsSystemDeletable())
    ret = false;

  return ret;
}
} // namespace urde
