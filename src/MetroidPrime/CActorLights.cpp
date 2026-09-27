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
  uint mAreaLightIdx;
  CVector3f mColor;
  float mColorMag;
  float mAccumulatedMag;
  EPVSVisSetState mVisibility;

  SLightValue(uint idx, const CVector3f& color, EPVSVisSetState visibility)
  : mAreaLightIdx(idx)
  , mColor(color)
  , mColorMag(color.Magnitude())
  , mAccumulatedMag(0.f)
  , mVisibility(visibility) {}

  struct CPredicate {
    bool operator()(SLightValue& a, SLightValue& b) const {
      return a.mColorMag > b.mColorMag ? true : false;
    }
  };
};
CHECK_SIZEOF(SLightValue, 0x1c);

CActorLights::CActorLights(const uint areaUpdateFramePeriod, CVector3f lightingPositionOffset,
                           const int maxDynamicLights, const int maxAreaLights,
                           float positionUpdateThreshold, const bool ambientChannelOverflow,
                           const bool useLightSet2, const bool disableWorldLights)
: mAmbientColor(CVector3f::Zero())
, mAid(kInvalidAreaId)
, mDirty(true)
, mCastShadows(true)
, mHasAreaLights(false)
, mFindShadowLight(false)
, mInArea(!disableWorldLights && maxAreaLights > 0)
, mAmbienceGenerated(ambientChannelOverflow)
, mLayer2(useLightSet2)
, mDisableWorldLights(disableWorldLights)
, mInBrightLight(true)
, mUseBrightLightLag(false)
, mAmbientOnly(false)
, mFindNearestDynamicLights(false)
, mShadowLightArrIdx(kInvalidShadowLightIndex)
, mShadowLightIdx(kInvalidShadowLightIndex)
, mLastUpdateFrame(0)
, mAreaUpdateFramePeriod(areaUpdateFramePeriod)
, mLightingPositionOffset(lightingPositionOffset)
, mMaxAreaLights(maxAreaLights)
, mMaxDynamicLights(maxDynamicLights)
, mLastActorPos(CVector3f::Zero())
, mActorPositionDeltaUpdateThreshold(positionUpdateThreshold * positionUpdateThreshold)
, mShadowDynamicRangeThreshold(0.f)
, mWorldLightingLevel(1.f)
, mBrightLightIdx(-1)
, mBrightLightLag(0) {
  ++sFrameSchedulerCount;
  sFrameSchedulerCount &= 7;
}

CActorLights::~CActorLights() {}

uint CActorLights::GetActiveLightCount() const {
  if (mInArea)
    return mAreaLights.size() + mDynamicLights.size();
  return mDynamicLights.size();
}

const CLight& CActorLights::GetLight(uint idx) const {
  if (mInArea) {
    if (idx < mAreaLights.size())
      return mAreaLights[idx];
    return mDynamicLights[idx - mAreaLights.size()];
  }
  return mDynamicLights[idx];
}

void CActorLights::SetAmbientColor(const CColor& color) {
  mAmbientColor.SetX(color.GetRed());
  mAmbientColor.SetY(color.GetGreen());
  mAmbientColor.SetZ(color.GetBlue());
}

void CActorLights::ActivateLights() const {
  if (mInArea) {
    if (!mHasAreaLights || mAmbientOnly) {
      gpRender->SetAmbientColor(CColor::White());
      CGraphics::DisableAllLights();
      return;
    }
    gpRender->SetAmbientColor(
        CColor(mAmbientColor.GetX(), mAmbientColor.GetY(), mAmbientColor.GetZ(), 1.f));
  } else {
    gpRender->SetAmbientColor(
        CColor(mAmbientColor.GetX(), mAmbientColor.GetY(), mAmbientColor.GetZ(), 1.f));
  }

  uint lightIdx = 0;
  if (!mAreaLights.empty()) {
    if (static_cast< int >(mBrightLightLag) != 0 && mUseBrightLightLag) {
      CLight light = mAreaLights[0];
      CColor color = light.GetColor();
      float r, g, b;
      color.Get(r, g, b);
      float level = 1.f - static_cast< int >(mBrightLightLag) / 15.f;
      color.Set(r * level, g * level, b * level, 1.f);
      light.SetColor(color);
      CGraphics::LoadLight(kLight0, light);
    } else {
      CGraphics::LoadLight(kLight0, mAreaLights[0]);
    }
    lightIdx = 1;
    for (int i = 1; i < mAreaLights.size(); ++i, ++lightIdx)
      CGraphics::LoadLight(static_cast< ERglLight >(i), mAreaLights[i]);
  }
  for (int i = 0; i < mDynamicLights.size(); ++i, ++lightIdx)
    CGraphics::LoadLight(static_cast< ERglLight >(lightIdx), mDynamicLights[i]);
  if (lightIdx != 0)
    CGraphics::SetLightState((1 << lightIdx) - 1);
  else
    CGraphics::DisableAllLights();

  if (mDisableWorldLights) {
    uchar value = CCast::ToUint8(mWorldLightingLevel * 255.f);
    CColor color(value, value, value, uchar(255));
    gpRender->SetAmbientColor(CColor::Black());
    gpRender->SetGXRegister1Color(color);
  }
}

void CActorLights::UpdateBrightLight() {
  if (static_cast< int >(mBrightLightLag) > 0 && mInBrightLight)
    --mBrightLightLag;
  else if (mBrightLightLag < 15 && !mInBrightLight)
    ++mBrightLightLag;
  mUseBrightLightLag = true;
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
      mLayer2 ? area.GetLightsB() : area.GetLightsA();
  mHasAreaLights = initialLights.size() != 0;
  if (!mHasAreaLights || !mInArea) {
    if (mDisableWorldLights)
      mWorldLightingLevel = area.GetPostConstructed()->mWorldLightingLevel;
    mShadowLightArrIdx = kInvalidShadowLightIndex;
    return true;
  }

  CVector3f pos = CVector3f::Zero();
  if (!mDirty && mAid == area.GetId()) {
    if (mgr.GetInputFrameIdx() - mLastUpdateFrame < mAreaUpdateFramePeriod)
      return false;
    mLastUpdateFrame = mgr.GetInputFrameIdx();
    pos = aabb.GetCenterPoint() + mLightingPositionOffset;
    if (mWorldLightingLevel == area.GetPostConstructed()->mWorldLightingLevel &&
        (mLastActorPos - pos).MagSquared() < mActorPositionDeltaUpdateThreshold)
      return false;
    mLastActorPos = pos;
  } else {
    if (mAid != area.GetId())
      mBrightLightIdx = -1;
    mLastUpdateFrame = sFrameSchedulerCount + mgr.GetInputFrameIdx();
    pos = aabb.GetCenterPoint() + mLightingPositionOffset;
    mLastActorPos = pos;
  }
  mWorldLightingLevel = area.GetPostConstructed()->mWorldLightingLevel;
  mDirty = false;
  mAid = area.GetId();
  mShadowLightArrIdx = kInvalidShadowLightIndex;
  const rstl::vector< CWorldLight >& lightList =
      mLayer2 ? area.GetLightsB() : area.GetLightsA();
  const rstl::vector< CLight >& gfxLights = mLayer2
                                                ? area.GetPostConstructed()->mGfxLightsB
                                                : area.GetPostConstructed()->mGfxLightsA;
  SLightValue* values = static_cast< SLightValue* >(alloca(lightList.size() * sizeof(SLightValue)));
  int valueCount = 0;
  mAmbientColor = CVector3f::Zero();
  const CPVSAreaSet* areaPVS = area.GetAreaVisSet();
  const bool usePVS = areaPVS && gkPVSEnabled == 1;
  const bool useSecondLayer =
      mLayer2 ? (usePVS ? areaPVS->Has2ndLayerLights() : true) : false;
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
      mAmbientColor = light->GetNormalIndependentLightingAtPoint(pos);
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
  if (mFindShadowLight) {
    float mag = mAmbientColor.Magnitude();
    for (int i = valueCount - 1; i >= 0; --i) {
      mag += values[i].mColorMag;
      values[i].mAccumulatedMag = mag;
    }
  }

  CVector3f overflowAmbient = CVector3f::Zero();
  CLight overflowLight = CLight::BuildCustom(CVector3f::Zero(), CVector3f::Zero(), CColor::Black(),
                                             0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
  CVector3f overflowColor = CVector3f::Zero();
  float overflowMag = 0.f;
  const bool useOverflowLight = !mAmbienceGenerated && sUseOverflowLight;
  int maxAreaLights = useOverflowLight ? mMaxAreaLights - 1 : mMaxAreaLights;
  mAreaLights.clear();
  int mostSignificantLight = 0;
  const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid),
      CMaterialList(kMT_Projectile, kMT_ProjectilePassthrough, kMT_SeeThrough));
  for (int i = 0; i < valueCount; ++i) {
    const SLightValue& value = values[i];
    if (mAreaLights.size() < maxAreaLights) {
      bool contact = true;
      int lightIndex = value.mAreaLightIdx;
      bool castShadows =
          lightList[lightIndex].DoesCastShadows() == true && mCastShadows == true;
      bool outOfBounds = usePVS && value.mVisibility == kVSS_OutOfBounds;
      if (castShadows) {
        const CLight& light = gfxLights[lightIndex];
        CVector3f rayStart = pos;
        CVector3f delta = light.GetPosition() - rayStart;
        float distance = delta.Magnitude();
        bool shadowCandidate = mFindShadowLight &&
                               mShadowLightArrIdx == kInvalidShadowLightIndex &&
                               light.GetType() != kLT_LocalAmbient && distance > 2.f &&
                               !aabb.PointInside(light.GetPosition());
        bool useShadow = shadowCandidate;
        if (shadowCandidate) {
          bool significantLight =
              mAreaLights.size() == 0 ||
              (mAreaLights.size() == 1 &&
               value.mColorMag / values[mostSignificantLight].mColorMag > 0.5f);
          useShadow = significantLight;
          if (significantLight)
            useShadow = value.mColorMag / value.mAccumulatedMag >
                        mShadowDynamicRangeThreshold / (1.f + mShadowDynamicRangeThreshold);
        }
        if (useShadow) {
          mShadowLightArrIdx = mAreaLights.size();
          mShadowLightIdx = lightIndex;
        } else if (!outOfBounds) {
          delta *= 1.f / distance;
          contact =
              CGameCollision::RayStaticIntersectionArea(area, rayStart, delta, distance, filter);
          if (i == 0) {
            if (contact)
              mInBrightLight = true;
            else
              mInBrightLight = false;
            if (mBrightLightIdx != lightIndex) {
              if (contact)
                mBrightLightLag = 0;
              else
                mBrightLightLag = 15;
              mBrightLightIdx = lightIndex;
            }
            mUseBrightLightLag = false;
            contact = true;
          }
        }
      }
      if (contact) {
        if (mAreaLights.size() == 0)
          mostSignificantLight = i;
        mAreaLights.push_back(gfxLights[lightIndex]);
      }
    } else {
      if (useOverflowLight && value.mColorMag > 0.001f) {
        MergeOverflowLight(overflowLight, overflowColor, gfxLights[value.mAreaLightIdx],
                           value.mColorMag);
        overflowMag += value.mColorMag;
      } else {
        overflowAmbient += value.mColor;
      }
    }
  }
  if (useOverflowLight)
    AddOverflowToLights(overflowLight, overflowColor, overflowMag);
  else
    MoveAmbienceToLights(overflowAmbient);
  if (mAmbientColor.GetX() > 1.f)
    mAmbientColor.SetX(1.f);
  if (mAmbientColor.GetY() > 1.f)
    mAmbientColor.SetY(1.f);
  if (mAmbientColor.GetZ() > 1.f)
    mAmbientColor.SetZ(1.f);
  if (area.GetPostConstructed()->mWorldLightingLevel < 1.f)
    MultiplyLightingLevels(area.GetPostConstructed()->mWorldLightingLevel);
  return true;
}

void CActorLights::MultiplyLightingLevels(float level) {
  mAmbientColor *= level;
  for (int i = 0; i < mAreaLights.size(); ++i) {
    CColor color = mAreaLights[i].GetColor();
    float r, g, b;
    color.Get(r, g, b);
    color.Set(r * level, g * level, b * level, 1.f);
    mAreaLights[i].SetColor(color);
  }
}

void CActorLights::AddOverflowToLights(const CLight& light, const CVector3f& color, float mag) {
  if (mag < 0.001f || mMaxAreaLights < 1)
    return;
  mag = 1.f / mag;
  CVector3f scaledColor = color * mag;
  CColor useColor(scaledColor.GetX(), scaledColor.GetY(), scaledColor.GetZ(), 1.f);
  CLight overflowLight = CLight::BuildCustom(
      light.GetPosition() * mag, light.GetDirection() * mag, useColor,
      light.GetAttenuationConstant() * mag, light.GetAttenuationLinear() * mag,
      light.GetAttenuationQuadratic() * mag, light.GetAngleAttenuationConstant() * mag,
      light.GetAngleAttenuationLinear() * mag, light.GetAngleAttenuationQuadratic() * mag);
  mAreaLights.push_back(overflowLight);
}

void CActorLights::MoveAmbienceToLights(const CVector3f& color) {
  if (mAmbienceGenerated || !sUseOverflowLight || mAreaLights.empty()) {
    mAmbientColor += color * (1.f / 3.f);
    return;
  }
  CLight& light = mAreaLights[0];
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
  mAmbientOnly = false;
  mDynamicLights.clear();
  const rstl::vector< CLight >& lights = mgr.GetDynamicLightList();
  if (!mFindNearestDynamicLights) {
    for (int i = 0; i < lights.size() && mDynamicLights.size() < mMaxDynamicLights; ++i) {
      const CLight& light = lights[i];
      if (CollisionUtil::AABoxSphereIntersection(aabb,
                                                 CSphere(light.GetPosition(), light.GetRadius())))
        mDynamicLights.push_back(light);
    }
  } else {
    int ids[4];
    float radii[4] = {-1.f, -1.f, -1.f, -1.f};
    for (int i = 0; i < lights.size() && mDynamicLights.size() < 4; ++i) {
      const CLight& light = lights[i];
      bool handled = false;
      for (int j = 0; j < mDynamicLights.size(); ++j) {
        if (ids[j] == light.GetId()) {
          float radius = CollisionUtil::AABoxSphereIntersectionRadius(
              aabb, CSphere(light.GetPosition(), light.GetRadius()));
          if (radius >= 0.f && radii[j] > radius) {
            radii[j] = radius;
            mDynamicLights[j] = light;
            handled = true;
          }
          break;
        }
      }
      if (!handled) {
        radii[mDynamicLights.size()] = CollisionUtil::AABoxSphereIntersectionRadius(
            aabb, CSphere(light.GetPosition(), light.GetRadius()));
        if (radii[mDynamicLights.size()] >= 0.f) {
          ids[mDynamicLights.size()] = light.GetId();
          mDynamicLights.push_back(light);
        }
      }
    }
  }
}

void CActorLights::BuildFaceLightList(const CStateManager& mgr, const CGameArea& area,
                                      const CAABox& aabb) {
  CTransform4f cameraTransform = mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform();
  mHasAreaLights = true;
  mAmbientColor = CVector3f::Zero();
  const CObjectList& lights = mgr.GetObjectListById(kOL_GameLight);
  mDynamicLights.clear();
  CVector3f accumulatedColor = CVector3f::Zero();
  for (int i = lights.GetFirstObjectIndex();
       i != -1 && mDynamicLights.size() < mMaxDynamicLights;
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
                mDynamicLights.push_back(reflectedLight);
            }
          }
        }
      }
    }
  }
  float grayscale = 0.3f * accumulatedColor.GetX() + 0.6f * accumulatedColor.GetY() +
                    0.1f * accumulatedColor.GetZ();
  if (grayscale < 0.012f)
    mDynamicLights.clear();
  if (grayscale > 0.03f) {
    grayscale = 0.03f / grayscale;
    float attenuation = 1.f / grayscale;
    for (AUTO(it, mDynamicLights.begin()); it != mDynamicLights.end(); ++it)
      it->SetAttenuation(it->GetAttenuationConstant() * attenuation,
                         it->GetAttenuationLinear() * attenuation,
                         it->GetAttenuationQuadratic() * attenuation);
  }
}

void CActorLights::BuildFakeLightList(const rstl::vector< CLight >& lights, const CColor& color) {
  BuildConstantAmbientLighting(color);
  mAreaLights.clear();
  mDynamicLights.clear();
  for (int i = 0; i < 4; ++i) {
    if (i == lights.size())
      break;
    mDynamicLights.push_back(lights[i]);
  }
}

void CActorLights::BuildConstantAmbientLighting(const CColor& color) {
  mAmbientOnly = false;
  color.Get(mAmbientColor[kDX], mAmbientColor[kDY], mAmbientColor[kDZ]);
  mAid = kInvalidAreaId;
  mDirty = true;
  mHasAreaLights = true;
  mShadowLightArrIdx = kInvalidShadowLightIndex;
  mShadowLightIdx = kInvalidShadowLightIndex;
}

void CActorLights::BuildConstantAmbientLighting() {
  mAmbientOnly = true;
  mDirty = true;
  mShadowLightArrIdx = kInvalidShadowLightIndex;
  mShadowLightIdx = kInvalidShadowLightIndex;
}
