#include "Runtime/Character/CActorLights.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CFirstPersonCamera.hpp"
#include "Runtime/Collision/CGameCollision.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/World/CExplosion.hpp"
#include "Runtime/World/CGameArea.hpp"
#include "Runtime/World/CGameLight.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

s32 CActorLights::sFrameSchedulerCount = 0;
CActorLights::CActorLights(u32 areaUpdateFramePeriod, const zeus::CVector3f& actorPosBias, int maxDynamicLights,
                           int maxAreaLights, bool ambientChannelOverflow, bool layer2, bool disableWorldLights,
                           float positionUpdateThreshold)
: x298_28_inArea(!disableWorldLights && maxAreaLights > 0)
, x298_29_ambienceGenerated(ambientChannelOverflow)
, x298_30_layer2(layer2)
, x298_31_disableWorldLights(disableWorldLights)
, x2a8_areaUpdateFramePeriod(areaUpdateFramePeriod)
, x2ac_actorPosBias(actorPosBias)
, x2b8_maxAreaLights(maxAreaLights)
, x2bc_maxDynamicLights(maxDynamicLights)
, x2cc_actorPositionDeltaUpdateThreshold(positionUpdateThreshold * positionUpdateThreshold) {
  sFrameSchedulerCount++;
  sFrameSchedulerCount &= 7;
}

void CActorLights::BuildConstantAmbientLighting() {
  x299_26_ambientOnly = true;
  x298_24_dirty = true;
  x29c_shadowLightArrIdx = -1;
  x2a0_shadowLightIdx = -1;
}

void CActorLights::BuildConstantAmbientLighting(const zeus::CColor& color) {
  x299_26_ambientOnly = false;
  x288_ambientColor = color;
  x294_aid = kInvalidAreaId;
  x298_24_dirty = true;
  x298_26_hasAreaLights = true;
  x29c_shadowLightArrIdx = -1;
  x2a0_shadowLightIdx = -1;
}

void CActorLights::BuildFakeLightList(const std::vector<CLight>& lights, const zeus::CColor& color) {
  BuildConstantAmbientLighting(color);
  x0_areaLights.clear();
  x144_dynamicLights = lights;
}

void CActorLights::BuildFaceLightList(const CStateManager& mgr, const CGameArea& area, const zeus::CAABox& aabb) {
  zeus::CTransform fpTransform = mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform();
  x298_26_hasAreaLights = true;
  x288_ambientColor = zeus::skBlack;
  x144_dynamicLights.clear();
  zeus::CColor accumColor = zeus::skBlack;
  for (CEntity* light : mgr.GetLightObjectList()) {
    if (!light || !light->GetActive())
      continue;
    CGameLight* castLight = static_cast<CGameLight*>(light);
    if (TCastToConstPtr<CExplosion> explosion = mgr.GetObjectById(castLight->GetParentId())) {
      CLight originalLight = castLight->GetLight();
      CLight explosionLight = originalLight;
      explosionLight.SetAttenuation(
          explosionLight.GetAttenuationConstant() * g_tweakGui->GetExplosionLightFalloffMultConstant(),
          explosionLight.GetAttenuationLinear() * g_tweakGui->GetExplosionLightFalloffMultLinear(),
          explosionLight.GetAttenuationQuadratic() * g_tweakGui->GetExplosionLightFalloffMultQuadratic());
      zeus::CVector3f camToExplo = explosion->GetTranslation() - fpTransform.origin;
      if (fpTransform.transposeRotate(camToExplo).dot(zeus::skForward) >= 0.f) {
        camToExplo.y() = -camToExplo.y() + ITweakGui::FaceReflectionDistanceDebugValueToActualValue(
                                               g_tweakGui->GetFaceReflectionDistance());
        camToExplo.z() = -camToExplo.z() +
                         ITweakGui::FaceReflectionHeightDebugValueToActualValue(g_tweakGui->GetFaceReflectionHeight());
        explosionLight.SetPosition(fpTransform * camToExplo);
        zeus::CSphere sphere(originalLight.GetPosition(), originalLight.GetRadius());
        if (aabb.intersects(sphere)) {
          accumColor += explosionLight.GetNormalIndependentLightingAtPoint(fpTransform.origin);
          if (originalLight.GetIntensity() > FLT_EPSILON && originalLight.GetRadius() > FLT_EPSILON)
            x144_dynamicLights.push_back(explosionLight);
        }
      }
    }
  }

  float greyscale = accumColor.rgbDot(zeus::CColor(0.3f, 0.6f, 0.1f));
  if (greyscale < 0.012f)
    x144_dynamicLights.clear();

  if (greyscale > 0.03f) {
    float attMul = 1.f / (0.03f / greyscale);
    for (CLight& light : x144_dynamicLights)
      light.SetAttenuation(light.GetAttenuationConstant() * attMul, light.GetAttenuationLinear() * attMul,
                           light.GetAttenuationQuadratic() * attMul);
  }
}

struct SLightValue {
  u32 x0_areaLightIdx;
  zeus::CColor x4_color;
  float x10_colorMag;
  float x14_accumulatedMag = 0.f;
  EPVSVisSetState x18_visiblity;
};

void CActorLights::MergeOverflowLight(CLight& out, zeus::CColor& color, const CLight& in, float colorMag) {
  color += in.GetColor() * colorMag;
  out.SetAngleAttenuation(in.GetAngleAttenuationConstant() * colorMag + out.GetAngleAttenuationConstant(),
                          in.GetAngleAttenuationLinear() * colorMag + out.GetAngleAttenuationLinear(),
                          in.GetAngleAttenuationQuadratic() * colorMag + out.GetAngleAttenuationQuadratic());
  out.SetAttenuation(in.GetAttenuationConstant() * colorMag + out.GetAttenuationConstant(),
                     in.GetAttenuationLinear() * colorMag + out.GetAttenuationLinear(),
                     in.GetAttenuationQuadratic() * colorMag + out.GetAttenuationQuadratic());
  out.SetPosition(in.GetPosition() * colorMag + out.GetPosition());
  out.SetDirection(in.GetDirection() * colorMag + out.GetDirection());
}

void CActorLights::AddOverflowToLights(const CLight& light, const zeus::CColor& color, float mag) {
  if (mag < 0.001f || x2b8_maxAreaLights < 1)
    return;

  mag = 1.f / mag;
  zeus::CColor useColor = color * mag;
  useColor.a() = 1.f;
  x0_areaLights.push_back(
      CLight::BuildCustom(light.GetPosition() * mag, light.GetDirection() * mag, useColor,
                          light.GetAttenuationConstant() * mag, light.GetAttenuationLinear() * mag,
                          light.GetAttenuationQuadratic() * mag, light.GetAngleAttenuationConstant() * mag,
                          light.GetAngleAttenuationLinear() * mag, light.GetAngleAttenuationQuadratic() * mag));
}

void CActorLights::MoveAmbienceToLights(const zeus::CColor& color) {
  if (x298_29_ambienceGenerated) {
    x288_ambientColor += color * 0.333333f;
    x288_ambientColor.a() = 1.f;
    return;
  }

  zeus::CColor useColor = x0_areaLights[0].GetColor() + color;
  float maxComponent = std::max(useColor.r(), std::max(useColor.g(), useColor.b()));
  if (maxComponent > FLT_EPSILON)
    useColor *= (1.f / maxComponent);
  useColor.a() = 1.f;
  x0_areaLights[0].SetColor(useColor);
}

void CActorLights::MultiplyLightingLevels(float level) {
  x288_ambientColor *= level;
  for (CLight& light : x0_areaLights) {
    zeus::CColor color = light.GetColor();
    color *= level;
    color.a() = 1.f;
    light.SetColor(color);
  }
}

void CActorLights::UpdateBrightLight() {
  if (x2dc_brightLightLag > 0 && x299_24_inBrightLight)
    --x2dc_brightLightLag;
  else if (x2dc_brightLightLag < 15 && !x299_24_inBrightLight)
    ++x2dc_brightLightLag;
  x299_25_useBrightLightLag = true;
}

bool CActorLights::BuildAreaLightList(const CStateManager& mgr, const CGameArea& area, const zeus::CAABox& aabb) {
  const std::vector<CWorldLight>& lightList =
      x298_30_layer2 ? area.GetPostConstructed()->x80_lightsB : area.GetPostConstructed()->x60_lightsA;
  const std::vector<CLight>& gfxLightList =
      x298_30_layer2 ? area.GetPostConstructed()->x90_gfxLightsB : area.GetPostConstructed()->x70_gfxLightsA;
  float worldLightingLevel = area.GetPostConstructed()->x1128_worldLightingLevel;
  x298_26_hasAreaLights = lightList.size() != 0;
  if (!x298_26_hasAreaLights || !x298_28_inArea) {
    /* World lights disabled */
    if (x298_31_disableWorldLights)
      x2d4_worldLightingLevel = worldLightingLevel;
    x29c_shadowLightArrIdx = -1;
    return true;
  }

  zeus::CVector3f vec;
  if (!x298_24_dirty && x294_aid == area.GetAreaId()) {
    /* Early return if not ready for update */
    if (mgr.GetInputFrameIdx() - x2a4_lastUpdateFrame < x2a8_areaUpdateFramePeriod)
      return false;
    vec = aabb.center() + x2ac_actorPosBias;
    if (x2d4_worldLightingLevel == worldLightingLevel)
      if ((x2c0_lastActorPos - vec).magSquared() < x2cc_actorPositionDeltaUpdateThreshold)
        return false;
    x2c0_lastActorPos = vec;
  } else {
    if (x294_aid != area.GetAreaId())
      x2d8_brightLightIdx = -1;
    x2a4_lastUpdateFrame = sFrameSchedulerCount + mgr.GetInputFrameIdx();
    vec = aabb.center() + x2ac_actorPosBias;
    x2c0_lastActorPos = vec;
  }

  /* Reset lighting state */
  x2d4_worldLightingLevel = worldLightingLevel;
  x298_24_dirty = false;
  x294_aid = area.GetAreaId();
  x29c_shadowLightArrIdx = -1;
  x288_ambientColor = zeus::skClear;

  /* Find candidate lights via PVS */
  bool use2ndLayer;
  if (x298_30_layer2) {
    if (const CPVSAreaSet* pvs = area.GetAreaVisSet())
      use2ndLayer = pvs->Has2ndLayerLights();
    else
      use2ndLayer = true;
  } else {
    use2ndLayer = false;
  }

  CPVSVisSet sets[3];
  sets[0].Reset(EPVSVisSetState::OutOfBounds);
  sets[1].Reset(EPVSVisSetState::OutOfBounds);
  sets[2].Reset(EPVSVisSetState::OutOfBounds);

  if (const CPVSAreaSet* pvs = area.GetAreaVisSet()) {
    zeus::CVector3f localVec = area.GetInverseTransform() * vec;
    sets[0].SetTestPoint(pvs->GetVisOctree(), localVec);
    localVec = area.GetInverseTransform() * aabb.max;
    sets[1].SetTestPoint(pvs->GetVisOctree(), localVec);
    localVec = area.GetInverseTransform() * aabb.min;
    sets[2].SetTestPoint(pvs->GetVisOctree(), localVec);
  }

  std::vector<SLightValue> valList;
  valList.reserve(lightList.size());

  auto lightIt = lightList.begin();
  int lightIdx = 0;
  for (const CLight& light : gfxLightList) {
    if (light.GetType() == ELightType::LocalAmbient) {
      /* Take ambient here */
      x288_ambientColor = light.GetNormalIndependentLightingAtPoint(vec);
    } else {
      EPVSVisSetState visible = EPVSVisSetState::OutOfBounds;
      if (area.GetAreaVisSet()) {
        if (lightIt->DoesCastShadows()) {
          u32 pvsIdx;
          if (use2ndLayer)
            pvsIdx = area.Get2ndPVSLightFeature(lightIdx);
          else
            pvsIdx = area.Get1stPVSLightFeature(lightIdx);
          visible = sets[0].GetVisible(pvsIdx);
          if (visible != EPVSVisSetState::OutOfBounds)
            visible = std::max(visible, sets[1].GetVisible(pvsIdx));
          if (visible != EPVSVisSetState::OutOfBounds)
            visible = std::max(visible, sets[2].GetVisible(pvsIdx));
        }
      }
      if (visible != EPVSVisSetState::EndOfTree) {
        zeus::CSphere sphere(light.GetPosition(), light.GetRadius() * 2.f);
        if (aabb.intersects(sphere)) {
          /* Light passes as candidate */
          SLightValue& value = valList.emplace_back();
          value.x0_areaLightIdx = lightIdx;
          value.x4_color = light.GetNormalIndependentLightingAtPoint(vec);
          value.x4_color.a() = 0.f;
          value.x10_colorMag = value.x4_color.magnitude();
          value.x18_visiblity = visible;
        }
      }
    }
    ++lightIt;
    ++lightIdx;
  }

  /* Sort lights most intense to least intense */
  std::sort(valList.begin(), valList.end(),
            [](const SLightValue& a, const SLightValue& b) { return a.x10_colorMag > b.x10_colorMag; });

  if (x298_27_findShadowLight) {
    /* Accumulate magnitudes up to most intense for shadow dynamic range check */
    x288_ambientColor.a() = 0.f;
    float mag = x288_ambientColor.magnitude();
    for (auto it = valList.rbegin(); it != valList.rend(); ++it) {
      mag += it->x10_colorMag;
      it->x14_accumulatedMag = mag;
    }
  }

  /* Ambient color for overflow area lights */
  zeus::CColor overflowAmbColor = zeus::skClear;

  /* Averaged light for overflow area lights */
  CLight overflowLight = CLight::BuildCustom(zeus::skZero3f, zeus::skZero3f, zeus::skBlack,
                                             0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
  zeus::CColor overflowLightColor = zeus::skClear;
  float overflowMag = 0.f;

  /* Max significant lights */
  int maxAreaLights = !x298_29_ambienceGenerated ? x2b8_maxAreaLights - 1 : x2b8_maxAreaLights;
  x0_areaLights.clear();

  /* Filter for performing final light visibility test */
  CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(EMaterialTypes::Solid),
      CMaterialList(EMaterialTypes::Projectile, EMaterialTypes::ProjectilePassthrough, EMaterialTypes::SeeThrough));
  u32 mostSigLightIdx = 0;

  /* Narrowphase test candidates starting with most intense */
  for (size_t i = 0; i < valList.size(); ++i) {
    const SLightValue& value = valList[i];
    const CLight& light = gfxLightList[value.x0_areaLightIdx];
    if (x0_areaLights.size() < maxAreaLights) {
      /* Significant light */
      bool actorToLightContact = true;
      bool castShadows = lightList[value.x0_areaLightIdx].DoesCastShadows() && x298_25_castShadows;
      bool outOfBounds = area.GetAreaVisSet() && value.x18_visiblity == EPVSVisSetState::OutOfBounds;
      if (castShadows) {
        /* Process shadow cast */
        zeus::CVector3f delta = light.GetPosition() - vec;
        float deltaMag = delta.magnitude();
        bool useShadow = false;
        if (x298_27_findShadowLight && x29c_shadowLightArrIdx == -1 && light.GetType() != ELightType::LocalAmbient &&
            deltaMag > 2.f && !aabb.pointInside(light.GetPosition())) {
          /* Perform shadow dynamic range check */
          if (!x0_areaLights.size() ||
              (x0_areaLights.size() == 1 && value.x10_colorMag / valList[mostSigLightIdx].x10_colorMag > 0.5f)) {
            useShadow = value.x10_colorMag / value.x14_accumulatedMag >
                        x2d0_shadowDynamicRangeThreshold / (1.f + x2d0_shadowDynamicRangeThreshold);
          }
        }
        if (useShadow) {
          /* Note shadow light */
          x29c_shadowLightArrIdx = x0_areaLights.size();
          x2a0_shadowLightIdx = value.x0_areaLightIdx;
        } else if (!outOfBounds) {
          /* Note brightest light contact */
          delta = delta * 1.f / deltaMag;
          actorToLightContact = CGameCollision::RayStaticIntersectionArea(area, vec, delta, deltaMag, filter);
          if (i == 0) {
            x299_24_inBrightLight = actorToLightContact;
            if (x2d8_brightLightIdx != value.x0_areaLightIdx) {
              x2dc_brightLightLag = actorToLightContact ? 0 : 15;
              x2d8_brightLightIdx = value.x0_areaLightIdx;
            }
            x299_25_useBrightLightLag = false;
            actorToLightContact = true;
          }
        }
      }
      if (actorToLightContact) {
        /* Add to final list */
        if (x0_areaLights.size() == 0)
          mostSigLightIdx = i;
        x0_areaLights.push_back(light);
      }
    } else {
      /* Overflow light */
      if (!x298_29_ambienceGenerated && value.x10_colorMag > 0.001f) {
        /* Average parameters into final light */
        MergeOverflowLight(overflowLight, overflowLightColor, light, value.x10_colorMag);
        overflowMag += value.x10_colorMag;
      } else {
        /* Average color into ambient channel */
        overflowAmbColor += value.x4_color;
      }
    }
  }

  /* Finalize overflow lights */
  if (!x298_29_ambienceGenerated)
    AddOverflowToLights(overflowLight, overflowLightColor, overflowMag);
  else
    MoveAmbienceToLights(overflowAmbColor);

  /* Clamp ambient color */
  if (x288_ambientColor.r() > 1.f)
    x288_ambientColor.r() = 1.f;
  if (x288_ambientColor.g() > 1.f)
    x288_ambientColor.g() = 1.f;
  if (x288_ambientColor.b() > 1.f)
    x288_ambientColor.b() = 1.f;
  x288_ambientColor.a() = 1.f;

  /* Multiply down lighting with world fader level */
  if (worldLightingLevel < 1.f)
    MultiplyLightingLevels(worldLightingLevel);

  return true;
}

void CActorLights::BuildDynamicLightList(const CStateManager& mgr, const zeus::CAABox& aabb) {
  UpdateBrightLight();
  x299_26_ambientOnly = false;
  x144_dynamicLights.clear();

  if (!x29a_findNearestDynamicLights) {
    for (const CLight& light : mgr.GetDynamicLightList()) {
      zeus::CSphere sphere(light.GetPosition(), light.GetRadius());
      if (aabb.intersects(sphere))
        x144_dynamicLights.push_back(light);
      if (x144_dynamicLights.size() >= x2bc_maxDynamicLights)
        break;
    }
  } else {
    const CLight* addedLights[8] = {};
    for (int i = 0; i < x2bc_maxDynamicLights && i < 8; ++i) {
      float minRad = FLT_MAX;
      for (const CLight& light : mgr.GetDynamicLightList()) {
        zeus::CSphere sphere(light.GetPosition(), light.GetRadius());
        float intRadius = aabb.intersectionRadius(sphere);
        if (intRadius >= 0.f && intRadius < minRad) {
          bool alreadyIn = false;
          for (int j = 0; j < i; ++j) {
            if (&light == addedLights[j]) {
              alreadyIn = true;
              break;
            }
          }
          if (alreadyIn)
            continue;
          addedLights[i] = &light;
          minRad = intRadius;
        }
      }
      if (addedLights[i])
        x144_dynamicLights.push_back(*addedLights[i]);
      if (x144_dynamicLights.size() >= x2bc_maxDynamicLights)
        break;
    }
  }
}

std::vector<CLight> CActorLights::BuildLightVector() const {
  std::vector<CLight> lights;

  if (x0_areaLights.size()) {
    if (x2dc_brightLightLag && x299_25_useBrightLightLag) {
      CLight overrideLight = x0_areaLights[0];
      overrideLight.SetColor(overrideLight.GetColor() * (1.f - x2dc_brightLightLag / 15.f));
      lights.push_back(overrideLight);
    } else
      lights.push_back(x0_areaLights[0]);

    for (auto it = x0_areaLights.begin() + 1; it != x0_areaLights.end(); ++it) {
      lights.push_back(*it);
    }

    if (x29c_shadowLightArrIdx > 0) {
      /* Ensure shadow light comes first for shader extension */
      std::swap(lights[0], lights[x29c_shadowLightArrIdx]);
    }
  }

  for (const CLight& light : x144_dynamicLights)
    lights.push_back(light);

  zeus::CColor ambColor = x288_ambientColor;
  ambColor.a() = 1.f;
  lights.push_back(CLight::BuildLocalAmbient(zeus::skZero3f, ambColor));

  return lights;
}

void CActorLights::ActivateLights(CBooModel& model) const {
  std::vector<CLight> lights;

  if (x298_28_inArea) {
    if (!x298_26_hasAreaLights || x299_26_ambientOnly) {
      // g_Renderer->SetAmbientColor(zeus::skWhite);
      lights.push_back(CLight::BuildLocalAmbient(zeus::skZero3f, zeus::skWhite));
      model.ActivateLights(lights);
      return;
    }
  }

  lights = BuildLightVector();
  model.ActivateLights(lights);

  if (x298_31_disableWorldLights) {
    zeus::CColor color(x2d4_worldLightingLevel);
    g_Renderer->SetGXRegister1Color(color);
  }
}

void CActorLights::DisableAreaLights() {
  x2b8_maxAreaLights = 0;
  x298_26_hasAreaLights = false;
  x298_28_inArea = false;
}

const CLight& CActorLights::GetLight(u32 idx) const {
  if (x298_28_inArea) {
    if (idx < x0_areaLights.size())
      return x0_areaLights[idx];
    return x144_dynamicLights[idx - x0_areaLights.size()];
  }
  return x144_dynamicLights[idx];
}

u32 CActorLights::GetActiveLightCount() const {
  if (x298_28_inArea)
    return x0_areaLights.size() + x144_dynamicLights.size();
  return x144_dynamicLights.size();
}

} // namespace urde
