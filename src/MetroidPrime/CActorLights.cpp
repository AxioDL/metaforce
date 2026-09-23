#include "MetroidPrime/CActorLights.hpp"

#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/PVS/CPVSVisSet.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "WorldFormat/CPVSAreaSet.hpp"
#include "WorldFormat/CWorldLight.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

#if defined(_MSC_VER)
#include <malloc.h>
#else
#include <alloca.h>
#endif
#include <float.h>

const float CActorLights::kDefaultPositionUpdateThreshold = 0.1f;
const int CActorLights::kInvalidShadowLightIndex = -1;
int CActorLights::sFrameSchedulerCount = 0;
static bool sUseOverflowLight = true;

struct SLightValue {
  uint x0_areaLightIdx;
  CVector3f x4_color;
  float x10_colorMag;
  float x14_accumulatedMag;
  EPVSVisSetState x18_visibility;

  SLightValue(uint idx, const CVector3f& color, EPVSVisSetState visibility)
  : x0_areaLightIdx(idx)
  , x4_color(color)
  , x10_colorMag(color.Magnitude())
  , x14_accumulatedMag(0.f)
  , x18_visibility(visibility) {}

  struct CPredicate {
    bool operator()(SLightValue& a, SLightValue& b) const {
      return a.x10_colorMag > b.x10_colorMag ? true : false;
    }
  };
};
CHECK_SIZEOF(SLightValue, 0x1c);

CActorLights::CActorLights(const uint areaUpdateFramePeriod, CVector3f lightingPositionOffset,
                           const int maxDynamicLights, const int maxAreaLights,
                           float positionUpdateThreshold, const bool ambientChannelOverflow,
                           const bool useLightSet2, const bool disableWorldLights)
: x288_ambientColor(CVector3f::Zero())
, x294_aid(kInvalidAreaId)
, x298_24_dirty(true)
, x298_25_castShadows(true)
, x298_26_hasAreaLights(false)
, x298_27_findShadowLight(false)
, x298_28_inArea(!disableWorldLights && maxAreaLights > 0)
, x298_29_ambienceGenerated(ambientChannelOverflow)
, x298_30_layer2(useLightSet2)
, x298_31_disableWorldLights(disableWorldLights)
, x299_24_inBrightLight(true)
, x299_25_useBrightLightLag(false)
, x299_26_ambientOnly(false)
, x29a_findNearestDynamicLights(false)
, x29c_shadowLightArrIdx(kInvalidShadowLightIndex)
, x2a0_shadowLightIdx(kInvalidShadowLightIndex)
, x2a4_lastUpdateFrame(0)
, x2a8_areaUpdateFramePeriod(areaUpdateFramePeriod)
, x2ac_lightingPositionOffset(lightingPositionOffset)
, x2b8_maxAreaLights(maxAreaLights)
, x2bc_maxDynamicLights(maxDynamicLights)
, x2c0_lastActorPos(CVector3f::Zero())
, x2cc_actorPositionDeltaUpdateThreshold(positionUpdateThreshold * positionUpdateThreshold)
, x2d0_shadowDynamicRangeThreshold(0.f)
, x2d4_worldLightingLevel(1.f)
, x2d8_brightLightIdx(-1)
, x2dc_brightLightLag(0) {
  ++sFrameSchedulerCount;
  sFrameSchedulerCount &= 7;
}

CActorLights::~CActorLights() {}

uint CActorLights::GetActiveLightCount() const {
  if (x298_28_inArea)
    return x0_areaLights.size() + x144_dynamicLights.size();
  return x144_dynamicLights.size();
}

const CLight& CActorLights::GetLight(uint idx) const {
  if (x298_28_inArea) {
    if (idx < x0_areaLights.size())
      return x0_areaLights[idx];
    return x144_dynamicLights[idx - x0_areaLights.size()];
  }
  return x144_dynamicLights[idx];
}

void CActorLights::SetAmbientColor(const CColor& color) {
  x288_ambientColor.SetX(color.GetRed());
  x288_ambientColor.SetY(color.GetGreen());
  x288_ambientColor.SetZ(color.GetBlue());
}

void CActorLights::ActivateLights() const {
  if (x298_28_inArea) {
    if (!x298_26_hasAreaLights || x299_26_ambientOnly) {
      gpRender->SetAmbientColor(CColor::White());
      CGraphics::DisableAllLights();
      return;
    }
    gpRender->SetAmbientColor(
        CColor(x288_ambientColor.GetX(), x288_ambientColor.GetY(), x288_ambientColor.GetZ(), 1.f));
  } else {
    gpRender->SetAmbientColor(
        CColor(x288_ambientColor.GetX(), x288_ambientColor.GetY(), x288_ambientColor.GetZ(), 1.f));
  }

  uint lightIdx = 0;
  if (!x0_areaLights.empty()) {
    if (static_cast< int >(x2dc_brightLightLag) != 0 && x299_25_useBrightLightLag) {
      CLight light = x0_areaLights[0];
      CColor color = light.GetColor();
      float r, g, b;
      color.Get(r, g, b);
      float level = 1.f - static_cast< int >(x2dc_brightLightLag) / 15.f;
      color.Set(r * level, g * level, b * level, 1.f);
      light.SetColor(color);
      CGraphics::LoadLight(kLight0, light);
    } else {
      CGraphics::LoadLight(kLight0, x0_areaLights[0]);
    }
    lightIdx = 1;
    for (int i = 1; i < x0_areaLights.size(); ++i, ++lightIdx)
      CGraphics::LoadLight(static_cast< ERglLight >(i), x0_areaLights[i]);
  }
  for (int i = 0; i < x144_dynamicLights.size(); ++i, ++lightIdx)
    CGraphics::LoadLight(static_cast< ERglLight >(lightIdx), x144_dynamicLights[i]);
  if (lightIdx != 0)
    CGraphics::SetLightState((1 << lightIdx) - 1);
  else
    CGraphics::DisableAllLights();

  if (x298_31_disableWorldLights) {
    uchar value = CCast::ToUint8(x2d4_worldLightingLevel * 255.f);
    CColor color(value, value, value, uchar(255));
    gpRender->SetAmbientColor(CColor::Black());
    gpRender->SetGXRegister1Color(color);
  }
}

void CActorLights::UpdateBrightLight() {
  if (static_cast< int >(x2dc_brightLightLag) > 0 && x299_24_inBrightLight)
    --x2dc_brightLightLag;
  else if (x2dc_brightLightLag < 15 && !x299_24_inBrightLight)
    ++x2dc_brightLightLag;
  x299_25_useBrightLightLag = true;
}

void CActorLights::MergeOverflowLight(CLight& out, CVector3f& color, const CLight& in, float mag) {
  CVector3f lightColor = CVector3f::Zero();
  in.GetColor().Get(lightColor[kDX], lightColor[kDY], lightColor[kDZ]);
  color += mag * lightColor;
  out.SetAngleAttenuation(
      in.GetAngleAttenuationConstant() * mag + out.GetAngleAttenuationConstant(),
      in.GetAngleAttenuationLinear() * mag + out.GetAngleAttenuationLinear(),
      in.GetAngleAttenuationQuadratic() * mag + out.GetAngleAttenuationQuadratic());
  out.SetAttenuation(in.GetAttenuationConstant() * mag + out.GetAttenuationConstant(),
                     in.GetAttenuationLinear() * mag + out.GetAttenuationLinear(),
                     in.GetAttenuationQuadratic() * mag + out.GetAttenuationQuadratic());
  out.SetPosition(out.GetPosition() + in.GetPosition() * mag);
  out.SetDirection(out.GetDirection() + in.GetDirection() * mag);
}

bool CActorLights::BuildAreaLightList(const CStateManager& mgr, const CGameArea& area,
                                      const CAABox& aabb) {
  const rstl::vector< CWorldLight >& initialLights =
      x298_30_layer2 ? area.GetLightsB() : area.GetLightsA();
  x298_26_hasAreaLights = initialLights.size() != 0;
  if (!x298_26_hasAreaLights || !x298_28_inArea) {
    if (x298_31_disableWorldLights)
      x2d4_worldLightingLevel = area.GetPostConstructed()->x1128_worldLightingLevel;
    x29c_shadowLightArrIdx = kInvalidShadowLightIndex;
    return true;
  }

  CVector3f pos = CVector3f::Zero();
  if (!x298_24_dirty && x294_aid == area.GetId()) {
    if (mgr.GetInputFrameIdx() - x2a4_lastUpdateFrame < x2a8_areaUpdateFramePeriod)
      return false;
    x2a4_lastUpdateFrame = mgr.GetInputFrameIdx();
    pos = aabb.GetCenterPoint() + x2ac_lightingPositionOffset;
    if (x2d4_worldLightingLevel == area.GetPostConstructed()->x1128_worldLightingLevel &&
        (x2c0_lastActorPos - pos).MagSquared() < x2cc_actorPositionDeltaUpdateThreshold)
      return false;
    x2c0_lastActorPos = pos;
  } else {
    if (x294_aid != area.GetId())
      x2d8_brightLightIdx = -1;
    x2a4_lastUpdateFrame = sFrameSchedulerCount + mgr.GetInputFrameIdx();
    pos = aabb.GetCenterPoint() + x2ac_lightingPositionOffset;
    x2c0_lastActorPos = pos;
  }
  x2d4_worldLightingLevel = area.GetPostConstructed()->x1128_worldLightingLevel;
  x298_24_dirty = false;
  x294_aid = area.GetId();
  x29c_shadowLightArrIdx = kInvalidShadowLightIndex;
  const rstl::vector< CWorldLight >& lightList =
      x298_30_layer2 ? area.GetLightsB() : area.GetLightsA();
  const rstl::vector< CLight >& gfxLights = x298_30_layer2
                                                ? area.GetPostConstructed()->x90_gfxLightsB
                                                : area.GetPostConstructed()->x70_gfxLightsA;
  SLightValue* values = static_cast< SLightValue* >(alloca(lightList.size() * sizeof(SLightValue)));
  int valueCount = 0;
  x288_ambientColor = CVector3f::Zero();
  const CPVSAreaSet* areaPVS = area.GetAreaVisSet();
  const bool usePVS = areaPVS && gkPVSEnabled == 1;
  const bool useSecondLayer =
      x298_30_layer2 ? (usePVS ? areaPVS->Has2ndLayerLights() : true) : false;
  CPVSVisSet centerSet(kVSS_OutOfBounds);
  CPVSVisSet maxSet(kVSS_OutOfBounds);
  CPVSVisSet minSet(kVSS_OutOfBounds);
  if (usePVS) {
    centerSet = areaPVS->GetVisOctree().GetVisSet(area.GetInverseTransform() * pos);
    maxSet = areaPVS->GetVisOctree().GetVisSet(area.GetInverseTransform() * aabb.GetMaxPoint());
    minSet = areaPVS->GetVisOctree().GetVisSet(area.GetInverseTransform() * aabb.GetMinPoint());
  }

  for (int i = 0; i < gfxLights.size(); ++i) {
    const CLight* const light = &gfxLights[i];
    if (light->GetType() == kLT_LocalAmbient) {
      x288_ambientColor = light->GetNormalIndependentLightingAtPoint(pos);
    } else {
      EPVSVisSetState visible = kVSS_OutOfBounds;
      if (usePVS && lightList[i].DoesCastShadows()) {
        uint feature =
            useSecondLayer ? area.Get2ndPVSLightFeature(i) : area.Get1stPVSLightFeature(i);
        visible = centerSet.GetVisible(feature);
        if (visible != kVSS_OutOfBounds)
          visible = rstl::max_val(maxSet.GetVisible(feature), visible);
        if (visible != kVSS_OutOfBounds)
          visible = rstl::max_val(minSet.GetVisible(feature), visible);
      }
      if (visible != kVSS_EndOfTree) {
        if (CollisionUtil::AABoxSphereIntersection(
                aabb, CSphere(light->GetPosition(), light->GetRadius() * 2.f))) {
          values[valueCount] =
              SLightValue(i, light->GetNormalIndependentLightingAtPoint(pos), visible);
          ++valueCount;
        }
      }
    }
  }
  rstl::sort(values, values + valueCount, SLightValue::CPredicate());
  if (x298_27_findShadowLight) {
    float mag = x288_ambientColor.Magnitude();
    for (int i = valueCount - 1; i >= 0; --i) {
      mag += values[i].x10_colorMag;
      values[i].x14_accumulatedMag = mag;
    }
  }

  CVector3f overflowAmbient = CVector3f::Zero();
  CLight overflowLight = CLight::BuildCustom(CVector3f::Zero(), CVector3f::Zero(), CColor::Black(),
                                             0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
  CVector3f overflowColor = CVector3f::Zero();
  float overflowMag = 0.f;
  const bool useOverflowLight = !x298_29_ambienceGenerated && sUseOverflowLight;
  int maxAreaLights = useOverflowLight ? x2b8_maxAreaLights - 1 : x2b8_maxAreaLights;
  x0_areaLights.clear();
  int mostSignificantLight = 0;
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid),
      CMaterialList(kMT_Projectile, kMT_ProjectilePassthrough, kMT_SeeThrough));
  for (int i = 0; i < valueCount; ++i) {
    const SLightValue& value = values[i];
    if (x0_areaLights.size() < maxAreaLights) {
      bool contact = true;
      int lightIndex = value.x0_areaLightIdx;
      bool castShadows =
          lightList[lightIndex].DoesCastShadows() == true && x298_25_castShadows == true;
      bool outOfBounds = usePVS && value.x18_visibility == kVSS_OutOfBounds;
      if (castShadows) {
        const CLight& light = gfxLights[lightIndex];
        CVector3f rayStart = pos;
        CVector3f delta = light.GetPosition() - rayStart;
        float distance = delta.Magnitude();
        bool shadowCandidate = x298_27_findShadowLight &&
                               x29c_shadowLightArrIdx == kInvalidShadowLightIndex &&
                               light.GetType() != kLT_LocalAmbient && distance > 2.f &&
                               !aabb.PointInside(light.GetPosition());
        bool useShadow = shadowCandidate;
        if (shadowCandidate) {
          bool significantLight =
              x0_areaLights.size() == 0 ||
              (x0_areaLights.size() == 1 &&
               value.x10_colorMag / values[mostSignificantLight].x10_colorMag > 0.5f);
          useShadow = significantLight;
          if (significantLight)
            useShadow = value.x10_colorMag / value.x14_accumulatedMag >
                        x2d0_shadowDynamicRangeThreshold / (1.f + x2d0_shadowDynamicRangeThreshold);
        }
        if (useShadow) {
          x29c_shadowLightArrIdx = x0_areaLights.size();
          x2a0_shadowLightIdx = lightIndex;
        } else if (!outOfBounds) {
          delta *= 1.f / distance;
          contact =
              CGameCollision::RayStaticIntersectionArea(area, rayStart, delta, distance, filter);
          if (i == 0) {
            if (contact)
              x299_24_inBrightLight = true;
            else
              x299_24_inBrightLight = false;
            if (x2d8_brightLightIdx != lightIndex) {
              if (contact)
                x2dc_brightLightLag = 0;
              else
                x2dc_brightLightLag = 15;
              x2d8_brightLightIdx = lightIndex;
            }
            x299_25_useBrightLightLag = false;
            contact = true;
          }
        }
      }
      if (contact) {
        if (x0_areaLights.size() == 0)
          mostSignificantLight = i;
        x0_areaLights.push_back(gfxLights[lightIndex]);
      }
    } else {
      if (useOverflowLight && value.x10_colorMag > 0.001f) {
        MergeOverflowLight(overflowLight, overflowColor, gfxLights[value.x0_areaLightIdx],
                           value.x10_colorMag);
        overflowMag += value.x10_colorMag;
      } else {
        overflowAmbient += value.x4_color;
      }
    }
  }
  if (useOverflowLight)
    AddOverflowToLights(overflowLight, overflowColor, overflowMag);
  else
    MoveAmbienceToLights(overflowAmbient);
  if (x288_ambientColor.GetX() > 1.f)
    x288_ambientColor.SetX(1.f);
  if (x288_ambientColor.GetY() > 1.f)
    x288_ambientColor.SetY(1.f);
  if (x288_ambientColor.GetZ() > 1.f)
    x288_ambientColor.SetZ(1.f);
  if (area.GetPostConstructed()->x1128_worldLightingLevel < 1.f)
    MultiplyLightingLevels(area.GetPostConstructed()->x1128_worldLightingLevel);
  return true;
}

void CActorLights::MultiplyLightingLevels(float level) {
  x288_ambientColor *= level;
  for (int i = 0; i < x0_areaLights.size(); ++i) {
    CColor color = x0_areaLights[i].GetColor();
    float r, g, b;
    color.Get(r, g, b);
    color.Set(r * level, g * level, b * level, 1.f);
    x0_areaLights[i].SetColor(color);
  }
}

void CActorLights::AddOverflowToLights(const CLight& light, const CVector3f& color, float mag) {
  if (mag < 0.001f || x2b8_maxAreaLights < 1)
    return;
  mag = 1.f / mag;
  CVector3f scaledColor = color * mag;
  CColor useColor(scaledColor.GetX(), scaledColor.GetY(), scaledColor.GetZ(), 1.f);
  CLight overflowLight = CLight::BuildCustom(
      light.GetPosition() * mag, light.GetDirection() * mag, useColor,
      light.GetAttenuationConstant() * mag, light.GetAttenuationLinear() * mag,
      light.GetAttenuationQuadratic() * mag, light.GetAngleAttenuationConstant() * mag,
      light.GetAngleAttenuationLinear() * mag, light.GetAngleAttenuationQuadratic() * mag);
  x0_areaLights.push_back(overflowLight);
}

void CActorLights::MoveAmbienceToLights(const CVector3f& color) {
  if (x298_29_ambienceGenerated || !sUseOverflowLight || x0_areaLights.empty()) {
    x288_ambientColor += color * (1.f / 3.f);
    return;
  }
  CLight& light = x0_areaLights[0];
  float r, g, b;
  light.GetColor().Get(r, g, b);
  CVector3f useColor = color + CVector3f(r, g, b);
  float maxComponent = rstl::max_val(useColor[kDX], useColor[kDY]);
  maxComponent = rstl::max_val(maxComponent, useColor[kDZ]);
  if (maxComponent > FLT_EPSILON)
    useColor *= 1.f / maxComponent;
  light.SetColor(CColor(useColor.GetX(), useColor.GetY(), useColor.GetZ(), 1.f));
}

void CActorLights::BuildDynamicLightList(const CStateManager& mgr, const CAABox& aabb) {
  UpdateBrightLight();
  x299_26_ambientOnly = false;
  x144_dynamicLights.clear();
  const rstl::vector< CLight >& lights = mgr.GetDynamicLightList();
  if (!x29a_findNearestDynamicLights) {
    for (int i = 0; i < lights.size() && x144_dynamicLights.size() < x2bc_maxDynamicLights; ++i) {
      const CLight& light = lights[i];
      if (CollisionUtil::AABoxSphereIntersection(aabb,
                                                 CSphere(light.GetPosition(), light.GetRadius())))
        x144_dynamicLights.push_back(light);
    }
  } else {
    int ids[4];
    float radii[4] = {-1.f, -1.f, -1.f, -1.f};
    for (int i = 0; i < lights.size() && x144_dynamicLights.size() < 4; ++i) {
      const CLight& light = lights[i];
      bool handled = false;
      for (int j = 0; j < x144_dynamicLights.size(); ++j) {
        if (ids[j] == light.GetId()) {
          float radius = CollisionUtil::AABoxSphereIntersectionRadius(
              aabb, CSphere(light.GetPosition(), light.GetRadius()));
          if (radius >= 0.f && radii[j] > radius) {
            radii[j] = radius;
            x144_dynamicLights[j] = light;
            handled = true;
          }
          break;
        }
      }
      if (!handled) {
        radii[x144_dynamicLights.size()] = CollisionUtil::AABoxSphereIntersectionRadius(
            aabb, CSphere(light.GetPosition(), light.GetRadius()));
        if (radii[x144_dynamicLights.size()] >= 0.f) {
          ids[x144_dynamicLights.size()] = light.GetId();
          x144_dynamicLights.push_back(light);
        }
      }
    }
  }
}

void CActorLights::BuildFaceLightList(const CStateManager& mgr, const CGameArea& area,
                                      const CAABox& aabb) {
  CTransform4f cameraTransform = mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform();
  x298_26_hasAreaLights = true;
  x288_ambientColor = CVector3f::Zero();
  const CObjectList& lights = mgr.GetObjectListById(kOL_GameLight);
  x144_dynamicLights.clear();
  CVector3f accumulatedColor = CVector3f::Zero();
  for (int i = lights.GetFirstObjectIndex();
       i != -1 && x144_dynamicLights.size() < x2bc_maxDynamicLights;
       i = lights.GetNextObjectIndex(i)) {
    const CEntity* entity = lights[i];
    if (entity != nullptr) {
      if (entity->GetActive()) {
        const CGameLight* light = TCastToConstPtr< CGameLight >(entity);
        if (const CExplosion* explosion =
                TCastToConstPtr< CExplosion >(mgr.GetObjectById(light->GetParentId()))) {
          const CLight& originalLight = light->GetLight();
          CLight reflectedLight = originalLight;
          float attenConstant = gpTweakGui->GetExplosionLightFalloffMultConstant() *
                                reflectedLight.GetAttenuationConstant();
          float attenLinear = gpTweakGui->GetExplosionLightFalloffMultLinear() *
                              reflectedLight.GetAttenuationLinear();
          float attenQuadratic = gpTweakGui->GetExplosionLightFalloffMultQuadratic() *
                                 reflectedLight.GetAttenuationQuadratic();
          reflectedLight.SetAttenuation(attenConstant, attenLinear, attenQuadratic);
          CVector3f cameraToExplosion =
              cameraTransform.TransposeMultiply(explosion->GetTranslation());
          if (!(CVector3f::Dot(CVector3f::Forward(), cameraToExplosion) < 0.f)) {
            cameraToExplosion[kDY] =
                -cameraToExplosion[kDY] + CTweakGui::FaceReflectionDistanceDebugValueToActualValue(
                                              gpTweakGui->GetFaceReflectionDistance());
            cameraToExplosion[kDZ] =
                -cameraToExplosion[kDZ] + CTweakGui::FaceReflectionHeightDebugValueToActualValue(
                                              gpTweakGui->GetFaceReflectionHeight());
            reflectedLight.SetPosition(cameraTransform * cameraToExplosion);
            if (CollisionUtil::AABoxSphereIntersection(
                    aabb, CSphere(originalLight.GetPosition(), originalLight.GetRadius()))) {
              accumulatedColor += reflectedLight.GetNormalIndependentLightingAtPoint(
                  cameraTransform.GetTranslation());
              if (originalLight.GetIntensity() > FLT_EPSILON &&
                  originalLight.GetRadius() > FLT_EPSILON)
                x144_dynamicLights.push_back(reflectedLight);
            }
          }
        }
      }
    }
  }
  float grayscale = 0.3f * accumulatedColor.GetX() + 0.6f * accumulatedColor.GetY() +
                    0.1f * accumulatedColor.GetZ();
  if (grayscale < 0.012f)
    x144_dynamicLights.clear();
  if (grayscale > 0.03f) {
    grayscale = 0.03f / grayscale;
    float attenuation = 1.f / grayscale;
    for (AUTO(it, x144_dynamicLights.begin()); it != x144_dynamicLights.end(); ++it)
      it->SetAttenuation(it->GetAttenuationConstant() * attenuation,
                         it->GetAttenuationLinear() * attenuation,
                         it->GetAttenuationQuadratic() * attenuation);
  }
}

void CActorLights::BuildFakeLightList(const rstl::vector< CLight >& lights, const CColor& color) {
  BuildConstantAmbientLighting(color);
  x0_areaLights.clear();
  x144_dynamicLights.clear();
  for (int i = 0; i < 4; ++i) {
    if (i == lights.size())
      break;
    x144_dynamicLights.push_back(lights[i]);
  }
}

void CActorLights::BuildConstantAmbientLighting(const CColor& color) {
  x299_26_ambientOnly = false;
  color.Get(x288_ambientColor[kDX], x288_ambientColor[kDY], x288_ambientColor[kDZ]);
  x294_aid = kInvalidAreaId;
  x298_24_dirty = true;
  x298_26_hasAreaLights = true;
  x29c_shadowLightArrIdx = kInvalidShadowLightIndex;
  x2a0_shadowLightIdx = kInvalidShadowLightIndex;
}

void CActorLights::BuildConstantAmbientLighting() {
  x299_26_ambientOnly = true;
  x298_24_dirty = true;
  x29c_shadowLightArrIdx = kInvalidShadowLightIndex;
  x2a0_shadowLightIdx = kInvalidShadowLightIndex;
}
