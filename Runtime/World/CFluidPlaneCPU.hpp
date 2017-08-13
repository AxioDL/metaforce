#ifndef __URDE_CFLUIDPLANECPU_HPP__
#define __URDE_CFLUIDPLANECPU_HPP__

#include "CFluidPlane.hpp"
#include "CRipple.hpp"
#include "Graphics/Shaders/CFluidPlaneShader.hpp"

namespace urde
{
class CFluidUVMotion;
class CFluidPlaneCPU : public CFluidPlane
{
    class CTurbulence
    {
        float x0_speed;
        float x4_distance;
        float x8_freqMax;
        float xc_freqMin;
        float x10_phaseMax;
        float x14_phaseMin;
        float x18_amplitudeMax;
        float x1c_amplitudeMin;
        std::unique_ptr<float[]> x20_table; // x140
        u32 x24_tableCount = 0; // x144
        float x28_heightSelPitch = 0.f; // x148
        float x2c_ooTurbSpeed; // x14c
        float x30_ooTurbDistance; // x150
        bool x34_hasTurbulence = false; // x154
    public:
        CTurbulence(float speed, float distance, float freqMax, float freqMin,
                    float phaseMax, float phaseMin, float amplitudeMax, float amplitudeMin);
        float GetHeight(float sel) const { return x20_table[(x24_tableCount - 1) & int(sel * x28_heightSelPitch)]; }
        float GetOODistance() const { return x30_ooTurbDistance; }
        float GetOOSpeed() const { return x2c_ooTurbSpeed; }
        bool HasTurbulence() const { return x34_hasTurbulence; }
    };

    CAssetId xa0_texIdBumpMap;
    CAssetId xa4_texIdEnvMap;
    CAssetId xa8_texIdEnvBumpMap;
    CAssetId xac_texId4;
    std::experimental::optional<TLockedToken<CTexture>> xb0_bumpMap;
    std::experimental::optional<TLockedToken<CTexture>> xc0_envMap;
    std::experimental::optional<TLockedToken<CTexture>> xd0_envBumpMap;
    std::experimental::optional<TLockedToken<CTexture>> xe0_lightmap;
    zeus::CVector3f xf0_bumpLightDir;
    float xfc_bumpScale;
    float x100_tileSize;
    int x104_tileSubdivisions;
    float x108_rippleResolution;
    float x10c_specularMin;
    float x110_specularMax;
    float x114_reflectionBlend;
    float x118_reflectionSize;
    float x11c_unitsPerLightmapTexel;
    CTurbulence x120_turbulence;

    u32 m_maxVertCount;

    mutable std::vector<CFluidPlaneShader::Vertex> m_verts;
    mutable std::experimental::optional<CFluidPlaneShader> m_shader;
    mutable bool m_cachedDoubleLightmapBlend;
    mutable bool m_cachedAdditive;

    struct RenderSetupInfo
    {
        zeus::CMatrix4f texMtxs[6];
        zeus::CMatrix4f normMtx;
        float indScale = 1.f;
        zeus::CColor kColors[4];
        std::vector<CLight> lights;
    };
public:
    CFluidPlaneCPU(CAssetId texPattern1, CAssetId texPattern2, CAssetId texColor, CAssetId bumpMap, CAssetId envMap, CAssetId envBumpMap,
                   CAssetId lightMap, float unitsPerLightmapTexel, float tileSize, u32 tileSubdivisions,
                   EFluidType fluidType, float alpha, const zeus::CVector3f& bumpLightDir, float bumpScale,
                   const CFluidUVMotion& mot, float turbSpeed, float turbDistance, float turbFreqMax,
                   float turbFreqMin, float turbPhaseMax, float turbPhaseMin, float turbAmplitudeMax,
                   float turbAmplitudeMin, float specularMin, float specularMax, float reflectionBlend,
                   float reflectionSize, float rippleIntensity, u32 maxVertCount);
    void CreateRipple(const CRipple& ripple, CStateManager& mgr);
    void CalculateLightmapMatrix(const zeus::CTransform& areaXf, const zeus::CTransform& xf,
                                 const zeus::CAABox& aabb, zeus::CMatrix4f& mtxOut) const;
    RenderSetupInfo RenderSetup(const CStateManager& mgr, float, const zeus::CTransform& xf,
                                const zeus::CTransform& areaXf, const zeus::CAABox& aabb,
                                const CScriptWater* water) const;
    void Render(const CStateManager& mgr, float alpha, const zeus::CAABox& aabb, const zeus::CTransform& xf,
                const zeus::CTransform& areaXf, bool noNormals, const zeus::CFrustum& frustum,
                const std::experimental::optional<CRippleManager>& rippleManager, TUniqueId waterId,
                const bool* gridFlags, u32 gridDimX, u32 gridDimY, const zeus::CVector3f& areaCenter) const;
    void RenderCleanup() const;
    float GetReflectionBlend() const { return x114_reflectionBlend; }
    float GetSpecularMax() const { return x110_specularMax; }
    float GetSpecularMin() const { return x10c_specularMin; }
    float GetReflectionSize() const { return x118_reflectionSize; }
    float GetBumpScale() const { return xfc_bumpScale; }
    bool HasBumpMap() const { return xb0_bumpMap.operator bool(); }
    const CTexture& GetBumpMap() const { return **xb0_bumpMap; }
    bool HasEnvMap() const { return xc0_envMap.operator bool(); }
    const CTexture& GetEnvMap() const { return **xc0_envMap; }
    bool HasEnvBumpMap() const { return xd0_envBumpMap.operator bool(); }
    const CTexture& GetEnvBumpMap() const { return **xd0_envBumpMap; }
    bool HasLightMap() const { return xe0_lightmap.operator bool(); }
    const CTexture& GetLightMap() const { return **xe0_lightmap; }
    const zeus::CVector3f& GetBumpLightDir() const { return xf0_bumpLightDir; }
    float GetTileSize() const { return x100_tileSize; }
    int GetTileSubdivisions() const { return x104_tileSubdivisions; }
    float GetRippleResolution() const { return x108_rippleResolution; }
    float GetTurbulenceHeight(float sel) const { return x120_turbulence.GetHeight(sel); }
    float GetOOTurbulenceDistance() const { return x120_turbulence.GetOODistance(); }
    float GetOOTurbulenceSpeed() const { return x120_turbulence.GetOOSpeed(); }
    bool HasTurbulence() const { return x120_turbulence.HasTurbulence(); }
};
}

#endif // __URDE_CFLUIDPLANECPU_HPP__
