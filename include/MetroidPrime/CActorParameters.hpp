#ifndef _CACTORPARAMETERS
#define _CACTORPARAMETERS

#include "types.h"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/pair.hpp"

class CActorLights;

class CLightParameters {
public:
  enum EShadowTessellation {
    kST_Invalid = -1,
    kST_Zero,
  };

  enum EWorldLightingOptions {
    kLO_Zero,
    kLO_NormalWorld,
    kLO_NoShadowCast,
    kLO_DisableWorld,
  };

  enum ELightRecalculationOptions {
    kLR_Never,
    kLR_EightFrames,
    kLR_FourFrames,
    kLR_OneFrame,
  };

  CLightParameters();
  CLightParameters(const bool castShadow, float shadowScale,
                   CLightParameters::EShadowTessellation shadowTess, float shadowAlpha,
                   float maxShadowHeight, const CColor& ambientColor, const bool makeLights,
                   CLightParameters::EWorldLightingOptions useWorldLighting,
                   CLightParameters::ELightRecalculationOptions lightRecalcOpts,
                   const CVector3f& lightingPositionOffset, int maxDynamicLights, int maxAreaLights,
                   const bool ambChannelOverflow, int useLightSet);
  virtual ~CLightParameters();

  const CColor& GetAmbientColor() const { return mAmbientColor; }
  bool ShouldMakeLights() const { return mMakeLights; }
  bool GetAmbientChannelOverflow() const { return mAmbientChannelOverflow; }
  const CVector3f& GetLightingPositionOffset() const { return mLightingPositionOffset; }
  int GetMaxDynamicLights() const { return mMaxDynamicLights; }
  int GetMaxAreaLights() const { return mMaxAreaLights; }

  static CLightParameters None() { return CLightParameters(); }

  static uint GetFramesBetweenRecalculation(ELightRecalculationOptions opts);
  rstl::auto_ptr< CActorLights > MakeActorLights() const;

private:
  bool mCastShadow;
  float mShadowScale;
  EShadowTessellation mShadowTesselation;
  float mShadowAlpha;
  float mMaxShadowHeight;
  CColor mAmbientColor;
  bool mMakeLights;
  bool mAmbientChannelOverflow;
  EWorldLightingOptions mUseWorldLighting;
  ELightRecalculationOptions mLightRecalculation;
  int mUseLightSet;
  CVector3f mLightingPositionOffset;
  int mMaxDynamicLights;
  int mMaxAreaLights;
};
CHECK_SIZEOF(CLightParameters, 0x40)

class CScannableParameters {
public:
  CScannableParameters() {}
  CScannableParameters(CAssetId scanId) : mScanId(scanId) {}

  CAssetId GetScannableObject0() const { return mScanId; }

private:
  CAssetId mScanId;
};
CHECK_SIZEOF(CScannableParameters, 0x4)

class CVisorParameters {
public:
  CVisorParameters(uchar mask, bool b1, bool scanPassthrough)
  : mMask(mask), mB1(b1), mScanPassthrough(scanPassthrough) {}

  uchar GetMask() const { return mMask; }
  // TODO: GetIsBlockXRay__16CVisorParametersCFv?
  bool GetBool1() const { return mB1; }
  bool GetScanPassthrough() const { return mScanPassthrough; }

  static CVisorParameters None() { return CVisorParameters(0xF, false, false); }

private:
  uint mMask : 4;
  uint mB1 : 1;
  uint mScanPassthrough : 1;
};
CHECK_SIZEOF(CVisorParameters, 0x4)

class CActorParameters {
public:
  CActorParameters();
  CActorParameters(const CLightParameters& lightParms, const CScannableParameters& scanParms,
                   const rstl::pair< CAssetId, CAssetId >& xrayAssets,
                   const rstl::pair< CAssetId, CAssetId >& thermalAssets,
                   const CVisorParameters& visorParms, bool globalTimeProvider, bool thermalHeat,
                   bool renderUnsorted, bool noSortThermal, float fadeInTime, float fadeOutTime,
                   float thermalMag);
  ~CActorParameters() {}

  CActorParameters Scannable(const CScannableParameters& sParms) const;
  CActorParameters HotInThermal(bool hot) const;
  CActorParameters MakeDamageableTriggerActorParms(const CVisorParameters& visorParam) const;

  const CLightParameters& GetLighting() const { return mLighting; }
  const CScannableParameters& GetScannable() const { return mScannable; }
  const rstl::pair< CAssetId, CAssetId >& GetXRay() const { return mXrayAssets; }
  const rstl::pair< CAssetId, CAssetId >& GetInfra() const { return mThermalAssets; }
  const CVisorParameters& GetVisorParameters() const { return mVisor; }
  float GetThermalMag() const { return mThermalMag; }
  bool UseGlobalRenderTime() const { return mUseGlobalRenderTime; }
  bool IsHotInThermal() const { return mThermalHeat; }
  bool ForceRenderUnsorted() const { return mForceRenderUnsorted; }
  bool NoSortThermal() const { return mNoSortThermal; }
  float GetFadeInTime() const { return mFadeInTime; }
  float GetFadeOutTime() const { return mFadeOutTime; }

  static CActorParameters None() { return CActorParameters(); }

private:
  CLightParameters mLighting;
  CScannableParameters mScannable;
  rstl::pair< CAssetId, CAssetId > mXrayAssets;
  rstl::pair< CAssetId, CAssetId > mThermalAssets;
  CVisorParameters mVisor;
  bool mUseGlobalRenderTime : 1;
  bool mThermalHeat : 1;
  bool mForceRenderUnsorted : 1;
  bool mNoSortThermal : 1;
  float mFadeInTime;
  float mFadeOutTime;
  float mThermalMag;
};
CHECK_SIZEOF(CActorParameters, 0x68)

#endif // _CACTORPARAMETERS
