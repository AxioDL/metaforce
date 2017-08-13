#ifndef __URDE_CSCRIPTWATER_HPP__
#define __URDE_CSCRIPTWATER_HPP__

#include "CScriptTrigger.hpp"
#include "CFluidPlaneCPU.hpp"
namespace urde
{

class CDamageInfo;
class CFluidUVMotion;

class CScriptWater : public CScriptTrigger
{
private:
    static const float kSplashScales[6];
    zeus::CFrustum x150_frustum;
    std::unique_ptr<CFluidPlaneCPU> x1b4_fluidPlane;
    zeus::CVector3f x1b8_positionMorphed;
    zeus::CVector3f x1c4_extentMorphed;
    float x1d0_morphInTime;
    zeus::CVector3f x1d4_positionOrig;
    zeus::CVector3f x1e0_extentOrig;
    float x1ec_damageOrig;
    float x1f0_damageMorphed;
    float x1f4_morphOutTime;
    float x1f8_morphFactor = 0.f;
    std::list<std::pair<TUniqueId, bool>> x1fc_waterInhabitants;
<<<<<<< HEAD
    float x214_fogBias;
    float x218_fogMagnitude;
    float x21c_origFogBias;
    float x220_origFogMagnitude;
    float x224_fogSpeed;
    zeus::CColor x228_fogColor;
    ResId x22c_splashParticle1Id;
    ResId x230_splashParticle2Id;
    ResId x234_splashParticle3Id;
    ResId x238_particle4Id;
=======
    u32 x210_;
    float x214_;
    float x218_;
    float x21c_;
    float x220_;
    float x224_;
    zeus::CColor x228_;
    CAssetId x22c_splashParticle1Id;
    CAssetId x230_splashParticle2Id;
    CAssetId x234_splashParticle3Id;
    CAssetId x238_particle4Id;
>>>>>>> 11d4aad746973443508adbff80b9da9eb0b4c60c
    std::experimental::optional<TLockedToken<CGenDescription>> x23c_;
    CAssetId x24c_particle5Id;
    std::experimental::optional<TLockedToken<CGenDescription>> x250_visorRunoffEffect;
    u16 x260_unkSfx;
    u16 x262_visorRunoffSfx;
    rstl::reserved_vector<std::experimental::optional<TLockedToken<CGenDescription>>, 3> x264_splashEffects;
    rstl::reserved_vector<u16, 3> x298_splashSounds;
    zeus::CColor x2a4_splashColor;
    zeus::CColor x2a8_unkColor;
    float x2ac_alphaInTime;
    float x2b0_alphaOutTime;
    float x2b4_alphaInRecip;
    float x2b8_alphaOutRecip;
    float x2bc_alpha;
    float x2c0_tileSize;
    int x2c4_gridDimX = 0;
    int x2c8_gridDimY = 0;
    int x2cc_gridCellCount = 0;
    int x2d0_patchDimX = 0;
    int x2d4_patchDimY = 0;
    std::unique_ptr<bool[]> x2d8_tileIntersects;
    std::unique_ptr<bool[]> x2dc_vertIntersects;
    std::unique_ptr<u8[]> x2e0_patchIntersects; // 0: all clear, 1: all intersect, 2: partial intersect
    int x2e4_gridCellCount2 = 0;
    union
    {
        struct
        {
            bool x2e8_24_b4 : 1;
            bool x2e8_25_morphIn : 1;
            bool x2e8_26_morphing : 1;
            bool x2e8_27_allowRender : 1;
            bool x2e8_28_recomputeClipping : 1;
            bool x2e8_29_alphaIn : 1;
            bool x2e8_30_alphaOut : 1;
        };
        u32 _dummy = 0;
    };
    void SetupGrid(bool recomputeClipping);
    void SetupGridClipping(CStateManager& mgr, int computeVerts);
    void UpdateSplashInhabitants(CStateManager& mgr);
public:
    CScriptWater(CStateManager& mgr, TUniqueId uid, const std::string& name, const CEntityInfo& info,
                 const zeus::CVector3f& pos, const zeus::CAABox& box, const urde::CDamageInfo& dInfo,
                 zeus::CVector3f& orientedForce, ETriggerFlags triggerFlags, bool thermalCold,
                 bool allowRender, CAssetId patternMap1, CAssetId patternMap2, CAssetId colorMap,
                 CAssetId bumpMap, CAssetId envMap, CAssetId envBumpMap, CAssetId unusedMap,
                 const zeus::CVector3f& bumpLightDir, float bumpScale, float morphInTime, float morphOutTime,
                 bool active, CFluidPlane::EFluidType fluidType, bool b4, float alpha,
                 const CFluidUVMotion& uvMot, float turbSpeed, float turbDistance, float turbFreqMax,
                 float turbFreqMin, float turbPhaseMax, float turbPhaseMin, float turbAmplitudeMax,
                 float turbAmplitudeMin, const zeus::CColor& splashColor, const zeus::CColor& unkColor,
                 CAssetId splashParticle1, CAssetId splashParticle2, CAssetId splashParticle3,
                 CAssetId particle4, CAssetId particle5, s32 unkSfx, s32 visorRunoffSfx, s32 splashSfx1,
                 s32 splashSfx2, s32 splashSfx3, float tileSize, u32 tileSubdivisions, float specularMin,
                 float specularMax, float reflectionSize, float rippleIntensity, float reflectionBlend,
                 float fogBias, float fogMagnitude, float fogSpeed, const zeus::CColor& fogColor,
                 CAssetId lightmapId, float unitsPerLightmapTexel, float alphaInTime, float alphaOutTime,
                 u32, u32, bool, s32, s32, std::unique_ptr<u32[]>&& u32Arr);

    void Think(float, CStateManager&);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void PreRender(CStateManager &, const zeus::CFrustum &);
    void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const;
    void Render(const CStateManager&) const;
    void Touch(CActor &, CStateManager &);
    void CalculateRenderBounds();
    zeus::CAABox GetSortingBounds(const CStateManager&) const;
    EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&, CWeaponMode&,
                                                           int);

    s16 GetSplashSound(float) const;
    const std::experimental::optional<TLockedToken<CGenDescription>>& GetSplashEffect(float) const;
    float GetSplashEffectScale(float) const;
    u32 GetSplashIndex(float) const;
    CFluidPlaneCPU& FluidPlane() { return *x1b4_fluidPlane; }
    zeus::CPlane GetWRSurfacePlane() const;
    float GetSurfaceZ() const;
    bool IsMorphing() const { return x2e8_26_morphing; }
    void SetMorphing(bool);
    float GetMorphFactor() const { return x1f8_morphFactor; }
    zeus::CColor GetSplashColor() const { return x2a4_splashColor; }
    void SetFrustumPlanes(const zeus::CFrustum& frustum) { x150_frustum = frustum; }
    const zeus::CFrustum& GetFrustumPlanes() const { return x150_frustum; }
    CFluidPlaneCPU& GetFluidPlane() const { return *x1b4_fluidPlane; }
    const std::experimental::optional<TLockedToken<CGenDescription>>& GetVisorRunoffEffect() const
    { return x250_visorRunoffEffect; }
    u16 GetVisorRunoffSfx() const { return x262_visorRunoffSfx; }
    const CScriptWater* GetNextConnectedWater(const CStateManager& mgr) const;
    u8 GetPatchRenderFlags(int x, int y) const { return x2e0_patchIntersects[y * x2d0_patchDimX + x]; }
    int GetPatchDimensionX() const { return x2d0_patchDimX; }
    int GetPatchDimensionY() const { return x2d4_patchDimY; }
};
}

#endif // __URDE_CSCRIPTWATER_HPP__
