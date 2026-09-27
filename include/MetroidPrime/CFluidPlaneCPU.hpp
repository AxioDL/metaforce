#ifndef _CFLUIDPLANECPU
#define _CFLUIDPLANECPU

#include "MetroidPrime/CFluidPlaneManager.hpp"

class CFluidUVMotion;

class CFluidPlaneCPU : public CFluidPlane {
public:
  class CTurbulence {
  public:
    static int kTableSize;

    CTurbulence(float speed, float distance, float freqMax, float freqMin, float phaseMax,
                float phaseMin, float amplitudeMax, float amplitudeMin);
    ~CTurbulence();

    float GetHeight(float sel) const {
      return mTable[(mTableCount - 1) & int(sel * mHeightSelPitch)];
    }
    float GetOODistance() const { return mOoTurbDistance; }
    float GetOOSpeed() const { return mOoTurbSpeed; }
    bool HasTurbulence() const { return mHasTurbulence; }

  private:
    float mSpeed;
    float mDistance;
    float mFreqMax;
    float mFreqMin;
    float mPhaseMax;
    float mPhaseMin;
    float mAmplitudeMax;
    float mAmplitudeMin;
    float* mTable;
    int mTableCount;
    float mHeightSelPitch;
    float mOoTurbSpeed;
    float mOoTurbDistance;
    bool mHasTurbulence;
  };

public:
  CFluidPlaneCPU(uint patternMap1, uint patternMap2, uint colorMap, uint bumpMap, uint envMap,
                 uint envBumpMap, float unitsPerLightmapTexel, uint lightmap, uint tileSubdivisions,
                 EFluidType fluidType, float tileSize, const CVector3f& bumpLightDir, float alpha,
                 const CFluidUVMotion& uvMotion, float bumpScale, float turbSpeed,
                 float turbDistance, float turbFreqMax, float turbFreqMin, float turbPhaseMax,
                 float turbPhaseMin, float turbAmplitudeMax, float turbAmplitudeMin,
                 float specularMin, float specularMax, float reflectionBlend, float reflectionSize,
                 float rippleIntensity);
  ~CFluidPlaneCPU() {}

  void Render(const CStateManager& mgr, float alpha, const CAABox& aabb, const CTransform4f& xf,
              const CTransform4f& areaXf, bool noNormals, const CFrustumPlanes& frustum,
              const rstl::optional_object< CRippleManager >& rippleManager, TUniqueId waterId,
              const char* gridFlags, int gridDimX, int gridDimY,
              const CVector3f& areaCenter) const override;

  void RenderSetup(const CStateManager& mgr, float alpha, const CTransform4f& xf,
                   const CTransform4f& areaXf, const CAABox& aabb, CScriptWater* water) const;
  void RenderCleanup() const;
  void CalculateLightmapMtx(const CTransform4f& areaXf, const CTransform4f& xf, const CAABox& aabb,
                            int idx) const;

  float GetReflectionBlend() const { return mReflectionBlend; }
  float GetSpecularMax() const { return mSpecularMax; }
  float GetSpecularMin() const { return mSpecularMin; }
  float GetReflectionSize() const { return mReflectionSize; }
  float GetBumpScale() const { return mBumpScale; }
  bool HasBumpMap() const { return mBumpMap; }
  const TLockedToken< CTexture >& GetBumpMap() const { return *mBumpMap; }
  bool HasEnvMap() const { return mEnvMap; }
  const TLockedToken< CTexture >& GetEnvMap() const { return *mEnvMap; }
  bool HasEnvBumpMap() const { return mEnvBumpMap.valid(); }
  const TLockedToken< CTexture >& GetEnvBumpMap() const { return *mEnvBumpMap; }
  bool HasLightMap() const { return mLightmap.valid(); }
  const TLockedToken< CTexture >& GetLightMap() const { return *mLightmap; }
  const CVector3f& GetBumpLightDir() const { return mBumpLightDir; }
  float GetTileSize() const { return mTileSize; }
  int GetTileSubdivisions() const { return mTileSubdivisions; }
  float GetRippleResolution() const { return mRippleResolution; }
  float GetTurbulenceHeight(float sel) const { return mTurbulence.GetHeight(sel); }
  float GetOOTurbulenceDistance() const { return mTurbulence.GetOODistance(); }
  float GetOOTurbulenceSpeed() const { return mTurbulence.GetOOSpeed(); }
  bool HasTurbulence() const { return mTurbulence.HasTurbulence(); }

private:
  CAssetId mTexIdBumpMap;
  CAssetId mTexIdEnvMap;
  CAssetId mTexIdEnvBumpMap;
  CAssetId mTexIdLightmap;
  rstl::optional_object< TLockedToken< CTexture > > mBumpMap;
  rstl::optional_object< TLockedToken< CTexture > > mEnvMap;
  rstl::optional_object< TLockedToken< CTexture > > mEnvBumpMap;
  rstl::optional_object< TLockedToken< CTexture > > mLightmap;
  CVector3f mBumpLightDir;
  float mBumpScale;
  float mTileSize;
  int mTileSubdivisions;
  float mRippleResolution;
  float mSpecularMin;
  float mSpecularMax;
  float mReflectionBlend;
  float mReflectionSize;
  float mUnitsPerLightmapTexel;
  CTurbulence mTurbulence;
};
CHECK_SIZEOF(CFluidPlaneCPU, 0x158)

#endif // _CFLUIDPLANECPU
