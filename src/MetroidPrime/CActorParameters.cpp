#include "MetroidPrime/CActorParameters.hpp"

#include "MetroidPrime/CActorLights.hpp"

CActorParameters::CActorParameters()
: mLighting(CLightParameters::None())
, mScannable(kInvalidAssetId)
, mXrayAssets(0, 0)
, mThermalAssets(0, 0)
, mVisor(CVisorParameters::None())
, mUseGlobalRenderTime(true)
, mThermalHeat(false)
, mNoSortThermal(false)
, mFadeInTime(0.f)
, mFadeOutTime(0.f)
, mThermalMag(0.f) {}

CActorParameters::CActorParameters(const CLightParameters& lightParms,
                                   const CScannableParameters& scanParms,
                                   const rstl::pair< CAssetId, CAssetId >& xrayAssets,
                                   const rstl::pair< CAssetId, CAssetId >& thermalAssets,
                                   const CVisorParameters& visorParms,
                                   const bool globalTimeProvider, const bool thermalHeat,
                                   const bool renderUnsorted, const bool noSortThermal,
                                   const float fadeInTime, const float fadeOutTime,
                                   const float thermalMag)
: mLighting(lightParms)
, mScannable(scanParms)
, mXrayAssets(xrayAssets)
, mThermalAssets(thermalAssets)
, mVisor(visorParms)
, mUseGlobalRenderTime(globalTimeProvider)
, mThermalHeat(thermalHeat)
, mForceRenderUnsorted(renderUnsorted)
, mNoSortThermal(noSortThermal)
, mFadeInTime(fadeInTime)
, mFadeOutTime(fadeOutTime)
, mThermalMag(thermalMag) {}

CActorParameters CActorParameters::Scannable(const CScannableParameters& sParms) const {
  CActorParameters result(*this);
  result.mScannable = sParms;
  return result;
}

CActorParameters CActorParameters::HotInThermal(bool hot) const {
  CActorParameters result(*this);
  result.mThermalHeat = hot;
  return result;
}

CActorParameters
CActorParameters::MakeDamageableTriggerActorParms(const CVisorParameters& visorParam) const {
  CActorParameters result(*this);
  result.mVisor = visorParam;
  return result;
}

CLightParameters::CLightParameters(bool castShadow, float shadowScale,
                                   CLightParameters::EShadowTessellation shadowTess,
                                   float shadowAlpha, float maxShadowHeight,
                                   const CColor& ambientColor, bool makeLights,
                                   CLightParameters::EWorldLightingOptions useWorldLighting,
                                   CLightParameters::ELightRecalculationOptions lightRecalculation,
                                   const CVector3f& lightingPositionOffset, int maxDynamicLights,
                                   int maxAreaLights, bool ambChannelOverflow, int useLightSet)
: mCastShadow(castShadow)
, mShadowScale(shadowScale)
, mShadowTesselation(shadowTess)
, mShadowAlpha(shadowAlpha)
, mMaxShadowHeight(maxShadowHeight)
, mAmbientColor(ambientColor)
, mMakeLights(makeLights)
, mAmbientChannelOverflow(ambChannelOverflow)
, mUseWorldLighting(useWorldLighting)
, mLightRecalculation(lightRecalculation)
, mUseLightSet(useLightSet)
, mLightingPositionOffset(lightingPositionOffset)
, mMaxDynamicLights(maxDynamicLights)
, mMaxAreaLights(maxAreaLights) {
  if (mMaxDynamicLights > 4 || mMaxDynamicLights == -1)
    mMaxDynamicLights = 4;
  if (mMaxAreaLights > 4 || mMaxAreaLights == -1)
    mMaxAreaLights = 4;
}

CLightParameters::CLightParameters()
: mCastShadow(false)
, mShadowScale(0.f)
, mShadowTesselation(kST_Zero)
, mShadowAlpha(0.f)
, mMaxShadowHeight(0.f)
, mAmbientColor(CColor::White())
, mMakeLights(false)
, mAmbientChannelOverflow(false)
, mUseWorldLighting(kLO_Zero)
, mLightRecalculation(kLR_EightFrames)
, mUseLightSet(0)
, mLightingPositionOffset(CVector3f::Zero())
, mMaxDynamicLights(4)
, mMaxAreaLights(4) {}

CLightParameters::~CLightParameters() {}

uint CLightParameters::GetFramesBetweenRecalculation(ELightRecalculationOptions opts) {
  switch (opts) {
  case kLR_Never:
    return 0x3FFFFFFF;
  case kLR_EightFrames:
    return 8;
  case kLR_FourFrames:
    return 4;
  case kLR_OneFrame:
    return 1;
  default:
    return 8;
  }
}

rstl::auto_ptr< CActorLights > CLightParameters::MakeActorLights() const {
  rstl::auto_ptr< CActorLights > result;
  if (mMakeLights) {
    result = rs_new CActorLights(
        GetFramesBetweenRecalculation(mLightRecalculation), mLightingPositionOffset,
        mMaxDynamicLights, mMaxAreaLights, CActorLights::kDefaultPositionUpdateThreshold,
        mAmbientChannelOverflow, mUseLightSet == 1, mUseWorldLighting == kLO_DisableWorld);
    if (mUseWorldLighting == kLO_NoShadowCast) {
      result->SetCastShadows(false);
    }
    if (mMaxAreaLights == 0 && mMakeLights) {
      result->SetAmbientColor(mAmbientColor);
    }
  }
  return result;
}
